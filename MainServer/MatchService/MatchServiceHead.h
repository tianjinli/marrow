#pragma once

//////////////////////////////////////////////////////////////////////////////////

// 平台定义
#include "GlobalDefine/Platform.h"

// 平台文件
#include "GameService/GameServiceHead.h"
#include "KernelEngine/KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 导出定义
#ifdef _WIN32
#ifdef MATCH_SERVICE_DLL
#define MATCH_SERVICE_CLASS __declspec(dllexport)
#else
#define MATCH_SERVICE_CLASS __declspec(dllimport)
#endif
#define MATCH_SERVICE_DLL_NAME TEXT("MatchService.dll") // 组件名字
#else
#define MATCH_SERVICE_CLASS
#define MATCH_SERVICE_DLL_NAME TEXT("MatchService.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////

// 取消报名原因
#define UNSIGNUP_REASON_PLAYER 0 // 玩家取消
#define UNSIGNUP_REASON_SYSTEM 1 // 系统取消

// 用户在比赛的时候 输赢是否是当前数据库的SCORE
#define TREASURE true

//////////////////////////////////////////////////////////////////////////////////
// 类型声明
class CLockTimeMatch;

//////////////////////////////////////////////////////////////////////////////////
// 比赛参数
struct tagMatchManagerParameter {
  // 配置参数
  tagGameMatchOption* pGameMatchOption; // 比赛配置
  tagGameServiceOption* pGameServiceOption; // 服务配置
  tagGameServiceAttrib* pGameServiceAttrib; // 服务属性

  // 内核组件
  ITimerEngine* pITimerEngine; // 时间引擎
  IDBCorrespondManager* pICorrespondManager; // 数据引擎
  ITCPNetworkEngineEvent* pTCPNetworkEngine; // 网络引擎

  // 服务组件
  IAndroidUserManager* pIAndroidUserManager; // 机器管理
  IServerUserManager* pIServerUserManager; // 用户管理
  IMainServiceFrame* pIMainServiceFrame; // 服务框架
  IServerUserItemSink* pIServerUserItemSink; // 用户接口
};

// 排行信息
struct tagMatchRankInfo {
  WORD wRankID; // 比赛名次
  DWORD dwUserID; // 用户标识
  SCORE lMatchScore; // 比赛分数
  SCORE lRewardGold; // 金币奖励
  SCORE lRewardIngot; // 元宝奖励
  DWORD dwRewardExperience; // 经验奖励
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMatchServiceManager INTERFACE_VERSION(1, 1)
static const GUID IID_IMatchServiceManager = {0xd513eace, 0xb67d, 0x43d9, 0x0097, 0xfa, 0xd8, 0xa7, 0x9d, 0x31, 0x39, 0x9b};


// 比赛服务器管理接口
interface IMatchServiceManager : public IUnknownEx {
  // 控制接口
public:
  // 停止服务
  virtual bool StopService() = 0;
  // 启动服务
  virtual bool StartService() = 0;

  // 管理接口
public:
  // 创建比赛
  virtual bool CreateGameMatch(BYTE cbMatchType) = 0;
  // 绑定桌子
  virtual bool BindTableFrame(ITableFrame * pTableFrame, WORD wChairID) = 0;
  // 初始化接口
  virtual bool InitMatchInterface(tagMatchManagerParameter & MatchManagerParameter) = 0;

  // 系统事件
public:
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
  // 数据库事件
  virtual bool OnEventDataBase(WORD wRequestID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize, DWORD dwContextID) = 0;

  // 网络事件
public:
  // 比赛事件
  virtual bool OnEventSocketMatch(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem, DWORD dwSocketID) = 0;
  // 用户接口
public:
  // 用户登录
  virtual bool OnEventUserLogon(IServerUserItem * pIServerUserItem) = 0;
  // 用户登出
  virtual bool OnEventUserLogout(IServerUserItem * pIServerUserItem) = 0;
  // 登录完成
  virtual bool OnEventUserLogonFinish(IServerUserItem * pIServerUserItem) = 0;
  // 进入事件
  virtual bool OnEventEnterMatch(DWORD dwSocketID, VOID * pData, DWORD dwUserIP, bool bIsMobile) = 0;
  // 用户参赛
  virtual bool OnEventUserJoinMatch(IServerUserItem * pIServerUserItem, BYTE cbReason, DWORD dwSocketID) = 0;
  // 用户退赛
  virtual bool OnEventUserQuitMatch(IServerUserItem * pIServerUserItem, BYTE cbReason, WORD* pBestRank = NULL, DWORD dwSocketID = INVALID_WORD) = 0;

  // 接口信息
public:
  // 用户接口
  virtual IUnknownEx* GetServerUserItemSink() = 0;
  // 用户接口
  // virtual IUnknownEx * GetMatchUserItemSink()=NULL;
};


//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameMatchItem INTERFACE_VERSION(1, 1)
static const GUID IID_IGameMatchItem = {0xd513eace, 0xb67d, 0x43d9, 0x0097, 0xfa, 0xd8, 0xa7, 0x9d, 0x31, 0x39, 0x9b};

// 游戏比赛接口
interface IGameMatchItem : public IUnknownEx {
  // 控制接口
public:
  // 启动通知
  virtual void OnStartService() = 0;

  // 管理接口
public:
  // 绑定桌子
  virtual bool BindTableFrame(ITableFrame * pTableFrame, WORD wTableID) = 0;
  // 初始化接口
  virtual bool InitMatchInterface(tagMatchManagerParameter & MatchManagerParameter) = 0;

  // 系统事件
public:
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
  // 数据库事件
  virtual bool OnEventDataBase(WORD wRequestID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize, DWORD dwContextID) = 0;

  // 网络事件
public:
  // 比赛事件
  virtual bool OnEventSocketMatch(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem, DWORD dwSocketID) = 0;

  // 信息接口
public:
  // 用户登录
  virtual bool OnEventUserLogon(IServerUserItem * pIServerUserItem) = 0;
  // 用户登出
  virtual bool OnEventUserLogout(IServerUserItem * pIServerUserItem) = 0;
  // 登录完成
  virtual bool OnEventUserLogonFinish(IServerUserItem * pIServerUserItem) = 0;
  // 进入事件
  virtual bool OnEventEnterMatch(DWORD dwSocketID, VOID * pData, DWORD dwUserIP, bool bIsMobile) = 0;
  // 用户参赛
  virtual bool OnEventUserJoinMatch(IServerUserItem * pIServerUserItem, BYTE cbReason, DWORD dwSocketID) = 0;
  // 用户退赛
  virtual bool OnEventUserQuitMatch(IServerUserItem * pIServerUserItem, BYTE cbReason, WORD* pBestRank = NULL, DWORD dwSocketID = INVALID_WORD) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMatchEventSink INTERFACE_VERSION(1, 1)
static const GUID IID_IMatchEventSink = {0x9d49ab20, 0x472c, 0x4b3a, 0x00bc, 0xb4, 0x92, 0xfe, 0x8c, 0x41, 0xcd, 0xaa};

// 游戏事件
interface IMatchEventSink : public IUnknownEx {
public:
  // 游戏开始
  virtual bool OnEventGameStart(ITableFrame * pITableFrame, WORD wChairCount) = 0;
  // 游戏结束
  virtual bool OnEventGameEnd(ITableFrame * pITableFrame, WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;

  // 用户事件
public:
  // 玩家返赛
  virtual bool OnEventUserReturnMatch(ITableFrame * pITableFrame, IServerUserItem * pIServerUserItem) = 0;

  // 玩家动作
public:
  // 用户坐下
  virtual bool OnActionUserSitDown(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
  // 用户起来
  virtual bool OnActionUserStandUp(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
  // 用户同意
  virtual bool OnActionUserOnReady(WORD wTableID, WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) = 0;
};

///////////////////////////////////////////////////////////////////////////

#define VER_ITableFrameHook INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrameHook = {0xe9f19de8, 0xfccb, 0x42bd, 0x0099, 0x85, 0xac, 0xe9, 0x26, 0xf3, 0xc4, 0x2b};

// 桌子钩子接口
interface ITableFrameHook : public IUnknownEx {
  // 管理接口
public:
  // 设置接口
  virtual bool SetMatchEventSink(IUnknownEx * pIUnknownEx) = 0;
  // 初始化
  virtual bool InitTableFrameHook(IUnknownEx * pIUnknownEx) = 0;

  // 游戏事件
public:
  // 游戏开始
  virtual bool OnEventGameStart(WORD wChairCount) = 0;
  // 游戏结束
  virtual bool OnEventGameEnd(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;

  // 用户事件
public:
  // 玩家返赛
  virtual bool OnEventUserReturnMatch(IServerUserItem * pIServerUserItem) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

// 游戏服务
DECLARE_MODULE_HELPER(MatchServiceManager, MATCH_SERVICE_DLL_NAME, "CreateMatchServiceManager")

//////////////////////////////////////////////////////////////////////////////////
