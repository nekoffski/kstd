#include "kstd/containers/SlotBuffer.hh"

#include "Core.hh"

using namespace kstd;

constexpr u32 defaultCapacity = 16;

TEST(SlotBufferTests, empty) {
    SlotBuffer<int, defaultCapacity> sb;
    ASSERT_TRUE(sb.empty());
    ASSERT_FALSE(sb.full());
    ASSERT_EQ(sb.capacity(), defaultCapacity);
    ASSERT_EQ(sb.freeSlots(), defaultCapacity);
    ASSERT_EQ(sb.size(), 0u);
}

TEST(SlotBufferTests, insertEraseSimple) {
    SlotBuffer<int, defaultCapacity> sb;
    auto it = sb.insert(15);
    ASSERT_EQ(*it, 15);
    ASSERT_EQ(sb.freeSlots(), defaultCapacity - 1);
    ASSERT_EQ(sb.size(), 1);
    sb.erase(*it);
    ASSERT_EQ(sb.freeSlots(), defaultCapacity);
    ASSERT_EQ(sb.size(), 0);
}

TEST(SlotBufferTests, find) {
    SlotBuffer<Foo, defaultCapacity> sb;
    ASSERT_EQ(sb.findIf([](auto& f) { return f.x == 0; }), nullptr);
    sb.emplace(1, 2, 3);
    auto record = sb.findIf([](auto& f) { return f.x == 1; });
    ASSERT_NE(record, nullptr);
    ASSERT_EQ(record->x, 1);
    ASSERT_EQ(record->y, 2);
    ASSERT_EQ(record->z, 3);
    sb.eraseIf([](auto& f) { return f.x == 1; });
    ASSERT_EQ(sb.findIf([](auto& f) { return f.x == 0; }), nullptr);
}

TEST(SlotBufferTests, forEach) {
    SlotBuffer<int, defaultCapacity> sb;
    sb.insert(100);
    sb.insert(101);
    sb.insert(102);

    int sum = 0;
    sb.forEach([&](int x) { sum += x; });
    ASSERT_EQ(sum, 100 + 101 + 102);
}