#include "TimerEngine.h"

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

// 开始服务
bool CTimerEngine::StartService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(io_context != nullptr && !is_running_.load(std::memory_order_relaxed));
  if (io_context == nullptr || is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  io_context_ = std::move(io_context);

  strand_ = asio::make_strand(io_context_->get_executor());
  // 初始化事件通知
  OnTimerEvent.Setup(strand_);
  return is_running_.exchange(true, std::memory_order_relaxed) == false; // exchange 返回调用前原子变量的值
}

// 停止服务
bool CTimerEngine::ConcludeService() {
  // 设置变量
  if (!is_running_.exchange(false, std::memory_order_relaxed)) {
    return true;
  }

  OnTimerEvent.Clear();
  // 清空全部活动定时器
  asio::post(strand_, [this]() {
    for (const auto& item: timers_map_ | std::views::values) {
      asio::error_code ignored_ec;
      item->timer.cancel(ignored_ec);
    }
    timers_map_.clear();
  });

  return true;
}

// 配置时钟基础节拍空间
bool CTimerEngine::SetTimeCell(DWORD time_cell) {
  timer_space_.store(std::max(time_cell, static_cast<DWORD>(TIME_CELL)), std::memory_order_relaxed);
  return true;
}

// 设置接口
bool CTimerEngine::SetTimerEngineEvent(void* object_ptr) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITimerEngineEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITimerEngineEvent);

  if (event_ptr == nullptr) {
    return false;
  }
  OnTimerEvent += MEMBER_DELEGATE(&ITimerEngineEvent::OnEventTimer, event_ptr);
  return true;
}

// 设置定时器
bool CTimerEngine::SetTimer(DWORD timer_id, DWORD elapse_ms, DWORD repeat_times, WPARAM bind_param) {
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 读取设定的时钟基础节拍空间
  DWORD current_cell = timer_space_.load(std::memory_order_relaxed);

  // 强行投递进入非指针形式的底层串行化单链中排队
  asio::post(strand_, [this, timer_id, elapse_ms, repeat_times, bind_param, current_cell]() {
    // 1. 如果已经存在同 ID 的定时器，优雅中断并将其抹去
    if (const auto itr = timers_map_.find(timer_id); itr != timers_map_.end()) {
      asio::error_code ignored_ec;
      itr->second->timer.cancel(ignored_ec);
      timers_map_.erase(itr);
    }

    // 2. 对齐 TimeCell 逻辑
    const DWORD final_elapse = (std::max(elapse_ms, current_cell) + current_cell - 1) / current_cell * current_cell;

    // 3. 构建纯粹由 asio 驱动的新生计时节点
    auto item = std::make_shared<TimerItem>(TimerItem{.timer_id = timer_id,
                                                      .elapse_ms = final_elapse,
                                                      .repeat_times = repeat_times,
                                                      .bind_param = bind_param,
                                                      .timer = asio::steady_timer(*io_context_)});
    timers_map_[timer_id] = item;

    // 5. 展开捕获驱动轮询状态机（无外部引入函数，就地链式步进）
    item->timer.expires_after(std::chrono::milliseconds(item->elapse_ms));
    item->timer.async_wait(asio::bind_executor(strand_, [this, item](const asio::error_code& error) {
      OnTimerExpired(item, error); // 后续迭代全是真实到期
    }));
  });

  return true;
}

// 删除定时器
bool CTimerEngine::KillTimer(DWORD timer_id) {
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  asio::post(strand_, [this, timer_id]() {
    if (const auto itr = timers_map_.find(timer_id); itr != timers_map_.end()) {
      asio::error_code ignored_ec;
      itr->second->timer.cancel(ignored_ec);
      timers_map_.erase(itr);
    }
  });
  return true;
}

void CTimerEngine::OnTimerExpired(std::shared_ptr<TimerItem> item, const asio::error_code& ec) {
  // 过滤主动打断、取消状态或者外部停止服务的噪声
  if (ec == asio::error::operation_aborted || !is_running_.load(std::memory_order_relaxed)) {
    return;
  }

  // 调用装配完毕的 EventDelegate 触发上层业务
  OnTimerEvent(item->timer_id, item->bind_param);

  // 自动处理无限重复循环与限次重复循环的数学模型退化
  bool is_next_loop = item->repeat_times == kTimesInfinity;
  if (!is_next_loop) {
    if (item->repeat_times > 1) {
      is_next_loop = true;
      item->repeat_times--;
    }
  }
  if (is_next_loop) {
    // 直接在现有的 timer 控件上设置相对到期时间，原地复用内存与底层 OS 描述符
    item->timer.expires_after(std::chrono::milliseconds(item->elapse_ms));
    // 维持 Strand 强绑定锁定，原地展开下一轮链式监听
    item->timer.async_wait(asio::bind_executor(strand_, [this, item](const asio::error_code& error) {
      OnTimerExpired(item, error); // 后续迭代全是真实到期
    }));
  } else {
    // 次数耗尽，优雅从主 Map 中抹除，释放最后的智能指针
    timers_map_.erase(item->timer_id);
  }
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(TimerEngine);

//////////////////////////////////////////////////////////////////////////////////
