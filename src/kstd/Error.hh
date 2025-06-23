
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

struct CancelledError : Error {
    using Error::Error;
};

struct AlreadyExistsError : Error {
    using Error::Error;
};

struct InvalidArgumentError : Error {
    using Error::Error;
};

}  // namespace kstd
