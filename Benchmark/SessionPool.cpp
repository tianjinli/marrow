#include "SessionPool.h"

#include "KernelEngine/TraceService.h"

/**
 * @brief 会话连接池
 * @param io_context io上下文
 * @param endpoint IP 地址 + 端口号
 */
SessionPool::SessionPool(asio::io_context& io_context, asio::ip::tcp::endpoint endpoint) :
    io_context_(io_context), server_endpoint_(std::move(endpoint)) {
}

/**
 * @brief 会话连接池
 * @param io_context io上下文
 * @param address IP 地址(没有使用 std::string 类型避免异常)
 * @param port 端口号
 */
SessionPool::SessionPool(asio::io_context& io_context, const asio::ip::address& address, uint16_t port) :
    io_context_(io_context), server_endpoint_(address, port) {
}

void SessionPool::Spawn(const std::function<std::shared_ptr<SessionLogic>(std::shared_ptr<SessionContext>)>& create_logic,
                        const size_t session_count) {
  session_count_.store(static_cast<int>(session_count));
  CLogger::Warn("即将创建 {} 个会话", session_count);

  for (size_t i = 0; i < session_count; ++i) {
    auto name = std::format("#{:03}", i + 1);
    auto socket = asio::ip::tcp::socket(io_context_);
    auto connected_at = std::chrono::system_clock::now();
    const auto ctx = std::make_shared<SessionContext>(std::move(name), std::move(socket), std::move(connected_at));
    co_spawn(io_context_, Run(create_logic(ctx)), asio::detached);
  }
  co_spawn(
      io_context_,
      [this]() -> asio::awaitable<void> {
        using namespace std::chrono_literals;
        asio::steady_timer timer(io_context_);
        while (session_count_ > 0) {
          timer.expires_after(100ms);
          co_await timer.async_wait();
        }
        CLogger::Info("所有任务已完成程序即将退出");
        timer.expires_after(500ms);
        co_await timer.async_wait();
        io_context_.stop();
        co_return;
      },
      asio::detached);
}

auto SessionPool::Run(const std::shared_ptr<SessionLogic> logic) -> asio::awaitable<void> {
  const auto ctx = logic->Context();
#ifndef DEBUG_ENABLED
  try {
#endif // !DEBUG_ENABLED
    co_await ctx->socket.async_connect(server_endpoint_, asio::use_awaitable);
    CLogger::Info("{} 准备运行...", ctx->name);
    co_await logic->Run();
#ifndef DEBUG_ENABLED
  } catch (const std::exception& ex) {
    CLogger::Warn("{} 异常退出: {}", ctx->name, ToSimpleUtf8(ex.what()));
  } catch (...) {
    CLogger::Error("{} 未知异常退出", ctx->name);
  }
#endif // !DEBUG_ENABLED
  session_count_.fetch_sub(1);
  co_return;
}
