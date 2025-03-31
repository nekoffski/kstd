#include "Log.hh"

namespace kstd::log {

void init(std::string_view applicationName) {
    const auto pattern = fmt::format(
      "%^[%d-%m-%Y %T] [Th: %t] %-7l [{}]: %v [%s:%#] %$", applicationName
    );
    spdlog::set_pattern(pattern);
    spdlog::set_level(spdlog::level::trace);
}

}  // namespace kstd::log
