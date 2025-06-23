#include "FreeBlockList.hh"

#include "kstd/Log.hh"
#include "kstd/Scope.hh"

namespace kstd {

FreeBlockList::FreeBlockList(u64 size, u64 minBlockSize) :
    m_spaceLeft(size), m_size(size), m_minBlockSize(minBlockSize) {
    log::expect(
      size % m_minBlockSize == 0, "Size must be a multiple of minBlockSize={}",
      m_minBlockSize
    );
    m_blocks.pushBack(Block{ .offset = 0u, .size = size });
}

void FreeBlockList::releaseBlock(const Block& b) {
    log::expect(
      b.size > 0, "Could not free block with invalid size ({})", b.offset, b.size
    );

    if (m_blocks.empty()) {
        m_blocks.pushBack(b);
        m_spaceLeft += b.size;
        return;
    }

    for (auto it = m_blocks.head(); it != nullptr; it = it->next) {
        if (auto& block = it->value; block.offset > b.offset) {
            m_spaceLeft += b.size;
            m_blocks.pushBefore(it, b);
            defragment();
            return;
        }
    }
    log::warn("Could not find block to free: offset={}, size={}", b.offset, b.size);
}

std::optional<FreeBlockList::Block> FreeBlockList::acquireBlock(u64 size) {
    log::expect(
      size > 0 && size % m_minBlockSize == 0, "Invalid block size: {}", size
    );

    if (size > m_spaceLeft) return {};

    for (auto it = m_blocks.head(); it != nullptr; it = it->next) {
        auto& b = it->value;

        if (b.size >= size) {
            Block block{ .offset = b.offset, .size = size };
            m_spaceLeft -= size;

            if (b.size == size) {
                m_blocks.erase(it);
            } else {
                b.size -= size;
                b.offset += size;
            }
            return block;
        }
    }
    return {};
}

u64 FreeBlockList::spaceLeft() { return m_spaceLeft; }

void FreeBlockList::clear() {
    m_blocks.clear();
    m_spaceLeft = m_size;
}

void FreeBlockList::defragment() {
    static auto endOffset = [](const Block& b) { return b.offset + b.size; };

    for (auto it = m_blocks.head(); it->next != nullptr;) {
        if (endOffset(it->value) == it->next->value.offset) {
            it->value.size += it->next->value.size;
            m_blocks.erase(it->next);
        } else {
            it = it->next;
        }
    }
}

}  // namespace kstd
