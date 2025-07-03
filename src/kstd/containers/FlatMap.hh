#pragma once

#include <vector>

#include "kstd/Concepts.hh"
#include "kstd/Algorithms.hh"
#include "SlotBuffer.hh"

namespace kstd {

namespace details {

template <typename K, typename V, template <typename> class Buffer> class FlatMap {
    struct Record {
        template <typename... Args>
        requires std::constructible_from<V, Args...>
        explicit Record(const K& k, Args&&... v) :
            key(k), value(std::forward<Args>(v)...) {}

        explicit Record(const K& k, V&& v) : key(k), value(std::move(v)) {}

        K key;
        V value;
    };

public:
    explicit FlatMap() = default;

    FlatMap(FlatMap&& oth)            = default;
    FlatMap& operator=(FlatMap&& oth) = default;

    FlatMap(const FlatMap& oth)            = default;
    FlatMap& operator=(const FlatMap& oth) = default;

    auto begin() { return m_buffer.begin(); }
    auto end() { return m_buffer.end(); }

    auto cbegin() { return m_buffer.cbegin(); }
    auto cend() { return m_buffer.cend(); }

    auto begin() const { return m_buffer.begin(); }
    auto end() const { return m_buffer.end(); }

    bool has(const K& k) const { return contains(k); }
    bool contains(const K& k) const { return get(k) != nullptr; }

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

    V& insert(const K& k, const V& v) {
        if (get(k) != nullptr) throw AlreadyExistsError{};
        auto copy = v;
        return insert(k, std::move(copy));
    }

    V& insert(const K& k, V&& v) {
        if (get(k) != nullptr) throw AlreadyExistsError{};
        return m_buffer.emplace(k, std::move(v)).value;
    }

    template <typename... Args>
    requires std::constructible_from<V, Args...>
    V& emplace(const K& k, Args&&... args) {
        return m_buffer.emplace(k, std::forward<Args>(args)...).value;
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
        return insert(k, std::move(v));
    }

    bool erase(const K& k) {
        return m_buffer.eraseIf([&](auto& r) { return r.key == k; }) != 0u;
    }

    template <typename Callback>
    requires Callable<Callback, bool, const K&, const V&>
    bool eraseIf(Callback&& c) {
        return m_buffer.eraseIf([&](auto& r) { return c(r.key, r.value); }) != 0u;
    }

    void clear() { m_buffer.clear(); }

    u64 size() const { return m_buffer.size(); }
    bool empty() const { return size() == 0u; }

    std::vector<const K*> getKeys() const {
        return transform(m_buffer, [](const auto& record) -> const K* {
            return &record.key;
        });
    }

    std::vector<V> getValues() {
        return transform(m_buffer, [&](const Record& record) -> V {
            return record.value;
        });
    }

    template <typename Pred>
    auto getValues(Pred&& pred) -> std::vector<std::result_of_t<Pred(V&)>> {
        return transform(m_buffer, [&](const Record& record) {
            return pred(record.value);
        });
    }

    template <typename Pred> std::vector<V> filterValues(Pred&& pred) {
        return filter(getValues(), pred);
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
        T& insert(T&& v) {
            this->push_back(std::move(v));
            return this->back();
        }

        template <typename... Args>
        requires std::constructible_from<T, Args...>
        T& emplace(Args&&... args) {
            this->emplace_back(std::forward<Args>(args)...);
            return this->back();
        }

        template <typename Callback>
        requires Callable<Callback, bool, T&>
        u64 eraseIf(Callback&& c) {
            return std::erase_if(*this, std::forward<Callback>(c));
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
