#include "Config.hh"

namespace kstd {

ConfigStorage::ConfigStorage(const Config& config) : m_config(config) {}

Config::Buffer& Config::buffer() { return m_buffer; }

}  // namespace kstd
