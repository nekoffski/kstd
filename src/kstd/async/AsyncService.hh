#pragma once

#include "Core.hh"

namespace kstd {

class AsyncService {
public:
    struct Config {
        std::chrono::nanoseconds updateInterval;
    };

    virtual ~AsyncService() = default;

    virtual coro<void> init() { co_return; };
    virtual coro<void> deinit() { co_return; }
    virtual coro<void> update() = 0;

private:
};

}  // namespace kstd
