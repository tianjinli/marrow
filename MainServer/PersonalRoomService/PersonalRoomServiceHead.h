#pragma once

//////////////////////////////////////////////////////////////////////////////////

// 平台定义
#include "GlobalDefine/Platform.h"

// 平台定义
#include "KernelEngine/KernelEngineHead.h"
#include "ServiceCore/ServiceCoreHead.h"

// 消息定义
#include "MessageDefine/CMD_Commom.h"
#include "MessageDefine/CMD_Correspond.h"
#include "MessageDefine/CMD_GameServer.h"

//////////////////////////////////////////////////////////////////////////////////
// 导出定义

// 导出定义
#ifdef _WIN32
#ifdef PERSONAL_ROOM_SERVICE_DLL
#define PERSONAL_ROOM_SERVICE_CLASS __declspec(dllexport)
#else
#define PERSONAL_ROOM_SERVICE_CLASS __declspec(dllimport)
#endif
#define PERSONAL_ROOM_SERVICE_DLL_NAME TEXT("PersonalRoomService.dll") // 组件名字
#else
#define PERSONAL_ROOM_SERVICE_CLASS
#define PERSONAL_ROOM_SERVICE_DLL_NAME TEXT("PersonalRoomService.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////

// 取消报名原因
#define UNSIGNUP_REASON_PLAYER 0 // 玩家取消
#define UNSIGNUP_REASON_SYSTEM 1 // 系统取消

//////////////////////////////////////////////////////////////////////////////////

// 用户在比赛的时候 输赢是否是当前数据库的SCORE
#define TREASURE true

//////////////////////////////////////////////////////////////////////////////////

#define VER_IPersonalRoomServiceManager INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalRoomServiceManager = {0xd513eace, 0xb67d, 0x43d9, 0x0097, 0xfa, 0xd8, 0xa7, 0x9d, 0x31, 0x39, 0x9b};

class CTableFrame;
using CTableFrameArray = std::vector<CTableFrame*>; // 桌子数组
struct tagBindParameter;
// 比赛服务器管理接口
interface IPersonalRoomServiceManager : public IUnknownEx {
  // 控制接口
public:
  // 停止服务
  virtual bool StopService() = 0;
  // 启动服务
  virtual bool StartService() = 0;

  // 管理接口
public:
  // 接口信息
public:
  // 用户接口
  virtual IUnknownEx* GetServerUserItemSink() = 0;
  // 用户接口
  // virtual IUnknownEx * GetMatchUserItemSink()=NULL;
  // 测试
public:
  virtual void TestPersonal() = 0;
  // 查询房间
  virtual bool OnTCPNetworkSubMBQueryGameServer(VOID * pData, WORD wDataSize, DWORD dwSocketID, tagBindParameter * pBindParameter,
                                                ITCPSocketService * pITCPSocketService) = 0;

  // 搜索房间桌号
  virtual bool OnTCPNetworkSubMBSearchServerTable(VOID * pData, WORD wDataSize, DWORD dwSocketID, tagBindParameter * pBindParameter,
                                                  ITCPSocketService * pITCPSocketService) = 0;

  // 强制解散搜索房间桌号
  virtual bool OnTCPNetworkSubMBDissumeSearchServerTable(VOID * pData, WORD wDataSize, DWORD dwSocketID, tagBindParameter * pBindParameter,
                                                         ITCPSocketService * pITCPSocketService) = 0;

  // 私人房间配置
  virtual bool OnTCPNetworkSubMBPersonalParameter(VOID * pData, WORD wDataSize, DWORD dwSocketID, IDataBaseEngine * pIDataBaseEngine) = 0;

  // 查询私人房间列表
  virtual bool OnTCPNetworkSubMBQueryPersonalRoomList(VOID * pData, WORD wDataSize, DWORD dwSocketID, ITCPSocketService * pITCPSocketService) = 0;

  /// 玩家请求房间成绩
  virtual bool OnTCPNetworkSubQueryUserRoomScore(VOID * pData, WORD wDataSize, DWORD dwSocketID, IDataBaseEngine * pIDataBaseEngine) = 0;
};


//////////////////////////////////////////////////////////////////////////////////

#define VER_IPersonalRoomItem INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalRoomItem = {0xd513eace, 0xb67d, 0x43d9, 0x0097, 0xfa, 0xd8, 0xa7, 0x9d, 0x31, 0x39, 0x9b};

// 游戏比赛接口
interface IPersonalRoomItem : public IUnknownEx {
  // 控制接口
public:
  // 启动通知
  virtual void OnStartService() = 0;

  // 管理接口
public:
};

//////////////////////////////////////////////////////////////////////////////////

// #define VER_IPersonalRoomEventSink INTERFACE_VERSION(1, 1)
// static const GUID IID_IPersonalRoomEventSink = {0x9d49ab20, 0x472c, 0x4b3a, 0x00bc, 0xb4, 0x92, 0xfe, 0x8c, 0x41, 0xcd, 0xaa};

// // 游戏事件
// interface IPersonalRoomEventSink : public IUnknownEx {
// public:
//   // 游戏开始
//   virtual bool OnEventGameStart(ITableFrame * pITableFrame, WORD wChairCount) = 0;
//   // 游戏结束
//   virtual bool OnEventGameEnd(ITableFrame * pITableFrame, WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;

//   // 用户事件
// public:
//   // 玩家返赛
//   virtual bool OnEventUserReturnMatch(ITableFrame * pITableFrame, IServerUserItem * pIServerUserItem) = 0;

//   // 玩家动作
// public:
//   // 用户坐下
//   virtual bool OnActionUserSitDown(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
//   // 用户起来
//   virtual bool OnActionUserStandUp(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
//   // 用户同意
//   virtual bool OnActionUserOnReady(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) = 0;
// };

///////////////////////////////////////////////////////////////////////////

// #define VER_ITableFrameHook INTERFACE_VERSION(1, 1)
// static const GUID IID_ITableFrameHook = {0xe9f19de8, 0xfccb, 0x42bd, 0x0099, 0x85, 0xac, 0xe9, 0x26, 0xf3, 0xc4, 0x2b};

// // 桌子钩子接口
// interface ITableFrameHook : public IUnknownEx {
//   // 管理接口
// public:
//   // 设置接口
//   virtual bool SetMatchEventSink(IUnknownEx * pIUnknownEx) = 0;
//   // 初始化
//   virtual bool InitTableFrameHook(IUnknownEx * pIUnknownEx) = 0;

//   // 游戏事件
// public:
//   // 游戏开始
//   virtual bool OnEventGameStart(WORD wChairCount) = 0;
//   // 游戏结束
//   virtual bool OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;

//   // 用户事件
// public:
//   // 玩家返赛
//   virtual bool OnEventUserReturnMatch(IServerUserItem * pIServerUserItem) = 0;
// };

//////////////////////////////////////////////////////////////////////////////////

// 游戏服务
DECLARE_MODULE_HELPER(PersonalRoomServiceManager, PERSONAL_ROOM_SERVICE_DLL_NAME, "CreatePersonalRoomServiceManager")

//////////////////////////////////////////////////////////////////////////////////
