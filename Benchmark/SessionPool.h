#pragma once

#include <string>

#include "SessionLogic.h"

/**
 * @brief 会话连接池
 */
class SessionPool final {
public:
  explicit SessionPool(asio::io_context& io_context, asio::ip::tcp::endpoint endpoint);
  explicit SessionPool(asio::io_context& io_context, const asio::ip::address& address, uint16_t port);
  virtual ~SessionPool() = default;

public:
  void Spawn(const std::function<std::shared_ptr<SessionLogic>(std::shared_ptr<SessionContext>)>& create_logic,
             size_t session_count = std::thread::hardware_concurrency());

private:
  [[nodiscard]] auto Run(std::shared_ptr<SessionLogic> logic)  -> asio::awaitable<void>;

  asio::io_context& io_context_;
  asio::ip::tcp::endpoint server_endpoint_;
  std::atomic_int session_count_{0};
};
