#pragma once

#include "GameService/GameServiceHead.h"

#include "CMD_Game.h"

// 导出定义
#ifdef _WIN32
#define PLATFORM_EXPORT __declspec(dllexport)
#else
#define PLATFORM_EXPORT
#endif

//////////////////////////////////////////////////////////////////////////////////

// 组件定义
#ifdef _WIN32
#define GAME_COMPONENT_DLL_NAME TEXT("LandGameComponent.dll") // 组件名字
#else
#define GAME_COMPONENT_DLL_NAME TEXT("LandGameComponent.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////

// 控制定义
#ifdef _WIN32
#define GAME_CONTROL_DLL_NAME TEXT("LandGameControl.dll") // 组件名字
#else
#define GAME_CONTROL_DLL_NAME TEXT("LandGameControl.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////

// 机器定义
#ifdef _WIN32
#define ANDROID_SERVICE_DLL_NAME TEXT("LandAndroidService.dll") // 组件名字
#else
#define ANDROID_SERVICE_DLL_NAME TEXT("LandAndroidService.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////

// 配置结构
struct tagCustomRule {
  // 其他定义
  WORD wMaxScoreTimes; // 最大倍数
  WORD wFleeScoreTimes; // 逃跑倍数
  BYTE cbFleeScorePatch; // 逃跑补偿

  // 时间定义
  BYTE cbTimeOutCard; // 出牌时间
  BYTE cbTimeCallScore; // 叫分时间
  BYTE cbTimeStartGame; // 开始时间
  BYTE cbTimeHeadOutCard; // 首出时间
};

//////////////////////////////////////////////////////////////////////////////////
