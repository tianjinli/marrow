#pragma once

#include <any>
#include <asio.hpp>
#include <functional>
#include <optional>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "asio/awaitable.hpp"

template<typename>
struct IsAwaitable : std::false_type {};
template<typename T>
struct IsAwaitable<asio::awaitable<T>> : std::true_type {};

/**
 * 会话事件总线（仅单 vector<uint8_t> 参数）
 *
 * ReturnType: void / asio::awaitable<void>
 * EventType : 可用 uint32_t 或 enum class
 */
template<typename ReturnType, typename EventType = uint32_t>
class SessionEvents {
  using Executor = asio::any_io_executor;
  // 存储 Subscriber：同步时 void(std::vector<std::any> const&)，异步时 awaitable<void>(std::vector<std::any> const&)
  using Subscriber = std::conditional_t<IsAwaitable<ReturnType>::value, std::function<asio::awaitable<void>(std::vector<std::any> const&)>,
                                        std::function<void(std::vector<std::any> const&)>>;
  static_assert(std::is_same_v<ReturnType, void> || std::is_same_v<ReturnType, asio::awaitable<void>>,
                "ReturnType must be void or asio::awaitable<void>");

public:
  SessionEvents() = default;
  explicit SessionEvents(const Executor& executor) : strand_(asio::make_strand(executor)) {
  }

  /**
   * 订阅：只能接收 std::vector<uint8_t> 参数的 Handler
   *
   * Handler 类型示例：
   *   void    handler(const std::vector<uint8_t>& data);
   *   awaitable<void> handler(const std::vector<uint8_t>& data);
   */
  template<typename Handler>
  auto Subscribe(EventType id, Handler&& handler) -> std::conditional_t<IsAwaitable<ReturnType>::value, asio::awaitable<void>, void> {
    // 确保 Handler 可调用且签名为 handler(vector<uint8_t>)
    static_assert(std::is_invocable_v<Handler, const std::vector<uint8_t>&>,
                  "Subscribe handler must be invocable with (const std::vector<uint8_t>&)");
    if (subscribers_.count(id)) {
#if defined(_MSC_VER)
#pragma message("Warning: Duplicate subscription detected")
#else
#warning "Duplicate subscription detected"
#endif
    }

    if constexpr (IsAwaitable<ReturnType>::value) {
      // 异步 wrapper
      Subscriber wrapper = [handler = std::forward<Handler>(handler)](std::vector<std::any> const& args) -> asio::awaitable<void> {
        auto data = std::any_cast<const std::vector<uint8_t>&>(args[0]);
        co_await handler(std::move(data));
        co_return;
      };

      co_await MaybeDispatch();
      subscribers_[id] = std::move(wrapper);
      co_return;
    } else {
      // 同步 wrapper
      Subscriber wrapper = [handler = std::forward<Handler>(handler)](std::vector<std::any> const& args) {
        auto data = std::any_cast<const std::vector<uint8_t>&>(args[0]);
        handler(std::move(data));
      };

      MaybeDispatch([this, id, wrapper = std::move(wrapper)]() mutable {
        subscribers_[id] = std::move(wrapper);
      });
    }
  }

  /**
   * 发布：必须传入一个 const std::vector<uint8_t>&
   */
  auto Publish(EventType id, const std::vector<uint8_t>& data) -> std::conditional_t<IsAwaitable<ReturnType>::value, asio::awaitable<void>, void> {
    std::vector<std::any> packed{data};

    if constexpr (IsAwaitable<ReturnType>::value) {
      co_await MaybeDispatch();
      if (auto fn = subscribers_[id]; fn) {
        co_await fn(packed);
      }
      co_return;
    } else {
      MaybeDispatch([this, id, packed = std::move(packed)]() mutable {
        if (auto fn = subscribers_[id]; fn) {
          fn(packed);
        }
      });
    }
  }

private:
  // 协程中切到 strand
  asio::awaitable<void> MaybeDispatch() const {
    if (strand_) {
      co_await asio::dispatch(*strand_, asio::use_awaitable);
    }
  }

  // 同步分发
  template<typename Handler>
  void MaybeDispatch(Handler&& handler) {
    if (strand_) {
      asio::dispatch(*strand_, std::forward<Handler>(handler));
    } else {
      handler();
    }
  }

  std::optional<asio::strand<Executor>> strand_;
  std::unordered_map<EventType, Subscriber> subscribers_;
};
