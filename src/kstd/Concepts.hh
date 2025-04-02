#pragma once

#include <concepts>
#include <utility>

namespace kstd {

struct NonCopyable {
    NonCopyable()                              = default;
    NonCopyable(NonCopyable const&)            = delete;
    NonCopyable& operator=(NonCopyable const&) = delete;
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

}  // namespace kstd
