#pragma once

#include "KernelEngineHead.h"

// 宏定义
#define NO_TIME_LEAVE DWORD(-1) // 不响应时间

//////////////////////////////////////////////////////////////////////////////////

constexpr DWORD kTimesInfinity = static_cast<uint32_t>(-1);

// 定时器引擎
class CTimerEngine : public ITimerEngine {
private:
  struct TimerItem {
    DWORD timer_id;
    DWORD elapse_ms;
    DWORD repeat_times;
    WPARAM bind_param;
    std::unique_ptr<asio::steady_timer> timer;
  };
  // 状态变量
protected:
  bool is_running_ = false; // 运行标志
  // 配置定义
protected:
  // ITimerEngineEvent* event_handler_ = nullptr; // 事件接口

  // 组件变量
protected:
  std::shared_ptr<asio::io_context> io_context_;
  std::shared_ptr<asio::strand<executor_type>> strand_;
  std::mutex timers_mutex_;
  std::vector<std::shared_ptr<TimerItem>> timers_;

  // 函数定义
public:
  // 构造函数
  explicit CTimerEngine() = default;
  // 析构函数
  virtual ~CTimerEngine();

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() { delete this; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 服务接口
public:
  // 启动服务
  virtual bool InitiateService(std::shared_ptr<asio::io_context> io_context);
  // 停止服务
  virtual bool ConcludeService();

  // 配置接口
public:
  // 设置接口
  virtual bool SetTimerEngineEvent(IUnknownEx* sink_any);

  // 接口函数
public:
  // 设置定时器
  virtual bool SetTimer(DWORD timer_id, DWORD elapse_ms, DWORD repeat_times, WPARAM bind_param);
  // 删除定时器
  virtual bool KillTimer(DWORD timer_id);
  // 删除定时器
  virtual bool KillAllTimers();

  // 内部函数
private:
  // 定时器通知
  void OnTimerThreadSink(std::shared_ptr<TimerItem> item);
};

//////////////////////////////////////////////////////////////////////////////////
