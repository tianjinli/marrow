#include "TimeHelper.h"

void GetAsSystemTime(const time_t unix_time, SYSTEMTIME* system_time) {
  ASSERT(system_time);

  std::tm tm{};

#if defined(_WIN32)
  // Windows: localtime_s(dst, src)
  localtime_s(&tm, &unix_time);
#else
  // Linux/macOS: localtime_r(src, dst)
  localtime_r(&unix_time, &tm);
#endif

  system_time->wYear = tm.tm_year + 1900;
  system_time->wMonth = tm.tm_mon + 1;
  system_time->wDay = tm.tm_mday;
  system_time->wHour = tm.tm_hour;
  system_time->wMinute = tm.tm_min;
  system_time->wSecond = tm.tm_sec;
  system_time->wMilliseconds = 0;
  system_time->wDayOfWeek = tm.tm_wday;
}

#ifndef _WIN32
#include <chrono>
#include <ctime>

void GetLocalTime(SYSTEMTIME* system_time) {
  using namespace std::chrono;

  // 获取当前时间点
  auto now = system_clock::now();

  // 转换为 time_t（秒）
  auto now_time_t = system_clock::to_time_t(now);
  std::tm local_tm{};
  localtime_r(&now_time_t, &local_tm);
  // 计算毫秒
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  system_time->wYear = local_tm.tm_year + 1900;
  system_time->wMonth = local_tm.tm_mon + 1;
  system_time->wDay = local_tm.tm_mday;
  system_time->wHour = local_tm.tm_hour;
  system_time->wMinute = local_tm.tm_min;
  system_time->wSecond = local_tm.tm_sec;
  system_time->wMilliseconds = (int) ms.count();
}
#endif

time_t GetUnixTime(const SYSTEMTIME* system_time) {
  ASSERT(system_time);

  std::tm local_tm{};
  local_tm.tm_year = system_time->wYear - 1900;
  local_tm.tm_mon = system_time->wMonth - 1;
  local_tm.tm_mday = system_time->wDay;
  local_tm.tm_hour = system_time->wHour;
  local_tm.tm_min = system_time->wMinute;
  local_tm.tm_sec = system_time->wSecond;

  // SYSTEMTIME 是本地时间 → 使用 mktime()
  return std::mktime(&local_tm);
}

int CompareSystemTime(const SYSTEMTIME* system_time1, const SYSTEMTIME* system_time2) {
  ASSERT(system_time1 && system_time2);

  time_t time1 = GetUnixTime(system_time1);
  time_t time2 = GetUnixTime(system_time2);

  if (time1 < time2) {
    return -1;
  } else if (time1 > time2) {
    return 1;
  } else {
    return 0;
  }
}

time_t DiffSystemTime(const SYSTEMTIME* system_time1, const SYSTEMTIME* system_time2) {
  ASSERT(system_time1 && system_time2);
  return GetUnixTime(system_time1) - GetUnixTime(system_time2);
}
