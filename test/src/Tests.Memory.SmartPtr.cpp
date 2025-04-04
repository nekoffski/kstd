#include "Core.hh"

#include "kstd/memory/Allocator.hh"
#include "kstd/memory/UniquePtr.hh"
#include "kstd/memory/SharedPtr.hh"

using namespace kstd;

struct UniquePtrProxy {
    template <typename T> using Ptr = UniquePtr<T>;

    template <typename T, typename... Args>
    static Ptr<T> create(Allocator* allocator, Args&&... args) {
        return makeUniqueWithAllocator<T>(allocator, std::forward<Args>(args)...);
    }
};

struct SharedPtrProxy {
    template <typename T> using Ptr = SharedPtr<T>;

    template <typename T, typename... Args>
    static auto create(Allocator* allocator, Args&&... args) {
        return makeSharedWithAllocator<T>(allocator, std::forward<Args>(args)...);
    }
};

template <typename PtrProxy> struct SmartPtrBasicTests : testing::Test {
    template <typename T> using Ptr = typename PtrProxy::Ptr<T>;

    void SetUp() override {
        LifetimeProbe<Foo>::reset();
        LifetimeProbe<int>::reset();
    }

    template <typename T, typename... Args> Ptr<T> create(Args&&... args) {
        return PtrProxy::template create<T>(&allocator, std::forward<Args>(args)...);
    }

    AllocatorMock allocator;
};

class NameGenerator {
public:
    template <typename T> static std::string GetName(int) {
        if constexpr (std::is_same_v<T, UniquePtrProxy>)
            return "UniquePtr";
        else if constexpr (std::is_same_v<T, SharedPtrProxy>)
            return "SharedPtr";
    }
};

using TestTypes = ::testing::Types<UniquePtrProxy, SharedPtrProxy>;

TYPED_TEST_SUITE(SmartPtrBasicTests, TestTypes, NameGenerator);

TYPED_TEST(SmartPtrBasicTests, empty) {
    typename TestFixture::Ptr<LifetimeProbe<>> ptr;

    ASSERT_FALSE((bool)ptr);
    ASSERT_TRUE(ptr.empty());

    ASSERT_EQ(ptr.get(), nullptr);
    ASSERT_EQ(ptr.getAllocator(), nullptr);

    ASSERT_EQ(LifetimeProbe<>::ctorCalls, 0u);
    ASSERT_EQ(LifetimeProbe<>::dctorCalls, 0u);
}

TYPED_TEST(SmartPtrBasicTests, clear) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    {
        auto ptr = this->template create<LifetimeProbe<int>>(1);
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ptr.clear();
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
    }

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TYPED_TEST(SmartPtrBasicTests, moveSemantics) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(2);
    EXPECT_CALL(this->allocator, deallocate).Times(2);

    auto lhs = this->template create<LifetimeProbe<int>>(1);

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);

    {
        auto rhs = this->template create<LifetimeProbe<int>>(1);
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

TYPED_TEST(SmartPtrBasicTests, objectLifetime) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    {
        auto ptr = this->template create<LifetimeProbe<int>>(1);
        ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
        ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 0u);
        ASSERT_EQ(ptr->value, 1);
    }

    ASSERT_EQ(LifetimeProbe<int>::ctorCalls, 1u);
    ASSERT_EQ(LifetimeProbe<int>::dctorCalls, 1u);
}

TYPED_TEST(SmartPtrBasicTests, throwingConstructorClearsMemory) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    struct Throwing {
        Throwing() { throw std::runtime_error{ "" }; }
    };

    EXPECT_THROW(this->template create<Throwing>(), std::runtime_error);
}

TYPED_TEST(SmartPtrBasicTests, polymorphism) {
    EXPECT_CALL(this->allocator, allocateRaw).Times(1);
    EXPECT_CALL(this->allocator, deallocate).Times(1);

    Base::reset();
    Derived::reset();

    typename TestFixture::Ptr<Base> ptr = this->template create<Derived>();
    ptr->foo();

    EXPECT_TRUE(Derived::called);
    EXPECT_FALSE(Base::called);
}
