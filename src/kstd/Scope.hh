#pragma once

#include <exception>

#include "Macros.hh"

namespace kstd::details {

template <typename Callback, typename Condition> class ScopeGuard {
public:
    explicit ScopeGuard(Callback&& callback, Condition&& condition) :
        m_callback(std::forward<Callback>(callback)),
        m_condition(std::forward<Condition>(condition)) {}

    ~ScopeGuard() {
        if (m_condition()) m_callback();
    }

private:
    Callback m_callback;
    Condition m_condition;
};

enum class _ScopeGuardOnExit {};

struct _ScopeGuardOnExitCondition {
    bool operator()() const { return true; }
};

template <typename Callback>
ScopeGuard<Callback, decltype(_ScopeGuardOnExitCondition{})> operator+(
  _ScopeGuardOnExit, Callback&& callback
) {
    return ScopeGuard<Callback, decltype(_ScopeGuardOnExitCondition{})>(
      std::forward<Callback>(callback), _ScopeGuardOnExitCondition{}
    );
}

enum class _ScopeGuardOnFailure {};

struct _ScopeGuardOnFailureCondition {
    bool operator()() const {
        return exceptionsInFlight != std::uncaught_exceptions();
    }

    const int exceptionsInFlight = std::uncaught_exceptions();
};

template <typename Callback>
ScopeGuard<Callback, decltype(_ScopeGuardOnFailureCondition{})> operator+(
  _ScopeGuardOnFailure, Callback&& callback
) {
    return ScopeGuard<Callback, decltype(_ScopeGuardOnFailureCondition{})>(
      std::forward<Callback>(callback), _ScopeGuardOnFailureCondition{}
    );
}

enum class _ScopeGuardOnSuccess {};

struct _ScopeGuardOnSuccessCondition {
    bool operator()() const {
        return exceptionsInFlight == std::uncaught_exceptions();
    }

    const int exceptionsInFlight = std::uncaught_exceptions();
};

template <typename Callback>
ScopeGuard<Callback, decltype(_ScopeGuardOnSuccessCondition{})> operator+(
  _ScopeGuardOnSuccess, Callback&& callback
) {
    return ScopeGuard<Callback, decltype(_ScopeGuardOnSuccessCondition{})>(
      std::forward<Callback>(callback), _ScopeGuardOnSuccessCondition{}
    );
}

}  // namespace kstd::details

// clang-format off
#define ON_SCOPE_EXIT    auto ANONYMOUS_VAR(SCOPE_EXIT)    = kstd::details::_ScopeGuardOnExit      {} + [&]()
#define ON_SCOPE_FAIL    auto ANONYMOUS_VAR(SCOPE_FAIL)    = kstd::details::_ScopeGuardOnFailure   {} + [&]()
#define ON_SCOPE_SUCCESS auto ANONYMOUS_VAR(SCOPE_SUCCESS) = kstd::details::_ScopeGuardOnSuccess   {} + [&]()
// clang-format on
