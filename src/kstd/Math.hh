#pragma once

#include <limits>

namespace kstd {

template <typename T> T max() { return std::numeric_limits<T>::max(); }
template <typename T> T min() { return std::numeric_limits<T>::min(); }

}  // namespace kstd
