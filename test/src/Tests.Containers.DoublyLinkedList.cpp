#include "kstd/containers/DoublyLinkedList.hh"

#include "Core.hh"

using namespace kstd;

TEST(DoubleLinkedList, empty) {
    DoublyLinkedList<int> dll;
    ASSERT_NULLPTR(dll.head());
    ASSERT_EQ(dll.size(), 0u);
    ASSERT_TRUE(dll.empty());
}

TEST(DoublyLinkedList, pushFrontBack) {
    DoublyLinkedList<int> dll;

    auto n = dll.pushBack(1);
    ASSERT_EQ(n->value, 1);
    ASSERT_NULLPTR(n->prev);
    ASSERT_NULLPTR(n->next);

    auto n2 = dll.pushFront(2);
    ASSERT_EQ(n2->next, n);
    ASSERT_EQ(n->prev, n2);
}

TEST(DoublyLinkedList, pushAfterBefore) {
    DoublyLinkedList<int> dll;

    auto n  = dll.pushBack(1);
    auto n2 = dll.pushBack(2);

    ASSERT_EQ(n->next, n2);
    ASSERT_EQ(n, n2->prev);

    auto n3 = dll.pushAfter(n2, 3);
    auto n4 = dll.pushBefore(n2, 4);

    ASSERT_EQ(n2->next, n3);
    ASSERT_EQ(n3->prev, n2);

    ASSERT_NULLPTR(n3->next);
    ASSERT_EQ(n4, n2->prev);
    ASSERT_EQ(n2, n4->next);
}

TEST(DoublyLinkedList, erase) {
    DoublyLinkedList<int> dll;

    auto n  = dll.pushBack(1);
    auto n2 = dll.pushBack(2);
    auto n3 = dll.pushBack(3);

    ASSERT_EQ(dll.size(), 3);

    dll.erase(n2);

    ASSERT_EQ(dll.size(), 2);
    ASSERT_EQ(n->next, n3);
    ASSERT_EQ(n, n3->prev);
}

TEST(DoublyLinkedList, forEach) {
    DoublyLinkedList<int> dll;

    dll.pushBack(1);
    dll.pushBack(2);
    dll.pushBack(3);

    int sum = 0;
    dll.forEach([&](int v) { sum += v; });
    ASSERT_EQ(sum, 6);
    ASSERT_EQ(dll.size(), 3);
}

TEST(DoublyLinkedList, clear) {
    DoublyLinkedList<int> dll;

    dll.pushBack(1);
    dll.pushBack(2);
    dll.pushBack(3);
    ASSERT_EQ(dll.size(), 3);

    dll.clear();
    ASSERT_EQ(dll.size(), 0);
}
