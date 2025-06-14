#include "kstd/containers/SlotBuffer.hh"

#include "Core.hh"

#include "kstd/memory/UniquePtr.hh"

using namespace kstd;

constexpr u32 defaultCapacity = 16u;

template <typename SlotBuffer> struct SlotBufferTests : testing::Test {
    kstd::UniquePtr<SlotBuffer> sb;

    void SetUp() {
        if constexpr (std::is_same_v<
                        SlotBuffer, StackSlotBuffer<Foo, defaultCapacity>>) {
            sb = kstd::makeUnique<StackSlotBuffer<Foo, defaultCapacity>>();
        } else if constexpr (std::is_same_v<SlotBuffer, HeapSlotBuffer<Foo>>) {
            sb = kstd::makeUnique<HeapSlotBuffer<Foo>>(defaultCapacity);
        }
    }
};

class NameGenerator {
public:
    template <typename T> static std::string GetName(int) {
        if constexpr (std::is_same_v<T, StackSlotBuffer<Foo, defaultCapacity>>)
            return "StackSlotBuffer";
        else if constexpr (std::is_same_v<T, HeapSlotBuffer<Foo>>)
            return "HeapSlotBuffer";
    }
};

using TestTypes =
  ::testing::Types<StackSlotBuffer<Foo, defaultCapacity>, HeapSlotBuffer<Foo>>;

TYPED_TEST_SUITE(SlotBufferTests, TestTypes, NameGenerator);

TYPED_TEST(SlotBufferTests, empty) {
    auto& sb = *this->sb;

    ASSERT_TRUE(sb.empty());
    ASSERT_FALSE(sb.full());
    ASSERT_EQ(sb.capacity(), defaultCapacity);
    ASSERT_EQ(sb.freeSlots(), defaultCapacity);
    ASSERT_EQ(sb.size(), 0u);
}

TYPED_TEST(SlotBufferTests, insertEraseSimple) {
    auto& sb = *this->sb;

    auto it = sb.insert(Foo{ 15, 1, 1 });
    ASSERT_EQ(it->x, 15);
    ASSERT_EQ(sb.freeSlots(), defaultCapacity - 1);
    ASSERT_EQ(sb.size(), 1);

    ASSERT_TRUE(sb.erase(*it));
    ASSERT_EQ(sb.freeSlots(), defaultCapacity);
    ASSERT_EQ(sb.size(), 0);
}

TYPED_TEST(SlotBufferTests, insertEmplace) {
    auto& sb = *this->sb;

    sb.emplace(1, 2, 3);

    Foo foo{ 1, 2, 3 };
    sb.insert(foo);
    sb.insert(std::move(foo));

    const Foo foo2{ 1, 2, 3 };
    sb.insert(foo);
    sb.insert(Foo{ 1, 2, 3 });

    ASSERT_EQ(sb.size(), 5);
}

TYPED_TEST(SlotBufferTests, find) {
    auto& sb = *this->sb;

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

TYPED_TEST(SlotBufferTests, forEach) {
    auto& sb = *this->sb;

    sb.insert(Foo{ 100, 1, 1 });
    sb.insert(Foo{ 101, 1, 1 });
    sb.insert(Foo{ 102, 1, 1 });

    int sum = 0;
    sb.forEach([&](Foo& x) { sum += x.x; });
    ASSERT_EQ(sum, 100 + 101 + 102);
}

TYPED_TEST(SlotBufferTests, clear) {
    auto& sb = *this->sb;

    sb.insert(Foo{ 100, 1, 1 });
    sb.insert(Foo{ 101, 1, 1 });
    sb.insert(Foo{ 102, 1, 1 });
    ASSERT_EQ(sb.size(), 3u);

    sb.clear();
    ASSERT_EQ(sb.size(), 0u);
    ASSERT_TRUE(sb.empty());
}

TYPED_TEST(SlotBufferTests, iteratorsForLoop) {
    auto& sb = *this->sb;

    sb.insert(Foo{ 100, 1, 1 });
    auto handle = sb.insert(Foo{ 101, 1, 1 });
    sb.insert(Foo{ 102, 1, 1 });

    int sum = 0;
    for (auto& v : sb) sum += v.x;
    ASSERT_EQ(sum, 100 + 101 + 102);

    sum = 0;
    ASSERT_TRUE(sb.erase(*handle));
    for (const auto& v : sb) sum += v.x;
    ASSERT_EQ(sum, 100 + 102);
}

TYPED_TEST(SlotBufferTests, copy) {
    auto& sb = *this->sb;
    sb.insert(Foo{ 100, 1, 1 });

    auto sb2 = sb;
    ASSERT_EQ(sb.size(), sb2.size());

    auto it1 = sb.findIf([](auto& v) { return v.x == 100; });
    auto it2 = sb2.findIf([](auto& v) { return v.x == 100; });

    ASSERT_NO_NULLPTR(it1);
    ASSERT_NO_NULLPTR(it2);

    ASSERT_EQ(*it1, *it2);
    ASSERT_NE(it1, it2);
}

TYPED_TEST(SlotBufferTests, move) {
    auto& sb = *this->sb;
    sb.insert(Foo{ 101, 1, 1 });

    {
        auto sb2 = sb;
        sb2.insert(Foo{ 100, 1, 1 });
        sb = std::move(sb2);
    }

    auto it1 = sb.findIf([](auto& v) { return v.x == 100; });
    auto it2 = sb.findIf([](auto& v) { return v.x == 101; });
    ASSERT_NO_NULLPTR(it1);
    ASSERT_NO_NULLPTR(it2);
}

TYPED_TEST(SlotBufferTests, iteratorsFind) {
    auto& sb = *this->sb;

    auto it = std::find_if(sb.begin(), sb.end(), [](auto& f) { return f.x == 5; });
    ASSERT_EQ(it, sb.end());

    sb.insert(Foo{ 5, 1, 1 });
    it = std::find_if(sb.begin(), sb.end(), [](const auto& f) { return f.x == 5; });
    ASSERT_NE(it, sb.end());
    ASSERT_EQ(it->x, 5);
}
