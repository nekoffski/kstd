#include "Core.hh"

#include "kstd/Functional.hh"

TEST(ToVectorTests, basic) {
    std::vector<int> v{ 1, 2, 3, 4 };
    std::vector<int> output =
      v | std::views::transform([](int x) { return x * 2; }) | kstd::toVector<int>();

    ASSERT_EQ(output.size(), v.size());

    for (int i = 1; i <= 4; ++i) ASSERT_EQ(output[i - 1], i * 2);
}

TEST(GuardCallTests, callOnDestructor) {
    bool called = false;
    {
        kstd::GuardCall guard{ [&]() { called = true; } };
        ASSERT_FALSE(called);
    }
    ASSERT_TRUE(called);
}
