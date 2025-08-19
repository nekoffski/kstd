#pragma once

#include <boost/asio/experimental/channel.hpp>

#include "kstd/Concepts.hh"
#include "Core.hh"

namespace kstd {

template <typename T>
class AsyncPromise : public virtual NonCopyable, public virtual NonMovable {
    using Channel =
      boost::asio::experimental::channel<void(boost::system::error_code, T)>;

public:
    template <typename Executor>
    explicit AsyncPromise(const Executor& ex) : m_oneShotChannel(ex, 1u) {}

    Coro<T> wait() {
        co_return (co_await m_oneShotChannel.async_receive(boost::asio::use_awaitable
        ));
    }

    Coro<void> set(T&& value) {
        co_return (co_await m_oneShotChannel.async_send(
          boost::system::error_code{}, std::move(value), boost::asio::use_awaitable
        ));
    }

private:
    Channel m_oneShotChannel;
};

}  // namespace kstd
