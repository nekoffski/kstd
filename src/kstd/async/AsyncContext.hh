#pragma once

#include <vector>
#include <atomic>

#include <boost/asio/experimental/channel.hpp>

#include "kstd/Concepts.hh"
#include "kstd/memory/Mallocator.hh"
#include "kstd/memory/UniquePtr.hh"

#include "Core.hh"
#include "AsyncService.hh"
#include "AsyncMessage.hh"

namespace kstd {

class AsyncContext : public NonCopyable, public NonMovable {
    friend class AsyncService::Messenger;

    static constexpr u64 channelSize = 16u;

    using Channel = boost::asio::experimental::channel<
      void(boost::system::error_code, UniquePtr<AsyncMessage>)>;

    struct ServiceContext {
        AsyncService::Config config;
        UniquePtr<AsyncService> service;
        Channel channel;
    };

public:
    explicit AsyncContext();

    void run() {
        start();
        m_ctx.run();
    }

    template <typename T, typename... Args>
    requires(std::is_base_of_v<AsyncService, T> && std::constructible_from<T, Args...>)
    void addService(const AsyncService::Config& config, Args&&... args) {
        m_services.emplace_back(
          config,
          makeUniqueWithAllocator<T>(
            &m_serviceAllocator, std::forward<Args>(args)...
          ),
          Channel{ m_ctx.get_executor(), channelSize }
        );
    }

    auto getExecutor() { return m_ctx.get_executor(); }
    void stop();

private:
    void start();
    void setupSignals();

    Coro<void> runService(ServiceContext& ctx);

    boost::asio::io_context m_ctx;
    boost::asio::signal_set m_signals;

    std::atomic_bool m_isRunning;

    Mallocator<> m_serviceAllocator;
    std::vector<ServiceContext> m_services;
};

}  // namespace kstd
