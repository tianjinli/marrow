#include "TimerEngine.h"

#include "AsyncEventHub.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CTimerEngine::~CTimerEngine() {
  // 停止服务
  CTimerEngine::ConcludeService();
}

// 接口查询
VOID* CTimerEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(ITimerEngine, Guid, dwQueryVer);
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(ITimerEngine, Guid, dwQueryVer);
  return nullptr;
}

// 设置定时器
bool CTimerEngine::SetTimer(DWORD timer_id, DWORD elapse_ms, DWORD repeat_times, WPARAM bind_param) {
  if (!is_running_)
    return false;
  std::lock_guard lock(timers_mutex_);

  // 检查是否已存在
  const auto it = std::find_if(timers_.begin(), timers_.end(), [timer_id](const std::shared_ptr<TimerItem>& item) {
    return item->timer_id == timer_id;
  });
  if (it != timers_.end())
    return false;

  auto item = std::make_shared<TimerItem>();
  item->elapse_ms = elapse_ms;
  item->timer_id = timer_id;
  item->repeat_times = repeat_times;
  item->bind_param = bind_param;
  item->timer = std::make_unique<asio::steady_timer>(*io_context_);

  item->timer->expires_after(std::chrono::milliseconds(elapse_ms));
  item->timer->async_wait([this, item](const asio::error_code& ec) mutable {
    if (!ec) {
      OnTimerThreadSink(item);
    }
  });

  timers_.emplace_back(item);
  return true;
}

// 删除定时器
bool CTimerEngine::KillTimer(DWORD timer_id) {
  std::lock_guard lock(timers_mutex_);
  const auto it = std::find_if(timers_.begin(), timers_.end(), [timer_id](const std::shared_ptr<TimerItem>& item) {
    return item->timer_id == timer_id;
  });
  if (it != timers_.end()) {
    (*it)->timer->cancel();
    timers_.erase(it);
    return true;
  }
  return false;
}

bool CTimerEngine::KillAllTimers() {
  std::lock_guard lock(timers_mutex_);
  for (auto& item: timers_) {
    item->timer->cancel();
  }
  timers_.clear();
  return true;
}

void CTimerEngine::OnTimerThreadSink(std::shared_ptr<TimerItem> item) {
  asio::post(*strand_, [this, item]() mutable {
    // if (event_handler_) {
    // event_handler_->OnEventTimer(item->timer_id, item->bind_param);
    // }
    GlobalEventBus::Get()->Publish<AsyncTimerEventTag>(item->timer_id, item->bind_param);
    if (item->repeat_times == kTimesInfinity || --item->repeat_times > 0) {
      item->timer->expires_after(std::chrono::milliseconds(item->elapse_ms));
      item->timer->async_wait([this, item](const asio::error_code& ec) mutable {
        if (!ec) {
          OnTimerThreadSink(item);
        }
      });
    } else {
      KillTimer(item->timer_id);
    }
  });
}

// 开始服务
bool CTimerEngine::InitiateService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // 设置变量
  is_running_ = true;
  io_context_ = std::move(io_context);
  strand_ = std::make_shared<asio::strand<executor_type>>(io_context_->get_executor());

  // GlobalEventBus::Get()->Publish<AsyncStartEventTag>(QUERY_ME_INTERFACE(IUnknownEx));
  return true;
}

// 停止服务
bool CTimerEngine::ConcludeService() {
  if (!is_running_)
    return true;

  // 设置变量
  is_running_ = false;

  CTimerEngine::KillAllTimers();
  // GlobalEventBus::Get()->Publish<AsyncStopEventTag>(QUERY_ME_INTERFACE(IUnknownEx));
  return true;
}

// 设置接口
bool CTimerEngine::SetTimerEngineEvent(IUnknownEx* sink_any) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // // 查询接口
  // ASSERT(QUERY_OBJECT_PTR_INTERFACE(sink_any, ITimerEngineEvent) != nullptr);
  // event_handler_ = QUERY_OBJECT_PTR_INTERFACE(sink_any, ITimerEngineEvent);
  //
  // // 错误判断
  // if (event_handler_ == nullptr) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(TimerEngine)

//////////////////////////////////////////////////////////////////////////////////
