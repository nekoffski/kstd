#pragma once

#include <optional>
#include <string>
#include <vector>
#include <algorithm>

#include "Core.hh"

namespace kstd {

template <u64 N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }
    char value[N];
};

enum class NameExtractionMode {
    withExtension,
    withoutFullExtension,
    withoutLastExtensionChunk
};
enum class ExtensionExtractionMode { full, lastChunk };

std::string nameFromPath(
  const std::string& path,
  NameExtractionMode mode = NameExtractionMode::withoutFullExtension
);

std::optional<std::string> extensionFromPath(
  const std::string& path,
  ExtensionExtractionMode mode = ExtensionExtractionMode::full
);

std::vector<std::string> split(
  const std::string& input, const std::string& separator
);

}  // namespace kstd
