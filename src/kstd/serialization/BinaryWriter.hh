#pragma once

#include <concepts>
#include <string>
#include <cstring>

#include "Core.hh"
#include "kstd/Log.hh"

namespace kstd {

class BinaryWriter {
public:
    BinaryBufferView getBuffer() const;

    template <typename T>
    requires std::is_arithmetic_v<T>
    BinaryWriter& write(T v) {
        write(&v, sizeof(T));
        return *this;
    }

    template <typename T>
    requires requires(BinaryWriter& bw, const T& t) { serialize(bw, t); }
    BinaryWriter& write(const T& v) {
        serialize(*this, v);
        return *this;
    }

    BinaryWriter& write(const std::string& v) {
        auto size = v.size();
        log::expect(
          size <= std::numeric_limits<u8>::max(), "String too long to serialize"
        );

        write(&size, 1);
        write(v.data(), size);

        return *this;
    }

private:
    template <typename T> void write(const T* ptr, u8 bytes) {
        const auto bufferSize = m_buffer.size();
        m_buffer.resize(bufferSize + bytes);
        std::memcpy(m_buffer.data() + bufferSize, ptr, bytes);
    }

    BinaryBuffer m_buffer;
};

}  // namespace kstd
