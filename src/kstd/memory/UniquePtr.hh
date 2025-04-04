#pragma once

#include <utility>

#include "kstd/Concepts.hh"
#include "kstd/Scope.hh"
#include "Allocator.hh"
#include "Mallocator.hh"

namespace kstd {

template <typename T> class UniquePtr : public NonCopyable {
    template <typename U> friend class UniquePtr;

    struct Passkey {};

    template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
    friend UniquePtr<U> makeUniqueWithAllocator(
      Allocator* allocator, Args&&... args
    );

    template <typename U, typename... Args>
    requires std::constructible_from<U, Args...>
    friend UniquePtr<U> makeUnique(Args&&... args);

public:
    explicit UniquePtr() : m_allocator(nullptr), m_object(nullptr) {}
    UniquePtr(std::nullptr_t) : UniquePtr() {}

    ~UniquePtr() { clear(); }

    template <typename U>
    requires std::is_base_of_v<T, U>
    UniquePtr(UniquePtr<U>&& oth) :
        m_allocator(std::exchange(oth.m_allocator, nullptr)),
        m_object(std::exchange(oth.m_object, nullptr)) {}

    UniquePtr& operator=(UniquePtr&& oth) {
        clear();

        m_allocator = std::exchange(oth.m_allocator, nullptr);
        m_object    = std::exchange(oth.m_object, nullptr);
        return *this;
    }

    void clear() noexcept {
        if (m_allocator && m_object) {
            m_object->~T();
            m_allocator->deallocate(m_object);
        }
        m_allocator = nullptr;
        m_object    = nullptr;
    }

    T& operator*() { return *m_object; }
    const T& operator*() const { return *m_object; }

    T* operator->() { return m_object; }
    const T* operator->() const { return m_object; }

    T* get() { return m_object; }
    const T* get() const { return m_object; }

    Allocator* getAllocator() { return m_allocator; }

    bool empty() const { return m_object == nullptr; }
    operator bool() const { return not empty(); }

private:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit UniquePtr(
      [[maybe_unused]] Passkey, Allocator* allocator, Args&&... args
    ) : m_allocator(allocator), m_object(m_allocator->allocate<T>()) {
        ON_SCOPE_FAIL { clear(); };
        new (m_object) T(std::forward<Args>(args)...);
    }

    Allocator* m_allocator;
    T* m_object;

    inline static Mallocator<> s_defaultAllocator;
};

template <typename U, typename... Args>
requires std::constructible_from<U, Args...>
UniquePtr<U> makeUniqueWithAllocator(Allocator* allocator, Args&&... args) {
    return UniquePtr<U>{
        typename UniquePtr<U>::Passkey{}, allocator, std::forward<Args>(args)...
    };
}

template <typename U, typename... Args>
requires std::constructible_from<U, Args...>
UniquePtr<U> makeUnique(Args&&... args) {
    return UniquePtr<U>{
        typename UniquePtr<U>::Passkey{}, &UniquePtr<U>::s_defaultAllocator,
        std::forward<Args>(args)...
    };
}

}  // namespace kstd
