#pragma once

#include <optional>
#include <string>

namespace kstd {

enum class NameExtractionMode { withExtension, withoutExtension };
enum class ExtensionExtractionMode { full, lastChunk };

std::string nameFromPath(
  const std::string& path,
  NameExtractionMode mode = NameExtractionMode::withoutExtension
);

std::optional<std::string> extensionFromPath(
  const std::string& path,
  ExtensionExtractionMode mode = ExtensionExtractionMode::full
);

}  // namespace kstd
