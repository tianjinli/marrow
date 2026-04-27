#pragma once

#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 追踪服务
class CTraccService;
class KERNEL_ENGINE_CLASS CLogger {
  friend class CTraccService;
  static std::shared_ptr<spdlog::logger> logger_;
  // 函数定义
protected:
  // 构造函数
  CLogger() = default;

  // 窄字符版本
private:
  template<typename... Args>
  static void Log(spdlog::level::level_enum lvl, fmt::format_string<Args...> fmt, Args&&... args) {
    // 强制替换 fmt 为 FMT_STRING(fmt) 会影响编译效率
    const auto msg = fmt::format(fmt, std::forward<Args>(args)...);
    logger_->log(lvl, std::move(msg));
  }

  // 窄字符版本
public:
  //! 模板函数必须放在头文件，否则会 undefined reference
  //! 如果硬要放在源文件里面的话则需要显式实例化你需要的版本（不现实）
  template<typename... Args>
  static void Debug(fmt::format_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Info(fmt::format_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::info, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Warn(fmt::format_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Error(fmt::format_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::err, fmt, std::forward<Args>(args)...);
  }

#ifdef _WIN32
  // 宽字符版本
private:
  template<typename... Args>
  static void Log(spdlog::level::level_enum lvl, fmt::wformat_string<Args...> fmt, Args&&... args) {
    const auto msg = fmt::format(fmt, std::forward<Args>(args)...);
    logger_->log(lvl, ToSimpleUtf8(msg));
  }

  // 宽字符版本
public:
  template<typename... Args>
  static void Debug(fmt::wformat_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::debug, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Info(fmt::wformat_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::info, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Warn(fmt::wformat_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void Error(fmt::wformat_string<Args...> fmt, Args&&... args) {
    Log(spdlog::level::err, fmt, std::forward<Args>(args)...);
  }

#endif

public:
  // 初始化异步日志
  static void Initialize();
};

//////////////////////////////////////////////////////////////////////////////////

// 追踪服务
class KERNEL_ENGINE_CLASS CTraccService : public ITraceService {
  // 函数定义
public:
  // 构造函数
  explicit CTraccService() = default;
  // 析构函数
  virtual ~CTraccService() = default;

  // 基础接口
public:
  // 释放对象
  VOID Release() override {}
  // 接口查询
  VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer) override;

private:
  static void TraceLog(spdlog::level::level_enum lvl, const StringT& msg);

public:
  void TraceDebug(const StringT& msg) override;
  void TraceInfo(const StringT& msg) override;
  void TraceWarn(const StringT& msg) override;
  void TraceError(const StringT& msg) override;

  // 信息接口
public:
  // 追踪信息
  static void TraceString(const StringT& msg, enTraceLevel level);
};

//////////////////////////////////////////////////////////////////////////////////

// extern CTraccService tracc_service;

//////////////////////////////////////////////////////////////////////////////////
