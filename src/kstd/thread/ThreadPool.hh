#pragma once

#include <thread>
#include <vector>
#include <atomic>
#include <queue>
#include <optional>

#include "kstd/memory/SharedPtr.hh"

#include "kstd/Core.hh"
#include "Promise.hh"

namespace kstd {

class ThreadPool {
    using Task = std::function<void()>;

    class Queue {
    public:
        Task pop();
        std::optional<Task> tryPop();

        bool tryPush(Task& task);
        void push(Task& task);

        void stop();

    private:
        Task getFront();

        std::queue<Task> m_q;
        std::mutex m_mutex;
        std::condition_variable m_cv;
    };

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

        auto promise = makeShared<Promise<R>>();
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
            if (m_queues[index].tryPush(task)) return future;
        }

        m_queues[workerId].push(task);
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
    std::vector<Queue> m_queues;
};

}  // namespace kstd
