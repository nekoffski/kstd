#include "Log.hh"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace kstd::log {

void init(std::string_view applicationName) {
    auto colorSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    colorSink->set_color(spdlog::level::trace, colorSink->green);
    colorSink->set_color(spdlog::level::debug, colorSink->magenta);
    colorSink->set_color(spdlog::level::info, colorSink->white);
    colorSink->set_color(spdlog::level::warn, colorSink->yellow);
    colorSink->set_color(spdlog::level::err, colorSink->red);
    colorSink->set_color(spdlog::level::critical, colorSink->red_bold);
    colorSink->set_color(spdlog::level::off, colorSink->reset);

    auto logger = std::make_shared<spdlog::logger>("global_logger", colorSink);
    spdlog::set_default_logger(logger);

    const auto pattern = fmt::format(
      "%^[%d-%m-%Y %T] [Th: %t] %-7l [{}]: %v [%s:%#] %$", applicationName
    );

    spdlog::set_pattern(pattern);
    spdlog::set_level(spdlog::level::trace);
}

}  // namespace kstd::log
