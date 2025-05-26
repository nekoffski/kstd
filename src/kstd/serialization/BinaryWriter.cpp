#include "BinaryWriter.hh"

#include <cstring>

namespace kstd {

BinaryBufferView BinaryWriter::getBuffer() const { return m_buffer; }

}  // namespace kstd
