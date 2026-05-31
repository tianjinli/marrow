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
#ifdef GAME_SERVICE_DLL
#define GAME_SERVICE_CLASS __declspec(dllexport)
#else
#define GAME_SERVICE_CLASS __declspec(dllimport)
#endif
#define GAME_SERVICE_DLL_NAME TEXT("GameService.dll") // 组件名字
#else
#define GAME_SERVICE_CLASS
#define GAME_SERVICE_DLL_NAME TEXT("GameService.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////
// 接口说明

// 用户接口
interface IServerUserItem;
interface IServerUserManager;
interface IServerUserItemSink;

// 代打接口
interface IAndroidUserItem;
interface IAndroidUserManager;
interface IAndroidUserItemSink;

// 桌子接口
interface ITableFrame;
interface ITableFrameSink;
interface ITableUserAction;

// 服务接口
interface IMainServiceFrame;
interface IGameServiceManager;
interface IGameServiceCustomRule;
interface IGameServiceCustomTime;
interface IGameServicePersonalRule;

//////////////////////////////////////////////////////////////////////////////////
// 常量定义

// 群发掩码
#define BG_MOBILE (BYTE)(0x01) // 手机群发
#define BG_COMPUTER (BYTE)(0x02) // 电脑群发
#define BG_ALL_CLIENT (BYTE)(0xFF) // 全体群发

// 索引定义
#define INDEX_SOCKET (WORD)(0x0000) // 网络索引
#define INDEX_ANDROID (WORD)(0x2000) // 机器索引

// 创建函数
#define GAME_SERVICE_CREATE_NAME "CreateGameServiceManager" // 创建函数

//////////////////////////////////////////////////////////////////////////////////
// 常量定义
// 机器动作
#define ANDROID_WAITSTANDUP 0x01 // 等待起立
#define ANDROID_WAITLEAVE 0x02 // 等待离开

// 结束原因
#define GER_NORMAL 0x00 // 常规结束
#define GER_DISMISS 0x01 // 游戏解散
#define GER_USER_LEAVE 0x02 // 用户离开
#define GER_NETWORK_ERROR 0x03 // 网络错误

// 离开原因
#define LER_NORMAL 0x00 // 常规离开
#define LER_SYSTEM 0x01 // 系统原因
#define LER_NETWORK 0x02 // 网络原因
#define LER_USER_IMPACT 0x03 // 用户冲突
#define LER_SERVER_FULL 0x04 // 人满为患
#define LER_SERVER_CONDITIONS 0x05 // 条件限制

// 积分类型
#define SCORE_TYPE_NULL 0x00 // 无效积分
#define SCORE_TYPE_WIN 0x01 // 胜局积分
#define SCORE_TYPE_LOSE 0x02 // 输局积分
#define SCORE_TYPE_DRAW 0x03 // 和局积分
#define SCORE_TYPE_FLEE 0x04 // 逃局积分
#define SCORE_TYPE_PRESENT 0x10 // 赠送积分
#define SCORE_TYPE_SERVICE 0x11 // 服务积分

// 开始模式
#define START_MODE_ALL_READY 0x00 // 所有准备
#define START_MODE_FULL_READY 0x01 // 满人开始
#define START_MODE_PAIR_READY 0x02 // 配对开始
#define START_MODE_TIME_CONTROL 0x10 // 时间控制
#define START_MODE_MASTER_CONTROL 0x11 // 管理控制

// 分组选项
#define DISTRIBUTE_ALLOW 0x01 // 允许分组
#define DISTRIBUTE_LAST_TABLE 0x02 // 同桌选项
#define DISTRIBUTE_SAME_ADDRESS 0x04 // 地址选项

//////////////////////////////////////////////////////////////////////////////////
// 时间标识

// 调度范围
#define IDI_MAIN_MODULE_START 1 // 起始标识
#define IDI_MAIN_MODULE_FINISH 99 // 终止标识

// 机器范围
#define IDI_REBOT_MODULE_START 100 // 起始标识
#define IDI_REBOT_MODULE_FINISH 199 // 终止标识

// 比赛范围
#define IDI_MATCH_MODULE_START 200 // 起始标识
#define IDI_MATCH_MODULE_FINISH 999 // 结束标识
// 桌子范围
#define IDI_TABLE_MODULE_START 10000 // 起始标识
#define IDI_TABLE_MODULE_FINISH 50000 // 终止标识

//////////////////////////////////////////////////////////////////////////////////
// 时间范围

// 游戏时间
#define TIME_TABLE_SINK_RANGE 20 // 标识范围
#define TIME_TABLE_KERNEL_RANGE 30 // 标识范围
#define TIME_TABLE_MODULE_RANGE 50 // 标识范围

//////////////////////////////////////////////////////////////////////////////////
// 常量定义

// 常量定义
#define RESERVE_USER_COUNT 40L // 保留数目

//////////////////////////////////////////////////////////////////////////////////
// 结构定义

// 配置参数
struct tagGameParameter {
  // 汇率信息
  DWORD dwMedalRate; // 奖牌汇率
  DWORD dwRevenueRate; // 银行税收
  DWORD dwExchangeRate; // 兑换比率
  DWORD dwPresentExchangeRate; // 魅力游戏币兑换率
  DWORD dwRateGold; // 游戏豆游戏币兑换率

  // 经验奖励
  DWORD dwWinExperience; // 赢局经验

  // 版本信息
  DWORD dwClientVersion; // 客户版本
  DWORD dwServerVersion; // 服务版本

  SCORE lEducateGrantScore; // 练习赠送
};

// 服务属性
struct tagGameServiceAttrib {
  // 内核属性
  WORD wKindID; // 名称号码
  WORD wChairCount; // 椅子数目
  WORD wSupporType; // 支持类型
  TCHAR szGameName[LEN_KIND]; // 游戏名字

  // 功能标志
  BYTE cbAndroidUser; // 机器标志
  BYTE cbDynamicJoin; // 动态加入
  BYTE cbOffLineTrustee; // 断线代打

  // 服务属性
  DWORD dwServerVersion; // 游戏版本
  DWORD dwClientVersion; // 游戏版本
  TCHAR szDataBaseName[32]; // 游戏库名
  TCHAR szServerDLLName[LEN_PROCESS]; // 进程名字
  TCHAR szClientEXEName[LEN_PROCESS]; // 进程名字
};

// 服务配置
struct tagGameServiceOption {
  // 挂接属性
  WORD wKindID; // 挂接类型
  WORD wNodeID; // 挂接节点
  WORD wSortID; // 排列标识
  WORD wServerID; // 房间标识

  // 税收配置
  LONG lCellScore; // 单位积分
  WORD wRevenueRatio; // 税收比例
  SCORE lServiceScore; // 服务费用

  // 房间配置
  SCORE lRestrictScore; // 限制积分
  SCORE lMinTableScore; // 最低积分
  SCORE lMinEnterScore; // 最低积分
  SCORE lMaxEnterScore; // 最高积分

  // 会员限制
  BYTE cbMinEnterMember; // 最低会员
  BYTE cbMaxEnterMember; // 最高会员

  // 房间配置
  DWORD dwServerRule; // 房间规则
  DWORD dwAttachUserRight; // 附加权限

  // 房间属性
  WORD wMaxPlayer; // 最大数目
  WORD wTableCount; // 桌子数目
  WORD wServerPort; // 服务端口
  WORD wServerKind; // 房间类别
  WORD wServerType; // 房间类型
  WORD wServerLevel; // 房间等级
  TCHAR szServerName[LEN_SERVER]; // 房间名称
  TCHAR szServerPasswd[LEN_PASSWORD]; // 房间密码

  // 分组设置
  BYTE cbDistributeRule; // 分组规则
  WORD wMinDistributeUser; // 最少人数
  WORD wDistributeTimeSpace; // 分组间隔
  WORD wDistributeDrawCount; // 分组局数
  WORD wMinPartakeGameUser; // 最少人数
  WORD wMaxPartakeGameUser; // 最多人数


  // 连接设置
  TCHAR szDataBaseAddr[16]; // 连接地址
  TCHAR szDataBaseName[32]; // 数据库名

  // 自定规则
  BYTE cbCustomRule[1024]; // 自定规则

  // 约战房间自定规则
  BYTE cbPersonaRule[1024]; // 自定规则
};


// 比赛奖励
struct tagGameMatchReward {
  WORD wRewardCount; // 奖励人数
  tagMatchRewardInfo MatchRewardInfo[64]; // 奖励信息
};


// 比赛配置
struct tagGameMatchOption {
  // 基本信息
  DWORD dwMatchID; // 比赛标识
  LONGLONG lMatchNo; // 比赛场次
  BYTE cbMatchStatus; // 比赛状态
  BYTE cbMatchType; // 比赛类型
  TCHAR szMatchName[32]; // 比赛名称

  // 报名信息
  BYTE cbFeeType; // 费用类型
  BYTE cbDeductArea; // 缴费区域
  LONGLONG lSignupFee; // 报名费用
  BYTE cbSignupMode; // 报名方式
  BYTE cbJoinCondition; // 参赛条件
  BYTE cbMemberOrder; // 会员等级
  DWORD dwExperience; // 玩家经验
  DWORD dwFromMatchID; // 比赛标识
  BYTE cbFilterType; // 筛选方式
  WORD wMaxRankID; // 最大名次
  SYSTEMTIME MatchEndDate; // 结束日期
  SYSTEMTIME MatchStartDate; // 起始日期

  // 排名方式
  BYTE cbRankingMode; // 排名方式
  WORD wCountInnings; // 统计局数
  BYTE cbFilterGradesMode; // 筛选方式

  // 分组设置
  BYTE cbDistributeRule; // 分组规则
  WORD wMinDistributeUser; // 最少人数
  WORD wDistributeTimeSpace; // 分组间隔
  WORD wMinPartakeGameUser; // 最少人数
  WORD wMaxPartakeGameUser; // 最多人数

  // 比赛规则
  BYTE cbMatchRule[512]; // 比赛规则

  // 奖励信息
  WORD wRewardCount; // 奖励人数
  tagMatchRewardInfo MatchRewardInfo[3]; // 比赛奖励
};

// 桌子参数
struct tagTableFrameParameter {
  // 内核组件
  ITimerEngine* pITimerEngine; // 时间引擎
  IDataBaseEngine* pIRecordDataBaseEngine; // 数据引擎
  IDataBaseEngine* pIKernelDataBaseEngine; // 数据引擎

  // 服务组件
  IMainServiceFrame* pIMainServiceFrame; // 服务框架
  IAndroidUserManager* pIAndroidUserManager; // 机器管理
  IGameServiceManager* pIGameServiceManager; // 服务管理

  // 配置变量
  tagGameParameter* pGameParameter; // 配置参数
  tagGameMatchOption* pGameMatchOption; // 比赛配置
  tagGameServiceAttrib* pGameServiceAttrib; // 桌子属性
  tagGameServiceOption* pGameServiceOption; // 配置参数
};

// 机器参数
struct tagAndroidUserParameter {
  // 服务配置
  bool bServiceContinue; // 服务延续
  DWORD dwMinSitInterval; // 坐下间隔
  DWORD dwMaxSitInterval; // 坐下间隔

  // 配置变量
  tagGameParameter* pGameParameter; // 配置参数
  tagGameServiceAttrib* pGameServiceAttrib; // 桌子属性
  tagGameServiceOption* pGameServiceOption; // 配置参数
  tagGameMatchOption* pGameMatchOption; // 比赛配置

  // 内核组件
  ITimerEngine* pITimerEngine; // 时间引擎
  IServerUserManager* pIServerUserManager; // 用户管理
  IGameServiceManager* pIGameServiceManager; // 服务管理
  IGameServiceCustomTime* pIGameServiceSustomTime; // 时间配置
  ITCPNetworkEngineEvent* pITCPNetworkEngineEvent; // 事件接口
};

//////////////////////////////////////////////////////////////////////////////////
// 辅助结构

// 积分信息
struct tagScoreInfo {
  BYTE cbType; // 积分类型
  SCORE lScore; // 用户分数
  SCORE lGrade; // 用户成绩
  SCORE lRevenue; // 游戏税收
};

// 变更信息
struct tagVariationInfo {
  // 积分信息
  SCORE lScore; // 用户分数
  SCORE lGrade; // 用户成绩
  SCORE lInsure; // 用户银行
  SCORE lRevenue; // 游戏税收

  // 游戏信息
  DWORD dwWinCount; // 胜利盘数
  DWORD dwLostCount; // 失败盘数
  DWORD dwDrawCount; // 和局盘数
  DWORD dwFleeCount; // 逃跑盘数
  SCORE lIntegralCount; // 积分总数(当前房间)
  DWORD dwPlayTimeCount; // 游戏时长

  // 全局信息
  SCORE lIngot; // 用户元宝
  DWORD dwExperience; // 用户经验
  LONG lLoveLiness; // 用户魅力
};

// 游戏记录
struct tagGameScoreRecord {
  // 用户信息
  DWORD dwUserID; // 用户标识
  WORD wChairID; // 椅子号码
  BYTE cbAndroid; // 机器标志

  // 成绩信息
  SCORE lScore; // 用户分数
  SCORE lGrade; // 用户成绩
  SCORE lRevenue; // 游戏税收

  // 用户信息
  DWORD dwDBQuestID; // 请求标识
  DWORD dwInoutIndex; // 记录索引

  // 附加信息
  DWORD dwUserMemal; // 奖牌数目
  DWORD dwPlayTimeCount; // 游戏时长
};

// 用户规则
struct tagUserRule {
  // 规则标志
  bool bLimitSameIP; // 效验地址
  bool bLimitWinRate; // 限制胜率
  bool bLimitFleeRate; // 限制逃率
  bool bLimitGameScore; // 限制分数

  // 规则属性
  WORD wMinWinRate; // 最低胜率
  WORD wMaxFleeRate; // 最高逃率
  LONG lMaxGameScore; // 最高分数
  LONG lMinGameScore; // 最低分数
  TCHAR szPassword[LEN_PASSWORD]; // 桌子密码
};

// 任务入口
struct tagUserTaskInfo {
  WORD wTaskID; // 任务标识
  BYTE cbTaskStatus; // 任务状态
  WORD wTaskProgress; // 任务进度
  DWORD dwResidueTime; // 剩余时间
  DWORD dwLastUpdateTime; // 更新时间
};

// 任务入口
struct tagUserTaskEntry {
  BYTE cbTaskStatus; // 任务状态
  WORD wTaskProgress; // 任务进度
  DWORD dwResidueTime; // 剩余时间
  DWORD dwLastUpdateTime; // 更新时间
  tagTaskParameter* pTaskParameter; // 任务参数
  tagUserTaskEntry* pNextTaskEntry; // 下一任务
  tagUserTaskEntry* pNextStatusEntry; // 下一任务
};

// 用户信息
struct tagUserInfoPlus {
  // 登录信息
  DWORD dwLogonTime; // 登录时间
  DWORD dwInoutIndex; // 进出标识

  // 连接信息
  WORD wBindIndex; // 绑定索引
  DWORD dwClientAddr; // 连接地址
  TCHAR szMachineID[LEN_MACHINE_ID]; // 机器标识

  // 附加变量
  DWORD dwUserRight; // 用户权限
  DWORD dwMasterRight; // 管理权限
  SCORE lRestrictScore; // 限制积分

  // 辅助变量
  bool bMobileUser; // 手机用户
  bool bAndroidUser; // 机器用户
  TCHAR szPassword[LEN_MD5]; // 桌子密码
  TCHAR szUserGameData[LEN_GAME_DATA]; // 游戏数据
};

// 请求结果
struct tagRequestResult {
  BYTE cbFailureCode; // 失败原因
  TCHAR szFailureReason[128]; // 失败原因
};

// 桌子状况
struct tagTableUserInfo {
  WORD wMinUserCount; // 最少人数
  WORD wTableUserCount; // 用户数目
  WORD wTableReadyCount; // 准备数目
  WORD wTableAndroidCount; // 机器数目
};

// 机器状况
struct tagAndroidUserInfo {
  WORD wFreeUserCount; // 用户数目
  WORD wPlayUserCount; // 用户数目
  WORD wSitdownUserCount; // 用户数目
  IAndroidUserItem* pIAndroidUserFree[MAX_ANDROID]; // 机器接口
  IAndroidUserItem* pIAndroidUserPlay[MAX_ANDROID]; // 机器接口
  IAndroidUserItem* pIAndroidUserSitdown[MAX_ANDROID]; // 机器接口
};

// 服务信息
struct tagAndroidService {
  DWORD dwSwitchTableInnings; // 游戏局数
};

// 帐号信息
struct tagAndroidAccountsInfo {
  DWORD dwUserID; // 机器标识
  TCHAR szPassword[LEN_PASSWORD]; // 机器密码
};

// 机器配置
struct tagAndroidItemConfig {
  tagAndroidAccountsInfo AndroidAccountsInfo; // 帐号信息
  tagAndroidParameter* pAndroidParameter; // 机器参数
};

// 游戏数据
struct tagGameDataItem {
  INT nKey; // 子项索引
  TCHAR szValue[64]; // 子项内容
};
//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceManager INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceManager = {0xa975cceb, 0x0331, 0x4553, 0xa1, 0xe0, 0xa7, 0xc7, 0x7a, 0x7c, 0x4e, 0xfd};

// 游戏接口
interface IGameServiceManager : public IUnknownEx {
  // 创建接口
public:
  // 创建桌子
  virtual VOID* CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer) = 0;
  // 创建机器
  virtual VOID* CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer) = 0;
  // 创建数据
  virtual VOID* CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer) = 0;

  // 参数接口
public:
  // 组件属性
  virtual bool GetServiceAttrib(tagGameServiceAttrib & GameServiceAttrib) = 0;
  // 调整参数
  virtual bool RectifyParameter(tagGameServiceOption & GameServiceOption) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceCustomRule INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceCustomRule = {0x74a43b7d, 0x7c29, 0x4fb8, 0xa5, 0x00, 0x8e, 0x8d, 0xff, 0x6e, 0x2a, 0xdd};

#define VER_IGameServicePersonalRule INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServicePersonalRule = {0x18f8efaf, 0xf170, 0x4ef1, 0x90, 0x2c, 0x85, 0x31, 0x3c, 0x57, 0xa3, 0x76};

// 配置接口
interface IGameServiceCustomRule : public IUnknownEx {
  // 获取配置
  virtual bool SaveCustomRule(LPBYTE pcbCustomRule, WORD wCustonSize) = 0;
  // 默认配置
  virtual bool DefaultCustomRule(LPBYTE pcbCustomRule, WORD wCustonSize) = 0;
};

// 配置接口
interface IGameServicePersonalRule : public IUnknownEx {
  // 获取配置
  virtual bool SavePersonalRule(LPBYTE pcbCustomRule, WORD wCustonSize) = 0;
  // 默认配置
  virtual bool DefaultPersonalRule(LPBYTE pcbCustomRule, WORD wCustonSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceCustomTime INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceCustomTime = {0xf57573b0, 0x63c3, 0x43a9, 0x8c, 0xb1, 0xe2, 0x22, 0xd3, 0x93, 0xe2, 0xb};

// 时间配置
interface IGameServiceCustomTime : public IUnknownEx {
  // 机器脉冲
  virtual DWORD GetAndroidTimerPulse() = 0;
  // 时间范围
  virtual DWORD GetTableSinkTimeRange() = 0;
  // 时间单元
  virtual DWORD GetTimerEngineTimeCell() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameDataBaseEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IGameDataBaseEngine = {0x47dcd531, 0x2a19, 0x4c0a, 0x89, 0xb6, 0x9f, 0xd8, 0xe0, 0xa7, 0x85, 0xee};

// 游戏数据
interface IGameDataBaseEngine : public IUnknownEx {
  // 配置参数
public:
  // 自定配置
  virtual VOID* GetCustomRule() = 0;
  // 服务属性
  virtual tagGameServiceAttrib* GetGameServiceAttrib() = 0;
  // 服务配置
  virtual tagGameServiceOption* GetGameServiceOption() = 0;

  // 获取对象
public:
  // 获取对象
  virtual VOID* GetDataBase(REFGUID Guid, DWORD dwQueryVer) = 0;
  // 获取对象
  virtual VOID* GetDataBaseEngine(REFGUID Guid, DWORD dwQueryVer) = 0;

  // 功能接口
public:
  // 投递结果
  virtual bool PostGameDataBaseResult(WORD wRequestID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameDataBaseEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IGameDataBaseEngineSink = {0x9b9111d9, 0x7a71, 0x41a1, 0xae, 0x78, 0xd4, 0xf3, 0x20, 0x08, 0x24, 0xdf};

// 数据接口
interface IGameDataBaseEngineSink : public IUnknownEx {
  // 配置接口
  virtual bool InitializeSink(IUnknownEx * pIUnknownEx) = 0;
  // 数据处理
  virtual bool OnGameDataBaseRequest(DWORD dwUserID, WORD wTableID, WORD wChairID);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserItem INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserItem = {0xcd43dce8, 0x1e12, 0x43be, 0x8b, 0x4f, 0x94, 0x95, 0x92, 0xa4, 0xf6, 0x19};

// 用户接口
interface IServerUserItem : public IUnknownEx {
  // 属性信息
public:
  // 用户索引
  virtual WORD GetBindIndex() = 0;
  // 用户地址
  virtual DWORD GetClientAddr() = 0;
  // 机器标识
  virtual LPCTSTR GetMachineID() = 0;

  // 登录信息
public:
  // 请求标识
  virtual DWORD GetDBQuestID() = 0;
  // 登录时间
  virtual DWORD GetLogonTime() = 0;
  // 记录索引
  virtual DWORD GetInoutIndex() = 0;

  // 用户信息
public:
  // 用户信息
  virtual tagUserInfo* GetUserInfo() = 0;
  // 用户规则
  virtual tagUserRule* GetUserRule() = 0;
  // 道具信息
  virtual tagUserProperty* GetUserProperty() = 0;
  // 时间信息
  virtual tagTimeInfo* GetTimeInfo() = 0;

  // 游戏数据
public:
  // 游戏数据
  virtual LPCTSTR GetUserGameData() = 0;
  // 游戏数据
  virtual VOID GetUserGameData(INT nKey, LPTSTR pszValue, INT nMaxCount) = 0;
  // 游戏数据
  virtual VOID SetUserGameData(LPCTSTR pszValue, INT nMaxCount) = 0;
  // 游戏数据
  virtual VOID SetUserGameData(INT nKey, LPCTSTR pszValue, INT nMaxCount) = 0;
  // 游戏数据
  virtual VOID WriteUserGameData() = 0;

  // 属性信息
public:
  // 用户性别
  virtual BYTE GetGender() = 0;
  // 用户标识
  virtual DWORD GetUserID() = 0;
  // 游戏标识
  virtual DWORD GetGameID() = 0;
  // 用户昵称
  virtual LPCTSTR GetNickName() = 0;

  // 状态接口
public:
  // 桌子号码
  virtual WORD GetTableID() = 0;
  // 桌子号码
  virtual WORD GetLastTableID() = 0;
  // 椅子号码
  virtual WORD GetChairID() = 0;
  // 用户状态
  virtual BYTE GetUserStatus() = 0;

  // 权限信息
public:
  // 用户权限
  virtual DWORD GetUserRight() = 0;
  // 管理权限
  virtual DWORD GetMasterRight() = 0;

  // 权限信息
public:
  // 用户权限
  virtual BYTE GetMemberOrder() = 0;
  // 管理权限
  virtual BYTE GetMasterOrder() = 0;

  // 积分信息
public:
  // 用户积分
  virtual SCORE GetUserScore() = 0;
  // 用户成绩
  virtual SCORE GetUserGrade() = 0;
  // 用户银行
  virtual SCORE GetUserInsure() = 0;

  // 托管信息
public:
  // 托管积分
  virtual SCORE GetTrusteeScore() = 0;
  // 锁定积分
  virtual SCORE GetFrozenedScore() = 0;

  // 积分信息
public:
  // 用户胜率
  virtual WORD GetUserWinRate() = 0;
  // 用户输率
  virtual WORD GetUserLostRate() = 0;
  // 用户和率
  virtual WORD GetUserDrawRate() = 0;
  // 用户逃率
  virtual WORD GetUserFleeRate() = 0;
  // 游戏局数
  virtual DWORD GetUserPlayCount() = 0;

  // 效验接口
public:
  // 对比帐号
  virtual bool ContrastNickName(LPCTSTR pszNickName) = 0;
  // 对比密码
  virtual bool ContrastLogonPass(LPCTSTR pszPassword) = 0;

  // 托管状态
public:
  // 判断状态
  virtual bool IsTrusteeUser() = 0;
  // 设置状态
  virtual VOID SetTrusteeUser(bool bTrusteeUser) = 0;

  // 游戏状态
public:
  // 连接状态
  virtual bool IsClientReady() = 0;
  // 设置连接
  virtual VOID SetClientReady(bool bClientReady) = 0;

  // 手机用户
public:
  // 控制状态
  virtual bool IsMobileUser() = 0;
  // 设置控制
  virtual VOID SetMobileUser(bool bMobileUser) = 0;

  // 控制用户
public:
  // 控制状态
  virtual bool IsAndroidUser() = 0;
  // 设置控制
  virtual VOID SetAndroidUser(bool bbMachineUser) = 0;

  // 比赛接口
public:
  // 报名数据
  virtual VOID* GetMatchData() = 0;
  // 报名数据
  virtual VOID SetMatchData(VOID * pMatchData) = 0;
  // 报名时间
  virtual DWORD GetSignUpTime() = 0;
  // 报名时间
  virtual VOID SetSignUpTime(DWORD dwSignUpTime) = 0;
  // 比赛状态
  virtual BYTE GetUserMatchStatus() = 0;
  // 比赛状态
  virtual VOID SetUserMatchStatus(BYTE cbMatchStatus) = 0;

  // 记录接口
public:
  // 变更判断
  virtual bool IsVariation() = 0;
  // 查询记录
  virtual bool QueryRecordInfo(tagVariationInfo & UserRecordInfo) = 0;
  // 提取变更
  virtual bool DistillVariation(tagVariationInfo & UserVariationInfo) = 0;

  // 管理接口
public:
  // 设置状态
  virtual bool SetUserStatus(BYTE cbUserStatus, WORD wTableID, WORD wChairID) = 0;
  // 写入积分
  virtual bool WriteUserScore(SCORE lScore, SCORE lGrade, SCORE lRevenue, SCORE lIngot, BYTE cbScoreType, DWORD dwPlayTimeCount,
                              DWORD dwWinExperience) = 0;
  // 领取奖励
  virtual bool SetUserTaskReward(SCORE lScore, SCORE lIngot) = 0;
  // 修改权限
  virtual VOID ModifyUserRight(DWORD dwAddRight, DWORD dwRemoveRight, BYTE cbRightKind = UR_KIND_GAME) = 0;

  // 冻结接口
public:
  // 冻结积分
  virtual bool FrozenedUserScore(SCORE lScore) = 0;
  // 解冻积分
  virtual bool UnFrozenedUserScore(SCORE lScore) = 0;

  // 修改接口
public:
  // 修改信息
  virtual bool ModifyUserProperty(SCORE lScore, LONG lLoveLiness) = 0;

  // 高级接口
public:
  // 解除绑定
  virtual bool DetachBindStatus() = 0;
  // 银行操作
  virtual bool ModifyUserInsure(SCORE lScore, SCORE lInsure, SCORE lRevenue) = 0;
  // 设置参数
  virtual bool SetUserParameter(DWORD dwClientAddr, WORD wBindIndex, TCHAR szMachineID[LEN_MACHINE_ID], bool bAndroidUser, bool bClientReady) = 0;

  // 手机定义
public:
  // 手机规则
  virtual WORD GetMobileUserRule() = 0;
  // 设置定义
  virtual VOID SetMobileUserRule(WORD wMobileUserRule) = 0;
  // 当前分页
  virtual WORD GetMobileUserDeskPos() = 0;
  // 当前分页
  virtual VOID SetMobileUserDeskPos(WORD wMobileUserDeskPos) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserService INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserService = {0x2f4e25a9, 0xad87, 0x4a37, 0x98, 0x96, 0x5c, 0x50, 0xf9, 0x91, 0x05, 0x31};

// 用户服务
interface IServerUserService : public IUnknownEx{};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMatchUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IMatchUserItemSink = {0x62A8E2FE, 0xDB9F, 0x4C22, 0x80, 0xB9, 0x41, 0xCE, 0x9E, 0x94, 0xF1, 0xF6};

// 状态接口
interface IMatchUserItemSink : public IUnknownEx {
  // 用户状态
  virtual bool OnEventMatchUserStatus(IServerUserItem * pIServerUserItem, BYTE cbOldUserStatus, BYTE cbCurrUserStatus) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserItemSink = {0x415be3e4, 0xd48d, 0x4a77, 0x94, 0xb7, 0xd1, 0x05, 0xcd, 0xa3, 0x82, 0x81};

// 状态接口
interface IServerUserItemSink : public IUnknownEx {
  // 用户积分
  virtual bool OnEventUserItemScore(IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;
  // 用户数据
  virtual bool OnEventUserItemGameData(IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;
  // 用户状态
  virtual bool OnEventUserItemStatus(IServerUserItem * pIServerUserItem, WORD wOldTableID = INVALID_TABLE, WORD wOldChairID = INVALID_CHAIR) = 0;
  // 用户权限
  virtual bool OnEventUserItemRight(IServerUserItem * pIServerUserItem, DWORD dwAddRight, DWORD dwRemoveRight, BYTE cbRightKind) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserManager INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserManager = {0xeb413ed6, 0x185b, 0x4ceb, 0xa8, 0xbd, 0x54, 0x74, 0x19, 0x89, 0x6a, 0x53};

// 用户管理
interface IServerUserManager : public IUnknownEx {
  // 配置接口
public:
  // 设置接口
  virtual bool SetServerUserItemSink(IUnknownEx * pIUnknownEx) = 0;
  // 设置接口
  virtual bool SetMatchUserItemSink(IUnknownEx * pIUnknownEx) = 0;

  // 查找接口
public:
  // 枚举用户
  virtual IServerUserItem* EnumUserItem(WORD wEnumIndex) = 0;
  // 查找用户
  virtual IServerUserItem* SearchUserItem(DWORD dwUserID) = 0;
  // 查找用户
  virtual IServerUserItem* SearchUserItem(LPCTSTR pszNickName) = 0;

  // 统计接口
public:
  // 机器人数
  virtual DWORD GetAndroidCount() = 0;
  // 在线人数
  virtual DWORD GetUserItemCount() = 0;

  // 管理接口
public:
  // 删除用户
  virtual bool DeleteUserItem() = 0;
  // 删除用户
  virtual bool DeleteUserItem(IServerUserItem * pIServerUserItem) = 0;
  // 插入用户
  virtual bool InsertUserItem(IServerUserItem * *pIServerUserResult, tagUserInfo & UserInfo, tagUserInfoPlus & UserInfoPlus) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableFrame INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrame = {0x2e577d5f, 0x1e01, 0x44ff, 0x9f, 0xf4, 0x01, 0x16, 0x23, 0x1b, 0x76, 0x15};

// 桌子接口
interface ITableFrame : public IUnknownEx {
  // 属性接口
public:
  // 桌子号码
  virtual WORD GetTableID() = 0;
  // 游戏人数
  virtual WORD GetChairCount() = 0;
  // 空位置数目
  virtual WORD GetNullChairCount() = 0;

  // 配置参数
public:
  // 自定配置
  virtual VOID* GetCustomRule() = 0;
  // 比赛配置
  virtual tagGameMatchOption* GetGameMatchOption() = 0;
  // 服务属性
  virtual tagGameServiceAttrib* GetGameServiceAttrib() = 0;
  // 服务配置
  virtual tagGameServiceOption* GetGameServiceOption() = 0;

  // 配置接口
public:
  // 开始模式
  virtual BYTE GetStartMode() = 0;
  // 开始模式
  virtual VOID SetStartMode(BYTE cbStartMode) = 0;

  // 单元积分
public:
  // 单元积分
  virtual LONG GetCellScore() = 0;
  // 单元积分
  virtual VOID SetCellScore(LONG lCellScore) = 0;

  // 信息接口
public:
  // 锁定状态
  virtual bool IsTableLocked() = 0;
  // 游戏状态
  virtual bool IsGameStarted() = 0;
  // 游戏状态
  virtual bool IsDrawStarted() = 0;
  // 游戏状态
  virtual bool IsTableStarted() = 0;

  // 私人桌子
public:
  // 私人桌锁
  virtual bool IsPersonalTableLocked() = 0;
  // 设置锁定
  virtual VOID SetPersonalTableLlocked(bool bLocked) = 0;
  // 设置桌主
  virtual VOID SetTableOwner(DWORD dwUserID) = 0;
  // 获取桌主
  virtual DWORD GetTableOwner() = 0;
  // 设置桌子
  virtual VOID SetPersonalTable(DWORD dwDrawCountLimit, DWORD dwDrawTimeLimit, LONGLONG lIniScore) = 0;
  // 设置桌子参数
  virtual VOID SetPersonalTableParameter(tagPersonalTableParameter PersonalTableParameter, tagPersonalRoomOption PersonalRoomOption) = 0;

  // 获取分数
  virtual bool GetPersonalScore(DWORD dwUserID, LONGLONG & lScore) = 0;
  // 桌子信息
  virtual tagPersonalTableParameter GetPersonalTableParameter() = 0;
  // 桌子创建后多长时间未开始游戏 解散桌子
  virtual VOID SetTimerNotBeginAfterCreate() = 0;

  // 状态接口
public:
  // 获取状态
  virtual BYTE GetGameStatus() = 0;
  // 设置状态
  virtual VOID SetGameStatus(BYTE bGameStatus) = 0;

  // 控制接口
public:
  // 开始游戏
  virtual bool StartGame() = 0;
  // 解散游戏
  virtual bool DismissGame() = 0;
  // 结束游戏
  virtual bool ConcludeGame(BYTE cbGameStatus) = 0;

  // 写分接口
public:
  // 写入积分
  virtual bool WriteUserScore(WORD wChairID, tagScoreInfo & ScoreInfo, DWORD dwGameMemal = INVALID_DWORD,
                              DWORD dwPlayGameTime = INVALID_DWORD) = 0;
  // 写入积分
  virtual bool WriteTableScore(tagScoreInfo ScoreInfoArray[], WORD wScoreCount) = 0;


  // 计算接口
public:
  // 计算税收
  virtual SCORE CalculateRevenue(WORD wChairID, SCORE lScore) = 0;
  // 消费限额
  virtual SCORE QueryConsumeQuota(IServerUserItem * pIServerUserItem) = 0;

  // 用户接口
public:
  // 寻找用户
  virtual IServerUserItem* SearchUserItem(DWORD dwUserID) = 0;
  // 游戏用户
  virtual IServerUserItem* GetTableUserItem(WORD wChairID) = 0;
  // 旁观用户
  virtual IServerUserItem* EnumLookonUserItem(WORD wEnumIndex) = 0;

  // 时间接口
public:
  // 设置时间
  virtual bool SetGameTimer(DWORD dwTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM dwBindParameter) = 0;
  // 删除时间
  virtual bool KillGameTimer(DWORD dwTimerID) = 0;

  // 网络接口
public:
  // 发送数据
  virtual bool SendTableData(WORD wChairID, WORD wSubCmdID) = 0;
  // 发送数据
  virtual bool SendTableData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize, WORD wMainCmdID = MDM_GF_GAME) = 0;
  // 发送数据
  virtual bool SendLookonData(WORD wChairID, WORD wSubCmdID) = 0;
  // 发送数据
  virtual bool SendLookonData(WORD wChairID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 发送数据
  virtual bool SendUserItemData(IServerUserItem * pIServerUserItem, WORD wSubCmdID) = 0;
  // 发送数据
  virtual bool SendUserItemData(IServerUserItem * pIServerUserItem, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;

  // 功能接口
public:
  // 发送消息
  virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType) = 0;
  // 游戏消息
  virtual bool SendGameMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType) = 0;
  // 房间消息
  virtual bool SendRoomMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType) = 0;

  // 动作处理
public:
  // 起立动作
  virtual bool PerformStandUpAction(IServerUserItem * pIServerUserItem, bool bInitiative = false) = 0;
  // 旁观动作
  virtual bool PerformLookonAction(WORD wChairID, IServerUserItem * pIServerUserItem) = 0;
  // 坐下动作
  virtual bool PerformSitDownAction(WORD wChairID, IServerUserItem * pIServerUserItem, LPCTSTR lpszPassword = NULL) = 0;

  // 功能接口
public:
  // 发送场景
  virtual bool SendGameScene(IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) = 0;

  // 比赛接口
public:
  // 获取接口
  virtual IUnknownEx* GetTableFrameHook() = 0;
  // 设置接口
  virtual bool SetTableFrameHook(IUnknownEx * pIUnknownEx) = 0;
  // 伪造配置
  virtual bool ImitateGameOption(IServerUserItem * pIServerUserItem) = 0;

  // 聊天接口
public:
  virtual bool SendChatMessage(IServerUserItem * pIServerUserItem, IServerUserItem * pITargetServerUserItem, DWORD dwChatColor,
                               LPCTSTR lpszChatString, LPTSTR lpszDescribeString) = 0;

public:
  // 获取游戏规则
  virtual BYTE* GetGameRule() = 0;

  // 获取结算时的特殊信息
  virtual void GetSpeicalInfo(BYTE * cbSpecialInfo, int nSpecialLen) = 0;

  // 设置桌子上椅子的个数
  virtual VOID SetTableChairCount(WORD wChairCount) = 0;

public:
  // 私人房间是否解散
  virtual bool IsPersonalRoomDisumme() = 0;

  // 设置是金币数据库还是积分数据库,  0 为金币库 1 为 积分库
  virtual void SetDataBaseMode(BYTE cbDataBaseMode) = 0;

  // 获取数据库模式,  0 为金币库 1 为 积分库
  virtual BYTE GetDataBaseMode() = 0;
  // 设置创建时间
  virtual void SetCreatePersonalTime(SYSTEMTIME tm) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ICompilationSink INTERFACE_VERSION(1, 1)
static const GUID IID_ICompilationSink = {0x761A06DF, 0x2BCA, 0x4333, 0xAE, 0x96, 0xEF, 0x85, 0xBA, 0xB2, 0x45, 0xEB};

// 回调接口
interface ICompilationSink : public IUnknownEx {
  // 获取信息
public:
  // 获取信息
  virtual LPCTSTR GetCompilation() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableFrameSink INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrameSink = {0x9476b154, 0x8beb, 0x4f7e, 0xaf, 0x64, 0xd2, 0xb1, 0x1a, 0xda, 0x5e, 0xc4};

// 回调接口
interface ITableFrameSink : public IUnknownEx {
  // 管理接口
public:
  // 复位接口
  virtual VOID RepositionSink() = 0;
  // 配置接口
  virtual bool Initialization(IUnknownEx * pIUnknownEx) = 0;

  // 查询接口
public:
  // 查询限额
  virtual SCORE QueryConsumeQuota(IServerUserItem * pIServerUserItem) = 0;
  // 最少积分
  virtual SCORE QueryLessEnterScore(WORD wChairID, IServerUserItem * pIServerUserItem) = 0;
  // 查询是否扣服务费
  virtual bool QueryBuckleServiceCharge(WORD wChairID) = 0;

  // 游戏事件
public:
  // 游戏开始
  virtual bool OnEventGameStart() = 0;
  // 游戏结束
  virtual bool OnEventGameConclude(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;
  // 发送场景
  virtual bool OnEventSendGameScene(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbGameStatus, bool bSendSecret) = 0;

  // 事件接口
public:
  // 时间事件
  virtual bool OnTimerMessage(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
  // 数据事件
  virtual bool OnDataBaseMessage(WORD wRequestID, VOID * pData, WORD wDataSize) = 0;
  // 积分事件
  virtual bool OnUserScroeNotify(WORD wChairID, IServerUserItem * pIServerUserItem, BYTE cbReason) = 0;

  // 网络接口
public:
  // 游戏消息
  virtual bool OnGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem) = 0;
  // 框架消息
  virtual bool OnFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize, IServerUserItem * pIServerUserItem) = 0;

  // 比赛接口
public:
  // 设置基数
  virtual void SetGameBaseScore(LONG lBaseScore) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableUserAction INTERFACE_VERSION(1, 1)
static const GUID IID_ITableUserAction = {0x0f9aa3f9, 0xdba4, 0x49cb, 0x88, 0x4f, 0xd9, 0x11, 0xaf, 0x24, 0xfb, 0x8d};

// 用户动作
interface ITableUserAction : public IUnknownEx {
  // 用户断线
  virtual bool OnActionUserOffLine(WORD wChairID, IServerUserItem * pIServerUserItem) = 0;
  // 用户重入
  virtual bool OnActionUserConnect(WORD wChairID, IServerUserItem * pIServerUserItem) = 0;
  // 用户坐下
  virtual bool OnActionUserSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
  // 用户起来
  virtual bool OnActionUserStandUp(WORD wChairID, IServerUserItem * pIServerUserItem, bool bLookonUser) = 0;
  // 用户同意
  virtual bool OnActionUserOnReady(WORD wChairID, IServerUserItem * pIServerUserItem, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableUserRequest INTERFACE_VERSION(1, 1)
static const GUID IID_ITableUserRequest = {0x7a810ebe, 0x3835, 0x41b5, 0xba, 0x7e, 0x02, 0x97, 0x8c, 0x13, 0x73, 0x50};

// 用户请求
interface ITableUserRequest : public IUnknownEx {
  // 旁观请求
  virtual bool OnUserRequestLookon(WORD wChairID, IServerUserItem * pIServerUserItem, tagRequestResult & RequestResult) = 0;
  // 坐下请求
  virtual bool OnUserRequestSitDown(WORD wChairID, IServerUserItem * pIServerUserItem, tagRequestResult & RequestResult) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMainServiceFrame INTERFACE_VERSION(1, 1)
static const GUID IID_IMainServiceFrame = {0xef3efa64, 0x788b, 0x4299, 0x80, 0x99, 0xdd, 0xed, 0x08, 0xde, 0x57, 0xc1};

// 服务框架
interface IMainServiceFrame : public IUnknownEx {
  // 消息接口
public:
  // 房间消息
  virtual bool SendRoomMessage(LPCTSTR lpszMessage, WORD wType) = 0;
  // 游戏消息
  virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType) = 0;
  // 房间消息
  virtual bool SendRoomMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType) = 0;
  // 游戏消息
  virtual bool SendGameMessage(IServerUserItem * pIServerUserItem, LPCTSTR lpszMessage, WORD wType) = 0;
  // 房间消息
  virtual bool SendRoomMessage(DWORD dwSocketID, LPCTSTR lpszMessage, WORD wType, bool bAndroid) = 0;

  // 网络接口
public:
  // 发送数据
  virtual bool SendData(BYTE cbSendMask, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 发送数据
  virtual bool SendData(DWORD dwContextID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 发送数据
  virtual bool SendData(IServerUserItem * pIServerUserItem, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 群发数据
  virtual bool SendDataBatchToMobileUser(WORD wCmdTable, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;

  // 功能接口
public:
  // 断开协调
  virtual bool DisconnectCorrespond() = 0;
  // 插入分配
  virtual bool InsertDistribute(IServerUserItem * pIServerUserItem) = 0;
  // 删除用户
  virtual bool DeleteDistribute(IServerUserItem * pIServerUserItem) = 0;
  // 敏感词过滤
  virtual StringT SensitiveWordFilter(StringViewT msg) = 0;
  // 敏感词过滤
  virtual std::u16string SensitiveWordFilter(std::u16string_view msg) = 0;
  // 解锁机器人
  virtual VOID UnLockAndroidUser(WORD wServerID, WORD wBatchID) = 0;
  // 解散私人桌子
  virtual VOID DismissPersonalTable(WORD wServerID, WORD wTableID) = 0;
  // 取消创建
  virtual VOID CancelCreateTable(DWORD dwUserID, DWORD dwDrawCountLimit, DWORD dwDrawTimeLimit, DWORD dwReason, WORD wTableID,
                                 TCHAR * szRoomID) = 0;
  // 开始游戏写入参与信息
  virtual VOID PersonalRoomWriteJoinInfo(DWORD dwUserID, WORD wTableID, WORD wChairID, DWORD dwKindID, TCHAR * szRoomID,
                                         TCHAR * szPersonalRoomGUID) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserItem INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserItem = {0xf6856fe1, 0xc93e, 0x4166, 0xbc, 0x92, 0xf7, 0x43, 0xad, 0x97, 0x1c, 0xa8};

// 机器人接口
interface IAndroidUserItem : public IUnknownEx {
  // 信息接口
public:
  // 获取 I D
  virtual DWORD GetUserID() = 0;
  // 桌子号码
  virtual WORD GetTableID() = 0;
  // 椅子号码
  virtual WORD GetChairID() = 0;

  // 状态函数
public:
  // 获取状态
  virtual BYTE GetGameStatus() = 0;
  // 设置状态
  virtual VOID SetGameStatus(BYTE cbGameStatus) = 0;

  // 配置信息
public:
  // 获取状态
  virtual tagAndroidService* GetAndroidService() = 0;
  // 获取配置
  virtual tagAndroidParameter* GetAndroidParameter() = 0;

  // 功能接口
public:
  // 获取自己
  virtual IServerUserItem* GetMeUserItem() = 0;
  // 游戏用户
  virtual IServerUserItem* GetTableUserItem(WORD wChariID) = 0;

  // 银行接口
public:
  // 存入游戏币
  virtual bool PerformSaveScore(SCORE lScore) = 0;
  // 提取游戏币
  virtual bool PerformTakeScore(SCORE lScore) = 0;

  // 网络接口
public:
  // 发送函数
  virtual bool SendSocketData(WORD wSubCmdID) = 0;
  // 发送函数
  virtual bool SendSocketData(WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;

  // 动作接口
public:
  // 机器动作
  virtual bool JudgeAndroidActionAndRemove(WORD wAction) = 0;

  // 功能接口
public:
  // 删除时间
  virtual bool KillGameTimer(UINT nTimerID) = 0;
  // 设置时间
  virtual bool SetGameTimer(UINT nTimerID, UINT nElapse) = 0;
  // 发送准备
  virtual bool SendUserReady(VOID * pData, WORD wDataSize) = 0;
  // 发送聊天
  virtual bool SendChatMessage(DWORD dwTargetUserID, LPCTSTR pszChatString, COLORREF crFontColor) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserItemSink = {0x0967632c, 0x93da, 0x4f7f, 0x98, 0xe4, 0x6f, 0x9f, 0xf2, 0xca, 0x7b, 0xc4};

// 机器人接口
interface IAndroidUserItemSink : public IUnknownEx {
  // 控制接口
public:
  // 重置接口
  virtual bool RepositionSink() = 0;
  // 初始接口
  virtual bool Initialization(IUnknownEx * pIUnknownEx) = 0;

  // 游戏事件
public:
  // 时间消息
  virtual bool OnEventTimer(UINT nTimerID) = 0;
  // 游戏消息
  virtual bool OnEventGameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 游戏消息
  virtual bool OnEventFrameMessage(WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 场景消息
  virtual bool OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, VOID* pData, WORD wDataSize) = 0;

  // 用户事件
public:
  // 用户进入
  virtual VOID OnEventUserEnter(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser) = 0;
  // 用户离开
  virtual VOID OnEventUserLeave(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser) = 0;
  // 用户积分
  virtual VOID OnEventUserScore(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser) = 0;
  // 用户状态
  virtual VOID OnEventUserStatus(IAndroidUserItem * pIAndroidUserItem, bool bLookonUser) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IUserTaskManagerSink INTERFACE_VERSION(1, 1)
static const GUID IID_IUserTaskManagerSink = {0xe71c3ecf, 0x490e, 0x4a32, 0xb4, 0x75, 0x2c, 0xba, 0x9b, 0x55, 0x26, 0xd4};

// 任务接口
interface IUserTaskManagerSink : public IUnknownEx {
  // 任务参数
public:
  // 移除参数
  virtual VOID RemoveTaskParameter() = 0;
  // 查找参数
  virtual tagTaskParameter* SearchTaskParameter(WORD wTaskID) = 0;
  // 枚举参数
  virtual tagTaskParameter* EnumTaskParameter(POSITION & Position) = 0;
  // 添加参数
  virtual bool AddTaskParameter(tagTaskParameter TaskParameter[], WORD wPatemterCount) = 0;
  // 获取参数数目
  virtual WORD GetTaskParameterCount() = 0;

  // 用户任务
public:
  // 移除任务
  virtual VOID RemoveUserTask(DWORD dwUserID) = 0;
  // 获取任务
  virtual tagUserTaskEntry* GetUserTaskEntry(DWORD dwUserID) = 0;
  // 获取任务
  virtual tagUserTaskEntry* GetUserTaskEntry(DWORD dwUserID, BYTE cbTaskStatus) = 0;
  // 设置任务
  virtual VOID SetUserTaskInfo(DWORD dwUserID, tagUserTaskInfo UserTaskInfo[], WORD wTaskCount) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserManager INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserManager = {0x0c963240, 0xa798, 0x4e33, 0x81, 0x28, 0x97, 0x3d, 0x05, 0xe7, 0x7f, 0x89};

// 机器人接口
interface IAndroidUserManager : public IUnknownEx {
  // 控制接口
public:
  // 启动服务
  virtual bool StartService() = 0;
  // 停止服务
  virtual bool ConcludeService() = 0;

  // 配置接口
public:
  // 配置组件
  virtual bool InitAndroidUser(tagAndroidUserParameter & AndroidUserParameter) = 0;
  // 移除参数
  virtual bool RemoveAndroidParameter(DWORD dwBatchID) = 0;
  // 添加参数
  virtual bool AddAndroidParameter(tagAndroidParameter AndroidParameter[], WORD wParemeterCount) = 0;
  // 插入机器
  virtual bool InsertAndroidInfo(tagAndroidAccountsInfo AndroidAccountsInfo[], WORD wAndroidCount, DWORD dwBatchID) = 0;

  // 管理接口
public:
  // 删除机器
  virtual bool DeleteAndroidUserItem(DWORD dwAndroidID, bool bStockRetrieve) = 0;
  // 查找机器
  virtual IAndroidUserItem* SearchAndroidUserItem(DWORD dwUserID, DWORD dwContextID) = 0;
  // 创建机器
  virtual IAndroidUserItem* CreateAndroidUserItem(tagAndroidItemConfig & AndroidItemConfig) = 0;

  // 事件接口
public:
  // 脉冲事件
  virtual bool OnEventTimerPulse(DWORD dwTimerID, WPARAM dwBindParameter) = 0;

  // 状态接口
public:
  // 机器数目
  virtual WORD GetAndroidCount() = 0;
  // 加载机器
  virtual bool GetAndroidLoadInfo(DWORD & dwBatchID, DWORD & dwLoadCount) = 0;
  // 用户状况
  virtual WORD GetAndroidUserInfo(tagAndroidUserInfo & AndroidUserInfo, DWORD dwServiceMode) = 0;
  // 获取房间配置
  virtual tagGameServiceOption* GetGameServiceOption() = 0;
  // 获取游戏属性
  virtual tagGameServiceAttrib* GetGameServiceAttrib() = 0;
  // 获取比赛配置
  virtual tagGameMatchOption* GetGameMatchOption() = 0;

  // 网络接口
public:
  // 发送数据
  virtual bool SendDataToClient(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 发送数据
  virtual bool SendDataToClient(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 发送数据
  virtual bool SendDataToServer(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IQueryServiceSink INTERFACE_VERSION(1, 1)
static const GUID IID_IQueryServiceSink = {0x0c963248, 0xa798, 0x4e33, 0x88, 0x28, 0x98, 0x3d, 0x05, 0xe7, 0x7f, 0x89};

// 查询接口
interface IQueryServiceSink : public IUnknownEx {
  // 查询是否扣服务费
  virtual bool QueryBuckleServiceCharge(WORD wChairID) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDBCorrespondManager INTERFACE_VERSION(1, 1)
static const GUID IID_IDBCorrespondManager = {0x0c963248, 0xa796, 0x4e33, 0x86, 0x28, 0x98, 0x3d, 0x05, 0xe7, 0x7f, 0x89};

// 查询接口
interface IDBCorrespondManager : public IServiceModule {
  // 配置接口
public:
  // 配置模块
  virtual bool InitDBCorrespondManager(IDataBaseEngine * pIDataBaseEngine) = 0;

  // 控制事件
public:
  // 请求事件
  virtual bool PostDataBaseRequest(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize,
                                           BYTE cbCache = FALSE) = 0;

  // 同步事件
public:
  // 请求完成
  virtual bool OnPostRequestComplete(DWORD dwUserID, bool bSucceed) = 0;

  // 定时事件
public:
  // 定时事件
  virtual bool OnTimerNotify() = 0;
};
//////////////////////////////////////////////////////////////////////////////////

#define VER_ICompilationSink INTERFACE_VERSION(1, 1)
static const GUID IID_IGobalLogFile = {0x87cd59aa, 0x22bd, 0x4891, 0xb9, 0xa0, 0x1e, 0xf2, 0x6c, 0x36, 0x89, 0x86};

// 日志接口
interface IGobalLogFile : public IUnknownEx {
public:
  // 游戏日志
  virtual void WriteGameLog(LPCTSTR pszLogString) = 0;
};

//////////////////////////////////////////////////////////////////////////////////
// 包含文件
#ifndef GAME_SERVICE_DLL
#include "AndroidUserItem.h"
#include "AndroidUserManager.h"
#include "GamePropertyManager.h"
#include "ServerUserManager.h"
#include "UserTaskManager.h"
#endif

// 游戏服务
DECLARE_MODULE_DYNAMIC(GameServiceManager)
DECLARE_MODULE_DYNAMIC(AndroidUserItemSink)

//////////////////////////////////////////////////////////////////////////////////
