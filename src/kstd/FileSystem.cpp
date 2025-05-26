#include "FileSystem.hh"

#include <fstream>
#include <sstream>

#include "String.hh"

namespace kstd {

namespace details {

enum class WritePolicy { override, noOverride };

static void writeBinaryFile(
  const Path& path, BinaryBufferView buffer, WritePolicy writePolicy
) {
    const auto mode =
      std::ios::binary
      | (writePolicy == WritePolicy::override ? std::ios::trunc : std::ios::app);

    std::ofstream fileStream;
    fileStream.open(path, mode);
    fileStream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    fileStream.close();
}

static void writeFile(
  const Path& path, const std::string& buffer, WritePolicy writePolicy
) {
    const auto mode =
      writePolicy == WritePolicy::override ? std::ios::trunc : std::ios::app;

    std::ofstream fileStream;
    fileStream.open(path, mode);
    fileStream << buffer;
    fileStream.close();
}
}  // namespace details

void writeBinaryFile(const Path& path, BinaryBufferView buffer) {
    details::writeBinaryFile(path, buffer, details::WritePolicy::override);
}
void writeFile(const Path& path, const std::string& buffer) {
    details::writeFile(path, buffer, details::WritePolicy::override);
}

void appendBinaryFile(const Path& path, BinaryBufferView buffer) {
    details::writeBinaryFile(path, buffer, details::WritePolicy::noOverride);
}

void appendFile(const Path& path, const std::string& buffer) {
    details::writeFile(path, buffer, details::WritePolicy::noOverride);
}

bool isFile(const Path& path) { return std::filesystem::is_regular_file(path); }
bool isDirectory(const Path& path) { return std::filesystem::is_directory(path); }

std::vector<std::string> listDirectory(const Path& path) {
    std::vector<std::string> entries;
    for (auto& entry : std::filesystem::directory_iterator(path))
        entries.emplace_back(entry.path());
    return entries;
}

BinaryBuffer readBinaryFile(const Path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    BinaryBuffer buffer(size, 0u);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

std::string readFile(const Path& path) {
    std::ostringstream fileContentStream;
    std::ifstream fs;

    fs.open(path);
    fileContentStream << fs.rdbuf();
    fs.close();

    return fileContentStream.str();
}

std::vector<std::string> readLines(const Path& path) {
    static const std::string endOfLine = "\n";
    return split(readFile(path), endOfLine);
}

std::filesystem::file_time_type getLastFileModificationTime(const Path& path) {
    return std::filesystem::last_write_time(path);
}

}  // namespace kstd
