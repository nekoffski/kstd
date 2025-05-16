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

TEST(SlotBufferTests, insertEmplace) {
    SlotBuffer<Foo, defaultCapacity> sb;

    sb.emplace(1, 2, 3);

    Foo foo{ 1, 2, 3 };
    sb.insert(foo);
    sb.insert(std::move(foo));

    const Foo foo2{ 1, 2, 3 };
    sb.insert(foo);
    sb.insert(Foo{ 1, 2, 3 });

    ASSERT_EQ(sb.size(), 5);
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

TEST(SlotBufferTests, clear) {
    SlotBuffer<int, defaultCapacity> sb;

    sb.insert(100);
    sb.insert(101);
    sb.insert(102);
    ASSERT_EQ(sb.size(), 3u);

    sb.clear();
    ASSERT_EQ(sb.size(), 0u);
    ASSERT_TRUE(sb.empty());
}

TEST(SlotBufferTests, iteratorsForLoop) {
    SlotBuffer<int, defaultCapacity> sb;
    sb.insert(100);
    auto handle = sb.insert(101);
    sb.insert(102);

    int sum = 0;
    for (auto& v : sb) sum += v;
    ASSERT_EQ(sum, 100 + 101 + 102);

    sum = 0;
    sb.erase(*handle);
    for (const auto& v : sb) sum += v;
    ASSERT_EQ(sum, 100 + 102);
}

TEST(SlotBufferTests, iteratorsFind) {
    SlotBuffer<int, defaultCapacity> sb;

    auto it = std::find_if(sb.begin(), sb.end(), [](int x) { return x == 5; });
    ASSERT_EQ(it, sb.end());

    sb.insert(5);
    it = std::find_if(sb.begin(), sb.end(), [](const auto& x) { return x == 5; });
    ASSERT_NE(it, sb.end());
    ASSERT_EQ(*it, 5);
}
