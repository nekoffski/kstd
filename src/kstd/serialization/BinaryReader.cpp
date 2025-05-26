#include "BinaryReader.hh"

#include "kstd/Error.hh"

namespace kstd {

u8* BinaryReader::readImpl(u8 bytes) {
    if (m_buffer.size() < m_index + bytes) {
        throw Error{
            "BinaryReader buffer out of scope: {} < {}",
            m_buffer.size(),
            m_index + bytes,
        };
    }

    ON_SCOPE_EXIT { m_index += bytes; };
    return m_buffer.data() + m_index;
}

}  // namespace kstd
