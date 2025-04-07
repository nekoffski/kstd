#include "AsyncService.hh"

#include "AsyncContext.hh"

namespace kstd {

AsyncService::AsyncService(const std::string& name) : name(name) {}

AsyncService::Messenger::Messenger(AsyncContext& ctx) : m_ctx(ctx) {}

Coro<void> AsyncService::Messenger::sendImpl(
  const std::string& destination, UniquePtr<AsyncMessage> message
) {
    for (auto& serviceCtx : m_ctx.m_services) {
        if (destination == serviceCtx.service->name) {
            co_await serviceCtx.channel.async_send(
              boost::system::error_code{}, std::move(message),
              boost::asio::use_awaitable
            );
        }
    }
}

}  // namespace kstd
