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
    using value_type = T;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        Iterator(
          typename std::array<Slot, Capacity>::iterator current,
          typename std::array<Slot, Capacity>::iterator end
        ) : m_current(current), m_end(end) {
            advanceToValid();
        }

        T& operator*() const { return *m_current->get(); }
        T* operator->() const { return m_current->get(); }

        Iterator& operator++() {
            ++m_current;
            advanceToValid();
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.m_current == b.m_current;
        }

        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return !(a == b);
        }

    private:
        void advanceToValid() {
            while (m_current != m_end && !*m_current) ++m_current;
        }

        typename std::array<Slot, Capacity>::iterator m_current;
        typename std::array<Slot, Capacity>::iterator m_end;
    };

    class ConstIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        ConstIterator(
          typename std::array<Slot, Capacity>::const_iterator current,
          typename std::array<Slot, Capacity>::const_iterator end
        ) : m_current(current), m_end(end) {
            advanceToValid();
        }

        const T& operator*() const { return *m_current->get(); }
        const T* operator->() const { return m_current->get(); }

        ConstIterator& operator++() {
            ++m_current;
            advanceToValid();
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const ConstIterator& a, const ConstIterator& b) {
            return a.m_current == b.m_current;
        }

        friend bool operator!=(const ConstIterator& a, const ConstIterator& b) {
            return !(a == b);
        }

    private:
        void advanceToValid() {
            while (m_current != m_end && !*m_current) ++m_current;
        }

        typename std::array<Slot, Capacity>::const_iterator m_current;
        typename std::array<Slot, Capacity>::const_iterator m_end;
    };

    explicit SlotBuffer() { clear(); }

    SlotBuffer(SlotBuffer&& oth)            = default;
    SlotBuffer& operator=(SlotBuffer&& oth) = default;

    Iterator begin() { return Iterator{ m_slots.begin(), m_slots.end() }; }
    Iterator end() { return Iterator{ m_slots.end(), m_slots.end() }; }

    ConstIterator begin() const {
        return ConstIterator{ m_slots.begin(), m_slots.end() };
    }
    ConstIterator end() const {
        return ConstIterator{ m_slots.end(), m_slots.end() };
    }

    ConstIterator cbegin() const {
        return ConstIterator{ m_slots.begin(), m_slots.end() };
    }
    ConstIterator cend() const {
        return ConstIterator{ m_slots.end(), m_slots.end() };
    }

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
        std::queue<u64> empty;
        std::swap(m_freeSlots, empty);

        for (auto& slot : m_slots) slot.clear();
        for (u64 i = 0; i < Capacity; ++i) m_freeSlots.push(i);
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
