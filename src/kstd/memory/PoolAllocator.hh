#pragma once

#include <array>
#include <vector>

#include "Allocator.hh"

namespace kstd {

namespace details {

template <
  typename T, AllocatorReportStrategy ReportStrategy,
  AllocatorFailureStrategy FailureStrategy>
class PoolAllocator : public AllocatorBase<ReportStrategy> {
    struct Slot {
        u64 index;
        u64 elements;
        bool free;
    };

public:
    explicit PoolAllocator(u64 capacity, std::byte* buffer) noexcept :
        m_capacity(capacity), m_slotsLeft(capacity),
        m_buffer(reinterpret_cast<T*>(buffer)) {
        m_slots.reserve(capacity);
        for (u64 i = 0; i < capacity; ++i) m_slots.emplace_back(i, 0u, true);
    }

    u64 capacity() const { return m_capacity; }
    u64 slotsLeft() const { return m_slotsLeft; }

protected:
    std::byte* getBuffer() { return m_buffer; }

private:
    [[nodiscard]] void* allocateImpl(u64 n, [[maybe_unused]] u64) override {
        const auto elements = n / sizeof(T);

        for (u64 i = 0u; i < m_capacity; ++i) {
            if (auto& slot = m_slots[i]; slot.free) {
                u64 required = elements - 1;

                ++i;

                for (; required != 0u && i < m_capacity; ++i) {
                    if (not m_slots[i].free) break;
                    --required;
                }

                if (required == 0u) {
                    m_slotsLeft -= elements;
                    slot.elements = elements;
                    for (u64 j = 0u; j < elements; ++j)
                        m_slots[slot.index + j].free = false;
                    return static_cast<void*>(m_buffer + slot.index);
                }
            }
        }
        handleNull();
        return nullptr;
    }

    constexpr void handleNull() {
        if constexpr (FailureStrategy == AllocatorFailureStrategy::throwException)
            throw std::bad_alloc{};
        else if constexpr (FailureStrategy == AllocatorFailureStrategy::panic)
            log::panic("Could not allocate memory");
    }

    void deallocateImpl(void* p) noexcept override {
        i64 index = static_cast<i64>(static_cast<T*>(p) - m_buffer);
        log::expect(
          index >= 0u && index < m_capacity,
          "PoolAllocator tried to deallocate address out of range"
        );

        const auto elements = m_slots[index].elements;
        std::memset(p, 0, elements * sizeof(T));

        m_slotsLeft += elements;

        for (int i = 0; i < elements; ++i) {
            m_slots[index + i].elements = 0u;
            m_slots[index + i].free     = true;
        }
    }

    const u64 m_capacity;
    u64 m_slotsLeft;
    std::vector<Slot> m_slots;
    T* m_buffer;
};

class HeapStorage {
public:
    explicit HeapStorage(u64 size, u64 alignment) noexcept :
        m_buffer(static_cast<std::byte*>(std::aligned_alloc(alignment, size))) {}

    ~HeapStorage() { delete m_buffer; }

    std::byte* getBuffer() { return m_buffer; }

private:
    std::byte* m_buffer;
};

template <typename T, u64 Capacity> class StackStorage {
public:
    std::byte* getBuffer() { return &m_buffer[0]; }

private:
    alignas(T) std::byte m_buffer[Capacity * sizeof(T)];
};

}  // namespace details

template <
  typename T,
  AllocatorReportStrategy ReportStrategy   = AllocatorReportStrategy::enabled,
  AllocatorFailureStrategy FailureStrategy = AllocatorFailureStrategy::panic>
struct HeapPoolAllocator
    : public details::HeapStorage,
      public details::PoolAllocator<T, ReportStrategy, FailureStrategy> {
    explicit HeapPoolAllocator(u64 capacity) noexcept :
        details::HeapStorage(sizeof(T) * capacity, alignof(T)),
        details::PoolAllocator<T, ReportStrategy, FailureStrategy>(
          capacity, getBuffer()
        ) {}
};

template <
  typename T, u64 Capacity,
  AllocatorReportStrategy ReportStrategy   = AllocatorReportStrategy::enabled,
  AllocatorFailureStrategy FailureStrategy = AllocatorFailureStrategy::panic>
struct StackPoolAllocator
    : public details::StackStorage<T, Capacity>,
      public details::PoolAllocator<T, ReportStrategy, FailureStrategy> {
    explicit StackPoolAllocator() noexcept :
        details::PoolAllocator<T, ReportStrategy, FailureStrategy>(
          Capacity, details::StackStorage<T, Capacity>::getBuffer()
        ) {}
};

}  // namespace kstd
