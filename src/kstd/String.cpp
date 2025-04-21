#include "String.hh"

namespace kstd {

std::string nameFromPath(const std::string& path, NameExtractionMode mode) {
    const auto begin = path.find_last_of('/') + 1;
    const auto end =
      mode == NameExtractionMode::withExtension
        ? path.npos
        : path.find_first_of('.', begin);
    return path.substr(begin, end - begin);
}

std::optional<std::string> extensionFromPath(
  const std::string& path, ExtensionExtractionMode mode
) {
    const auto name = nameFromPath(path, NameExtractionMode::withExtension);
    const auto begin =
      mode == ExtensionExtractionMode::full
        ? name.find_first_of('.')
        : name.find_last_of('.');

    if (begin != path.npos) return name.substr(begin);
    return {};
}

}  // namespace kstd
