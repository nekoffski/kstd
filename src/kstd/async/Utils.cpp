#include "Utils.hh"

#include <boost/asio/consign.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>

namespace kstd {

namespace details {

void asyncSleepImpl(
  boost::asio::any_completion_handler<void(boost::system::error_code)> handler,
  boost::asio::any_io_executor ex, std::chrono::nanoseconds duration
) {
    auto timer = std::make_shared<boost::asio::steady_timer>(ex, duration);
    timer->async_wait(boost::asio::consign(std::move(handler), timer));
}

}  // namespace details

coro<void> asyncSleep(std::chrono::nanoseconds duration) {
    co_await details::asyncSleep(
      co_await boost::asio::this_coro::executor, duration, boost::asio::use_awaitable
    );
}

}  // namespace kstd
