#include "kstd/containers/FlatMap.hh"

#include "Core.hh"

using namespace kstd;

template <typename FlatMap> struct FlatMapTests : testing::Test {
    FlatMap fm;
};

class NameGenerator {
public:
    template <typename T> static std::string GetName(int) {
        if constexpr (std::is_same_v<T, DynamicFlatMap<std::string, int>>)
            return "DynamicFlatMap";
        else if constexpr (std::is_same_v<T, StaticFlatMap<std::string, int, 16u>>)
            return "StaticFlatMap";
    }
};

using TestTypes = ::testing::Types<
  DynamicFlatMap<std::string, int>, StaticFlatMap<std::string, int, 16u>>;

TYPED_TEST_SUITE(FlatMapTests, TestTypes, NameGenerator);

TYPED_TEST(FlatMapTests, empty) {
    ASSERT_EQ(this->fm.size(), 0u);
    ASSERT_TRUE(this->fm.empty());
}

TYPED_TEST(FlatMapTests, insertEmplaceClear) {
    this->fm.insert("a", 1);
    this->fm.insert("b", 2);
    this->fm.emplace("c", 3);

    ASSERT_EQ(this->fm.size(), 3u);

    this->fm.clear();
    ASSERT_EQ(this->fm.size(), 0u);
}

TYPED_TEST(FlatMapTests, get) {
    ASSERT_NULLPTR(this->fm.get("a"));

    this->fm.insert("a", 1);

    ASSERT_NOT_NULLPTR(this->fm.get("a"));
    ASSERT_EQ(*this->fm.get("a"), 1);
}

TYPED_TEST(FlatMapTests, put) {
    ASSERT_NULLPTR(this->fm.get("a"));

    ASSERT_EQ(this->fm.put("a", 1), 1);
    ASSERT_EQ(*this->fm.get("a"), 1);

    ASSERT_EQ(this->fm.put("a", 2), 2);
    ASSERT_EQ(*this->fm.get("a"), 2);
}

TYPED_TEST(FlatMapTests, insertTwice) {
    ASSERT_NE(this->fm.insert("a", 1), nullptr);
    ASSERT_EQ(this->fm.insert("a", 1), nullptr);
}

TYPED_TEST(FlatMapTests, erase) {
    this->fm.insert("a", 1);
    ASSERT_NOT_NULLPTR(this->fm.get("a"));

    this->fm.erase("a");
    ASSERT_NULLPTR(this->fm.get("a"));
}

TYPED_TEST(FlatMapTests, forEach) {
    int sum = 0;
    std::string key;
    this->fm.insert("a", 1);
    this->fm.insert("b", 2);

    this->fm.forEach([&](int v) { sum += v; });
    ASSERT_EQ(sum, 3);
    sum = 0;

    this->fm.forEach([&](const auto& k, const auto& v) {
        key += k;
        sum += v;
    });
    ASSERT_EQ(sum, 3);
    ASSERT_EQ(key, "ab");

    this->fm.forEach([&](int& v) { v += 5; });

    ASSERT_EQ(*this->fm.get("a"), 6);
    ASSERT_EQ(*this->fm.get("b"), 7);
}

TYPED_TEST(FlatMapTests, getKeys) {
    this->fm.insert("a", 1);
    this->fm.insert("b", 2);

    auto keys = this->fm.getKeys();
    ASSERT_EQ(keys.size(), 2);
    ASSERT_EQ(*keys[0], "a");
    ASSERT_EQ(*keys[1], "b");
}

TYPED_TEST(FlatMapTests, getValues) {
    this->fm.insert("a", 1);
    this->fm.insert("b", 2);

    auto values = this->fm.getValues();
    ASSERT_EQ(values.size(), 2);
    ASSERT_EQ(*values[0], 1);
    ASSERT_EQ(*values[1], 2);

    auto transformedValues = this->fm.getValues([](auto x) { return x * 2; });
    ASSERT_EQ(transformedValues.size(), 2);
    ASSERT_EQ(transformedValues[0], 2);
    ASSERT_EQ(transformedValues[1], 4);
}
