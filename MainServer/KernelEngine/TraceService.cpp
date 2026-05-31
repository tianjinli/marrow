#include "TraceService.h"

// 可能存在头文件引用污染需要重新包含
#include <filesystem>

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

//////////////////////////////////////////////////////////////////////////////////

void CLogger::Write(enTraceLevel lvl, const std::string& msg) {
  auto lg = std::static_pointer_cast<spdlog::logger>(logger_);

  switch (lvl) {
    case TraceLevel_Debug:
      lg->debug(msg);
      break;
    case TraceLevel_Info:
      lg->info(msg);
      break;
    case TraceLevel_Warning:
      lg->warn(msg);
      break;
    case TraceLevel_Exception:
      lg->error(msg);
      break;
  }
}

// 初始化异步日志
void CLogger::Initialize() {
  TCHAR name_without_ext[64] = TEXT("");
  CWHService::GetNameWithoutExt(name_without_ext, std::size(name_without_ext));

  TCHAR work_dir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(work_dir, std::size(work_dir));

  auto filename = std::filesystem::path(StringT(work_dir)) / TEXT("logs");
  if (!std::filesystem::exists(filename)) {
    std::filesystem::create_directories(filename);
  }
  filename /= std::format(TEXT("{}.txt"), StringT(name_without_ext));

  const auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filename, 0, 0);
  // const auto rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filename, 8192 * 1024, 10);

  std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks = {daily_sink /*, rotating_sink*/};
  auto log_name = ToSimpleUtf8(name_without_ext);
#ifdef DEBUG_ENABLED
  sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
  spdlog::init_thread_pool(8192, 1); // 初始化线程池，异步日志器需要线程池支持
  logger_ = std::make_shared<spdlog::async_logger>(log_name, sinks.begin(), sinks.end(), spdlog::thread_pool(),
                                                   spdlog::async_overflow_policy::block); // 异步日志器
#ifdef DEBUG_ENABLED
  logger_->set_level(spdlog::level::debug);
#else
  logger_->set_level(spdlog::level::info);
#endif
  logger_->set_pattern("%^[%H:%M:%S.%t] [%l] %v%$");
  using namespace std::chrono_literals;
  spdlog::flush_every(30s);
  // spdlog::flush_on(spdlog::level::info);
  // spdlog::set_default_logger 之后调用 spdlog::default_logger() != logger_
}

//////////////////////////////////////////////////////////////////////////////////

void CTraceService::TraceString(const StringT& msg, const enTraceLevel level) {
  CLogger::logger_->log(static_cast<spdlog::level::level_enum>(level), ToSimpleUtf8(msg));
}

//////////////////////////////////////////////////////////////////////////////////

// // 输出管理
// CTraccService tracc_service;
//
// // 组件创建函数
// extern "C" DYNLIB_EXPORT VOID* CreateTraceService(const GUID& Guid, DWORD dwInterfaceVer) {
//   return tracc_service.QueryInterface(Guid, dwInterfaceVer);
// }

//////////////////////////////////////////////////////////////////////////////////
