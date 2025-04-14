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
    LocalPtr<LifetimeProbe<int>> ptr{ 1 };
    LocalPtr<LifetimeProbe<int>> ptr2{ 2 };
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    ASSERT_EQ((*ptr).value, 1);

    auto ptr3 = std::move(ptr);
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    ASSERT_EQ((*ptr3).value, 1);
    ptr3 = std::move(ptr2);

    ASSERT_EQ((*ptr3).value, 2);
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);

    ptr3.clear();
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 2u);
}
