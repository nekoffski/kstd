#pragma once

#include <mutex>
#include <limits>
#include <concepts>
#include <optional>
#include <queue>
#include <type_traits>

#include <fmt/core.h>

#include "Core.hh"
#include "String.hh"
#include "Log.hh"
#include "Concepts.hh"
#include "Error.hh"

namespace kstd {

template <u64 Min, u64 Max> class SequenceGenerator {
    static_assert(Min < Max);
    static_assert(Max < std::numeric_limits<u64>::max());

public:
    SequenceGenerator() : m_current(0u) {}

    u64 get() {
        m_current %= (Max + 1u);
        return m_current++;
    }

private:
    u64 m_current;
};

using Uuid = std::string;

Uuid generateUuid();

class WithUuid : public virtual kstd::NonCopyable {
public:
    explicit WithUuid();

    WithUuid(WithUuid&& oth)            = default;
    WithUuid& operator=(WithUuid&& oth) = default;

    const Uuid& getUuid() const;

private:
    Uuid m_uuid;
};

template <typename T> class Identifiable : public virtual kstd::NonCopyable {
public:
    using Id = u64;

    explicit Identifiable() : m_id(createId()), m_shouldFree(true) {}

    Identifiable(Identifiable&& oth) noexcept : m_id(oth.m_id) {
        oth.m_shouldFree = false;
    }

    Identifiable& operator=(Identifiable&& oth) noexcept {
        free();
        m_id             = oth.m_id;
        oth.m_shouldFree = false;
        return *this;
    }

    ~Identifiable() { free(); }

    Id id() const { return m_id; }

private:
    void free() {
        if (m_shouldFree) {
            std::scoped_lock guard{ s_mutex };
            s_freeIds.push(m_id);
        }
    }

    static Id createId() {
        std::scoped_lock guard{ s_mutex };

        if (not s_freeIds.empty()) {
            const auto id = s_freeIds.front();
            s_freeIds.pop();
            return id;
        }

        return s_generator++;
    }

    Id m_id;
    bool m_shouldFree;

    inline static Id s_generator = 0;
    inline static std::queue<Id> s_freeIds;
    inline static std::mutex s_mutex;
};

template <typename T, StringLiteral NameGenerator, bool Const = true>
class Named : public Identifiable<T> {
    inline const static std::string baseName = NameGenerator.value;

public:
    explicit Named(std::optional<std::string> name = {}) :
        m_name(generateName(name)) {
        log::debug(
          "Creating {} - id={} name='{}'", baseName, Identifiable<T>::id(), m_name
        );
    }

    ~Named() {
        log::debug(
          "Destroying {} - id={} name='{}'", baseName, Identifiable<T>::id(), m_name
        );
    }

    Named(Named&& oth)            = default;
    Named& operator=(Named&& oth) = default;

    const std::string& name() const { return m_name; }

    void setName(const std::string& name)
    requires(!Const)
    {
        if (name.starts_with(baseName)) {
            throw InvalidArgumentError{
                "Name cannot start with '{}' to avoid conflicts", baseName
            };
        }
        m_name = name;
    }

private:
    std::string m_name;

    std::string generateName(std::optional<std::string> name) {
        return name.value_or(fmt::format("{}_{}", baseName, Identifiable<T>::id()));
    }
};

template <typename T>
concept IsIdentifiable = std::derived_from<T, Identifiable<T>>;

template <typename T>
requires std::is_integral_v<T>
class Id {
    static constexpr T invalidId = std::numeric_limits<T>::max();

public:
    using Type = T;

    Id() : m_value(invalidId) {}
    Id(T value) : m_value(value) {}

    T operator*() const { return get(); }
    T get() const { return m_value; }

    Id& operator=(T value) {
        m_value = value;
        return *this;
    }

    void invalidate() { m_value = invalidId; }

    bool hasValue() const { return m_value != invalidId; }

private:
    T m_value;
};

using Id8  = Id<u8>;
using Id16 = Id<u16>;
using Id32 = Id<u32>;
using Id64 = Id<u64>;

}  // namespace kstd
