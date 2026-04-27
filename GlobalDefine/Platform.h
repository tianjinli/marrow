#pragma once

#include "Typedef.h"
//////////////////////////////////////////////////////////////////////////////////
// 包含文件

// 定义文件
#include "Define.h"
#include "Macro.h"

// 结构文件
#include "Packet.h"
#include "Property.h"
#include "Struct.h"

// 模板文件
#include "Array.h"
#include "Module.h"
#include "PacketAide.h"
#include "RightDefine.h"
#include "ServerRule.h"

//////////////////////////////////////////////////////////////////////////////////

// 程序版本
#define VERSION_FRAME PROCESS_VERSION(7, 0, 1)          // 框架版本
#define VERSION_PLAZA PROCESS_VERSION(7, 0, 1)          // 大厅版本
#define VERSION_MOBILE_ANDROID PROCESS_VERSION(7, 0, 1) // 手机版本
#define VERSION_MOBILE_IOS PROCESS_VERSION(7, 0, 1)     // 手机版本

// 版本定义
#define VERSION_EFFICACY 0                        // 效验版本
#define VERSION_FRAME_SDK INTERFACE_VERSION(7, 1) // 框架版本

//////////////////////////////////////////////////////////////////////////////////
// 发布版本
//////////////////////////////////////////////////////////////////////////////////

// 数据库名
const TCHAR szPlatformDB[] = TEXT("WHJHPlatformDB");   // 平台数据库
const TCHAR szAccountsDB[] = TEXT("WHJHAccountsDB");   // 用户数据库
const TCHAR szTreasureDB[] = TEXT("WHJHTreasureDB");   // 财富数据库
const TCHAR szGameMatchDB[] = TEXT("WHJHGameMatchDB"); // 比赛数据库
const TCHAR szExerciseDB[] = TEXT("WHJHEducateDB");    // 练习数据库
const TCHAR szGameScoreDB[] = TEXT("WHJHGameScoreDB"); // 练习数据库

//////////////////////////////////////////////////////////////////////////////////

// 授权信息
const TCHAR szCompilation[] = TEXT("ED56BE63-3026-465B-9DFC-17F595145F3D");

//////////////////////////////////////////////////////////////////////////////////
