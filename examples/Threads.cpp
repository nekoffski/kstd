
#include "kstd/Core.hh"
#include "kstd/Log.hh"

#include <kstd/thread/ThreadPool.hh>

using namespace std::chrono_literals;

void foo(kstd::ThreadPool& tp) {
    {
        auto future = tp.call([&]() -> int {
            std::this_thread::sleep_for(5s);
            return 1;
        });

        future.on([&](int value) { kstd::log::debug("Got value: {}", value); });
    }

    tp.call(
        [&]() -> void {
            std::this_thread::sleep_for(1s);
            kstd::log::debug("Nothing to return here");
        }
    ).on([]() { kstd::log::debug("Done"); });
}

void bar(kstd::ThreadPool& tp) {
    kstd::FutureVector<void> fv;

    for (kstd::u16 i = 0; i < 16u; ++i) {
        fv.add(tp.call([i]() {
            std::this_thread::sleep_for(1s);
            kstd::log::debug("Worker '{}' finished", i);
        }));
    }

    fv.wait();
}

void bar2(kstd::ThreadPool& tp) {
    kstd::FutureVector<kstd::u16> fv;

    for (kstd::u16 i = 0; i < 16u; ++i) {
        fv.add(tp.call([i]() -> kstd::u16 {
            std::this_thread::sleep_for(1s);
            return i;
        }));
    }

    kstd::log::debug("Sum = '{}'", fv.accumulate());
}

int main() {
    kstd::log::init("threading");

    kstd::u16 threads = 4u;
    kstd::ThreadPool tp{ threads };

    foo(tp);
    bar(tp);
    bar2(tp);

    return 0;
}
