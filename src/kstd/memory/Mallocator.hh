#pragma once

#include "Allocator.hh"

#include <cstdlib>

namespace kstd {

template <AllocatorReportStrategy ReportStrategy = AllocatorReportStrategy::enabled>
class Mallocator : public AllocatorBase<ReportStrategy> {
private:
    [[nodiscard]] void* allocateImpl(u64 n, [[maybe_unused]] u64 alignment)
      override {
        return std::malloc(n);
    }

    void deallocateImpl(void* p) noexcept override { std::free(p); }
};

}  // namespace kstd
