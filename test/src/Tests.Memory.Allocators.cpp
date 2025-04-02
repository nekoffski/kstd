#include <gtest/gtest.h>

#include <memory>

#include "kstd/memory/Mallocator.hh"
#include "kstd/memory/AlignedAllocator.hh"

using namespace kstd;

struct T {
    u8 x;
    u32 y;
    u64 z;
};

TEST(AlignedAllocatorTests, invalidAlignment) {
    {
        AlignedAllocator a;
        EXPECT_EQ(a.allocateRaw(4u, 3u), nullptr);
    }
    {
        AlignedAllocator<
          AllocatorReportStrategy::disabled,
          AlignedAllocatorFailureStrategy::throwException>
          a;
        EXPECT_THROW(a.allocateRaw(4u, 3u), std::bad_alloc);
    }
}

struct AllocatorTestBase : testing::TestWithParam<std::shared_ptr<Allocator>> {
    static constexpr u64 elements = 10u;
};

struct AllocatorBasicTests : AllocatorTestBase {};

TEST_P(AllocatorBasicTests, allocDealloc) {
    Mallocator mallocator;
    int* ptr = mallocator.allocate<int>();
    *ptr     = 1;
    EXPECT_EQ(*ptr, 1);
    mallocator.deallocate(ptr);
}

TEST_P(AllocatorBasicTests, allocDeallocArray) {
    Mallocator mallocator;

    int* ptr = mallocator.allocate<int>(elements);
    for (auto i = 0u; i < elements; ++i) ptr[i] = i;
    for (auto i = 0u; i < elements; ++i) EXPECT_EQ(ptr[i], i);

    mallocator.deallocate(ptr);
}

struct AllocatorComplexTypeTests : AllocatorTestBase {};

TEST_P(AllocatorComplexTypeTests, allocDeallocComplexType) {
    Mallocator mallocator;
    T* ptr = mallocator.allocate<T>();
    ptr->x = 1u;
    ptr->y = 2u;
    ptr->z = 3u;

    EXPECT_EQ(ptr->x, 1u);
    EXPECT_EQ(ptr->y, 2u);
    EXPECT_EQ(ptr->z, 3u);
    mallocator.deallocate(ptr);
}

TEST_P(AllocatorComplexTypeTests, allocDeallocArrayComplexType) {
    Mallocator mallocator;

    T* ptr = mallocator.allocate<T>(elements);
    for (auto i = 0u; i < elements; ++i) {
        ptr[i].x = i;
        ptr[i].y = i * 2u;
        ptr[i].z = i * 3u;
    }
    for (auto i = 0u; i < elements; ++i) {
        EXPECT_EQ(ptr[i].x, i);
        EXPECT_EQ(ptr[i].y, i * 2u);
        EXPECT_EQ(ptr[i].z, i * 3u);
    }
    mallocator.deallocate(ptr);
}

auto createNameGetter() {
    // must match list returned by createAllocators() !!
    static std::vector<std::string> allocatorNames{
        "MallocatorDefault",
        "MallocatorDisabledReports",
        "AlignedAllocatorDefault",
        "AlignedAllocatorEnabledReportException",
        "AlignedAllocatorDisabledReports",
        "AlignedAllocatorDisabledReportsException",
    };
    return [&](const auto& info) -> std::string {
        return allocatorNames[info.index];
    };
}

static std::vector<std::shared_ptr<Allocator>> createBasicTestsAllocators() {
    return {
        std::make_shared<Mallocator<>>(),
        std::make_shared<Mallocator<AllocatorReportStrategy::disabled>>(),
        std::make_shared<AlignedAllocator<>>(),
        std::make_shared<AlignedAllocator<
          AllocatorReportStrategy::enabled,
          AlignedAllocatorFailureStrategy::throwException>>(),
        std::make_shared<AlignedAllocator<
          AllocatorReportStrategy::disabled,
          AlignedAllocatorFailureStrategy::throwException>>(),
    };
}

INSTANTIATE_TEST_SUITE_P(
  _, AllocatorBasicTests, testing::ValuesIn(createBasicTestsAllocators()),
  createNameGetter()
);

INSTANTIATE_TEST_SUITE_P(
  _, AllocatorComplexTypeTests, testing::ValuesIn(createBasicTestsAllocators()),
  createNameGetter()
);
