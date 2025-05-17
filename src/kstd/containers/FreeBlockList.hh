#pragma once

#include <vector>
#include <cstdint>
#include <optional>

#include "kstd/Core.hh"
#include "kstd/containers/DoublyLinkedList.hh"

namespace kstd {

class FreeBlockList {
public:
    static constexpr u64 minBlockSize = 64u;

    struct Block {
        u64 offset;
        u64 size;
    };

    explicit FreeBlockList(u64 size);

    void releaseBlock(const Block& b);
    std::optional<Block> acquireBlock(u64 size);
    u64 spaceLeft();

    void clear();

private:
    void defragment();

    u64 m_spaceLeft;
    u64 m_size;
    DoublyLinkedList<Block> m_blocks;
};

}  // namespace kstd
