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
#define SERVICE_CORE_CLASS _declspec(dllexport)
#else
#define SERVICE_CORE_CLASS _declspec(dllimport)
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
#include "WHBase64.h"
#include "WHCommandLine.h"
#include "WHDataLocker.h"
#include "WHDataQueue.h"
#include "WHEncrypt.h"
#include "WHIniData.h"
#include "WHMD5CheckSum.h"
#include "WHService.h"
#include "WHSha1.h"
#include "WHThread.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
