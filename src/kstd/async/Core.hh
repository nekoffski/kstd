#pragma once

#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio/async_result.hpp>
#include <boost/asio/experimental/promise.hpp>

namespace kstd {

using namespace std::chrono_literals;

namespace this_coro = boost::asio::this_coro;

template <typename T> using Coro         = boost::asio::awaitable<T>;
template <typename T> using AsyncPromise = boost::asio::experimental::promise<T>;

template <typename Executor, typename T, typename AwaitableExecutor>
void spawn(const Executor& ex, boost::asio::awaitable<T, AwaitableExecutor> a) {
    boost::asio::co_spawn(ex, std::move(a), boost::asio::detached);
}

template <typename Executor, typename F> void spawn(const Executor& ex, F&& f) {
    boost::asio::co_spawn(ex, std::forward<F>(f), boost::asio::detached);
}

}  // namespace kstd
