#include "Core.hh"

#include "kstd/memory/SharedPtr.hh"

using namespace kstd;

struct SharedPtrTests : testing::Test {
    void SetUp() { LifetimeProbe<int>::reset(); }

    AllocatorMock allocator;
};

TEST_F(SharedPtrTests, copyExtendsLifetime) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    auto ptr = makeSharedWithAllocator<LifetimeProbe<int>>(&allocator, 1);
    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    auto ptr2 = ptr;
    ptr.clear();

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    ASSERT_TRUE(ptr.empty());
    ASSERT_FALSE(ptr2.empty());
    ASSERT_EQ(ptr2->value, 1);

    ptr2.clear();

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TEST_F(SharedPtrTests, copyCastExtendsLifetime) {
    Derived::reset();
    Base::reset();

    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    auto ptr             = makeSharedWithAllocator<Derived>(&allocator);
    SharedPtr<Base> base = ptr;
    ptr.clear();

    ASSERT_FALSE(Derived::destroyed);
    ASSERT_FALSE(Base::destroyed);

    base->foo();

    ASSERT_TRUE(Derived::called);
    ASSERT_FALSE(Base::called);

    auto derived = sharedPtrCast<Derived>(base);
    base.clear();

    ASSERT_FALSE(Derived::destroyed);
    ASSERT_FALSE(Base::destroyed);

    derived.clear();

    ASSERT_TRUE(Derived::destroyed);
    ASSERT_TRUE(Base::destroyed);
}

TEST_F(SharedPtrTests, upcast) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    struct A {
        virtual ~A() = default;
    };

    struct B : A {
        ~B() = default;
    };

    struct C : A {
        ~C() = default;
    };

    SharedPtr<A> base;
    ASSERT_TRUE(sharedPtrCast<B>(base).empty());

    base = makeSharedWithAllocator<B>(&allocator);
    ASSERT_TRUE(sharedPtrCast<C>(base).empty());
    ASSERT_FALSE(sharedPtrCast<B>(base).empty());
}
