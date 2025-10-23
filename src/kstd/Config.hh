#pragma once

#include <unordered_map>

#include "Core.hh"
#include "Error.hh"
#include "Singleton.hh"

namespace kstd {

class Config {
    using Buffer = std::unordered_map<std::string, std::string>;

public:
    void set(const std::string& key, const char* value) { m_buffer[key] = value; }

    void set(const std::string& key, const std::string& value) {
        m_buffer[key] = value;
    }

    template <typename T> void set(const std::string& key, const T& value) {
        m_buffer[key] = std::to_string(value);
    }

    Buffer& buffer();

private:
    Buffer m_buffer;
};

class ConfigStorage : public Singleton<ConfigStorage> {
public:
    explicit ConfigStorage(const Config& config);

    template <typename T> void set(const std::string& label, const T& value) {
        m_config.set(label, value);
    }

    template <typename T = std::string> T read(const std::string& key) {
        auto& buffer  = m_config.buffer();
        const auto it = buffer.find(key);
        if (it == buffer.end()) throw Error{ "Key '{}' not found", key };

        auto& value = it->second;

        try {
            if constexpr (std::is_same_v<T, std::string>) {
                return std::string{ value };
            } else if constexpr (std::is_same_v<T, u64> || std::is_same_v<T, u32>
                                 || std::is_same_v<T, u16>
                                 || std::is_same_v<T, u8>) {
                return std::stoul(value);
            } else if constexpr (std::is_same_v<T, i64>) {
                return std::stol(value);
            } else if constexpr (std::is_same_v<T, i32>) {
                return std::stoi(value);
            } else if constexpr (std::is_same_v<T, f32>) {
                return std::stof(value);
            } else if constexpr (std::is_same_v<T, f64>) {
                return std::stod(value);
            }
        } catch (const std::logic_error& e) {
            throw Error{ "Could not parse value of: '{}' - {}", key, e.what() };
        }
        throw Error{ "Unknown type" };
    }

private:
    Config m_config;
};

template <typename T = std::string> T readConf(const std::string& key) {
    expectCreated<ConfigStorage>();
    return ConfigStorage::get().read<T>(key);
}

template <typename T> void setConf(const std::string& key, const T& value) {
    expectCreated<ConfigStorage>();
    return ConfigStorage::get().set(key, value);
}

}  // namespace kstd
