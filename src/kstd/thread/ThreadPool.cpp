#include "ThreadPool.hh"

#include "kstd/Error.hh"

namespace kstd {

ThreadPool::ThreadPool(u16 threadCount, u16 workStealingFactor) :
    m_isRunning(true), m_workerCount(threadCount),
    m_workStealingFactor(workStealingFactor), m_nextWorkerId(0u),
    m_queues(threadCount) {
    m_workers.reserve(m_workerCount);
    for (u16 i = 0; i < m_workerCount; ++i) {
        m_workers.emplace_back([&, i]() { startWorker(i); });
    }
}

ThreadPool::~ThreadPool() { stop(); }

u16 ThreadPool::getThreadCount() const { return m_workerCount; }

void ThreadPool::stop() {
    m_isRunning = false;
    for (auto& q : m_queues) q.stop();
}

void ThreadPool::startWorker(u16 id) {
    while (m_isRunning) {
        try {
            std::invoke(findTask(id));
        } catch ([[maybe_unused]] const CancelledError&) {
            log::debug("Thread pool stopped, shutting down worker: {}", id);
            break;
        }
    }
}

ThreadPool::Task ThreadPool::findTask(u16 workerIndex) {
    for (u16 i = 0; i < m_workerCount * m_workStealingFactor; ++i) {
        const auto index = (workerIndex + i) % m_workerCount;
        if (auto task = m_queues[index].tryPop(); task) return *task;
    }
    return m_queues[workerIndex].pop();
}

}  // namespace kstd
