#pragma once

#include <concepts>
#include <utility>

namespace kstd {

struct NonCopyable {
    NonCopyable()                              = default;
    NonCopyable(NonCopyable const&)            = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
    NonCopyable(NonCopyable&&)                 = default;
    NonCopyable& operator=(NonCopyable&&)      = default;
};

struct NonMovable {
    NonMovable()                        = default;
    NonMovable(NonMovable&&)            = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

template <typename C, typename R = void, typename... Args>
concept Callable = requires(C&& callback, Args&&... args) {
    { callback(std::forward<Args>(args)...) } -> std::same_as<R>;
};

template <typename T, typename... Ts>
concept OneOf = requires() {
    { (std::is_same_v<T, Ts> || ...) };
};

template <typename T>
concept Iterable = requires(T& t) {
    { t.begin() };
    { t.end() };
    { t.cbegin() };
    { t.cend() };
};

template <typename T>
concept Countable = requires(T& t) {
    { t.size() };
};

}  // namespace kstd
