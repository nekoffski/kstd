#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "kstd/Core.hh"
#include "kstd/memory/Allocator.hh"
#include "kstd/memory/Mallocator.hh"

struct Base {
    inline static bool called = false;

    static void reset() { called = false; }
    virtual void foo() { called = true; }
};

struct Derived : Base {
    inline static bool called = false;
    static void reset() { called = false; }

    virtual void foo() override { called = true; }
};

class AllocatorMock : public kstd::Allocator {
public:
    AllocatorMock(bool fallback = true) {
        if (fallback) {
            ON_CALL(*this, allocateRaw)
              .WillByDefault(
                ::testing::Invoke(&mallocator, &kstd::Mallocator<>::allocateRaw)
              );

            ON_CALL(*this, deallocate)
              .WillByDefault(
                ::testing::Invoke(&mallocator, &kstd::Mallocator<>::deallocate)
              );
        }
    }

    MOCK_METHOD(void*, allocateRaw, (kstd::u64 n, kstd::u64 alignment));
    MOCK_METHOD(void, deallocate, (void* ptr), (noexcept));

private:
    kstd::Mallocator<> mallocator;
};

struct Foo {
    kstd::u8 x;
    kstd::u32 y;
    kstd::u64 z;
};

template <typename T = Foo> struct LifetimeProbe {
    inline static kstd::u32 ctorCalls     = 0u;
    inline static kstd::u32 dctorCalls    = 0u;
    inline static kstd::u32 copyCtorCalls = 0u;
    inline static kstd::u32 moveCtorCalls = 0u;

    static void reset() {
        ctorCalls     = 0u;
        dctorCalls    = 0u;
        copyCtorCalls = 0u;
        moveCtorCalls = 0u;
    }

    template <typename... Args>
    LifetimeProbe(Args&&... args) : value(std::forward<Args>(args)...) {
        ++ctorCalls;
    }

    ~LifetimeProbe() { ++dctorCalls; }
    LifetimeProbe(const LifetimeProbe&) { ++copyCtorCalls; }
    LifetimeProbe(LifetimeProbe&&) { ++moveCtorCalls; }

    T value;
};
