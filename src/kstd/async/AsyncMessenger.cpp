#include "AsyncMessenger.hh"

#include "kstd/Log.hh"

namespace kstd {

AsyncMessenger::AsyncMessenger(boost::asio::io_context& ctx) : m_ctx(ctx) {}

Coro<void> AsyncMessenger::sendImpl(
  const std::string& destination, UniquePtr<AsyncMessage> message
) {
    auto queue = m_queues.find(destination);
    log::expect(
      queue != m_queues.end(), "Could not find {} queue as a destination",
      destination
    );

    auto& channel = queue->second->getChannel();
    co_await channel.async_send(
      boost::system::error_code{}, std::move(message), boost::asio::use_awaitable
    );
}

AsyncMessenger::Queue* AsyncMessenger::registerQueue(const std::string& name) {
    auto [it, inserted] = m_queues.emplace(name, makeUnique<Queue>(*this));
    log::expect(inserted, "Queue {} already registered", name);
    return it->second.get();
}

Coro<UniquePtr<AsyncMessage>> AsyncMessenger::Queue::wait() {
    co_return (co_await m_channel.async_receive(boost::asio::use_awaitable));
}

AsyncMessenger::Channel& AsyncMessenger::Queue::getChannel() { return m_channel; }

AsyncMessenger::Queue::Queue(AsyncMessenger& messenger
) : m_messenger(messenger), m_channel(m_messenger.getExecutor(), channelSize) {}

}  // namespace kstd
