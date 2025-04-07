
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include "kstd/async/Core.hh"
#include "kstd/async/Utils.hh"
#include "kstd/async/AsyncContext.hh"
#include "kstd/memory/UniquePtr.hh"

using namespace std::chrono_literals;

struct TestMessage {
    int x = 1337;
};

struct ServiceA : kstd::AsyncService {
    ServiceA() : AsyncService("ServiceA") {}

    kstd::Coro<void> update(Messenger&& messenger) override {
        co_await messenger.send<TestMessage>().to("ServiceB");
    }

    kstd::Coro<void> onMessage(const kstd::AsyncMessage& message) override {
        if (const auto msg = message.as<TestMessage>(); msg) {
            kstd::log::info("{} - got message: {}", name, msg->x);
        }
        co_return;
    }
};

struct ServiceB : kstd::AsyncService {
    ServiceB() : AsyncService("ServiceB") {}

    kstd::Coro<void> update([[maybe_unused]] Messenger&& messenger) override {
        co_return;
    }

    kstd::Coro<void> onMessage(const kstd::AsyncMessage& message) override {
        if (const auto msg = message.as<TestMessage>(); msg) {
            kstd::log::info("{} - got message: {}", name, msg->x);
        }
        co_return;
    }
};

int main() {
    kstd::log::init("Async");

    kstd::AsyncService::Config serviceConfig{ .updateInterval = 1000ms };

    kstd::AsyncContext ctx;

    ctx.addService<ServiceA>(serviceConfig);
    ctx.addService<ServiceB>(serviceConfig);

    ctx.run();

    return 0;
}
