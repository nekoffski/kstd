#include "Core.hh"

#include "kstd/memory/Allocator.hh"
#include "kstd/memory/UniquePtr.hh"

using namespace kstd;

struct UniquePtrTests : testing::Test {
    void SetUp() override {
        LifetimeProbe<Foo>::reset();
        LifetimeProbe<int>::reset();

        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 0u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ASSERT_EQ(LifetimeProbe<Foo>::ctorCalls, 0u);
        ASSERT_EQ(LifetimeProbe<Foo>::dctorCalls, 0u);
    }

    AllocatorMock allocator;
};

TEST_F(UniquePtrTests, empty) {
    UniquePtr<LifetimeProbe<>> ptr;

    ASSERT_FALSE((bool)ptr);
    ASSERT_TRUE(ptr.empty());

    ASSERT_EQ(ptr.get(), nullptr);
    ASSERT_EQ(ptr.getAllocator(), nullptr);

    ASSERT_EQ(LifetimeProbe<>::ctorCalls, 0u);
    ASSERT_EQ(LifetimeProbe<>::dctorCalls, 0u);
}

TEST_F(UniquePtrTests, clear) {
    EXPECT_CALL(allocator, allocateRaw).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    {
        auto ptr = makeUniqueWithAllocator<LifetimeProbe<int>>(&allocator, 1);
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ptr.clear();
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
    }

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TEST_F(UniquePtrTests, moveSemantics) {
    EXPECT_CALL(allocator, allocateRaw).Times(2);
    EXPECT_CALL(allocator, deallocate).Times(2);

    auto lhs = makeUniqueWithAllocator<LifetimeProbe<int>>(&allocator, 1);

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    {
        auto rhs = makeUniqueWithAllocator<LifetimeProbe<int>>(&allocator, 1);
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ASSERT_EQ(rhs->value, 1);

        auto ptr = std::move(rhs);
        ASSERT_TRUE(rhs.empty());
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ASSERT_EQ(ptr->value, 1);

        lhs = std::move(ptr);

        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
        ASSERT_TRUE(ptr.empty());
    }

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
    ASSERT_EQ(lhs->value, 1);

    lhs.clear();

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 2u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 2u);
}

TEST_F(UniquePtrTests, objectLifetime) {
    EXPECT_CALL(allocator, allocateRaw).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    {
        auto ptr = makeUniqueWithAllocator<LifetimeProbe<int>>(&allocator, 1);
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ASSERT_EQ(ptr->value, 1);
    }

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TEST_F(UniquePtrTests, throwingConstructorClearsMemory) {
    EXPECT_CALL(allocator, allocateRaw).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    struct Throwing {
        Throwing() { throw std::runtime_error{ "" }; }
    };

    EXPECT_THROW(makeUniqueWithAllocator<Throwing>(&allocator), std::runtime_error);
}

TEST_F(UniquePtrTests, polymorphism) {
    EXPECT_CALL(allocator, allocateRaw).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    Base::reset();
    Derived::reset();

    UniquePtr<Base> ptr = makeUniqueWithAllocator<Derived>(&allocator);
    ptr->foo();

    EXPECT_TRUE(Derived::called);
    EXPECT_FALSE(Base::called);
}
