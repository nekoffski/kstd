
#pragma once

#include "Log.hh"

namespace kstd {

class Error : public std::runtime_error {
public:
    template <typename... Args>
    explicit Error(kstd::log::details::FormatWithLocation format, Args&&... args) :
        runtime_error(
          fmt::format(fmt::runtime(format.fmt), std::forward<Args>(args)...)
        ),
        m_source(format.loc) {}

    std::string where() const;

private:
    spdlog::source_loc m_source;
};

}  // namespace kstd
