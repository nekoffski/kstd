#include <gtest/gtest.h>

#include <memory>

#include "kstd/memory/Mallocator.hh"
#include "kstd/memory/AlignedAllocator.hh"
#include "kstd/memory/PoolAllocator.hh"

#include "Utils.hh"

using namespace kstd;

TEST(AlignedAllocatorTests, invalidAlignment) {
    {
        AlignedAllocator<
          AllocatorReportStrategy::disabled, AllocatorFailureStrategy::panic>
          a;
        ASSERT_DEATH(a.allocateRaw(4u, 3u), "");
    }
    {
        AlignedAllocator<
          AllocatorReportStrategy::disabled, AllocatorFailureStrategy::returnNull>
          a;
        ASSERT_EQ(a.allocateRaw(4u, 3u), nullptr);
    }
    {
        AlignedAllocator<
          AllocatorReportStrategy::disabled,
          AllocatorFailureStrategy::throwException>
          a;
        ASSERT_THROW(a.allocateRaw(4u, 3u), std::bad_alloc);
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
    ASSERT_EQ(*ptr, 1);
    mallocator.deallocate(ptr);
}

TEST_P(AllocatorBasicTests, allocDeallocArray) {
    Mallocator mallocator;

    int* ptr = mallocator.allocate<int>(elements);
    for (auto i = 0u; i < elements; ++i) ptr[i] = i;
    for (auto i = 0u; i < elements; ++i) ASSERT_EQ(ptr[i], i);

    mallocator.deallocate(ptr);
}

struct AllocatorComplexTypeTests : AllocatorTestBase {};

TEST_P(AllocatorComplexTypeTests, allocDeallocComplexType) {
    Mallocator mallocator;
    Foo* ptr = mallocator.allocate<Foo>();
    ptr->x   = 1u;
    ptr->y   = 2u;
    ptr->z   = 3u;

    ASSERT_EQ(ptr->x, 1u);
    ASSERT_EQ(ptr->y, 2u);
    ASSERT_EQ(ptr->z, 3u);
    mallocator.deallocate(ptr);
}

TEST_P(AllocatorComplexTypeTests, allocDeallocArrayComplexType) {
    Mallocator mallocator;

    Foo* ptr = mallocator.allocate<Foo>(elements);
    for (auto i = 0u; i < elements; ++i) {
        ptr[i].x = i;
        ptr[i].y = i * 2u;
        ptr[i].z = i * 3u;
    }
    for (auto i = 0u; i < elements; ++i) {
        ASSERT_EQ(ptr[i].x, i);
        ASSERT_EQ(ptr[i].y, i * 2u);
        ASSERT_EQ(ptr[i].z, i * 3u);
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
        "HeapPoolAllocatorDefault",
        "StackPoolAllocatorDefault"
    };
    return [&](const auto& info) -> std::string {
        return allocatorNames[info.index];
    };
}

template <typename T>
static std::vector<std::shared_ptr<Allocator>> createAllocators() {
    return {
        std::make_shared<Mallocator<>>(),
        std::make_shared<Mallocator<AllocatorReportStrategy::disabled>>(),
        std::make_shared<AlignedAllocator<>>(),
        std::make_shared<AlignedAllocator<
          AllocatorReportStrategy::enabled,
          AllocatorFailureStrategy::throwException>>(),
        std::make_shared<AlignedAllocator<
          AllocatorReportStrategy::disabled,
          AllocatorFailureStrategy::throwException>>(),
        std::make_shared<HeapPoolAllocator<T>>(64u),
        std::make_shared<StackPoolAllocator<T, 64u>>(),
    };
}

INSTANTIATE_TEST_SUITE_P(
  _, AllocatorBasicTests, testing::ValuesIn(createAllocators<int>()),
  createNameGetter()
);

INSTANTIATE_TEST_SUITE_P(
  _, AllocatorComplexTypeTests, testing::ValuesIn(createAllocators<Foo>()),
  createNameGetter()
);
