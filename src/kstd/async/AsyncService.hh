#pragma once

#include <string>
#include <concepts>

#include "Core.hh"
#include "AsyncMessage.hh"
#include "AsyncPromise.hh"

namespace kstd {

class AsyncContext;

class AsyncService {
public:
    class Messenger;

    explicit AsyncService(const std::string& name);
    virtual ~AsyncService() = default;

    virtual Coro<void> init() { co_return; };
    virtual Coro<void> deinit() { co_return; }
    virtual Coro<void> update(Messenger&& messenger)    = 0;
    virtual Coro<void> onMessage(AsyncMessage& message) = 0;

    class Messenger {
        class SendProxy : public NonMovable, public NonCopyable {
        public:
            explicit SendProxy(
              UniquePtr<AsyncMessage> message, Messenger& messenger
            ) : m_message(std::move(message)), m_messenger(messenger) {}

            Coro<SharedPtr<AsyncMessage::Promise>> to(const std::string& destination
            ) && {
                auto promise = m_message->getPromise();
                co_await m_messenger.sendImpl(destination, std::move(m_message));
                co_return promise;
            }

        private:
            UniquePtr<AsyncMessage> m_message;
            Messenger& m_messenger;
        };

        friend class SendProxy;

    public:
        explicit Messenger(
          AsyncContext& ctx, const boost::asio::any_io_executor& ex
        );

        template <typename T, typename... Args>
        requires std::constructible_from<T, Args...>
        SendProxy send(Args&&... args) {
            return SendProxy{
                makeUnique<details::TypedAsyncMessage<T>>(
                  m_ex, std::forward<Args>(args)...
                ),
                *this
            };
        }

    private:
        Coro<void> sendImpl(
          const std::string& destination, UniquePtr<AsyncMessage> message
        );

        AsyncContext& m_ctx;
        const boost::asio::any_io_executor& m_ex;
    };

    struct Config {
        std::chrono::nanoseconds updateInterval;
    };

    const std::string name;
};

}  // namespace kstd
