#pragma once

#include <vector>
#include <cstdint>
#include <optional>

#include "kstd/Core.hh"
#include "kstd/containers/DoublyLinkedList.hh"

namespace kstd {

class FreeBlockList {
public:
    static constexpr u64 defaultMinBlockSize = 4u;

    struct Block {
        u64 offset;
        u64 size;
    };

    explicit FreeBlockList(u64 size, u64 minBlockSize = defaultMinBlockSize);

    void releaseBlock(const Block& b);
    std::optional<Block> acquireBlock(u64 size);
    u64 spaceLeft();

    void clear();

private:
    void defragment();

    u64 m_spaceLeft;
    u64 m_size;
    u64 m_minBlockSize;
    DoublyLinkedList<Block> m_blocks;
};

}  // namespace kstd
