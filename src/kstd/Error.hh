
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

    explicit Error() : Error("No details provided") {}

    std::string where() const;

private:
    spdlog::source_loc m_source;
};

#define KSTD_DECLARE_ERROR(ErrorName) \
    struct ErrorName : Error {        \
        using Error::Error;           \
    };

KSTD_DECLARE_ERROR(CancelledError);
KSTD_DECLARE_ERROR(AlreadyExistsError);
KSTD_DECLARE_ERROR(InvalidArgumentError);
KSTD_DECLARE_ERROR(OutOfSpaceError);
KSTD_DECLARE_ERROR(OutOfRangeError);

}  // namespace kstd
