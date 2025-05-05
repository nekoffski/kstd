#include "Error.hh"

#include <kstd/String.hh>

namespace kstd {

std::string Error::where() const {
    return fmt::format(
      "{}:{}",
      kstd::nameFromPath(m_source.filename, kstd::NameExtractionMode::withExtension),
      m_source.line
    );
}

}  // namespace kstd
