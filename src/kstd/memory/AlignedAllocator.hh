#pragma once

#include <cstdlib>
#include <new>

#include "Allocator.hh"

namespace kstd {

template <
  AllocatorReportStrategy ReportStrategy   = AllocatorReportStrategy::enabled,
  AllocatorFailureStrategy FailureStrategy = AllocatorFailureStrategy::panic>
class AlignedAllocator : public AllocatorBase<ReportStrategy> {
private:
    [[nodiscard]] void* allocateImpl(u64 n, u64 alignment) override {
        auto ptr = std::aligned_alloc(alignment, n);
        handleError(ptr);
        return ptr;
    }

    void deallocateImpl(void* p) noexcept override { std::free(p); }

    constexpr void handleError(const void* ptr) {
        if constexpr (FailureStrategy == AllocatorFailureStrategy::throwException) {
            if (ptr == nullptr) throw std::bad_alloc{};
        } else if constexpr (FailureStrategy == AllocatorFailureStrategy::panic) {
            if (ptr == nullptr) log::panic("Could not allocate memory");
        }
    }
};

}  // namespace kstd
