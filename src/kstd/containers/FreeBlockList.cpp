#include "FreeBlockList.hh"

#include "kstd/Log.hh"

namespace kstd {

FreeBlockList::Node::Node() {
    free = true;
    next = nullptr;
}

FreeBlockList::FreeBlockList(u64 size) :
    m_totalSize(size), m_maxEntries(size / sizeof(void*)), m_nodes(m_maxEntries),
    m_head(&m_nodes[0]) {
    m_head->offset = 0;
    m_head->size   = m_totalSize;
    m_head->next   = nullptr;
    m_head->free   = false;

    log::trace(
      "Creating free list with {}b capacity, maxEntries={}", m_totalSize,
      m_maxEntries
    );
}

void FreeBlockList::releaseBlock(const Block& b) {
    log::expect(
      b.size > 0, "Could not free block with invalid size ({})", b.offset, b.size
    );

    Node* previous = nullptr;
    Node* node     = m_head;

    if (m_head == nullptr) {
        Node* newNode   = getFreeNode();
        newNode->free   = false;
        newNode->offset = b.offset;
        newNode->size   = b.size;
        newNode->next   = nullptr;
        m_head          = newNode;
        return;
    }

    log::info(
      "release {} - {} - {} - {} - {}", b.offset, b.size, m_head->free,
      (void*)m_head, (void*)m_head->next
    );

    while (node != nullptr) {
        if (node->offset == b.offset) {
            node->size += b.size;

            if (node->next && node->next->offset == node->offset + node->size) {
                node->size += node->next->size;
                auto next  = node->next;
                node->next = next->next;
                next->free = true;
                next->next = nullptr;
            }
            return;
        } else if (node->offset > b.offset) {
            log::info("Found record");
            Node* newNode = getFreeNode();

            newNode->free   = false;
            newNode->offset = b.offset;
            newNode->size   = b.size;
            newNode->next   = node;

            if (previous) {
                previous->next = newNode;
            } else {
                m_head = newNode;
            }

            if (newNode->next
                && newNode->offset + newNode->size == newNode->next->offset) {
                newNode->size += newNode->next->size;
                auto rubbish  = newNode->next;
                newNode->next = rubbish->next;
                rubbish->free = true;
                rubbish->next = nullptr;
            }
            if (previous && previous->offset + previous->size == newNode->offset) {
                previous->size += newNode->size;
                previous->next = newNode->next;
                newNode->free  = true;
                newNode->next  = nullptr;
            }
            return;
        }
        previous = node;
        node     = node->next;
    }
    log::warn(
      "Unable to find block to free, that's unexpected: offset={}, size={}",
      b.offset, b.size
    );
}

std::optional<FreeBlockList::Block> FreeBlockList::acquireBlock(u64 size) {
    log::expect(size > 0, "Could not allocate block with size less or equal 0");

    Node* previous = nullptr;
    Node* node     = m_head;

    while (node != nullptr) {
        if (node->size == size) {
            u64 offset = node->offset;

            if (previous) {
                previous->next = node->next;
                node->free     = true;
                node->next     = nullptr;
            } else {
                m_head->free = true;
                m_head->next = nullptr;
                m_head       = node->next;
            }

            return Block{ offset, size };
        } else if (node->size > size) {
            u64 offset = node->offset;

            node->size -= size;
            node->offset += size;

            return Block{ offset, size };
        }
        previous = node;
        node     = node->next;
    }
    log::warn(
      "Could not find block with enough memory {} bytes requested, total space left: {}",
      size, spaceLeft()
    );
    return {};
}

u64 FreeBlockList::spaceLeft() {
    u64 totalSpace = 0u;
    for (Node* node = m_head; node != nullptr; node = node->next)
        totalSpace += node->size;
    return totalSpace;
}

void FreeBlockList::clear() {
    for (u64 i = 0; i < m_maxEntries; ++i) m_nodes[i].free = true;

    m_head->offset = 0;
    m_head->size   = m_totalSize;
    m_head->next   = nullptr;
}

FreeBlockList::Node* FreeBlockList::getFreeNode() {
    for (u64 i = 0; i < m_maxEntries; ++i) {
        if (m_nodes[i].free) {
            m_nodes[i].next = nullptr;
            return &m_nodes[i];
        }
    }
    return nullptr;
}

}  // namespace kstd
