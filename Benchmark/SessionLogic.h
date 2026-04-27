#pragma once

#include <asio.hpp>
#include <asio/awaitable.hpp>

struct SessionContext {
  std::string name;
  asio::ip::tcp::socket socket;
  std::chrono::system_clock::time_point connected_at;
};

/**
 * @brief 会话逻辑接口
 */
class SessionLogic {
public:
  explicit SessionLogic() = default;
  virtual ~SessionLogic() = default;

  SessionLogic(const SessionLogic& other) = delete;
  SessionLogic& operator=(const SessionLogic& other) = delete;

  /**
   * @brief 运行会话逻辑
   *
   * @return asio::awaitable<void> 异步操作结果
   */
  virtual asio::awaitable<void> Run() = 0;

public:
  /**
   * @brief 获取当前会话上下文
   *
   * @return 当前会话上下文
   */
  [[nodiscard]] virtual std::shared_ptr<SessionContext> Context() = 0;
};

/**
 * @brief 建议继承会话逻辑接口抽象类
 */
class BaseSessionLogic;
