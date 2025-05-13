#pragma once

#include <typeindex>
#include <concepts>

#include <boost/asio/experimental/channel.hpp>

#include "kstd/memory/UniquePtr.hh"
#include "kstd/memory/SharedPtr.hh"
#include "AsyncPromise.hh"

namespace kstd {

class AsyncResponse {
public:
    template <typename T> bool is() const { return getType() == typeid(T); }

    template <typename T> const T* as() const {
        return is<T>() ? static_cast<const T*>(getPointer()) : nullptr;
    }

    virtual std::type_index getType() const = 0;

private:
    virtual const void* getPointer() const = 0;
};

namespace details {

template <typename T> class TypedAsyncResponse : public AsyncResponse {
public:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit TypedAsyncResponse(Args&&... args
    ) : m_message(std::forward<Args>(args)...) {}

    std::type_index getType() const override { return typeid(T); }

private:
    const void* getPointer() const override {
        return static_cast<const void*>(&m_message);
    }

    T m_message;
};

}  // namespace details

class AsyncMessage : public AsyncResponse {
public:
    using Promise = AsyncPromise<UniquePtr<AsyncResponse>>;

    AsyncMessage(const boost::asio::any_io_executor& ex
    ) : m_promise(makeShared<Promise>(ex)) {}

    SharedPtr<Promise> getPromise() { return m_promise; }

    template <typename T, typename... Args> Coro<void> respond(Args&&... args) {
        co_return (co_await m_promise->set(
          makeUnique<details::TypedAsyncResponse<T>>(std::forward<Args>(args)...)
        ));
    }

private:
    SharedPtr<Promise> m_promise;
};

namespace details {

template <typename T> class TypedAsyncMessage : public AsyncMessage {
public:
    template <typename... Args>
    requires std::constructible_from<T, Args...>
    explicit TypedAsyncMessage(
      const boost::asio::any_io_executor& ex, Args&&... args
    ) : AsyncMessage(ex), m_message(std::forward<Args>(args)...) {}

    std::type_index getType() const override { return typeid(T); }

private:
    const void* getPointer() const override {
        return static_cast<const void*>(&m_message);
    }

    T m_message;
};

}  // namespace details
}  // namespace kstd
