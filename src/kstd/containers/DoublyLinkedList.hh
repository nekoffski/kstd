#pragma once

#include "kstd/memory/Allocator.hh"
#include "kstd/memory/Mallocator.hh"
#include "kstd/Concepts.hh"
#include "kstd/Log.hh"
#include "kstd/Scope.hh"

namespace kstd {

template <typename T> class DoublyLinkedList : public NonCopyable {
    inline static Mallocator s_mallocator;

public:
    struct Node {
        T value;
        Node* prev;
        Node* next;
    };

    explicit DoublyLinkedList(Allocator* allocator = &s_mallocator) :
        m_allocator(allocator), m_size(0u), m_head(nullptr) {}

    ~DoublyLinkedList() { clear(); }

    void clear() {
        for (auto n = m_head; n != nullptr; n = n->next) destroyNode(n);
        m_head = nullptr;
    }

    Node* erase(Node* node) {
        ON_SCOPE_EXIT { destroyNode(node); };

        if (m_head == node) {
            m_head = node->next;
            return m_head;
        } else {
            auto prev  = node->prev;
            auto next  = node->next;
            prev->next = next;
            if (next != nullptr) next->prev = prev;
            return next;
        }
    }

    template <typename Callback>
    requires Callable<Callback, bool, const T&>
    void eraseIf(Callback&& c) {
        for (auto it = m_head; it != nullptr; it = it->next) {
            if (c(it->value)) {
                erase(it);
                return;
            }
        }
    }

    template <typename Callback>
    requires Callable<Callback, void, T&>
    void forEach(Callback&& c) {
        for (auto it = m_head; it != nullptr; it = it->next) c(it->value);
    }

    template <typename Callback>
    requires Callable<Callback, void, const T&>
    void forEach(Callback&& c) const {
        for (auto it = m_head; it != nullptr; it = it->next) c(it->value);
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    Node* emplaceBefore(Node* place, Args&&... args) {
        return pushBefore(place, T{ std::forward<Args>(args)... });
    }

    Node* pushBefore(Node* place, const T& v) {
        auto copy = v;
        return pushBefore(place, std::move(copy));
    }

    Node* pushBefore(Node* place, T&& v) {
        log::expect(place != nullptr, "nullptr passed as place for pushBefore");
        if (place->prev == nullptr) return pushFront(std::move(v));
        return pushAfter(place->prev, std::move(v));
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    Node* emplaceAfter(Node* place, Args&&... args) {
        return pushAfter(place, T{ std::forward<Args>(args)... });
    }

    Node* pushAfter(Node* place, const T& v) {
        auto copy = v;
        return pushAfter(place, std::move(copy));
    }

    Node* pushAfter(Node* place, T&& v) {
        log::expect(place != nullptr, "nullptr passed as place for pushAfter");

        auto next         = place->next;
        place->next       = createNode(std::move(v));
        place->next->prev = place;
        place->next->next = next;

        if (next != nullptr) next->prev = place->next;

        return place->next;
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    Node* emplaceFront(Args&&... args) {
        return pushFront(T{ std::forward<Args>(args)... });
    }

    Node* pushFront(const T& v) {
        auto copy = v;
        return pushFront(std::move(copy));
    }

    Node* pushFront(T&& v) {
        auto rest = m_head;
        m_head    = createNode(std::move(v));

        if (rest != nullptr) {
            m_head->next = rest;
            rest->prev   = m_head;
        }

        return m_head;
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    Node* emplaceBack(Args&&... args) {
        return pushBack(T{ std::forward<Args>(args)... });
    }

    Node* pushBack(const T& v) {
        auto copy = v;
        return pushBack(std::move(copy));
    }

    Node* pushBack(T&& v) {
        if (m_head == nullptr) return pushFront(std::move(v));

        auto it = m_head;
        while (it->next != nullptr) it = it->next;

        it->next       = createNode(std::move(v));
        it->next->prev = it;

        return it->next;
    }

    Node* head() const { return m_head; }

    u64 size() const { return m_size; }
    bool empty() const { return m_size == 0; }

private:
    Node* createNode(T&& v) {
        ++m_size;
        auto node = m_allocator->allocate<Node>();
        new (node) Node{ std::move(v), nullptr, nullptr };
        return node;
    }

    void destroyNode(Node* n) {
        --m_size;
        n->~Node();
        m_allocator->deallocate(n);
    }

    Allocator* m_allocator;
    u64 m_size;
    Node* m_head;
};

}  // namespace kstd
