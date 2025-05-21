#pragma once

#include <cstring>

#include <utility>
#include <type_traits>

#include "kstd/Concepts.hh"
#include "kstd/Log.hh"

namespace kstd {

template <typename T> class LocalPtr : public NonCopyable {
public:
    LocalPtr() : m_pointer(nullptr) { resetBuffer(); }
    ~LocalPtr() { clear(); }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit LocalPtr(Args&&... args) : m_pointer(nullptr) {
        emplace(std::forward<Args>(args)...);
    }

    LocalPtr(LocalPtr&& rhs) : m_pointer(nullptr) {
        resetBuffer();

        if (rhs.m_pointer) {
            m_pointer = new (m_buffer) T(std::move(*rhs.m_pointer));
            rhs.clear();
        }
    }

    LocalPtr& operator=(LocalPtr&& rhs) {
        clear();
        resetBuffer();
        m_pointer = nullptr;

        if (rhs.m_pointer) {
            m_pointer = new (m_buffer) T(std::move(*rhs.m_pointer));
            rhs.clear();
        }

        return *this;
    }

    template <typename... Args>
    requires std::constructible_from<T, Args...>
    T* emplace(Args&&... args) {
        clear();
        m_pointer = new ((T*)&m_buffer) T(std::forward<Args>(args)...);
        return m_pointer;
    }

    T* get() { return m_pointer; }
    const T* get() const { return m_pointer; }

    T& operator*() { return *m_pointer; }
    const T& operator*() const { return *m_pointer; }
    T& value() { return *m_pointer; };
    const T& value() const { return *m_pointer; };

    T* operator->() { return m_pointer; }
    const T* operator->() const { return m_pointer; }

    bool empty() const { return m_pointer == nullptr; }
    operator bool() const { return not empty(); }

    void clear() {
        if (m_pointer) {
            m_pointer->~T();
            m_pointer = nullptr;
            resetBuffer();
        }
    }

private:
    void resetBuffer() { std::memset(m_buffer, 0u, sizeof(T)); }

    alignas(T) std::byte m_buffer[sizeof(T)];
    T* m_pointer;
};

}  // namespace kstd
