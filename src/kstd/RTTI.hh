#pragma once

#include <typeindex>

namespace kstd {

struct RTTI {
    virtual std::type_index getType() const = 0;

    template <typename T> bool is() const { return getType() == typeid(T); }
    template <typename T> T* as() { return static_cast<T*>(this); }
};

}  // namespace kstd
