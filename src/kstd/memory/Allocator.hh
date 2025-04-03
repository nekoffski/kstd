#pragma once

#include <cstddef>
#include <typeindex>

#include "kstd/Core.hh"
#include "kstd/Log.hh"

namespace kstd {

enum class AllocatorReportStrategy : u8 { enabled, disabled };
enum class AllocatorFailureStrategy : u8 { returnNull, throwException, panic };

struct Allocator {
    virtual ~Allocator() = default;

    template <typename T> [[nodiscard]] T* allocate(u64 n = 1u) {
        const auto bytes     = sizeof(T) * n;
        const auto alignment = alignof(T);
        return static_cast<T*>(allocateRaw(bytes, alignment));
    }

    virtual void* allocateRaw(u64 n, u64 alignment = alignof(std::max_align_t)) = 0;
    virtual void deallocate(void* ptr) noexcept                                 = 0;
};

template <AllocatorReportStrategy ReportStrategy = AllocatorReportStrategy::enabled>
class AllocatorBase : public Allocator {
public:
    void* allocateRaw(u64 n, u64 alignment) {
        auto ptr = allocateImpl(n, alignment);
        if constexpr (ReportStrategy == AllocatorReportStrategy::enabled)
            reportAlloc(ptr, alignment, n);
        return ptr;
    }

    void deallocate(void* ptr) noexcept {
        if constexpr (ReportStrategy == AllocatorReportStrategy::enabled)
            reportDealloc(ptr);
        deallocateImpl(ptr);
    }

private:
    void reportAlloc(void* p, u64 alignment, u64 n) const {
        log::internal::trace(
          "Allocating '{}' bytes aligned by '{}' at '{}'", n, alignment, p
        );
    }

    void reportDealloc(void* p) const {
        log::internal::trace("Deallocating '{}'", p);
    }

    virtual void* allocateImpl(u64 n, u64 alignment) = 0;
    virtual void deallocateImpl(void* ptr) noexcept  = 0;
};

}  // namespace kstd
