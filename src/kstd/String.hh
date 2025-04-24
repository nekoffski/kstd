#pragma once

#include <optional>
#include <string>
#include <vector>

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

std::vector<std::string> split(const std::string& input, char separator);

}  // namespace kstd
