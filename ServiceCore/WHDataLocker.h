#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 数据锁定
class [[deprecated]] SERVICE_CORE_CLASS CWHDataLocker {
  // 变量定义
private:
  int lock_count_ = 0;          // 锁定计数
  std::mutex& critical_section_; // 锁定对象

  // 函数定义
public:
  // 构造函数
  CWHDataLocker(std::mutex& critical_section, bool lock_at_once = true);
  // 析构函数
  virtual ~CWHDataLocker();

  // 操作函数
public:
  // 锁定函数
  VOID Lock();
  // 解锁函数
  VOID UnLock();
};

//////////////////////////////////////////////////////////////////////////////////
