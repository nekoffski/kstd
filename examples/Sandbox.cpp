
#include "kstd/Core.hh"
#include "kstd/Log.hh"
#include "kstd/memory/Mallocator.hh"

int main() {
    kstd::log::init("sandbox");
    kstd::log::info("Hello world!");

    kstd::Mallocator<kstd::AllocatorReportStrategy::disabled> m;

    auto p = m.allocate<int>();
    m.deallocate(p);

    return 0;
}
