#pragma once

#include "SessionEvents.h"
#include "SessionLogic.h"

/**
 * @brief 会话逻辑接口抽象类
 */
class BaseSessionLogic : public SessionLogic {
  /**
   * @brief 会话上下文
   */
  std::shared_ptr<SessionContext> ctx_;

protected:
  /**
   * @brief 会话事件总线
   */
  SessionEvents<asio::awaitable<void>> bus_;

public:
  explicit BaseSessionLogic(const std::shared_ptr<SessionContext>& ctx) : ctx_(ctx), bus_(ctx->socket.get_executor()) {
  }
  ~BaseSessionLogic() override = default;

  asio::awaitable<void> Run() override;

private:
  /**
   * @brief 处理会话逻辑
   *
   * @param  ctx 套接字引用
   * @return 处理结果
   */
  virtual asio::awaitable<bool> Execute(const std::shared_ptr<SessionContext>& ctx);

public:
  /**
   * @brief 准备会话数据
   * 请子类在此订阅事件
   *
   * @param ctx 会话上下文
   * @return 处理结果
   */
  virtual asio::awaitable<void> Prepare(const std::shared_ptr<SessionContext>& ctx) = 0;

  /**
   * @brief 发送数据
   *
   * @param main 主命令码
   * @param sub 子命令码
   * @return 发送结果
   */
  virtual asio::awaitable<bool> Sending(const uint16_t main, const uint16_t sub) {
    return Sending(main, sub, nullptr, 0);
  }

  /**
   * @brief 发送数据
   *
   * @param main 主命令码
   * @param sub 子命令码
   * @param body 包体数据
   * @return 发送结果
   */
  virtual asio::awaitable<bool> Sending(const uint16_t main, const uint16_t sub, const std::vector<uint8_t>& body) {
    return Sending(main, sub, body.data(), body.size());
  }

  /**
   * @brief 发送数据
   *
   * @param main 主命令码
   * @param sub 子命令码
   * @param body 包体数据
   * @param body_size 包体数据大小
   * @return 发送结果
   */
  virtual asio::awaitable<bool> Sending(uint16_t main, uint16_t sub, const uint8_t* body, size_t body_size);

  /**
   * @brief 优雅关闭连接
   *
   * @return
   */
  virtual asio::awaitable<void> Closing() {
    ctx_->socket.shutdown(asio::ip::tcp::socket::shutdown_send);
    co_return;
  }

  /**
   * @brief 发送数据
   *
   * @param main 主命令码
   * @param sub 子命令码
   * @param obj 包体对象
   * @return 发送结果
   */
  template<typename T>
  asio::awaitable<bool> Sending(const uint16_t main, const uint16_t sub, const T& obj) {
    static_assert(!std::is_pointer_v<T>, "不接受指针类型，请传值或引用");
    const auto body = reinterpret_cast<const uint8_t*>(&obj);
    return Sending(main, sub, body, sizeof(T));
  }

  /**
   * @brief 订阅事件
   *
   * @param event 事件 ID
   * @param handler 处理函数(std::bind)
   * @return
   */
  auto Subscribe(const uint32_t& event, std::function<asio::awaitable<bool>(const std::vector<uint8_t>&)> handler) -> asio::awaitable<void> {
    co_await bus_.Subscribe(event, std::move(handler));
    co_return;
  }

  /**
   * @brief 订阅事件
   *
   * @param event 事件 ID
   * @param handler 处理函数(类函数指针)
   * @return
   */
  template<typename C>
  asio::awaitable<void> Subscribe(const uint32_t event, asio::awaitable<void> (C::*handler)(const std::vector<uint8_t>&), C* instance) {
    auto wrapper = [instance, handler](std::vector<uint8_t> data) -> asio::awaitable<void> {
      co_await (instance->*handler)(data);
      co_return;
    };
    co_await bus_.Subscribe(event, std::move(wrapper));
    co_return;
  }

  /**
   * @brief 发布事件
   *
   * @param main 主命令码
   * @param sub 子命令码
   * @param body 包体数据
   * @return
   */
  auto Publish(uint16_t main, uint16_t sub, const std::vector<uint8_t>& body) -> asio::awaitable<void>;

  /**
   * @brief 获取当前会话上下文
   *
   * @return 当前会话上下文
   */
  [[nodiscard]] std::shared_ptr<SessionContext> Context() override {
    return ctx_;
  }
};
