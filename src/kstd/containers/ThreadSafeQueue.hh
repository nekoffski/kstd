#pragma once

#include <condition_variable>
#include <queue>
#include <mutex>
#include <optional>
#include <atomic>

#include "kstd/Core.hh"
#include "kstd/Error.hh"

namespace kstd {

namespace details {

template <typename T, u64 Capacity> class ThreadSafeQueueBase {
public:
    void stop() { m_cv.notify_all(); }

    bool tryPush(T&& value) {
        if (auto lk = std::unique_lock{ m_mutex, std::try_to_lock }; lk) {
            if constexpr (Capacity > 0u)
                if (m_q.size() >= Capacity) return false;

            m_q.push(std::move(value));
            m_cv.notify_one();
            return true;
        }
        return false;
    }

    u64 size() const { return m_q.size(); }

    void push(T&& value) {
        std::unique_lock lk{ m_mutex };

        if constexpr (Capacity > 0u) {
            while (m_q.size() >= Capacity) {
                m_cv.wait(lk);
                if (m_q.size() >= Capacity) throw CancelledError{};
            }
        }

        m_q.push(std::move(value));
        m_cv.notify_one();
    }

    std::optional<T> tryPop() {
        if (auto lk = std::unique_lock{ m_mutex, std::try_to_lock };
            lk && not m_q.empty()) {
            return getFront();
        }
        return {};
    }

    T pop() {
        std::unique_lock lk{ m_mutex };

        while (m_q.empty()) {
            m_cv.wait(lk);
            if (m_q.empty()) throw CancelledError{};
        }
        return getFront();
    }

private:
    T getFront() {
        auto t = std::move(m_q.front());
        m_q.pop();
        return t;
    }

    std::queue<T> m_q;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

}  // namespace details

template <typename T>
class ThreadSafeQueue : public details::ThreadSafeQueueBase<T, 0u> {};

template <typename T, u64 Capacity>
class FixedSizeThreadSafeQueue : public details::ThreadSafeQueueBase<T, Capacity> {
    static_assert(Capacity > 0u);
};

}  // namespace kstd