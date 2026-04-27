#include "WHDataLocker.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CWHDataLocker::CWHDataLocker(std::mutex& critical_section, const bool lock_at_once) : critical_section_(critical_section) {
  // 锁定对象
  if (lock_at_once) {
    Lock();
  }
}

// 析构函数
CWHDataLocker::~CWHDataLocker() {
  // 解除锁定
  while (lock_count_ > 0) {
    UnLock();
  }
}

// 锁定函数
VOID CWHDataLocker::Lock() {
  // 锁定对象
  critical_section_.lock();

  // 设置变量
  lock_count_++;
}

// 解锁函数
VOID CWHDataLocker::UnLock() {
  // 效验状态
  ASSERT(lock_count_ > 0);
  if (lock_count_ == 0)
    return;

  // 设置变量
  lock_count_--;
  // 解除锁定
  critical_section_.unlock();
}

//////////////////////////////////////////////////////////////////////////////////
