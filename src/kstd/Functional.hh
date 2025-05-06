#pragma once

#include <ranges>
#include <functional>

namespace kstd {

namespace details {

template <typename T> struct ToImpl {};

template <typename T, std::ranges::range R>
requires std::convertible_to<std::ranges::range_value_t<R>, T>
std::vector<T> operator|(R&& r, ToImpl<T>) {
    std::vector<T> out;

    if constexpr (std::ranges::sized_range<decltype(r)>)
        out.reserve(std::ranges::size(r));

    std::ranges::copy(r, std::back_inserter(out));
    return out;
}

}  // namespace details

template <typename T> auto toVector() { return details::ToImpl<T>{}; }

template <typename... Ts> struct Overloader : Ts... {
    using Ts::operator()...;
};

class GuardCall {
public:
    GuardCall() : m_callback([]() {}) {}

    template <typename Callback>
    GuardCall(Callback&& callback) : m_callback(std::forward<Callback>(callback)) {}

    ~GuardCall() { m_callback(); }

private:
    std::function<void()> m_callback;
};

}  // namespace kstd
