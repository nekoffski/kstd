#include <algorithm>
#include <random>

#include <benchmark/benchmark.h>

#include "Utils.hh"

#include "kstd/memory/PoolAllocator.hh"
#include "kstd/memory/UniquePtr.hh"

template <typename Pointer, typename Create>
static void iterateArrayOfPointers(benchmark::State& state, Create&& create) {
    const auto size = state.range(0);

    std::vector<Pointer> pointers;
    std::vector<Foo*> noise;
    pointers.reserve(size);
    for (auto i = 0u; i < size; ++i) {
        pointers.push_back(create());
        for (int j = 0u; j < 64; ++j) noise.push_back(new Foo{});
    }

    auto rng = std::default_random_engine{};
    std::ranges::shuffle(pointers, rng);

    for (auto _ : state) {
        for (auto i = 0u; i < size; ++i) {
            auto& r = pointers[i];
            r->x    = i + 13;
            r->y    = i * 2;
            r->z    = i * 100;
        }
        for (auto i = 0u; i < size; ++i) {
            auto& r = pointers[i];
            r->x += 10;
            r->y = r->x + 2;
            r->z = r->y * 2;
        }
        benchmark::DoNotOptimize(pointers);
    }
    benchmark::DoNotOptimize(noise);

    for (auto& r : noise) delete r;
}

static void StdUniquePtr_iterateArrayOfPointers(benchmark::State& state) {
    iterateArrayOfPointers<std::unique_ptr<Foo>>(state, [&]() {
        return std::make_unique<Foo>();
    });
}

static void UniquePtr_iterateArrayOfPointers(benchmark::State& state) {
    iterateArrayOfPointers<kstd::UniquePtr<Foo>>(state, [&]() {
        return kstd::makeUnique<Foo>();
    });
}

static void UniquePtrStackPoolAllocator_iterateArrayOfPointers(
  benchmark::State& state
) {
    kstd::StackPoolAllocator<Foo, 32u * 1024u> allocator;
    iterateArrayOfPointers<kstd::UniquePtr<Foo>>(state, [&]() {
        return kstd::makeUniqueWithAllocator<Foo>(&allocator);
    });
}

static void UniquePtrHeapPoolAllocator_iterateArrayOfPointers(benchmark::State& state
) {
    kstd::HeapPoolAllocator<Foo> allocator{ 32 * 1024 };
    iterateArrayOfPointers<kstd::UniquePtr<Foo>>(state, [&]() {
        return kstd::makeUniqueWithAllocator<Foo>(&allocator);
    });
}

BENCHMARK(StdUniquePtr_iterateArrayOfPointers)->Range(128, 32u * 1024u);
BENCHMARK(UniquePtr_iterateArrayOfPointers)->Range(128, 32 * 1024);
BENCHMARK(UniquePtrStackPoolAllocator_iterateArrayOfPointers)
  ->Range(128, 32u * 1024u);
BENCHMARK(UniquePtrHeapPoolAllocator_iterateArrayOfPointers)
  ->Range(128, 32u * 1024u);
