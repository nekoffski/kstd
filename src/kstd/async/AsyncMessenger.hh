#pragma once

#include <unordered_map>
#include <memory>

#include "kstd/Core.hh"
#include "kstd/Concepts.hh"
#include "Core.hh"
#include "AsyncMessage.hh"

namespace kstd {

class AsyncMessenger {
    static constexpr u64 channelSize = 32u;

    using Channel = boost::asio::experimental::channel<
      void(boost::system::error_code, std::unique_ptr<AsyncMessage>)>;

public:
    class Queue {
        friend class AsyncMessenger;

        class SendProxy : public NonMovable, public NonCopyable {
        public:
            explicit SendProxy(
              std::unique_ptr<AsyncMessage> message, AsyncMessenger& messenger
            ) : m_message(std::move(message)), m_messenger(messenger) {}

            Coro<std::shared_ptr<AsyncMessage::Promise>> to(
              const std::string& destination
            ) && {
                auto promise = m_message->getPromise();
                co_await m_messenger.sendImpl(destination, std::move(m_message));
                co_return promise;
            }

        private:
            std::unique_ptr<AsyncMessage> m_message;
            AsyncMessenger& m_messenger;
        };

    public:
        explicit Queue(AsyncMessenger& messenger);

        template <typename T, typename... Args>
        requires std::constructible_from<T, Args...>
        SendProxy send(Args&&... args) {
            return SendProxy{
                std::make_unique<details::TypedAsyncMessage<T>>(
                  m_messenger.getExecutor(), std::forward<Args>(args)...
                ),
                m_messenger
            };
        }

        void cancel();

        Coro<std::unique_ptr<AsyncMessage>> wait();

    private:
        Channel& getChannel();

        AsyncMessenger& m_messenger;
        Channel m_channel;
    };

    explicit AsyncMessenger(boost::asio::io_context& ctx);

    Queue* registerQueue(const std::string& name);

    auto getExecutor() { return m_ctx.get_executor(); }

private:
    Coro<void> sendImpl(
      const std::string& destination, std::unique_ptr<AsyncMessage> message
    );

    boost::asio::io_context& m_ctx;
    std::unordered_map<std::string, std::unique_ptr<Queue>> m_queues;
};

}  // namespace kstd
