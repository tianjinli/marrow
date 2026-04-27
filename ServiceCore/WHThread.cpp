#include "WHThread.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CWHThread::~CWHThread() {
  // 停止线程
  CWHThread::ConcludeThread(INFINITE);
}

// 状态判断
bool CWHThread::IsRunning() {
  // 运行检测
  return is_running_.load(std::memory_order_acquire);
}

// 启动线程
bool CWHThread::StartThread() {
  // 效验状态
  ASSERT(!IsRunning());
  if (IsRunning())
    return false;

  // 启动线程
  is_running_ = true;

  // // 任务需要配合线程一起使用
  // auto task = std::packaged_task<void()>([this]() { ThreadFunction(); });
  // future_ = task.get_future();
  // std::thread(std::move(task)).detach();

  // // 等待条件变量被唤醒
  // {
  //   m_bFinished = false;
  //   std::unique_lock<std::mutex> mtxWaitLock(m_hMutexFinish);
  //   m_hEventFinish.wait(mtxWaitLock, [&] { return m_bFinished.load(); });
  // }

  std::promise<bool> start_promise;
  auto start_future = start_promise.get_future();

  is_running_.store(true, std::memory_order_release);
  std::thread thread([this, &start_promise]() {
    // 随机种子
    srand(static_cast<uint32_t>(std::time(nullptr)));
    // 启动通知
    bool ok = false;
    try {
      ok = OnEventThreadStart();
    } catch (...) {
      ASSERT(FALSE);
      ok = false;
    }
    start_promise.set_value(ok);

    using namespace std::chrono_literals;

    // 线程处理
    if (ok) {
      // 线程运行
      while (is_running_.load(std::memory_order_acquire)) {
#ifdef DEBUG_ENABLED
        if (!OnEventThreadRun()) {
          break;
        }
#else
        try {
          if (!OnEventThreadRun()) {
            break;
          }
        } catch (...) {
        }
#endif
        // 使本线程休眠 1ms 避免过渡消耗 CPU
        std::this_thread::sleep_for(1ms);
      }

      // 停止通知
      try {
        OnEventThreadConclude();
      } catch (...) {
        ASSERT(FALSE);
      }
    }
  });
  thread_ = std::move(thread);
  bool started = start_future.get();
  if (!started) {
    ConcludeThread(INFINITE);
  }
  return started;
}

// 停止线程
bool CWHThread::ConcludeThread(DWORD mill_seconds) {
  if (IsRunning()) {
    is_running_.store(false, std::memory_order_release);
    if (thread_.joinable()) {
      if (mill_seconds == ULONG_MAX) {
        thread_.join();
      } else {
        // 标准库没有直接的 join 超时，这里用 detach 或条件变量实现
        auto start = std::chrono::steady_clock::now();
        while (IsRunning() && std::chrono::steady_clock::now() - start < std::chrono::milliseconds(mill_seconds)) {
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (thread_.joinable()) {
          ASSERT(false);
          return false; // thread_.join();
        }
      }
    }
  }
  return true;
}

size_t CWHThread::GetThreadID() const { return GetThreadID(thread_); }

size_t CWHThread::GetThreadID(const std::thread& thread) {
  static std::hash<std::thread::id> hash;
  return hash(thread.get_id());
}

//////////////////////////////////////////////////////////////////////////////////

void CWHThreadPool::StartThreadPool(std::function<void()> callback) {
  // 系统信息
  DWORD thread_count = std::max(std::thread::hardware_concurrency(), 1u);

  // 预分配空间，避免后续扩容时移动或复制 std::thread
  work_threads_.reserve(thread_count);
  for (DWORD i = 0; i < thread_count; i++) {
    work_threads_.emplace_back(callback);
  }
}

void CWHThreadPool::ConcludeThreadPool() {
  // 读写线程
  for (auto& thread: work_threads_) {
    if (thread.joinable())
      thread.join();
  }
  work_threads_.clear();
}
