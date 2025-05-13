#pragma once

#include <array>
#include <queue>

#include "kstd/Core.hh"
#include "kstd/Concepts.hh"
#include "kstd/memory/LocalPtr.hh"

namespace kstd {

template <typename T, u64 Capacity> class SlotBuffer : public NonCopyable {
    static_assert(Capacity > 0);

    using Slot = LocalPtr<T>;

public:
    explicit SlotBuffer() {
        for (u64 i = 0; i < Capacity; ++i) m_freeSlots.push(i);
    }

    SlotBuffer(SlotBuffer&& oth)            = default;
    SlotBuffer& operator=(SlotBuffer&& oth) = default;

    T* insert(T&& v) {
        if (full()) return nullptr;
        return getSlot().emplace(std::move(v));
    }

    T* insert(const T& v) {
        if (full()) return nullptr;
        return getSlot().emplace(v);
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    T* emplace(Args&&... args) {
        if (full()) return nullptr;
        return getSlot().emplace(std::forward<Args>(args)...);
    }

    void clear() {
        for (auto& slot : m_slots) slot.clear();
    }

    constexpr u64 capacity() const { return Capacity; }
    u64 size() const { return Capacity - freeSlots(); }
    u64 freeSlots() const { return m_freeSlots.size(); }

    bool full() const { return freeSlots() == 0; }
    bool empty() const { return freeSlots() == Capacity; }

    template <typename Callback>
    requires Callable<Callback, bool, const T&>
    const T* findIf(Callback&& callback) const {
        for (const auto& slot : m_slots)
            if (slot && callback(*slot)) return slot.get();
        return nullptr;
    }

    template <typename Callback>
    requires Callable<Callback, bool, T&>
    T* findIf(Callback&& callback) {
        for (auto& slot : m_slots)
            if (slot && callback(*slot)) return slot.get();
        return nullptr;
    }

    template <typename Callback>
    requires Callable<Callback, void, const T&>
    void forEach(Callback&& callback) const {
        for (auto& slot : m_slots)
            if (slot) callback(*slot);
    }

    template <typename Callback>
    requires Callable<Callback, void, T&>
    void forEach(Callback&& callback) {
        for (auto& slot : m_slots)
            if (slot) callback(*slot);
    }

    template <typename Callback>
    requires Callable<Callback, bool, const T&>
    void eraseIf(Callback&& callback) {
        for (auto& slot : m_slots)
            if (slot && callback(*slot)) slot.clear();
    }

    void erase(T& v) {
        for (u64 i = 0; i < Capacity; ++i) {
            if (auto& slot = m_slots[i]; slot.get() == &v) {
                slot.clear();
                m_freeSlots.push(i);
                return;
            }
        }
    }

private:
    Slot& getSlot() {
        auto id = m_freeSlots.front();
        m_freeSlots.pop();
        return m_slots[id];
    }

    std::array<Slot, Capacity> m_slots;
    std::queue<u64> m_freeSlots;
};

}  // namespace kstd
