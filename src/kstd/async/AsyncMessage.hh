#pragma once

#include <typeindex>
#include <concepts>

#include "kstd/memory/UniquePtr.hh"

namespace kstd {

class AsyncMessage {
public:
    template <typename T> bool is() const { return getType() == typeid(T); }

    template <typename T> const T* as() const {
        return is<T>() ? static_cast<const T*>(getPointer()) : nullptr;
    }

private:
    virtual std::type_index getType() const = 0;
    virtual const void* getPointer() const  = 0;
};

namespace details {

template <typename T> class TypedAsyncMessage : public AsyncMessage {
public:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit TypedAsyncMessage(Args&&... args
    ) : m_message(std::forward<Args>(args)...) {}

private:
    std::type_index getType() const override { return typeid(T); }

    const void* getPointer() const override {
        return static_cast<const void*>(&m_message);
    }

    T m_message;
};

}  // namespace details
}  // namespace kstd
