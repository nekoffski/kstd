#pragma once

#include <vector>
#include <array>

#include "kstd/Core.hh"
#include "kstd/memory/LocalPtr.hh"

namespace kstd {

template <typename T, u64 Capacity> struct SlotStackStorage {
    static_assert(Capacity > 0);

    SlotStackStorage([[maybe_unused]] u64 capacity) {}

    SlotStackStorage(SlotStackStorage&& oth)            = default;
    SlotStackStorage& operator=(SlotStackStorage&& oth) = default;

    SlotStackStorage(const SlotStackStorage& oth) {
        for (u64 i = 0; i < buffer.size(); ++i)
            if (oth.buffer[i]) buffer[i].emplace(*oth.buffer[i]);
    }

    SlotStackStorage& operator=(const SlotStackStorage& oth) {
        for (u64 i = 0; i < buffer.size(); ++i)
            if (oth.buffer[i]) buffer[i].emplace(*oth.buffer[i]);
        return *this;
    }

    LocalPtr<T>* begin() { return &(*buffer.begin()); }
    LocalPtr<T>* end() { return &(*buffer.end()); }

    std::array<LocalPtr<T>, Capacity> buffer;
};

template <typename T> struct SlotHeapStorage {
    explicit SlotHeapStorage(u64 capacity) : buffer(capacity) {}

    SlotHeapStorage(SlotHeapStorage&& oth)            = default;
    SlotHeapStorage& operator=(SlotHeapStorage&& oth) = default;

    SlotHeapStorage(const SlotHeapStorage& oth) {
        buffer.resize(oth.buffer.size());
        for (u64 i = 0; i < buffer.size(); ++i)
            if (oth.buffer[i]) buffer[i].emplace(*oth.buffer[i]);
    }

    SlotHeapStorage& operator=(const SlotHeapStorage& oth) {
        buffer.resize(oth.buffer.size());
        for (u64 i = 0; i < buffer.size(); ++i)
            if (oth.buffer[i]) buffer[i].emplace(*oth.buffer[i]);
        return *this;
    }

    LocalPtr<T>* begin() { return &(*buffer.begin()); }
    LocalPtr<T>* end() { return &(*buffer.end()); }

    std::vector<LocalPtr<T>> buffer;
};

}  // namespace kstd
