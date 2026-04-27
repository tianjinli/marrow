#pragma once

#include "KernelEngineHead.h"
#include "TraceService.h"

//-------------------- Event Tag & 参数列表 --------------------

#define DEFINE_EVENT_TAG(Name, ...)       \
  struct Name##Tag {                      \
    using Args = std::tuple<__VA_ARGS__>; \
  };

// 异步启动事件
// 订阅 IAttemperEngineSink::OnAttemperEngineStart
// 发布 CAttemperEngine::InitiateService
// DEFINE_EVENT_TAG(AsyncStartEvent, IUnknownEx*)
// 异步停止事件
// 订阅 IAttemperEngineSink::OnAttemperEngineConclude
// 发布 CAttemperEngine::ConcludeService
// DEFINE_EVENT_TAG(AsyncStopEvent, IUnknownEx*)
// 异步定时器事件
// 订阅 IAttemperEngineSink::OnEventTimer
// 发布 CTimerEngine::OnTimerThreadSink -> ITimerEngineEvent::OnEventTimer
DEFINE_EVENT_TAG(AsyncTimerEvent, DWORD, WPARAM)
// 异步控制事件
// 订阅 IAttemperEngineSink::OnEventControl
// 发布 CAttemperEngine::OnEventControl (主动调用)
DEFINE_EVENT_TAG(AsyncControlEvent, WORD, nf::BufferPtr)
// 数据库启动事件
// 订阅 IDataBaseEngineSink::OnDataBaseEngineStart
// 发布 CDataBaseEngine::InitiateService
DEFINE_EVENT_TAG(DataBaseStartEvent, IUnknownEx*)
// 数据库停止事件
// 订阅 IDataBaseEngineSink::OnDataBaseEngineConclude
// 发布 CDataBaseEngine::ConcludeService
DEFINE_EVENT_TAG(DataBaseStopEvent, IUnknownEx*)
// 数据库结果事件
// 订阅 IDataBaseEngineEvent::OnEventDataBase
// 发布 CDataBaseEngine::OnEventDataBase
DEFINE_EVENT_TAG(DataBaseResultEvent, WORD, DWORD, nf::BufferPtr)
// 数据库请求事件
// 订阅 IDataBaseEngineSink::OnDataBaseEngineRequest
// 发布 CDataBaseEngine::SetDataBaseEngineSink
DEFINE_EVENT_TAG(DataBaseRequestEvent, WORD, DWORD, nf::BufferPtr)
// 异步套接字连接事件
// 订阅 ITCPSocketEvent::OnEventTCPSocketLink
// 发布 CTCPSocketService::OnSocketLink
DEFINE_EVENT_TAG(TCPSocketLinkEvent, WORD, INT)
// 异步套接字关闭事件
// 订阅 ITCPSocketEvent::OnEventTCPSocketShut
// 发布 CTCPSocketService::OnSocketShut
DEFINE_EVENT_TAG(TCPSocketShutEvent, WORD, BYTE)
// 异步套接字读取事件
// 订阅 ITCPSocketEvent::OnEventTCPSocketRead
// 发布 CTCPSocketService::OnSocketRead
DEFINE_EVENT_TAG(TCPSocketReadEvent, WORD, TCP_Command, nf::BufferPtr)
// 异步网络绑定事件
// 订阅 ITCPNetworkEngineEvent::OnEventTCPNetworkBind
// 发布 CTCPNetworkEngine::OnEventSocketBind
DEFINE_EVENT_TAG(TCPNetworkBindEvent, DWORD, DWORD)
// 异步网络关闭事件
// 订阅 ITCPNetworkEngineEvent::OnEventTCPNetworkShut
// 发布 CTCPNetworkEngine::OnEventSocketShut
DEFINE_EVENT_TAG(TCPNetworkShutEvent, DWORD, DWORD, DWORD)
// 异步网络读取事件
// 订阅 ITCPNetworkEngineEvent::OnEventTCPNetworkRead
// 发布 CTCPNetworkEngine::OnEventSocketRead
DEFINE_EVENT_TAG(TCPNetworkReadEvent, DWORD, TCP_Command, nf::BufferPtr)

// Tuple -> std::function<bool(Args...)>
template<typename Tuple>
struct TupleToFunction;
template<typename... Ts>
struct TupleToFunction<std::tuple<Ts...>> {
  using type = std::function<bool(Ts...)>;
};

template<typename>
struct AlwaysFalse : std::false_type {};

//-------------------- EventBus --------------------

class AsyncEventBus {
public:
  explicit AsyncEventBus(asio::io_context& io) :
      universal_strand_(asio::make_strand(io.get_executor())), result_strand_(asio::make_strand(io.get_executor())),
      request_strand_(asio::make_strand(io.get_executor())), socket_strand_(asio::make_strand(io.get_executor())),
      network_strand_(asio::make_strand(io.get_executor())) {
  }

  // subscribe< Tag >(handler)
  template<typename Tag, typename Handler>
  void Subscribe(Handler&& handler) {
    using Fn = typename TupleToFunction<typename Tag::Args>::type;
    static_assert(std::is_convertible_v<Handler, Fn>, "Handler must be bool(Args...)");

    auto& sub = GetSubs<Tag>();
    if (sub) {
      CLogger::Error("EventBus: Duplicate subscription for {}", typeid(Tag).name());
      return;
    }
    CLogger::Debug("Subscribe {}", typeid(Tag).name());
    sub = std::forward<Handler>(handler);
  }

  // unsubscribe< Tag >()
  template<typename Tag>
  void Unsubscribe() {
    auto& sub = GetSubs<Tag>();
    sub = nullptr;
  }

  // publish< Tag >(args...)
  template<typename Tag, typename... Args>
  void Publish(Args&&... args) {
    auto& sub = GetSubs<Tag>(); // 直接获取引用
    if (!sub)
      return;

    auto tup = std::make_tuple(std::forward<Args>(args)...);

    auto work = [sub, tup = std::move(tup)]() mutable {
      try {
        std::apply(sub, tup);
      } catch (const std::exception& e) {
        CLogger::Error("EventBus: Exception in handler for {}: {}", typeid(Tag).name(), ToSimpleUtf8(e.what()));
      } catch (...) {
        CLogger::Error("EventBus: Unknown exception in handler for {}", typeid(Tag).name());
      }
    };

    asio::dispatch(GetStrand<Tag>(), std::move(work));
  }

  void ClearAll() {
    // async_start_subs_.clear();
    // async_stop_subs_.clear();
    async_timer_subs_ = nullptr;
    async_control_subs_ = nullptr;

    db_start_subs_ = nullptr;
    db_stop_subs_ = nullptr;
    db_result_subs_ = nullptr;
    db_request_subs_ = nullptr;

    sock_link_subs_ = nullptr;
    sock_shut_subs_ = nullptr;
    sock_read_subs_ = nullptr;

    net_bind_subs_ = nullptr;
    net_shut_subs_ = nullptr;
    net_read_subs_ = nullptr;
  }

private:
  asio::any_io_executor universal_strand_, result_strand_, request_strand_, socket_strand_, network_strand_;

  // 每个事件的订阅者表：owner(this指针) -> handler
  // std::unordered_map<void*, TupleToFunction<AsyncStartEventTag::Args>::type> async_start_subs_;
  // std::unordered_map<void*, TupleToFunction<AsyncStopEventTag::Args>::type> async_stop_subs_;
  TupleToFunction<AsyncTimerEventTag::Args>::type async_timer_subs_;
  TupleToFunction<AsyncControlEventTag::Args>::type async_control_subs_;

  TupleToFunction<DataBaseStartEventTag::Args>::type db_start_subs_;
  TupleToFunction<DataBaseStopEventTag::Args>::type db_stop_subs_;
  TupleToFunction<DataBaseResultEventTag::Args>::type db_result_subs_;
  TupleToFunction<DataBaseRequestEventTag::Args>::type db_request_subs_;

  TupleToFunction<TCPSocketLinkEventTag::Args>::type sock_link_subs_;
  TupleToFunction<TCPSocketShutEventTag::Args>::type sock_shut_subs_;
  TupleToFunction<TCPSocketReadEventTag::Args>::type sock_read_subs_;

  TupleToFunction<TCPNetworkBindEventTag::Args>::type net_bind_subs_;
  TupleToFunction<TCPNetworkShutEventTag::Args>::type net_shut_subs_;
  TupleToFunction<TCPNetworkReadEventTag::Args>::type net_read_subs_;

  // 获取对应事件的订阅表
  template<typename Tag>
  auto& GetSubs() {
    /*if constexpr (std::is_same_v<Tag, AsyncStartEventTag>) {
      return async_start_subs_;
    } else if constexpr (std::is_same_v<Tag, AsyncStopEventTag>) {
      return async_stop_subs_;
    } else*/
    if constexpr (std::is_same_v<Tag, AsyncTimerEventTag>) {
      return async_timer_subs_;
    } else if constexpr (std::is_same_v<Tag, AsyncControlEventTag>) {
      return async_control_subs_;
    } else if constexpr (std::is_same_v<Tag, DataBaseStartEventTag>) {
      return db_start_subs_;
    } else if constexpr (std::is_same_v<Tag, DataBaseStopEventTag>) {
      return db_stop_subs_;
    } else if constexpr (std::is_same_v<Tag, DataBaseResultEventTag>) {
      return db_result_subs_;
    } else if constexpr (std::is_same_v<Tag, DataBaseRequestEventTag>) {
      return db_request_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPSocketLinkEventTag>) {
      return sock_link_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPSocketShutEventTag>) {
      return sock_shut_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPSocketReadEventTag>) {
      return sock_read_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPNetworkBindEventTag>) {
      return net_bind_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPNetworkShutEventTag>) {
      return net_shut_subs_;
    } else if constexpr (std::is_same_v<Tag, TCPNetworkReadEventTag>) {
      return net_read_subs_;
    } else {
      static_assert(AlwaysFalse<Tag>::value, "Unknown event Tag");
    }
  }

  // strand 分配逻辑保持不变
  template<typename Tag>
  auto& GetStrand() {
    if constexpr (/*std::is_same_v<Tag, AsyncStartEventTag> || std::is_same_v<Tag, AsyncStopEventTag> ||*/
                  std::is_same_v<Tag, AsyncTimerEventTag> || std::is_same_v<Tag, AsyncControlEventTag> ||
                  std::is_same_v<Tag, DataBaseStartEventTag> || std::is_same_v<Tag, DataBaseStopEventTag>) {
      return universal_strand_;
    } else if constexpr (std::is_same_v<Tag, DataBaseResultEventTag>) {
      return result_strand_;
    } else if constexpr (std::is_same_v<Tag, DataBaseRequestEventTag>) {
      return request_strand_;
    } else if constexpr (std::is_same_v<Tag, TCPSocketLinkEventTag> || std::is_same_v<Tag, TCPSocketShutEventTag> ||
                         std::is_same_v<Tag, TCPSocketReadEventTag>) {
      return socket_strand_;
    } else if constexpr (std::is_same_v<Tag, TCPNetworkBindEventTag> || std::is_same_v<Tag, TCPNetworkShutEventTag> ||
                         std::is_same_v<Tag, TCPNetworkReadEventTag>) {
      return network_strand_;
    } else {
      static_assert(AlwaysFalse<Tag>::value, "Unknown event Tag");
    }
  }
};

//-------------------- 全局单例 --------------------

class KERNEL_ENGINE_CLASS GlobalEventBus {
public:
  static void Init(asio::io_context& io) {
    if (!inst_)
      inst_.reset(new AsyncEventBus(io));
  }
  static AsyncEventBus* Get() {
    return inst_.get();
  }
  static void Shutdown() {
    if (inst_) {
      inst_->ClearAll();
      inst_.reset();
    }
  }

private:
  static std::unique_ptr<AsyncEventBus> inst_;
};

// inline std::unique_ptr<AsyncEventBus> GlobalEventBus::inst_ = nullptr;
