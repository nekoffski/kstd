#pragma once

#include <concepts>
#include <string>

#include "Core.hh"
#include "kstd/Scope.hh"

namespace kstd {

class BinaryReader {
public:
    explicit BinaryReader(BinaryBufferView view
    ) : m_index(0u), m_buffer(view.begin(), view.end()) {}

    template <typename T>
    requires std::is_arithmetic_v<T>
    T read() {
        return *reinterpret_cast<T*>(readImpl(sizeof(T)));
    }

    template <typename T>
    requires std::is_same_v<T, std::string>
    T read() {
        const auto size = read<u8>();
        return std::string{ reinterpret_cast<char*>(readImpl(size)), size };
    }

    template <typename T>
    requires(
      std::is_default_constructible_v<T>
      && requires(BinaryReader& bw, T& t) { deserialize(bw, t); }
    )
    T read() {
        T out;
        deserialize(*this, out);
        return out;
    }

    template <typename T>
    requires requires(BinaryReader& br) {
        { br.read<T>() } -> std::same_as<T>;
    }
    BinaryReader& read(T& out) {
        out = read<T>();
        return *this;
    }

private:
    u8* readImpl(u8 bytes);

    u64 m_index;
    BinaryBuffer m_buffer;
};

}  // namespace kstd
