#pragma once

#include <vector>
#include <cstdint>
#include <optional>

#include "kstd/Core.hh"

namespace kstd {

// TODO: requires some refactoring, this is a copy-paste from Starlight
class FreeBlockList {
    struct Node {
        explicit Node();

        bool free;
        u64 offset;
        u64 size;
        Node* next;
    };

public:
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
    Node* getFreeNode();

    u64 m_totalSize;
    u64 m_maxEntries;

    std::vector<Node> m_nodes;
    Node* m_head;
};

}  // namespace kstd
