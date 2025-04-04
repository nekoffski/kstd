#include <benchmark/benchmark.h>

#include "Utils.hh"

#include "kstd/memory/AlignedAllocator.hh"
#include "kstd/memory/Mallocator.hh"

static void allocateIterateDeallocate(
  kstd::Allocator& allocator, benchmark::State& state
) {
    const auto n          = state.range(0);
    const auto iterations = state.range(1);

    for (auto _ : state) {
        for (int j = 0; j < iterations; ++j) {
            Foo* arr = allocator.allocate<Foo>(n);

            for (auto i = 0u; i < n; ++i) {
                auto& el = arr[i];
                el.x     = i + 100u;
                el.y     = i * 2u + 1337u;
                el.z     = i * 100u + 13u;
            }
            for (auto i = 0u; i < n; ++i) {
                auto& el = arr[i];
                el.x += i * 7u;
                el.y += i * 2u;
                el.z += i * 10u;
            }
            benchmark::DoNotOptimize(arr);
            allocator.deallocate(arr);
        }
    }
}

static void Mallocator_allocateIterateDeallocate(benchmark::State& state) {
    kstd::Mallocator<> mallocator;
    allocateIterateDeallocate(mallocator, state);
}

static void AlignedAllocator_allocateIterateDeallocate(benchmark::State& state) {
    kstd::AlignedAllocator<> aa;
    allocateIterateDeallocate(aa, state);
}

BENCHMARK(Mallocator_allocateIterateDeallocate)
  ->Ranges({
    { 1024, 8 * 1024  },
    { 1024, 16 * 1024 }
});

BENCHMARK(AlignedAllocator_allocateIterateDeallocate)
  ->Ranges({
    { 1024, 8 * 1024  },
    { 1024, 16 * 1024 }
});
