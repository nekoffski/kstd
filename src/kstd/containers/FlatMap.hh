#pragma once

#include <vector>

#include "kstd/Concepts.hh"
#include "kstd/Algorithms.hh"
#include "SlotBuffer.hh"

namespace kstd {

namespace details {

template <typename K, typename V, template <typename> class Buffer>
class FlatMap : public NonCopyable {
    struct Record {
        K key;
        V value;
    };

    struct IdentityTransformer {
        V* operator()(V& v) { return &v; }
    };

public:
    explicit FlatMap() = default;

    FlatMap(FlatMap&& oth)            = default;
    FlatMap& operator=(FlatMap&& oth) = default;

    V* get(const K& k) {
        auto it = std::find_if(m_buffer.begin(), m_buffer.end(), [&](auto& r) {
            return r.key == k;
        });
        return it == m_buffer.end() ? nullptr : &it->value;
    }

    const V* get(const K& k) const {
        const auto it =
          std::find_if(m_buffer.cbegin(), m_buffer.cend(), [&](const auto& r) {
              return r.key == k;
          });
        return it == m_buffer.end() ? nullptr : &it->value;
    }

    V* insert(const K& k, const V& v) {
        if (get(k) != nullptr) return nullptr;
        auto copy = v;
        return insert(k, std::move(copy));
    }

    V* insert(const K& k, V&& v) {
        if (get(k) != nullptr) return nullptr;
        return &m_buffer.insert({ k, std::move(v) })->value;
    }

    template <typename... Args>
    requires std::constructible_from<V, Args...>
    V* emplace(const K& k, Args&&... args) {
        return insert(k, V{ std::forward<Args>(args)... });
    }

    V& put(const K& k, const V& v) {
        auto copy = v;
        return put(k, std::move(copy));
    }

    V& put(const K& k, V&& v) {
        if (auto record = get(k); record != nullptr) {
            *record = std::move(v);
            return *record;
        }
        return *insert(k, std::move(v));
    }

    void erase(const K& k) {
        m_buffer.eraseIf([&](auto& r) { return r.key == k; });
    }

    void clear() { m_buffer.clear(); }

    u64 size() const { return m_buffer.size(); }
    bool empty() const { return size() == 0u; }

    std::vector<const K*> getKeys() const {
        return transform(m_buffer, [](const auto& record) -> const K* {
            return &record.key;
        });
    }

    template <typename Transformation = IdentityTransformer>
    auto getValues(Transformation&& t = IdentityTransformer{})
      -> std::vector<std::result_of_t<Transformation(V&)>> {
        return transform(m_buffer, [&](Record& record) { return t(record.value); });
    }

    template <typename Callback>
    requires Callable<Callback, void, V&>
    void forEach(Callback&& c) {
        for (auto& r : m_buffer) c(r.value);
    }

    template <typename Callback>
    requires Callable<Callback, void, const K&, V&>
    void forEach(Callback&& c) {
        for (auto& r : m_buffer) c(r.key, r.value);
    }

    template <typename Callback>
    requires Callable<Callback, void, const V&>
    void forEach(Callback&& c) const {
        for (const auto& r : m_buffer) c(r.value);
    }

    template <typename Callback>
    requires Callable<Callback, void, const K&, const V&>
    void forEach(Callback&& c) const {
        for (const auto& r : m_buffer) c(r.key, r.value);
    }

private:
    Buffer<Record>::Type m_buffer;
};

template <typename T> struct VectorAdapter {
    struct Type : public std::vector<T> {
        T* insert(T&& v) {
            this->push_back(std::move(v));
            return &this->back();
        }

        template <typename Callback>
        requires Callable<Callback, bool, T&>
        void eraseIf(Callback&& c) {
            std::erase_if(*this, std::forward<Callback>(c));
        }
    };
};

template <u64 Size> struct SlotBufferAdapter {
    template <typename T> struct Wrapper {
        using Type = StackSlotBuffer<T, Size>;
    };
};

}  // namespace details

template <typename K, typename V>
struct DynamicFlatMap : details::FlatMap<K, V, details::VectorAdapter> {};

template <typename K, typename V, u64 Size>
struct StaticFlatMap
    : details::FlatMap<K, V, details::SlotBufferAdapter<Size>::template Wrapper> {};

}  // namespace kstd
