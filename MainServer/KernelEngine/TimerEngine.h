#pragma once

#include "EventDelegate.h"
#include "KernelEngineHead.h"

// 宏定义
#define NO_TIME_LEAVE DWORD(-1) // 不响应时间

//////////////////////////////////////////////////////////////////////////////////

constexpr DWORD kTimesInfinity = static_cast<uint32_t>(-1);

// 定时器引擎
class CTimerEngine : public ITimerEngine {
  struct TimerItem {
    DWORD timer_id;
    DWORD elapse_ms;
    DWORD repeat_times;
    WPARAM bind_param;
    asio::steady_timer timer;
  };

public:
  // 构造函数
  explicit CTimerEngine() = default;
  // 析构函数
  virtual ~CTimerEngine();

  // 基础接口
  // 释放对象
  VOID Release() override { delete this; }
  // 接口查询
  VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer) override;

  // 服务接口
  // 启动服务
  bool StartService(std::shared_ptr<asio::io_context> io_context) override;
  // 停止服务
  bool ConcludeService() override;

  // 配置接口
  bool SetTimeCell(DWORD time_cell) override;
  // 设置接口
  bool SetTimerEngineEvent(void* object_ptr) override;

  // 接口函数
  // 设置定时器
  bool SetTimer(DWORD timer_id, DWORD elapse_ms, DWORD repeat_times, WPARAM bind_param) override;
  // 删除定时器
  bool KillTimer(DWORD timer_id) override;

private:
  // 🔥 新增：统一的定时器到期自迭代驱动状态机
  void OnTimerExpired(std::shared_ptr<TimerItem> item, const asio::error_code& ec);

public:
  // 代替旧版虚函数事件通知，支持多订阅、C++20 std::atomic<std::chrono::milliseconds> 耗时监控的高能总线
  mutable EventDelegate<DWORD, WPARAM> OnTimerEvent;

private:
  std::atomic<bool> is_running_{false};

  // 配置变量
  std::atomic<DWORD> timer_space_{TIME_CELL};

  // 组件变量
  std::shared_ptr<asio::io_context> io_context_;
  asio::strand<asio::any_io_executor> strand_{asio::make_strand(asio::system_executor())};

  // 原有的 CWHArray 重构为更符合哈希检索 O(1) 性能要求的无序映射表
  std::unordered_map<DWORD, std::shared_ptr<TimerItem>> timers_map_;
};

//////////////////////////////////////////////////////////////////////////////////
