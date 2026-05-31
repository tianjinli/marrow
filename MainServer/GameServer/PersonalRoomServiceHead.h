#pragma once

//////////////////////////////////////////////////////////////////////////////////

// 平台定义
#include "GlobalDefine/Platform.h"

// 消息定义
#include "MessageDefine/CMD_Commom.h"
#include "MessageDefine/CMD_Correspond.h"
#include "MessageDefine/CMD_GameServer.h"

// 平台文件
#include "GameService/GameServiceHead.h"
#include "KernelEngine/KernelEngineHead.h"
#include "ServiceCore/ServiceCoreHead.h"


//////////////////////////////////////////////////////////////////////////////////
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
// 类型声明
class CLockTimeMatch;

//////////////////////////////////////////////////////////////////////////////////
// 约战房参数
struct tagPersonalRoomManagerParameter {
  // 配置参数
  tagPersonalRoomOption* pPersonalRoomOption; // 约战房配置
  tagGameServiceOption* pGameServiceOption; // 服务配置
  tagGameServiceAttrib* pGameServiceAttrib; // 服务属性

  // 内核组件
  ITimerEngine* pITimerEngine; // 时间引擎
  IDBCorrespondManager* pICorrespondManager; // 数据引擎
  ITCPNetworkEngineEvent* pTCPNetworkEngine; // 网络引擎
  ITCPNetworkEngine* pITCPNetworkEngine; // 网络引擎
  ITCPSocketService* pITCPSocketService; // 网络服务

  // 服务组件
  IAndroidUserManager* pIAndroidUserManager; // 机器管理
  IServerUserManager* pIServerUserManager; // 用户管理
  IMainServiceFrame* pIMainServiceFrame; // 服务框架
  IServerUserItemSink* pIServerUserItemSink; // 用户接口
};


//////////////////////////////////////////////////////////////////////////////////

#define VER_IPersonalRoomServiceManager INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalRoomServiceManager = {0xaf116139, 0xa0, 0x40e3, 0xaa, 0x7b, 0x2f, 0x41, 0xf2, 0x3d, 0xb1, 0x89};

// 约战房服务器管理接口
interface IPersonalRoomServiceManager : public IUnknownEx {
  // 控制接口
public:
  // 停止服务
  virtual bool StopService();
  // 启动服务
  virtual bool StartService();

  // 管理接口
public:
  // 创建比赛
  virtual bool CreatePersonalRoom(BYTE cbPersonalRoomType);
  // 绑定桌子
  virtual bool BindTableFrame(ITableFrame * pTableFrame, WORD wChairID);
  // 初始化接口
  virtual bool InitPersonalRooomInterface(tagPersonalRoomManagerParameter & PersonalRoomManagerParameter);

  // 系统事件
public:
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter);
  // 数据库事件
  virtual bool OnEventDataBase(WORD wRequestID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize, DWORD dwContextID);

  // 网络事件
public:
  // 约战服务事件
  virtual bool OnEventSocketPersonalRoom(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem, DWORD dwSocketID);
  // 约战服务器事件
  virtual bool OnTCPSocketMainServiceInfo(WORD wSubCmdID, VOID * pData, WORD wDataSize);

  // 用户接口
public:
  // 用户登录
  virtual bool OnEventUserLogon(IServerUserItem * pIServerUserItem);
  // 用户登出
  virtual bool OnEventUserLogout(IServerUserItem * pIServerUserItem);
  // 登录完成
  virtual bool OnEventUserLogonFinish(IServerUserItem * pIServerUserItem);

  // 接口信息
public:
  // 用户接口
  virtual IUnknownEx* GetServerUserItemSink();
  // 用户接口
  // virtual IUnknownEx * GetMatchUserItemSink()=NULL;
};


//////////////////////////////////////////////////////////////////////////////////

#define VER_IPersonalRoomItem INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalRoomItem = {0x758b6167, 0x248f, 0x4138, 0xac, 0xcb, 0x1f, 0x72, 0x70, 0x8, 0x25, 0xc9};

// 游戏比赛接口
interface IPersonalRoomItem : public IUnknownEx {
  // 控制接口
public:
  // 启动通知
  virtual void OnStartService();

  // 管理接口
public:
  // 绑定桌子
  virtual bool BindTableFrame(ITableFrame * pTableFrame, WORD wTableID);
  // 初始化接口
  virtual bool InitPersonalRooomInterface(tagPersonalRoomManagerParameter & MatchManagerParameter);

  // 系统事件
public:
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter);
  // 数据库事件
  virtual bool OnEventDataBase(WORD wRequestID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize, DWORD dwContextID);

  // 网络事件
public:
  // 约战服务事件
  virtual bool OnEventSocketPersonalRoom(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem, DWORD dwSocketID);
  // 约战服务器事件
  virtual bool OnTCPSocketMainServiceInfo(WORD wSubCmdID, VOID * pData, WORD wDataSize);
  // 信息接口
public:
  // 用户登录
  virtual bool OnEventUserLogon(IServerUserItem * pIServerUserItem);
  // 用户登出
  virtual bool OnEventUserLogout(IServerUserItem * pIServerUserItem);
  // 登录完成
  virtual bool OnEventUserLogonFinish(IServerUserItem * pIServerUserItem);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IPersonalRoomEventSink INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalRoomEventSink = {0x8d288098, 0x818c, 0x40a8, 0x9b, 0x8d, 0xb5, 0x19, 0xbb, 0x94, 0x40, 0x7d};

// 游戏事件
interface IPersonalRoomEventSink : public IUnknownEx {
public:
  // 游戏开始
  virtual bool OnEventGameStart(ITableFrame * pITableFrame, WORD wChairCount);
  virtual void PersonalRoomWriteJoinInfo(DWORD dwUserID, WORD wTableID, WORD wChairID, DWORD dwKindID, TCHAR * szRoomID, TCHAR * szPersonalRoomGUID);

  // 游戏结束
  virtual bool OnEventGameEnd(ITableFrame * pITableFrame, WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
  virtual bool OnEventGameEnd(WORD wTableID, WORD wChairCount, DWORD dwDrawCountLimit, DWORD & dwPersonalPlayCount, int nSpecialInfoLen,
                              BYTE* cbSpecialInfo, SYSTEMTIME sysStartTime, tagPersonalUserScoreInfo* PersonalUserScoreInfo);

  // 玩家动作
public:
  // 用户坐下
  virtual bool OnActionUserSitDown(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
  // 用户起来
  virtual bool OnActionUserStandUp(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser);
  // 用户同意
  virtual bool OnActionUserOnReady(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize);
};

///////////////////////////////////////////////////////////////////////////

#define VER_IPersonalTableFrameHook INTERFACE_VERSION(1, 1)
static const GUID IID_IPersonalTableFrameHook = {0x958d9add, 0xe98c, 0x4067, 0x8a, 0xca, 0x1c, 0x32, 0x6c, 0xb8, 0x1e, 0x72};

// 桌子钩子接口
interface IPersonalTableFrameHook : public IUnknownEx {
  // 管理接口
public:
  // 设置接口
  virtual bool SetPersonalRoomEventSink(IUnknownEx * pIUnknownEx);
  // 初始化
  virtual bool InitTableFrameHook(IUnknownEx * pIUnknownEx);

  // 游戏事件
public:
  // 游戏开始
  virtual bool OnEventGameStart(WORD wChairCount);
  // 约战房写参与信息
  virtual void PersonalRoomWriteJoinInfo(DWORD dwUserID, WORD wTableID, WORD wChairID, DWORD dwKindID, TCHAR * szRoomID, TCHAR * szPersonalRoomGUID);

  // 游戏结束
  virtual bool OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason);
  // 游戏结束
  virtual bool OnEventGameEnd(WORD wTableID, WORD wChairCount, DWORD dwDrawCountLimit, DWORD & dwPersonalPlayCount, int nSpecialInfoLen,
                              BYTE* cbSpecialInfo, SYSTEMTIME sysStartTime, tagPersonalUserScoreInfo* PersonalUserScoreInfo);
};

//////////////////////////////////////////////////////////////////////////////////

// 游戏服务
DECLARE_MODULE_HELPER(PersonalRoomServiceManager, PERSONAL_ROOM_SERVICE_DLL_NAME, "CreatePersonalRoomServiceManager")

//////////////////////////////////////////////////////////////////////////////////
