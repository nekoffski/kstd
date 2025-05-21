#include "Core.hh"

#include "kstd/memory/LocalPtr.hh"

using namespace kstd;

struct LocalPtrTests : testing::Test {
    void SetUp() { LifetimeProbe<int>::reset(); }
};

TEST_F(LocalPtrTests, empty) {
    LocalPtr<LifetimeProbe<int>> ptr;
    ASSERT_TRUE(ptr.empty());
    ASSERT_FALSE((bool)ptr);
    ASSERT_EQ(ptr.get(), nullptr);

    ptr.emplace(1);
    ASSERT_FALSE(ptr.empty());
    ASSERT_TRUE((bool)ptr);
    ASSERT_EQ(ptr.get()->value, 1);
    ASSERT_EQ(ptr->value, 1);
}

TEST_F(LocalPtrTests, lifetime) {
    {
        LocalPtr<LifetimeProbe<int>> ptr;
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 0u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

        LocalPtr<LifetimeProbe<int>> ptr2{ 1 };
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
    }
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TEST_F(LocalPtrTests, clear) {
    LocalPtr<LifetimeProbe<int>> ptr{ 1 };
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
    ptr.clear();
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TEST_F(LocalPtrTests, move) {
    LocalPtr<LifetimeProbe<std::string>> ptr{ "a" };
    LocalPtr<LifetimeProbe<std::string>> ptr2{ "b" };
    ASSERT_EQ(LifetimeProbe<std::string>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<std::string>::dctorCalls, 0u);

    ASSERT_EQ((*ptr).value, "a");

    auto ptr3 = std::move(ptr);
    ASSERT_EQ(LifetimeProbe<std::string>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<std::string>::dctorCalls, 1u);

    ASSERT_EQ((*ptr3).value, "a");
    ptr3 = std::move(ptr2);

    ASSERT_EQ((*ptr3).value, "b");
    ASSERT_EQ(LifetimeProbe<std::string>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<std::string>::dctorCalls, 3u);

    ptr3.clear();
    ASSERT_EQ(LifetimeProbe<std::string>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<std::string>::dctorCalls, 4u);
}
