#pragma once

#include <vector>
#include <atomic>

#include "kstd/Concepts.hh"
#include "kstd/memory/Mallocator.hh"
#include "kstd/memory/UniquePtr.hh"

#include "Core.hh"
#include "AsyncService.hh"

namespace kstd {

class AsyncContext : public NonCopyable, public NonMovable {
    struct ServiceContext {
        AsyncService::Config config;
        UniquePtr<AsyncService> service;
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
          )
        );
    }

    auto getExecutor() { return m_ctx.get_executor(); }

    void stop();

private:
    void start();
    void setupSignals();

    boost::asio::io_context m_ctx;
    boost::asio::signal_set m_signals;

    std::atomic_bool m_isRunning;

    Mallocator<> m_serviceAllocator;
    std::vector<ServiceContext> m_services;
};

}  // namespace kstd
