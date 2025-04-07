#include "AsyncContext.hh"

#include "kstd/async/Utils.hh"

namespace kstd {

AsyncContext::AsyncContext() :
    m_ctx{ 1 }, m_signals(m_ctx, SIGINT, SIGTERM), m_isRunning(true) {
    setupSignals();
}

void AsyncContext::stop() {
    log::info("Received stop request");
    m_isRunning.store(false);

    for (auto& ctx : m_services) ctx.channel.cancel();

    m_signals.cancel();
    m_signals.clear();
}

void AsyncContext::start() {
    spawn(m_ctx.get_executor(), [&]() -> Coro<void> {
        for (auto& service : m_services) co_await runService(service);
    });
}

void AsyncContext::setupSignals() {
    m_signals.async_wait([&](boost::system::error_code ec, int signal) {
        if (not ec) {
            log::warn("Received signal ({})", signal);
            stop();
        }
    });
}

Coro<void> AsyncContext::runService(ServiceContext& ctx) {
    auto& [config, service, channel] = ctx;

    co_await service->init();

    spawn(m_ctx.get_executor(), [&]() -> Coro<void> {
        while (m_isRunning.load()) {
            auto msg = co_await channel.async_receive(boost::asio::use_awaitable);
            co_await service->onMessage(*msg);
        }
    });

    callAfter(m_ctx.get_executor(), config.updateInterval, [&]() {
        spawn(m_ctx.get_executor(), [&]() -> Coro<void> {
            while (m_isRunning.load()) {
                co_await service->update(AsyncService::Messenger{ *this });
                co_await asyncSleep(config.updateInterval);
            }
            co_await service->deinit();
        });
    });
}

}  // namespace kstd
