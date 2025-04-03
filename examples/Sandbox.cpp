
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include "kstd/memory/Mallocator.hh"
#include "kstd/memory/PoolAllocator.hh"

struct Foo {
    kstd::u32 x;
    kstd::u32 y;
};

int main() {
    kstd::log::init("sandbox");
    kstd::log::info("Hello world!");

    kstd::StackPoolAllocator<Foo, 32> stackAllocator;
    Foo* x = stackAllocator.allocate<Foo>();
    stackAllocator.deallocate(x);

    kstd::HeapPoolAllocator<Foo> heapAllocator{ 32u };

    return 0;
}
