#pragma once

#include <chrono>

#include "Core.hh"

#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/error.hpp>

namespace kstd {

namespace details {

void asyncSleepImpl(
  boost::asio::any_completion_handler<void(boost::system::error_code)> handler,
  boost::asio::any_io_executor ex, std::chrono::nanoseconds duration
);

template <typename CompletionToken>
inline auto asyncSleep(
  boost::asio::any_io_executor ex, std::chrono::nanoseconds duration,
  CompletionToken&& token
) {
    return boost::asio::async_initiate<
      CompletionToken, void(boost::system::error_code)>(
      asyncSleepImpl, token, std::move(ex), duration
    );
}

}  // namespace details

Coro<void> asyncSleep(std::chrono::nanoseconds duration);

template <typename Executor, typename Callback>
void callAfter(
  const Executor& ex, std::chrono::nanoseconds duration, Callback&& callback
) {
    details::asyncSleepImpl(
      [callback = std::forward<Callback>(callback)](boost::system::error_code ec) {
          if (not ec) callback();
      },
      ex, duration
    );
}

}  // namespace kstd