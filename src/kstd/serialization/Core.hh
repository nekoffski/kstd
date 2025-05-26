#pragma once

#include <vector>
#include <span>

#include "kstd/Core.hh"

namespace kstd {

using BinaryBuffer     = std::vector<u8>;
using BinaryBufferView = std::span<const u8>;

}  // namespace kstd
