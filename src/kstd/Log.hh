#pragma once

#include <stdlib.h>

#include <string_view>
#include <source_location>

#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kstd::log {

namespace detail {

struct FormatWithLocation {
    std::string_view fmt;
    spdlog::source_loc loc;

    template <typename String>
    FormatWithLocation(
      const String& s,
      const std::source_location& location = std::source_location::current()
    ) :
        fmt(s),
        loc(
          location.file_name(), static_cast<int>(location.line()),
          location.function_name()
        ) {}
};

template <typename... Args>
[[noreturn]] constexpr inline void abort(
  const FormatWithLocation& formatString, Args&&... args
) {
    fmt::println(fmt::runtime(formatString.fmt), std::forward<Args>(args)...);
    std::abort();
}
}  // namespace detail

void init(std::string_view applicationName);

template <typename... Args>
void debug(detail::FormatWithLocation fmt, Args&&... args) {
    spdlog::default_logger_raw()->log(
      fmt.loc, spdlog::level::debug, fmt::runtime(fmt.fmt),
      std::forward<Args>(args)...
    );
}

template <typename... Args>
void error(detail::FormatWithLocation fmt, Args&&... args) {
    spdlog::default_logger_raw()->log(
      fmt.loc, spdlog::level::err, fmt::runtime(fmt.fmt), std::forward<Args>(args)...
    );
}

template <typename... Args>
void info(detail::FormatWithLocation fmt, Args&&... args) {
    spdlog::default_logger_raw()->log(
      fmt.loc, spdlog::level::info, fmt::runtime(fmt.fmt),
      std::forward<Args>(args)...
    );
}

template <typename... Args>
void trace(detail::FormatWithLocation fmt, Args&&... args) {
    spdlog::default_logger_raw()->log(
      fmt.loc, spdlog::level::trace, fmt::runtime(fmt.fmt),
      std::forward<Args>(args)...
    );
}

template <typename... Args>
void warn(detail::FormatWithLocation fmt, Args&&... args) {
    spdlog::default_logger_raw()->log(
      fmt.loc, spdlog::level::warn, fmt::runtime(fmt.fmt),
      std::forward<Args>(args)...
    );
}

template <typename... Args>
[[noreturn]] void panic(detail::FormatWithLocation fmt, Args&&... args) {
    fmt::println(
      "!! PANIC, unexpected path executed: {}:{} - {}", fmt.loc.filename,
      fmt.loc.line, fmt.loc.funcname
    );
    detail::abort(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void expect(bool condition, detail::FormatWithLocation fmt, Args&&... args) {
    if (not condition) [[unlikely]] {
        fmt::println(
          "!! ASSERTION FAILED: {}:{} - {}", fmt.loc.filename, fmt.loc.line,
          fmt.loc.funcname
        );
        detail::abort(fmt, std::forward<Args>(args)...);
    }
}

namespace internal {

template <typename... Args>
void debug(detail::FormatWithLocation fmt, Args&&... args) {
#ifdef KSTD_ENABLE_INTERNAL_LOGGING
    log::debug(std::move(fmt), std::forward<Args>(args)...);
#endif
}

template <typename... Args>
void error(detail::FormatWithLocation fmt, Args&&... args) {
#ifdef KSTD_ENABLE_INTERNAL_LOGGING
    log::error(std::move(fmt), std::forward<Args>(args)...);
#endif
}

template <typename... Args>
void info(detail::FormatWithLocation fmt, Args&&... args) {
#ifdef KSTD_ENABLE_INTERNAL_LOGGING
    log::info(std::move(fmt), std::forward<Args>(args)...);
#endif
}

template <typename... Args>
void trace(detail::FormatWithLocation fmt, Args&&... args) {
#ifdef KSTD_ENABLE_INTERNAL_LOGGING
    log::trace(std::move(fmt), std::forward<Args>(args)...);
#endif
}

template <typename... Args>
void warn(detail::FormatWithLocation fmt, Args&&... args) {
#ifdef KSTD_ENABLE_INTERNAL_LOGGING
    log::warn(std::move(fmt), std::forward<Args>(args)...);
#endif
}

}  // namespace internal

}  // namespace kstd::log
