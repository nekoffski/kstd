#pragma once

#include <atomic>
#include <concepts>
#include <utility>
#include <cstddef>

#include "kstd/Core.hh"
#include "kstd/Scope.hh"

#include "Allocator.hh"
#include "Mallocator.hh"

namespace kstd {

template <typename T> class SharedPtr {
    struct Passkey {};

    template <typename U> friend class SharedPtr;

    template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
    friend SharedPtr<U> makeSharedWithAllocator(
      Allocator* allocator, Args&&... args
    );

    template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
    friend SharedPtr<U> makeShared(Args&&... args);

    template <typename Destination, typename Source>
    requires std::is_base_of_v<Source, Destination>
    friend SharedPtr<Destination> sharedPtrCast(SharedPtr<Source>& ptr);

public:
    struct ControlBlock {
        template <typename... Args>
        requires std::constructible_from<T, Args...>
        explicit ControlBlock(Args&&... args) : referenceCounter(1) {
            std::memset(&buffer, 0, sizeof(T));
            new (&buffer) T(std::forward<Args>(args)...);
        }

        ~ControlBlock() { convert()->~T(); }

        std::atomic<i64> referenceCounter;
        alignas(T) std::byte buffer[sizeof(T)];

        T* convert() { return reinterpret_cast<T*>(&buffer[0]); }
    };

    SharedPtr() : m_controlBlock(nullptr) {}
    SharedPtr(std::nullptr_t) : SharedPtr() {}

    ~SharedPtr() { clear(); }

    template <typename U>
    requires(std::is_base_of_v<T, U> || std::is_same_v<T, U>)
    SharedPtr(SharedPtr<U>&& oth) :
        m_allocator(std::exchange(oth.m_allocator, nullptr)),
        m_controlBlock(
          reinterpret_cast<ControlBlock*>(std::exchange(oth.m_controlBlock, nullptr))
        ) {}

    SharedPtr& operator=(SharedPtr&& oth) {
        clear();

        m_allocator    = std::exchange(oth.m_allocator, nullptr);
        m_controlBlock = std::exchange(oth.m_controlBlock, nullptr);
        return *this;
    }

    SharedPtr(const SharedPtr& oth
    ) : m_allocator(oth.m_allocator), m_controlBlock(oth.m_controlBlock) {
        if (m_controlBlock) m_controlBlock->referenceCounter++;
    }

    template <typename U>
    requires std::is_base_of_v<T, U>
    SharedPtr(const SharedPtr<U>& oth) :
        m_allocator(oth.m_allocator),
        m_controlBlock(reinterpret_cast<ControlBlock*>(oth.m_controlBlock)) {
        if (m_controlBlock) m_controlBlock->referenceCounter++;
    }

    template <typename U>
    requires std::is_base_of_v<T, U>
    SharedPtr<T>& operator=(const SharedPtr<U>& oth) {
        clear();

        m_allocator    = oth.m_allocator;
        m_controlBlock = reinterpret_cast<ControlBlock*>(oth.m_controlBlock);

        if (m_controlBlock) m_controlBlock->referenceCounter++;
        return *this;
    }

    SharedPtr& operator=(const SharedPtr& oth) {
        clear();

        m_allocator    = oth.m_allocator;
        m_controlBlock = oth.m_controlBlock;

        if (m_controlBlock) m_controlBlock->referenceCounter++;
        return *this;
    }

    void clear() noexcept {
        if (m_controlBlock && m_controlBlock->referenceCounter.fetch_sub(1) == 1) {
            log::expect(
              m_allocator != nullptr, "No allocator in SharedPtr that owns memory"
            );
            m_controlBlock->~ControlBlock();
            m_allocator->deallocate(m_controlBlock);
        }
        m_controlBlock = nullptr;
        m_allocator    = nullptr;
    }

    Allocator* getAllocator() { return m_allocator; }

    bool empty() const { return m_controlBlock == nullptr; }
    operator bool() const { return not empty(); }

    T& operator*() { return *m_controlBlock->convert(); }
    const T& operator*() const { return *m_controlBlock->convert(); }

    T* operator->() { return m_controlBlock->convert(); }
    const T* operator->() const { return m_controlBlock->convert(); }

    T* get() { return empty() ? nullptr : m_controlBlock->convert(); }
    const T* get() const { return empty() ? nullptr : m_controlBlock->convert(); }

private:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit SharedPtr(
      [[maybe_unused]] Passkey, Allocator* allocator, Args&&... args
    ) :
        m_allocator(allocator),
        m_controlBlock(m_allocator->allocate<ControlBlock>()) {
        ON_SCOPE_FAIL { m_allocator->deallocate(m_controlBlock); };
        new (m_controlBlock) ControlBlock(std::forward<Args>(args)...);
    }

    explicit SharedPtr(Allocator* allocator, ControlBlock* controlBlock) :
        m_allocator(allocator), m_controlBlock(controlBlock) {
        if (m_controlBlock) m_controlBlock->referenceCounter++;
    }

    Allocator* m_allocator;
    ControlBlock* m_controlBlock;

    inline static Mallocator<> s_defaultAllocator;
};

template <typename U, typename... Args>
requires std::constructible_from<U, Args...>
SharedPtr<U> makeSharedWithAllocator(Allocator* allocator, Args&&... args) {
    return SharedPtr<U>{
        typename SharedPtr<U>::Passkey{}, allocator, std::forward<Args>(args)...
    };
}

template <typename U, typename... Args>
requires std::constructible_from<U, Args...>
SharedPtr<U> makeShared(Args&&... args) {
    return SharedPtr<U>{
        typename SharedPtr<U>::Passkey{}, &SharedPtr<U>::s_defaultAllocator,
        std::forward<Args>(args)...
    };
}

template <typename Destination, typename Source>
requires std::is_base_of_v<Source, Destination>
SharedPtr<Destination> sharedPtrCast(SharedPtr<Source>& ptr) {
    if (ptr.empty()) return nullptr;

    auto& controlBlock = ptr.m_controlBlock;
    if (dynamic_cast<Destination*>(controlBlock->convert()) != nullptr) {
        return SharedPtr<Destination>{
            ptr.m_allocator,
            reinterpret_cast<SharedPtr<Destination>::ControlBlock*>(controlBlock)
        };
    }
    return nullptr;
}

}  // namespace kstd
