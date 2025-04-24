#pragma once

#include <typeindex>
#include <string>

#include <boost/core/demangle.hpp>

namespace kstd {

struct RTTI {
    virtual std::type_index getType() const = 0;

    template <typename T> bool is() const { return getType() == typeid(T); }
    template <typename T> T* as() { return static_cast<T*>(this); }
};

template <typename T> std::string getTypeName() {
    return boost::core::demangle(typeid(T).name());
}

}  // namespace kstd
