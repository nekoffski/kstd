#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <optional>
#include <memory>

#include "kstd/Core.hh"
#include "kstd/containers/ThreadSafeQueue.hh"
#include "Promise.hh"

namespace kstd {

class ThreadPool {
    using Task = std::function<void()>;

    static constexpr u16 defaultWorkStealingFactor = 2u;

public:
    explicit ThreadPool(
      u16 threadCount, u16 workStealingFactor = defaultWorkStealingFactor
    );
    ~ThreadPool();

    template <typename Callback>
    requires std::is_invocable_v<Callback>
    Future<std::result_of_t<Callback()>> call(Callback&& callback) {
        using R = std::result_of_t<Callback()>;

        auto promise = std::make_shared<Promise<R>>();
        Future<R> future{ promise };

        Task task =
          [promise, callback = std::forward<Callback>(callback)]() mutable {
              if constexpr (std::is_same_v<R, void>) {
                  callback();
                  promise->set();
              } else {
                  promise->set(callback());
              }
          };

        const auto workerId = m_nextWorkerId++;

        for (u16 i = 0; i < m_workerCount * m_workStealingFactor; ++i) {
            const auto index = (workerId + i) % m_workerCount;
            if (m_queues[index].tryPush(std::move(task))) return future;
        }

        m_queues[workerId].push(std::move(task));
        return future;
    }

    u16 getThreadCount() const;
    void stop();

private:
    void startWorker(u16 id);
    Task findTask(u16 id);

    std::atomic_bool m_isRunning;

    u16 m_workerCount;
    u16 m_workStealingFactor;
    std::atomic_uint16_t m_nextWorkerId;

    std::vector<std::jthread> m_workers;
    std::vector<ThreadSafeQueue<Task>> m_queues;
};

}  // namespace kstd
