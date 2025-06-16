#pragma once

#include <mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <numeric>

#include "kstd/Error.hh"
#include "kstd/Concepts.hh"
#include "kstd/memory/SharedPtr.hh"
#include "kstd/Algorithms.hh"

namespace kstd {

namespace details {

class PromiseBase : public NonCopyable, public NonMovable {
public:
    void cancel() { m_cv.notify_one(); }

protected:
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

}  // namespace details

template <typename T> class Promise : public details::PromiseBase {
    using Callback = std::function<void(T&&)>;

public:
    T wait() {
        std::unique_lock lk{ this->m_mutex };

        while (not m_value) {
            this->m_cv.wait(lk);
            if (not m_value) throw CancelledError{ "Promise has been cancelled" };
        }
        return std::move(*m_value);
    }

    std::optional<T> waitFor(std::chrono::nanoseconds duration) {
        std::unique_lock lk{ this->m_mutex };
        this->m_cv.wait_for(lk, duration);

        if (not m_value) return {};

        return std::move(*m_value);
    }

    void set(T&& value) {
        std::unique_lock lk{ m_mutex };

        if (m_callback)
            std::invoke(*m_callback, std::move(value));
        else
            m_value.emplace(std::move(value));

        this->m_cv.notify_one();
    }

    template <typename Callback>
    requires Callable<Callback, void, T&&>
    void on(Callback&& callback) {
        m_callback = std::forward<Callback>(callback);
    }

private:
    std::optional<T> m_value;
    std::optional<Callback> m_callback;
};

template <> class Promise<void> : public details::PromiseBase {
    using Callback = std::function<void()>;

public:
    void wait() {
        std::unique_lock lk{ this->m_mutex };

        while (not m_done) {
            this->m_cv.wait(lk);
            if (not m_done) throw CancelledError{ "Promise has been cancelled" };
        }
    }

    bool waitFor(std::chrono::nanoseconds duration) {
        std::unique_lock lk{ this->m_mutex };
        this->m_cv.wait_for(lk, duration);
        return m_done;
    }

    void set() {
        std::unique_lock lk{ m_mutex };

        if (m_callback)
            std::invoke(*m_callback);
        else
            m_done = true;

        this->m_cv.notify_one();
    }

    template <typename Callback>
    requires Callable<Callback, void>
    void on(Callback&& callback) {
        m_callback = std::forward<Callback>(callback);
    }

private:
    std::atomic_bool m_done;
    std::optional<Callback> m_callback;
};

template <typename T> class Future {
public:
    explicit Future(SharedPtr<Promise<T>> p) : m_promise(std::move(p)) {}

    T wait() { return m_promise->wait(); }

    std::optional<T> waitFor(std::chrono::nanoseconds duration) {
        return m_promise->waitFor(duration);
    }

    template <typename Callback>
    requires Callable<Callback, void, T&&>
    void on(Callback&& callback) {
        m_promise->on(std::forward<Callback>(callback));
    }

private:
    SharedPtr<Promise<T>> m_promise;
};

template <> class Future<void> {
public:
    explicit Future(SharedPtr<Promise<void>> p) : m_promise(std::move(p)) {}

    void wait() { m_promise->wait(); }

    bool waitFor(std::chrono::nanoseconds duration) {
        return m_promise->waitFor(duration);
    }

    template <typename Callback>
    requires Callable<Callback, void>
    void on(Callback&& callback) {
        m_promise->on(std::forward<Callback>(callback));
    }

private:
    SharedPtr<Promise<void>> m_promise;
};

namespace details {

template <typename T> class FutureVectorBase {
public:
    void add(Future<T>&& f) { m_futures.emplace_back(std::move(f)); }

protected:
    std::vector<Future<T>> m_futures;
};

}  // namespace details

template <typename T> class FutureVector : public details::FutureVectorBase<T> {
public:
    std::vector<T> wait() {
        return transform<T>(this->m_futures, [](auto& f) -> T { return f.wait(); });
    }

    T accumulate() {
        return std::accumulate(
          this->m_futures.begin(), this->m_futures.end(), T{ 0 },
          [](auto sum, auto f) { return sum + f.wait(); }
        );
    }
};

template <> class FutureVector<void> : public details::FutureVectorBase<void> {
public:
    void wait() {
        for (auto& f : this->m_futures) f.wait();
    }
};

}  // namespace kstd
