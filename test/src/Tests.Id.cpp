#include "kstd/Id.hh"

#include <ranges>
#include <unordered_map>

#include <gtest/gtest.h>

using namespace kstd;

static constexpr Id8::Type value1 = 1;
static constexpr Id8::Type value2 = 7;

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
