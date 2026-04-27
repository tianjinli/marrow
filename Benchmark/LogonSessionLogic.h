#pragma once

#include "BaseSessionLogic.h"

/**
 * @brief 登录会话逻辑实现类
 */
class LogonSessionLogic final : public BaseSessionLogic {
public:
  explicit LogonSessionLogic(const std::shared_ptr<SessionContext>& ctx) : BaseSessionLogic(ctx) {
  }
  ~LogonSessionLogic() override = default;

  /**
   * @brief 发送登录会话数据
   *
   * @param ctx 会话上下文
   * @return asio::awaitable<void> 异步操作结果
   */
  asio::awaitable<void> Prepare(const std::shared_ptr<SessionContext>& ctx) override;

  // 用户登录成功事件
  asio::awaitable<void> MbLogonSuccess(const std::vector<uint8_t>&);
  // 用户登录失败事件
  asio::awaitable<void> MbLogonFailure(const std::vector<uint8_t>&);
};
