#pragma once

#include <functional>

namespace kstd {

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
