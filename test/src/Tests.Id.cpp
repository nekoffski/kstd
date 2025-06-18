#include "kstd/Id.hh"

#include <ranges>
#include <unordered_map>

#include <gtest/gtest.h>

using namespace kstd;

static constexpr Id8::Type value1 = 1;
static constexpr Id8::Type value2 = 7;

TEST(WithNameTests, nameConflict) {
    struct Mutable : WithName<Mutable, "Mutable", false> {};
    Mutable m{};
    ASSERT_EQ(m.getName(), "Mutable_0");
    ASSERT_EQ(m.getId(), 0);
    m.setName("Mutable_1");
    Mutable m2{};
    ASSERT_EQ(m2.getName(), "Mutable_2");
    ASSERT_EQ(m2.getId(), 2);
}

TEST(WithNameTests, mutableOverload) {
    struct Mutable : WithName<Mutable, "Mutable", false> {};
    Mutable m{};
    ASSERT_EQ(m.getName(), "Mutable_0");
    m.setName("Test");
    ASSERT_EQ(m.getName(), "Test");
}

TEST(WithNameTests, constOverload) {
    struct Const : WithName<Const, "Const"> {};
    ASSERT_EQ(Const{}.getName(), "Const_0");
}

TEST(WithIdTests, basic) {
    struct Foo : WithId<Foo> {};

    Foo f1;
    ASSERT_EQ(f1.getId(), 0);
    Foo f2;
    ASSERT_EQ(f2.getId(), 1);

    {
        Foo f3;
        ASSERT_EQ(f3.getId(), 2);
    }

    Foo f4;
    ASSERT_EQ(f4.getId(), 2);

    {
        Foo f5;
        ASSERT_EQ(f5.getId(), 3);

        f4 = std::move(f5);
    }

    EXPECT_EQ(f4.getId(), 3);
    Foo f6;
    EXPECT_EQ(f6.getId(), 2);
}

TEST(WithUuidTests, basic) {}

TEST(SequenceGeneratorTests, generateSequence) {
    SequenceGenerator<0, 5> seq;
    for (int x = 0; x <= 5; ++x)
        for (int i = 0; i <= 5; ++i) ASSERT_EQ(seq.get(), i);
}

TEST(UuidTests, uniqueSimple) { EXPECT_NE(generateUuid(), generateUuid()); }

TEST(UuidTests, uniqueComplex) {
    static constexpr u32 uuidsToGenerate = 10000u;
    std::unordered_map<Uuid, int> uuids;

    for (int i = 0; i < uuidsToGenerate; ++i) uuids[generateUuid()]++;

    for (auto& uuidOccurences : uuids | std::views::values)
        ASSERT_EQ(uuidOccurences, 1) << "UUID conflict!";
}

TEST(IdTests, emptyIdToBoolean) {
    Id8 id;

    EXPECT_FALSE(id.hasValue());
}

TEST(IdTests, idToBoolean) {
    Id8 id(value1);
    EXPECT_TRUE(id.hasValue());
}

TEST(IdTests, getValue) {
    Id8 id(value1);
    ASSERT_TRUE(id.hasValue());
    EXPECT_EQ(value1, *id);
    EXPECT_EQ(value1, id.get());
}

TEST(IdTests, reassignValue) {
    Id8 id(value1);
    EXPECT_EQ(value1, *id);
    id = value2;
    EXPECT_EQ(value2, *id);
}

TEST(IdTests, invalidate) {
    Id8 id(value1);
    ASSERT_TRUE(id.hasValue());
    id.invalidate();
    EXPECT_FALSE(id.hasValue());
}
