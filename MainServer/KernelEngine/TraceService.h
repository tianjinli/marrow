#pragma once

#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

namespace spdlog {
  class logger;
}

// 追踪服务
class KERNEL_ENGINE_CLASS CLogger {
  friend class CTraceService;

public:
  //! 模板函数必须放在头文件，否则会 undefined reference
  //! 如果硬要放在源文件里面的话则需要显式实例化你需要的版本（不现实）

  // 窄字符版本
  template<typename... Args>
  static void Debug(std::format_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Debug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Info(std::format_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Info, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Warn(std::format_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Warning, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Error(std::format_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Exception, fmt, std::forward<Args>(args)...);
  }

#ifdef _WIN32
  // 宽字符版本
  template<typename... Args>
  static void Debug(std::wformat_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Debug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Info(std::wformat_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Info, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Warn(std::wformat_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Warning, fmt, std::forward<Args>(args)...);
  }

  // Warning 是 Warn 的别名
  template<typename... Args>
  static void Warning(std::wformat_string<Args...> fmt, Args&&... args) {
    Warn(fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Error(std::wformat_string<Args...> fmt, Args&&... args) {
    Log(TraceLevel_Exception, fmt, std::forward<Args>(args)...);
  }
#endif

  // 初始化异步日志
  static void Initialize();

private:
  // 构造函数
  CLogger() = default;

  // 窄字符版本 - Private helper
  template<typename... Args>
  static void Log(enTraceLevel lvl, std::format_string<Args...> fmt, Args&&... args) {
    // 强制替换 fmt 为 FMT_STRING(fmt) 会影响编译效率
    const auto msg = std::format(fmt, std::forward<Args>(args)...);
    Write(lvl, std::move(msg));
  }

#ifdef _WIN32
  // 宽字符版本 - Private helper
  template<typename... Args>
  static void Log(enTraceLevel lvl, std::wformat_string<Args...> fmt, Args&&... args) {
    const auto msg = std::format(fmt, std::forward<Args>(args)...);
    Write(lvl, ToSimpleUtf8(msg));
  }
#endif

  static void Write(enTraceLevel lvl, const std::string& msg); // ⭐ 非模板函数声明

  // C++ 17 确保全局物理唯一
  inline static std::shared_ptr<spdlog::logger> logger_;
};

//////////////////////////////////////////////////////////////////////////////////

// 追踪服务
class [[deprecated]] KERNEL_ENGINE_CLASS CTraceService {
public:
  // 构造函数
  explicit CTraceService() = default;
  // 析构函数
  virtual ~CTraceService() = default;

  // 信息接口
  // 追踪信息 - 导出函数若是内联不能放在源文件里，否则会 undefined reference
  static void TraceString(const StringT& msg, enTraceLevel level);
};

//////////////////////////////////////////////////////////////////////////////////
