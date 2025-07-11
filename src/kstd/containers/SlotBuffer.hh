#pragma once

#include <queue>

#include "kstd/Core.hh"
#include "kstd/Concepts.hh"
#include "kstd/Error.hh"

#include "Storage.hh"

namespace kstd {

namespace details {

template <typename T, typename Storage> class SlotBuffer {
public:
    using value_type = T;

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        Iterator(LocalPtr<T>* current, LocalPtr<T>* end) :
            m_current(current), m_end(end) {
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

        LocalPtr<T>* m_current;
        LocalPtr<T>* m_end;
    };

    class ConstIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;

        ConstIterator(LocalPtr<T>* current, LocalPtr<T>* end) :
            m_current(current), m_end(end) {
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

        LocalPtr<T>* m_current;
        LocalPtr<T>* m_end;
    };

    explicit SlotBuffer(u64 capacity) :
        m_storage(capacity), m_begin(m_storage.begin()), m_end(m_storage.end()),
        m_capacity(capacity) {
        clear();
    }

    SlotBuffer(SlotBuffer&& oth) :
        m_storage(std::move(oth.m_storage)), m_freeSlots(std::move(oth.m_freeSlots)),
        m_begin(m_storage.begin()), m_end(m_storage.end()),
        m_capacity(oth.m_capacity) {}

    SlotBuffer& operator=(SlotBuffer&& oth) {
        m_storage   = std::move(oth.m_storage);
        m_freeSlots = std::move(oth.m_freeSlots);
        m_begin     = m_storage.begin();
        m_end       = m_storage.end();
        m_capacity  = oth.m_capacity;
        return *this;
    }

    SlotBuffer(const SlotBuffer& oth) :
        m_storage(oth.m_storage), m_freeSlots(oth.m_freeSlots),
        m_begin(m_storage.begin()), m_end(m_storage.end()),
        m_capacity(oth.m_capacity) {}

    SlotBuffer& operator=(const SlotBuffer& oth) {
        m_storage   = oth.m_storage;
        m_freeSlots = oth.m_freeSlots;
        m_begin     = m_storage.begin();
        m_end       = m_storage.end();
        m_capacity  = oth.m_capacity;
        return *this;
    }

    Iterator begin() { return Iterator{ m_begin, m_end }; }
    Iterator end() { return Iterator{ m_end, m_end }; }

    ConstIterator begin() const { return ConstIterator{ m_begin, m_end }; }
    ConstIterator end() const { return ConstIterator{ m_end, m_end }; }

    ConstIterator cbegin() const { return ConstIterator{ m_begin, m_end }; }
    ConstIterator cend() const { return ConstIterator{ m_end, m_end }; }

    T& insert(T&& v) {
        if (full()) [[unlikely]]
            throw CollectionFullError{};
        return *getSlot().emplace(std::move(v));
    }

    T& insert(const T& v) {
        if (full()) [[unlikely]]
            throw CollectionFullError{};
        return *getSlot().emplace(v);
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    T& emplace(Args&&... args) {
        if (full()) [[unlikely]]
            throw CollectionFullError{};
        return *getSlot().emplace(std::forward<Args>(args)...);
    }

    void clear() {
        std::queue<u64> empty;
        std::swap(m_freeSlots, empty);

        for (auto it = m_begin; it != m_end; it++) it->clear();
        for (u64 i = 0; i < m_capacity; ++i) m_freeSlots.push(i);
    }

    u64 capacity() const { return m_capacity; }
    u64 size() const { return m_capacity - freeSlots(); }
    u64 freeSlots() const { return m_freeSlots.size(); }

    bool full() const { return freeSlots() == 0; }
    bool empty() const { return freeSlots() == m_capacity; }

    template <typename Callback>
    requires Callable<Callback, bool, const T&>
    const T* findIf(Callback&& callback) const {
        for (auto it = m_begin; it != m_end; it++)
            if (not it->empty() && callback(it->value())) return it->get();
        return nullptr;
    }

    template <typename Callback>
    requires Callable<Callback, bool, T&>
    T* findIf(Callback&& callback) {
        for (auto it = m_begin; it != m_end; it++)
            if (not it->empty() && callback(it->value())) return it->get();
        return nullptr;
    }

    template <typename Callback>
    requires Callable<Callback, void, const T&>
    void forEach(Callback&& callback) const {
        for (auto it = m_begin; it != m_end; it++)
            if (not it->empty()) callback(it->value());
    }

    template <typename Callback>
    requires Callable<Callback, void, T&>
    void forEach(Callback&& callback) {
        for (auto it = m_begin; it != m_end; it++)
            if (not it->empty()) callback(it->value());
    }

    template <typename Callback>
    requires Callable<Callback, bool, const T&>
    u64 eraseIf(Callback&& callback) {
        u64 removed = 0u;
        for (u64 i = 0; i < m_capacity; ++i) {
            if (auto& slot = *(m_begin + i); slot && callback(slot.value())) {
                slot.clear();
                m_freeSlots.push(i);
                ++removed;
            }
        }
        return removed;
    }

    bool erase(const T& v) {
        bool found = false;

        for (u64 i = 0; i < m_capacity; ++i) {
            if (auto& slot = *(m_begin + i); slot && slot.value() == v) {
                slot.clear();
                m_freeSlots.push(i);
                found = true;
            }
        }
        return found;
    }

protected:
    LocalPtr<T>& getSlot() {
        auto id = m_freeSlots.front();
        m_freeSlots.pop();
        return *(m_begin + id);
    }

    Storage m_storage;
    std::queue<u64> m_freeSlots;
    LocalPtr<T>* m_begin;
    LocalPtr<T>* m_end;
    u64 m_capacity;
};

}  // namespace details

template <typename T, u64 Capacity>
struct StackSlotBuffer
    : public details::SlotBuffer<T, SlotStackStorage<T, Capacity>> {
    StackSlotBuffer() :
        details::SlotBuffer<T, SlotStackStorage<T, Capacity>>(Capacity) {}
};

template <typename T>
struct HeapSlotBuffer : public details::SlotBuffer<T, SlotHeapStorage<T>> {
    explicit HeapSlotBuffer(u64 capacity
    ) : details::SlotBuffer<T, SlotHeapStorage<T>>(capacity) {}
};

}  // namespace kstd
