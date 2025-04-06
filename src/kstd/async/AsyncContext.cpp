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

    m_signals.cancel();
    m_signals.clear();
}

void AsyncContext::start() {
    spawn(m_ctx.get_executor(), [&]() -> coro<void> {
        for (auto& [config, service] : m_services) {
            co_await service->init();

            callAfter(m_ctx.get_executor(), config.updateInterval, [&]() {
                spawn(m_ctx.get_executor(), [&]() -> coro<void> {
                    while (m_isRunning.load()) {
                        co_await service->update();
                        co_await asyncSleep(config.updateInterval);
                    }

                    co_await service->deinit();
                    co_return;
                });
            });
        }
        co_return;
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

}  // namespace kstd
