
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include "kstd/async/Core.hh"
#include "kstd/async/Utils.hh"
#include "kstd/async/AsyncMessenger.hh"

using namespace std::chrono_literals;

struct TestMessage {
    int x = 1337;
};

struct TestResponse {
    int x = 2137;
};

int main() {
    kstd::log::init("Async");

    boost::asio::io_context ctx;
    boost::asio::signal_set signals{ ctx, SIGINT, SIGTERM };
    signals.async_wait([&](boost::system::error_code ec, int signal) {
        if (not ec) {
            ctx.stop();
        }
    });

    kstd::AsyncMessenger messenger{ ctx };

    kstd::spawn(
      ctx.get_executor(),
      [&, q = messenger.registerQueue("C1")]() -> kstd::Coro<void> {
          while (true) {
              auto promise = co_await q->send<TestMessage>().to("C2");
              kstd::log::debug("Coro: sent, waiting for response");
              co_await promise->wait();
              kstd::log::debug("Coro: got response");

              co_await kstd::asyncSleep(1000ms);
          }
          co_return;
      }
    );

    kstd::spawn(
      ctx.get_executor(),
      [&, q = messenger.registerQueue("C2")]() -> kstd::Coro<void> {
          while (true) {
              auto msg = co_await q->wait();
              kstd::log::debug("Coro2: got message");
              co_await msg->respond<TestResponse>();
              kstd::log::debug("Coro2: responded");

              co_await kstd::asyncSleep(1000ms);
          }
          co_return;
      }
    );

    ctx.run();
    return 0;
}
