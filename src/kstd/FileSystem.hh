#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Singleton.hh"
#include "serialization/Core.hh"

namespace kstd {

using Path = std::string;

bool isFile(const Path& path);
bool isDirectory(const Path& path);
std::vector<Path> listDirectory(const Path& path);

void writeBinaryFile(const Path& path, BinaryBufferView buffer);
void writeFile(const Path& path, const std::string& buffer);

void appendBinaryFile(const Path& path, BinaryBufferView buffer);
void appendFile(const Path& path, const std::string& buffer);

BinaryBuffer readBinaryFile(const Path& path);
std::string readFile(const Path& path);
std::vector<std::string> readLines(const Path& path);

std::filesystem::file_time_type getLastFileModificationTime(const Path& path);

}  // namespace kstd