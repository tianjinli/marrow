#pragma once

#include "ServiceCoreHead.h"

/**
 * @brief 将 UNIX 时间戳转换为 SYSTEMTIME
 *
 * @param unix_time UNIX 时间戳
 * @param system_time 系统时间
 */
SERVICE_CORE_CLASS void GetAsSystemTime(const time_t unix_time, SYSTEMTIME* system_time);

#ifndef _WIN32
/**
 * @brief 获取本地时间
 *
 * @param system_time 系统时间
 */
SERVICE_CORE_CLASS void GetLocalTime(SYSTEMTIME* system_time);
#endif

/**
 * @brief 获取 SYSTEMTIME 对应的 UNIX 时间戳
 *
 * @param system_time 系统时间
 * @return time_t UNIX 时间戳
 */
SERVICE_CORE_CLASS time_t GetUnixTime(const SYSTEMTIME* system_time);

/**
 * @brief 对比系统时间
 *
 * @param system_time1 系统时间1
 * @param system_time2 系统时间2
 * @return int 对比结果
 * @retval -1 时间1 小于时间2
 * @retval 0 时间1 等于时间2
 * @retval 1 时间1 大于时间2
 */
SERVICE_CORE_CLASS int CompareSystemTime(const SYSTEMTIME* system_time1, const SYSTEMTIME* system_time2);

/**
 * @brief 计算 UNIX 时间戳之间的差值
 *
 * @param system_time1 系统时间1
 * @param system_time2 系统时间2
 * @return time_t 时间差值
 */
SERVICE_CORE_CLASS time_t DiffSystemTime(const SYSTEMTIME* system_time1, const SYSTEMTIME* system_time2);
