#include "Core.hh"

#include <memory>

#include "kstd/memory/PoolAllocator.hh"

using namespace kstd;

static constexpr u64 defaultCapacity = 64u;
static constexpr u64 smallCapacity   = 6u;

TEST(HeapPoolAllocatorTests, defaultCapacity) {
    HeapPoolAllocator<Foo> allocator{ defaultCapacity };
    ASSERT_EQ(allocator.capacity(), defaultCapacity);
}

TEST(StackPoolAllocatorTests, defaultCapacity) {
    StackPoolAllocator<Foo, defaultCapacity> allocator;
    ASSERT_EQ(allocator.capacity(), defaultCapacity);
}

static void testSlotsLeftUpdates(auto& allocator) {
    ASSERT_EQ(allocator.slotsLeft(), defaultCapacity);

    Foo* ptr = allocator.template allocate<Foo>();
    ASSERT_EQ(allocator.slotsLeft(), defaultCapacity - 1);

    Foo* arr = allocator.template allocate<Foo>(10u);

    ASSERT_EQ(allocator.slotsLeft(), defaultCapacity - 11);
    ASSERT_EQ((int)(arr - ptr), 1);

    allocator.deallocate(ptr);
    ASSERT_EQ(allocator.slotsLeft(), defaultCapacity - 10);

    allocator.deallocate(arr);
    ASSERT_EQ(allocator.slotsLeft(), defaultCapacity);
}

TEST(HeapPoolAllocatorTests, slotsLeftUpdates) {
    HeapPoolAllocator<Foo> allocator{ defaultCapacity };
    testSlotsLeftUpdates(allocator);
}

TEST(StackPoolAllocatorTests, slotsLeftUpdates) {
    StackPoolAllocator<Foo, defaultCapacity> allocator;
    testSlotsLeftUpdates(allocator);
}

static void testAllocateOutOfSpace(auto& allocator) {
    std::array<Foo*, smallCapacity> container = { nullptr };

    for (u32 i = 0u; i < smallCapacity; ++i) {
        container[i] = allocator.template allocate<Foo>();
        ASSERT_NE(container[i], nullptr);
    }

    ASSERT_EQ(allocator.template allocate<Foo>(), nullptr);

    allocator.deallocate(container[0u]);
    auto ptr = allocator.template allocate<Foo>();
    ASSERT_EQ((int)(container[1] - ptr), 1);
    ASSERT_NE(ptr, nullptr);
    allocator.deallocate(ptr);

    allocator.deallocate(container[2u]);
    allocator.deallocate(container[4u]);

    ASSERT_EQ(allocator.slotsLeft(), 3u);
    ASSERT_EQ(allocator.template allocate<Foo>(3u), nullptr);

    allocator.deallocate(container[3u]);
    ASSERT_EQ(allocator.slotsLeft(), 4u);
    auto arr = allocator.template allocate<Foo>(3u);
    ASSERT_EQ((int)(arr - ptr), 2);
    ASSERT_NE(arr, nullptr);

    ASSERT_EQ(allocator.slotsLeft(), 1u);
    ASSERT_NE(allocator.template allocate<Foo>(1u), nullptr);

    ASSERT_EQ(allocator.slotsLeft(), 0u);
    ASSERT_EQ(allocator.template allocate<Foo>(1u), nullptr);
}

TEST(HeapPoolAllocatorTests, allocateOutOfSpace) {
    HeapPoolAllocator<
      Foo, AllocatorReportStrategy::disabled, AllocatorFailureStrategy::returnNull>
      allocator{ smallCapacity };
    testAllocateOutOfSpace(allocator);
}

TEST(StackPoolAllocatorTests, allocateOutOfSpace) {
    StackPoolAllocator<
      Foo, smallCapacity, AllocatorReportStrategy::disabled,
      AllocatorFailureStrategy::returnNull>
      allocator;
    testAllocateOutOfSpace(allocator);
}

static void testAllocateDeallocateArrays(auto& allocator) {
    auto arr1 = allocator.template allocate<Foo>(3u);
    ASSERT_NE(arr1, nullptr);
    for (u64 i = 0u; i < 3u; ++i) arr1[i].y = i;

    auto arr2 = allocator.template allocate<Foo>(3u);
    ASSERT_NE(arr2, nullptr);
    for (u64 i = 0u; i < 3u; ++i) arr2[i].y = i + 5u;

    ASSERT_EQ(allocator.template allocate<Foo>(3u), nullptr);

    for (u64 i = 0u; i < 3u; ++i) {
        EXPECT_EQ(arr1[i].y, i);
        EXPECT_EQ(arr2[i].y, i + 5u);
    }

    allocator.deallocate(arr1);

    for (u64 i = 0u; i < 3u; ++i) {
        EXPECT_EQ(arr1[i].y, 0u);
        EXPECT_EQ(arr2[i].y, i + 5u);
    }

    ASSERT_NE(allocator.template allocate<Foo>(2u), nullptr);
    ASSERT_NE(allocator.template allocate<Foo>(), nullptr);
    ASSERT_EQ(allocator.template allocate<Foo>(), nullptr);
}

TEST(HeapPoolAllocatorTests, allocateDeallocateArrays) {
    HeapPoolAllocator<
      Foo, AllocatorReportStrategy::disabled, AllocatorFailureStrategy::returnNull>
      allocator{ smallCapacity };
    testAllocateDeallocateArrays(allocator);
}

TEST(StackPoolAllocatorTests, allocateDeallocateArrays) {
    StackPoolAllocator<
      Foo, smallCapacity, AllocatorReportStrategy::disabled,
      AllocatorFailureStrategy::returnNull>
      allocator;
    testAllocateDeallocateArrays(allocator);
}

static void testDeallocatesClearsMemory(auto& allocator) {
    auto ptr = allocator.template allocate<Foo>();
    ptr->z   = 1337u;
    ASSERT_EQ(ptr->z, 1337u);
    allocator.deallocate(ptr);
    ASSERT_EQ(ptr->z, 0u);
}

TEST(HeapPoolAllocatorTests, deallocatesClearsMemory) {
    HeapPoolAllocator<
      Foo, AllocatorReportStrategy::disabled, AllocatorFailureStrategy::returnNull>
      allocator{ smallCapacity };
    testDeallocatesClearsMemory(allocator);
}

TEST(StackPoolAllocatorTests, deallocatesClearsMemory) {
    StackPoolAllocator<
      Foo, smallCapacity, AllocatorReportStrategy::disabled,
      AllocatorFailureStrategy::returnNull>
      allocator;
    testDeallocatesClearsMemory(allocator);
}

static void testAllocateInvalidSize(auto& allocator) {
    struct Bar {
        Foo foo;
        u64 b;
    };
    auto ptr = allocator.template allocate<Bar>();
    ASSERT_EQ(ptr, nullptr);
}

TEST(HeapPoolAllocatorTests, allocateInvalidSize) {
    HeapPoolAllocator<
      Foo, AllocatorReportStrategy::disabled, AllocatorFailureStrategy::returnNull>
      allocator{ smallCapacity };
    testAllocateInvalidSize(allocator);
}

TEST(StackPoolAllocatorTests, allocateInvalidSize) {
    StackPoolAllocator<
      Foo, smallCapacity, AllocatorReportStrategy::disabled,
      AllocatorFailureStrategy::returnNull>
      allocator;
    testAllocateInvalidSize(allocator);
}
