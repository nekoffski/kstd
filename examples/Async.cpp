
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include "kstd/async/Core.hh"
#include "kstd/async/Utils.hh"
#include "kstd/async/AsyncContext.hh"

using namespace std::chrono_literals;

struct ServiceA : kstd::AsyncService {
    kstd::coro<void> update() override {
        kstd::log::info("Hello from ServiceA!");
        co_return;
    }
};

struct ServiceB : kstd::AsyncService {
    kstd::coro<void> update() override {
        kstd::log::info("Hello from ServiceB!");
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
