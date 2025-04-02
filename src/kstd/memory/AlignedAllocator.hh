#pragma once

#include <cstdlib>
#include <new>

#include "Allocator.hh"

namespace kstd {

enum class AlignedAllocatorFailureStrategy : u8 { returnNull, throwException };

template <
  AllocatorReportStrategy ReportStrategy = AllocatorReportStrategy::enabled,
  AlignedAllocatorFailureStrategy FailureStrategy =
    AlignedAllocatorFailureStrategy::returnNull>
class AlignedAllocator : public AllocatorBase<ReportStrategy> {
private:
    [[nodiscard]] void* allocateImpl(u64 n, u64 alignment) override {
        auto ptr = std::aligned_alloc(alignment, n);
        handleError(ptr);
        return ptr;
    }

    void deallocateImpl(void* p) noexcept override { std::free(p); }

    constexpr void handleError(const void* ptr) {
        if constexpr (FailureStrategy
                      == AlignedAllocatorFailureStrategy::throwException) {
            if (ptr == nullptr) throw std::bad_alloc{};
        }
    }
};

}  // namespace kstd
