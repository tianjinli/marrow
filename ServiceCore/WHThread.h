#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 线程对象
class [[deprecated]] SERVICE_CORE_CLASS CWHThread {
  // 状态变量
private:
  std::atomic_bool is_running_ = false; // 运行标志

  // 线程变量
private:
  std::thread thread_;

  // 函数定义
protected:
  // 构造函数
  CWHThread() = default;
  // 析构函数
  virtual ~CWHThread();

  // 接口函数
public:
  // 获取状态
  virtual bool IsRunning();
  // 启动线程
  virtual bool StartThread();
  // 终止线程
  virtual bool ConcludeThread(DWORD mill_seconds);
  // 获取线程 ID
  virtual size_t GetThreadID() const;
  // 获取线程 ID
  static size_t GetThreadID(const std::thread &thread);

  // 事件函数
protected:
  // 运行事件
  virtual bool OnEventThreadRun() { return true; }
  // 开始事件
  virtual bool OnEventThreadStart() { return true; }
  // 终止事件
  virtual bool OnEventThreadConclude() { return true; }
};

class SERVICE_CORE_CLASS CWHThreadPool {
protected:
  std::vector<std::thread> work_threads_; // 工作线程

  explicit CWHThreadPool() = default;
  ~CWHThreadPool() = default;

  // —— 禁用拷贝语义 ——
  CWHThreadPool(const CWHThreadPool&) = delete;
  CWHThreadPool& operator=(const CWHThreadPool&) = delete;

public:
  void StartThreadPool(std::function<void()> callback);
  void ConcludeThreadPool();
};

//////////////////////////////////////////////////////////////////////////////////
