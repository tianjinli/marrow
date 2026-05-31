#pragma once

//////////////////////////////////////////////////////////////////////////////////
// 包含文件

// 系统文件
#include "GlobalDefine/Platform.h"

//////////////////////////////////////////////////////////////////////////////////
// 公共定义

// 导出定义
#ifdef _WIN32
#ifdef SERVICE_CORE_DLL
#define SERVICE_CORE_CLASS __declspec(dllexport)
#else
#define SERVICE_CORE_CLASS __declspec(dllimport)
#endif
#define SERVICE_CORE_DLL_NAME TEXT("ServiceCore.dll") // 组件名字
#define cSeparateChar TEXT('\\')
#else
#define SERVICE_CORE_CLASS
#define SERVICE_CORE_DLL_NAME TEXT("ServiceCore.so") // 组件名字
#define cSeparateChar TEXT('/')
#endif

//////////////////////////////////////////////////////////////////////////////////
// 导出文件

#ifndef SERVICE_CORE_DLL
#include "StringUtils.h"
#include "TimeHelper.h"
#include "WHEncrypt.h"
#include "WHIniData.h"
#include "WHService.h"
#include "WHThread.h"
#include "cmdline.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
