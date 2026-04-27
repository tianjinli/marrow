#pragma once

//////////////////////////////////////////////////////////////////////////////////
// 包含文件

// 系统文件
#include "GlobalDefine/Platform.h"

// 组件定义
#include "KernelEngine/KernelEngineHead.h"
#include "ServiceCore/ServiceCoreHead.h"

// 消息定义
#include "MessageDefine/CMD_Commom.h"
#include "MessageDefine/CMD_Correspond.h"
#include "MessageDefine/CMD_GameServer.h"

//////////////////////////////////////////////////////////////////////////////////
// 公共定义

// 导出定义
#ifdef _WIN32
#ifdef GAME_SERVICE_DLL
#define GAME_SERVICE_CLASS _declspec(dllexport)
#else
#define GAME_SERVICE_CLASS _declspec(dllimport)
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

// 类型定义

using CTaskParameterMap = std::unordered_map<WORD, tagTaskParameter *>;

//////////////////////////////////////////////////////////////////////////////////

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
#define TIME_TABLE_SINK_RANGE 1020 // 标识范围
#define TIME_TABLE_KERNEL_RANGE 1030 // 标识范围
#define TIME_TABLE_MODULE_RANGE 1050 // 标识范围

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
    WORD wSupportType; // 支持类型
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
    WCHAR szMatchName[32]; // 比赛名称

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
    ITimerEngine *pITimerEngine; // 时间引擎
    IDataBaseEngine *pIRecordDataBaseEngine; // 数据引擎
    IDataBaseEngine *pIKernelDataBaseEngine; // 数据引擎

    // 服务组件
    IMainServiceFrame *pIMainServiceFrame; // 服务框架
    IAndroidUserManager *pIAndroidUserManager; // 机器管理
    IGameServiceManager *pIGameServiceManager; // 服务管理

    // 配置变量
    tagGameParameter *pGameParameter; // 配置参数
    tagGameMatchOption *pGameMatchOption; // 比赛配置
    tagGameServiceAttrib *pGameServiceAttrib; // 桌子属性
    tagGameServiceOption *pGameServiceOption; // 配置参数
};

// 机器参数
struct tagAndroidUserParameter {
    // 服务配置
    bool bServiceContinue; // 服务延续
    DWORD dwMinSitInterval; // 坐下间隔
    DWORD dwMaxSitInterval; // 坐下间隔

    // 配置变量
    tagGameParameter *pGameParameter; // 配置参数
    tagGameServiceAttrib *pGameServiceAttrib; // 桌子属性
    tagGameServiceOption *pGameServiceOption; // 配置参数
    tagGameMatchOption *pGameMatchOption; // 比赛配置

    // 内核组件
    ITimerEngine *pITimerEngine; // 时间引擎
    IServerUserManager *pIServerUserManager; // 用户管理
    IGameServiceManager *pIGameServiceManager; // 服务管理
    IGameServiceCustomTime *pIGameServiceSustomTime; // 时间配置
    ITCPNetworkEngineEvent *pITCPNetworkEngineEvent; // 事件接口
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
    tagTaskParameter *pTaskParameter; // 任务参数
    tagUserTaskEntry *pNextTaskEntry; // 下一任务
    tagUserTaskEntry *pNextStatusEntry; // 下一任务
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
    IAndroidUserItem *pIAndroidUserFree[MAX_ANDROID]; // 机器接口
    IAndroidUserItem *pIAndroidUserPlay[MAX_ANDROID]; // 机器接口
    IAndroidUserItem *pIAndroidUserSitdown[MAX_ANDROID]; // 机器接口
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
    tagAndroidParameter *pAndroidParameter; // 机器参数
};

// 游戏数据
struct tagGameDataItem {
    INT nKey; // 子项索引
    TCHAR szValue[64]; // 子项内容
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceManager INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceManager = {
    0x4b2b9d8f, 0xce1b, 0x44f3, 0xa5, 0x22, 0x65, 0x1a, 0x65, 0xc9, 0x0a, 0x25
};

// 游戏接口
interface IGameServiceManager : public IUnknownEx {
    // 创建接口
public:
    // 创建桌子
    virtual VOID *CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer);

    // 创建机器
    virtual VOID *CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer);

    // 创建数据
    virtual VOID *CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer);

    // 参数接口
public:
    // 组件属性
    virtual bool GetServiceAttrib(tagGameServiceAttrib &GameServiceAttrib);

    // 调整参数
    virtual bool RectifyParameter(tagGameServiceOption &GameServiceOption);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceCustomRule INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceCustomRule = {
    0xc7ace01d, 0x75f8, 0x4af7, 0xb1, 0x80, 0xa8, 0x53, 0xcd, 0x2e, 0x0a, 0xb6
};

#define VER_IGameServiceCustomRule INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServicePersonalRule = {
    0xbb76044f, 0x388e, 0x4bd7, 0xa0, 0x15, 0xb0, 0x9b, 0x93, 0x4b, 0x9f, 0x9a
};

// 配置接口
interface IGameServiceCustomRule : public IUnknownEx {
    // 获取配置
    virtual bool SaveCustomRule(BYTE *pcbCustomRule, WORD wCustonSize);

    // 默认配置
    virtual bool DefaultCustomRule(BYTE *pcbCustomRule, WORD wCustonSize);

    // 创建窗口
    virtual bool CreateCustomRule(BYTE *pcbCustomRule, WORD wCustonSize);
};

// 配置接口
interface IGameServicePersonalRule : public IUnknownEx {
    // 获取配置
    virtual bool SavePersonalRule(BYTE *pcbCustomRule, WORD wCustonSize);

    // 默认配置
    virtual bool DefaultPersonalRule(BYTE *pcbCustomRule, WORD wCustonSize);

    // 创建窗口
    virtual bool CreatePersonalRule(BYTE *pcbCustomRule, WORD wCustonSize);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameServiceCustomTime INTERFACE_VERSION(1, 1)
static const GUID IID_IGameServiceCustomTime = {
    0xf57573b0, 0x63c3, 0x43a9, 0x8c, 0xb1, 0xe2, 0x22, 0xd3, 0x93, 0xe2, 0xb
};

// 时间配置
interface IGameServiceCustomTime : public IUnknownEx {
    // 机器脉冲
    virtual DWORD GetAndroidTimerPulse();

    // 时间范围
    virtual DWORD GetTableSinkTimeRange();

    // 时间单元
    virtual DWORD GetTimerEngineTimeCell();
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameDataBaseEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IGameDataBaseEngine = {
    0x4310e733, 0xc49b, 0x4592, 0xa3, 0xb1, 0x73, 0x18, 0xd1, 0x53, 0x2f, 0x3e
};

// 游戏数据
interface IGameDataBaseEngine : public IUnknownEx {
    // 配置参数
public:
    // 自定配置
    virtual VOID *GetCustomRule();

    // 服务属性
    virtual tagGameServiceAttrib *GetGameServiceAttrib();

    // 服务配置
    virtual tagGameServiceOption *GetGameServiceOption();

    // 获取对象
public:
    // 获取对象
    virtual VOID *GetDataBase(REFGUID Guid, DWORD dwQueryVer);

    // 获取对象
    virtual VOID *GetDataBaseEngine(REFGUID Guid, DWORD dwQueryVer);

    // 功能接口
public:
    // 投递结果
    virtual bool PostGameDataBaseResult(WORD wRequestID, VOID *pData, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IGameDataBaseEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IGameDataBaseEngineSink = {
    0xa6c5e2cc, 0x34c1, 0x422c, 0xa0, 0x1b, 0x54, 0xab, 0x68, 0xfa, 0xe6, 0x81
};

// 数据接口
interface IGameDataBaseEngineSink : public IUnknownEx {
    // 配置接口
    virtual bool InitializeSink(IUnknownEx *pIUnknownEx);

    // 数据处理
    virtual bool OnGameDataBaseRequest(DWORD dwUserID, WORD wTableID, WORD wChairID);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserItem INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserItem = {0xb5ce01a7, 0x5cd1, 0x4788, 0x94, 0x6c, 0xa1, 0xef, 0x5b, 0x30, 0x2c, 0xb7};

// 用户接口
interface IServerUserItem : public IUnknownEx {
    // 属性信息
public:
    // 用户索引
    virtual WORD GetBindIndex();

    // 用户地址
    virtual DWORD GetClientAddr();

    // 机器标识
    virtual LPCTSTR GetMachineID();

    // 登录信息
public:
    // 请求标识
    virtual DWORD GetDBQuestID();

    // 登录时间
    virtual DWORD GetLogonTime();

    // 记录索引
    virtual DWORD GetInoutIndex();

    // 用户信息
public:
    // 用户信息
    virtual tagUserInfo *GetUserInfo();

    // 用户规则
    virtual tagUserRule *GetUserRule();

    // 道具信息
    virtual tagUserProperty *GetUserProperty();

    // 时间信息
    virtual tagTimeInfo *GetTimeInfo();

    // 游戏数据
public:
    // 游戏数据
    virtual LPCTSTR GetUserGameData();

    // 游戏数据
    virtual VOID GetUserGameData(INT nKey, LPTSTR pszValue, INT nMaxCount);

    // 游戏数据
    virtual VOID SetUserGameData(LPCTSTR pszValue, INT nMaxCount);

    // 游戏数据
    virtual VOID SetUserGameData(INT nKey, LPCTSTR pszValue, INT nMaxCount);

    // 游戏数据
    virtual VOID WriteUserGameData();

    // 属性信息
public:
    // 用户性别
    virtual BYTE GetGender();

    // 用户标识
    virtual DWORD GetUserID();

    // 游戏标识
    virtual DWORD GetGameID();

    // 用户昵称
    virtual LPCTSTR GetNickName();

    // 状态接口
public:
    // 桌子号码
    virtual WORD GetTableID();

    // 桌子号码
    virtual WORD GetLastTableID();

    // 椅子号码
    virtual WORD GetChairID();

    // 用户状态
    virtual BYTE GetUserStatus();

    // 权限信息
public:
    // 用户权限
    virtual DWORD GetUserRight();

    // 管理权限
    virtual DWORD GetMasterRight();

    // 权限信息
public:
    // 用户权限
    virtual BYTE GetMemberOrder();

    // 管理权限
    virtual BYTE GetMasterOrder();

    // 积分信息
public:
    // 用户积分
    virtual SCORE GetUserScore();

    // 用户成绩
    virtual SCORE GetUserGrade();

    // 用户银行
    virtual SCORE GetUserInsure();

    // 托管信息
public:
    // 托管积分
    virtual SCORE GetTrusteeScore();

    // 锁定积分
    virtual SCORE GetFrozenedScore();

    // 积分信息
public:
    // 用户胜率
    virtual WORD GetUserWinRate();

    // 用户输率
    virtual WORD GetUserLostRate();

    // 用户和率
    virtual WORD GetUserDrawRate();

    // 用户逃率
    virtual WORD GetUserFleeRate();

    // 游戏局数
    virtual DWORD GetUserPlayCount();

    // 效验接口
public:
    // 对比帐号
    virtual bool ContrastNickName(LPCTSTR pszNickName);

    // 对比密码
    virtual bool ContrastLogonPass(LPCTSTR pszPassword);

    // 托管状态
public:
    // 判断状态
    virtual bool IsTrusteeUser();

    // 设置状态
    virtual VOID SetTrusteeUser(bool bTrusteeUser);

    // 游戏状态
public:
    // 连接状态
    virtual bool IsClientReady();

    // 设置连接
    virtual VOID SetClientReady(bool bClientReady);

    // 手机用户
public:
    // 控制状态
    virtual bool IsMobileUser();

    // 设置控制
    virtual VOID SetMobileUser(bool bMobileUser);

    // 控制用户
public:
    // 控制状态
    virtual bool IsAndroidUser();

    // 设置控制
    virtual VOID SetAndroidUser(bool bbMachineUser);

    // 比赛接口
public:
    // 报名数据
    virtual VOID *GetMatchData();

    // 报名数据
    virtual VOID SetMatchData(VOID *pMatchData);

    // 报名时间
    virtual DWORD GetSignUpTime();

    // 报名时间
    virtual VOID SetSignUpTime(DWORD dwSignUpTime);

    // 比赛状态
    virtual BYTE GetUserMatchStatus();

    // 比赛状态
    virtual VOID SetUserMatchStatus(BYTE cbMatchStatus);

    // 记录接口
public:
    // 变更判断
    virtual bool IsVariation();

    // 查询记录
    virtual bool QueryRecordInfo(tagVariationInfo &UserRecordInfo);

    // 提取变更
    virtual bool DistillVariation(tagVariationInfo &UserVariationInfo);

    // 管理接口
public:
    // 设置状态
    virtual bool SetUserStatus(BYTE cbUserStatus, WORD wTableID, WORD wChairID);

    // 写入积分
    virtual bool WriteUserScore(SCORE lScore, SCORE lGrade, SCORE lRevenue, SCORE lIngot, BYTE cbScoreType,
                                DWORD dwPlayTimeCount,
                                DWORD dwWinExperience);

    // 领取奖励
    virtual bool SetUserTaskReward(SCORE lScore, SCORE lIngot);

    // 修改权限
    virtual VOID ModifyUserRight(DWORD dwAddRight, DWORD dwRemoveRight, BYTE cbRightKind = UR_KIND_GAME);

    // 冻结接口
public:
    // 冻结积分
    virtual bool FrozonUserScore(SCORE lScore);

    // 解冻积分
    virtual bool UnFrozonUserScore(SCORE lScore);

    // 修改接口
public:
    // 修改信息
    virtual bool ModifyUserProperty(SCORE lScore, LONG lLoveLiness);

    // 高级接口
public:
    // 解除绑定
    virtual bool DetachBindStatus();

    // 银行操作
    virtual bool ModifyUserInsure(SCORE lScore, SCORE lInsure, SCORE lRevenue);

    // 设置参数
    virtual bool SetUserParameter(DWORD dwClientAddr, WORD wBindIndex, TCHAR szMachineID[LEN_MACHINE_ID],
                                  bool bAndroidUser, bool bClientReady);

    // 手机定义
public:
    // 手机规则
    virtual WORD GetMobileUserRule();

    // 设置定义
    virtual VOID SetMobileUserRule(WORD wMobileUserRule);

    // 当前分页
    virtual WORD GetMobileUserDeskPos();

    // 当前分页
    virtual VOID SetMobileUserDeskPos(WORD wMobileUserDeskPos);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserService INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserService = {0xcc4e885f, 0x3357, 0x4d68, 0xb2, 0x6b, 0x78, 0x8f, 0x4b, 0x3e, 0x64, 0xc3};

// 用户服务
interface IServerUserService : public IUnknownEx {
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMatchUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IMatchUserItemSink = {0xE1AEB013, 0xB63C, 0x45CB, 0x8B, 0x60, 0xA2, 0x75, 0x34, 0xE5, 0x2E, 0xDA};

// 状态接口
interface IMatchUserItemSink : public IUnknownEx {
    // 用户状态
    virtual bool OnEventMatchUserStatus(IServerUserItem *pIServerUserItem, BYTE cbOldUserStatus, BYTE cbCurrUserStatus);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserItemSink = {
    0x9d0cfe02, 0x0fe9, 0x4a8b, 0x97, 0x95, 0xac, 0x32, 0x67, 0x5a, 0xf8, 0xb1
};

// 状态接口
interface IServerUserItemSink : public IUnknownEx {
    // 用户积分
    virtual bool OnEventUserItemScore(IServerUserItem *pIServerUserItem, BYTE cbReason);

    // 用户数据
    virtual bool OnEventUserItemGameData(IServerUserItem *pIServerUserItem, BYTE cbReason);

    // 用户状态
    virtual bool OnEventUserItemStatus(IServerUserItem *pIServerUserItem, WORD wOldTableID = INVALID_TABLE,
                                       WORD wOldChairID = INVALID_CHAIR);

    // 用户权限
    virtual bool OnEventUserItemRight(IServerUserItem *pIServerUserItem, DWORD dwAddRight, DWORD dwRemoveRight,
                                      BYTE cbRightKind);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServerUserManager INTERFACE_VERSION(1, 1)
static const GUID IID_IServerUserManager = {0x77a3c4df, 0x1d95, 0x48c6, 0xac, 0x9d, 0x75, 0xd7, 0x6c, 0x2a, 0x3c, 0x0e};

// 用户管理
interface IServerUserManager : public IUnknownEx {
    // 配置接口
public:
    // 设置接口
    virtual bool SetServerUserItemSink(IUnknownEx *pIUnknownEx);

    // 设置接口
    virtual bool SetMatchUserItemSink(IUnknownEx *pIUnknownEx);

    // 查找接口
public:
    // 枚举用户
    virtual IServerUserItem *EnumUserItem(WORD wEnumIndex);

    // 查找用户
    virtual IServerUserItem *SearchUserItem(DWORD dwUserID);

    // 查找用户
    virtual IServerUserItem *SearchUserItem(LPCTSTR pszNickName);

    // 统计接口
public:
    // 机器人数
    virtual DWORD GetAndroidCount();

    // 在线人数
    virtual DWORD GetUserItemCount();

    // 管理接口
public:
    // 删除用户
    virtual bool DeleteUserItem();

    // 删除用户
    virtual bool DeleteUserItem(IServerUserItem *pIServerUserItem);

    // 插入用户
    virtual bool InsertUserItem(IServerUserItem * *pIServerUserResult, tagUserInfo &UserInfo,
                                tagUserInfoPlus &UserInfoPlus);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableFrame INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrame = {0x860cd1cf, 0x1a4f, 0x4e35, 0xb8, 0x0c, 0xd2, 0x46, 0xa4, 0xef, 0xfd, 0xfb};

// 桌子接口
interface ITableFrame : public IUnknownEx {
    // 属性接口
public:
    // 桌子号码
    virtual WORD GetTableID();

    // 游戏人数
    virtual WORD GetChairCount();

    // 空位置数目
    virtual WORD GetNullChairCount();

    // 配置参数
public:
    // 自定配置
    virtual VOID *GetCustomRule();

    // 比赛配置
    virtual tagGameMatchOption *GetGameMatchOption();

    // 服务属性
    virtual tagGameServiceAttrib *GetGameServiceAttrib();

    // 服务配置
    virtual tagGameServiceOption *GetGameServiceOption();

    // 配置接口
public:
    // 开始模式
    virtual BYTE GetStartMode();

    // 开始模式
    virtual VOID SetStartMode(BYTE cbStartMode);

    // 单元积分
public:
    // 单元积分
    virtual LONG GetCellScore();

    // 单元积分
    virtual VOID SetCellScore(LONG lCellScore);

    // 信息接口
public:
    // 锁定状态
    virtual bool IsTableLocked();

    // 游戏状态
    virtual bool IsGameStarted();

    // 游戏状态
    virtual bool IsDrawStarted();

    // 游戏状态
    virtual bool IsTableStarted();

    // 私人桌子
public:
    // 私人桌锁
    virtual bool IsPersonalTableLocked();

    // 设置锁定
    virtual VOID SetPersonalTableLlocked(bool bLocked);

    // 设置桌主
    virtual VOID SetTableOwner(DWORD dwUserID);

    // 获取桌主
    virtual DWORD GetTableOwner();

    // 设置桌子
    virtual VOID SetPersonalTable(DWORD dwDrawCountLimit, DWORD dwDrawTimeLimit, LONGLONG lIniScore);

    // 设置桌子参数
    virtual VOID SetPersonalTableParameter(tagPersonalTableParameter PersonalTableParameter,
                                           tagPersonalRoomOption PersonalRoomOption);

    // 获取分数
    virtual bool GetPersonalScore(DWORD dwUserID, LONGLONG &lScore);

    // 桌子信息
    virtual tagPersonalTableParameter GetPersonalTableParameter();

    // 桌子创建后多长时间未开始游戏 解散桌子
    virtual VOID SetTimerNotBeginAfterCreate();

    // 状态接口
public:
    // 获取状态
    virtual BYTE GetGameStatus();

    // 设置状态
    virtual VOID SetGameStatus(BYTE bGameStatus);

    // 控制接口
public:
    // 开始游戏
    virtual bool StartGame();

    // 解散游戏
    virtual bool DismissGame();

    // 结束游戏
    virtual bool ConcludeGame(BYTE cbGameStatus);

    // 写分接口
public:
    // 写入积分
    virtual bool WriteUserScore(WORD wChairID, tagScoreInfo &ScoreInfo, DWORD dwGameMemal = INVALID_DWORD,
                                DWORD dwPlayGameTime = INVALID_DWORD);

    // 写入积分
    virtual bool WriteTableScore(tagScoreInfo ScoreInfoArray[], WORD wScoreCount);

    // 计算接口
public:
    // 计算税收
    virtual SCORE CalculateRevenue(WORD wChairID, SCORE lScore);

    // 消费限额
    virtual SCORE QueryConsumeQuota(IServerUserItem *pIServerUserItem);

    // 用户接口
public:
    // 寻找用户
    virtual IServerUserItem *SearchUserItem(DWORD dwUserID);

    // 游戏用户
    virtual IServerUserItem *GetTableUserItem(WORD wChairID);

    // 旁观用户
    virtual IServerUserItem *EnumLookonUserItem(WORD wEnumIndex);

    // 时间接口
public:
    // 设置时间
    virtual bool SetGameTimer(DWORD dwTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM dwBindParameter);

    // 删除时间
    virtual bool KillGameTimer(DWORD dwTimerID);

    // 网络接口
public:
    // 发送数据
    virtual bool SendTableData(WORD wChairID, WORD wSubCmdID);

    // 发送数据
    virtual bool SendTableData(WORD wChairID, WORD wSubCmdID, VOID *pData, WORD wDataSize,
                               WORD wMainCmdID = MDM_GF_GAME);

    // 发送数据
    virtual bool SendLookonData(WORD wChairID, WORD wSubCmdID);

    // 发送数据
    virtual bool SendLookonData(WORD wChairID, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 发送数据
    virtual bool SendUserItemData(IServerUserItem *pIServerUserItem, WORD wSubCmdID);

    // 发送数据
    virtual bool SendUserItemData(IServerUserItem *pIServerUserItem, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 功能接口
public:
    // 发送消息
    virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType);

    // 游戏消息
    virtual bool SendGameMessage(IServerUserItem *pIServerUserItem, LPCTSTR lpszMessage, WORD wType);

    // 房间消息
    virtual bool SendRoomMessage(IServerUserItem *pIServerUserItem, LPCTSTR lpszMessage, WORD wType);

    // 动作处理
public:
    // 起立动作
    virtual bool PerformStandUpAction(IServerUserItem *pIServerUserItem, bool bInitiative = false);

    // 旁观动作
    virtual bool PerformLookonAction(WORD wChairID, IServerUserItem *pIServerUserItem);

    // 坐下动作
    virtual bool PerformSitDownAction(WORD wChairID, IServerUserItem *pIServerUserItem, LPCTSTR lpszPassword = NULL);

    // 功能接口
public:
    // 发送场景
    virtual bool SendGameScene(IServerUserItem *pIServerUserItem, VOID *pData, WORD wDataSize);

    // 比赛接口
public:
    // 获取接口
    virtual IUnknownEx *GetTableFrameHook();

    // 设置接口
    virtual bool SetTableFrameHook(IUnknownEx *pIUnknownEx);

    // 伪造配置
    virtual bool ImitateGameOption(IServerUserItem *pIServerUserItem);

    // 聊天接口
public:
    virtual bool SendChatMessage(IServerUserItem *pIServerUserItem, IServerUserItem *pITargetServerUserItem,
                                 DWORD dwChatColor,
                                 LPCTSTR lpszChatString, LPTSTR lpszDescribeString);

public:
    // 获取游戏规则
    virtual BYTE *GetGameRule();

    // 获取结算时的特殊信息
    virtual void GetSpeicalInfo(BYTE *cbSpecialInfo, int nSpecialLen);

    // 设置桌子上椅子的个数
    virtual VOID SetTableChairCount(WORD wChairCount);

public:
    // 私人房间是否解散
    virtual bool IsPersonalRoomDisumme();

    // 设置是金币数据库还是积分数据库,  0 为金币库 1 为 积分库
    virtual void SetDataBaseMode(BYTE cbDataBaseMode);

    // 获取数据库模式,  0 为金币库 1 为 积分库
    virtual BYTE GetDataBaseMode();

    // 设置创建时间
    virtual void SetCreatePersonalTime(SYSTEMTIME tm);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ICompilationSink INTERFACE_VERSION(1, 1)
static const GUID IID_ICompilationSink = {0xA37F720F, 0x15CD, 0x41E4, 0xB8, 0x6C, 0xE0, 0xA7, 0xEC, 0xAB, 0x31, 0x89};

// 回调接口
interface ICompilationSink : public IUnknownEx {
    // 获取信息
public:
    // 获取信息
    virtual LPCTSTR GetCompilation();
};

//////////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
#define VER_ITableFrameSink INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrameSink = {0x9476b154, 0x8beb, 0x4f7e, 0xaf, 0x64, 0xd2, 0xb1, 0x1a, 0xda, 0x5e, 0xc4};
#else
#define VER_ITableFrameSink INTERFACE_VERSION(1, 1)
static const GUID IID_ITableFrameSink = {0x38a74df5, 0x6245, 0x46c7, 0xb6, 0xce, 0x53, 0xf9, 0xd5, 0xbf, 0x6d, 0xe6};
#endif

// 回调接口
interface ITableFrameSink : public IUnknownEx {
    // 管理接口
public:
    // 复位接口
    virtual VOID RepositionSink();

    // 配置接口
    virtual bool Initialization(IUnknownEx *pIUnknownEx);

    // 查询接口
public:
    // 查询限额
    virtual SCORE QueryConsumeQuota(IServerUserItem *pIServerUserItem);

    // 最少积分
    virtual SCORE QueryLessEnterScore(WORD wChairID, IServerUserItem *pIServerUserItem);

    // 查询是否扣服务费
    virtual bool QueryBuckleServiceCharge(WORD wChairID);

    // 游戏事件
public:
    // 游戏开始
    virtual bool OnEventGameStart();

    // 游戏结束
    virtual bool OnEventGameConclude(WORD wChairID, IServerUserItem *pIServerUserItem, BYTE cbReason);

    // 发送场景
    virtual bool OnEventSendGameScene(WORD wChairID, IServerUserItem *pIServerUserItem, BYTE cbGameStatus,
                                      bool bSendSecret);

    // 事件接口
public:
    // 时间事件
    virtual bool OnTimerMessage(DWORD dwTimerID, WPARAM dwBindParameter);

    // 数据事件
    virtual bool OnDataBaseMessage(WORD wRequestID, VOID *pData, WORD wDataSize);

    // 积分事件
    virtual bool OnUserScoreNotify(WORD wChairID, IServerUserItem *pIServerUserItem, BYTE cbReason);

    // 网络接口
public:
    // 游戏消息
    virtual bool OnGameMessage(WORD wSubCmdID, VOID *pData, WORD wDataSize, IServerUserItem *pIServerUserItem);

    // 框架消息
    virtual bool OnFrameMessage(WORD wSubCmdID, VOID *pData, WORD wDataSize, IServerUserItem *pIServerUserItem);

    // 比赛接口
public:
    // 设置基数
    virtual void SetGameBaseScore(LONG lBaseScore);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableUserAction INTERFACE_VERSION(1, 1)
static const GUID IID_ITableUserAction = {0xc97c060b, 0xcf0e, 0x40b7, 0x93, 0x30, 0x97, 0xa4, 0xf6, 0x8c, 0xca, 0x84};

// 用户动作
interface ITableUserAction : public IUnknownEx {
    // 用户断线
    virtual bool OnActionUserOffLine(WORD wChairID, IServerUserItem *pIServerUserItem);

    // 用户重入
    virtual bool OnActionUserConnect(WORD wChairID, IServerUserItem *pIServerUserItem);

    // 用户坐下
    virtual bool OnActionUserSitDown(WORD wChairID, IServerUserItem *pIServerUserItem, bool bLookonUser);

    // 用户起来
    virtual bool OnActionUserStandUp(WORD wChairID, IServerUserItem *pIServerUserItem, bool bLookonUser);

    // 用户同意
    virtual bool OnActionUserOnReady(WORD wChairID, IServerUserItem *pIServerUserItem, VOID *pData, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITableUserRequest INTERFACE_VERSION(1, 1)
static const GUID IID_ITableUserRequest = {0x7ad17e89, 0xcb5b, 0x472a, 0xac, 0xeb, 0x84, 0x4d, 0x4f, 0xa1, 0x4c, 0x38};

// 用户请求
interface ITableUserRequest : public IUnknownEx {
    // 旁观请求
    virtual bool OnUserRequestLookon(WORD wChairID, IServerUserItem *pIServerUserItem, tagRequestResult &RequestResult);

    // 坐下请求
    virtual bool OnUserRequestSitDown(WORD wChairID, IServerUserItem *pIServerUserItem,
                                      tagRequestResult &RequestResult);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IMainServiceFrame INTERFACE_VERSION(1, 1)
static const GUID IID_IMainServiceFrame = {0xbaaf5584, 0xf9b4, 0x41b6, 0xae, 0x6b, 0xef, 0x4d, 0x54, 0x41, 0xf8, 0x32};

// 服务框架
interface IMainServiceFrame : public IUnknownEx {
    // 消息接口
public:
    // 房间消息
    virtual bool SendRoomMessage(LPCTSTR lpszMessage, WORD wType);

    // 游戏消息
    virtual bool SendGameMessage(LPCTSTR lpszMessage, WORD wType);

    // 房间消息
    virtual bool SendRoomMessage(IServerUserItem *pIServerUserItem, LPCTSTR lpszMessage, WORD wType);

    // 游戏消息
    virtual bool SendGameMessage(IServerUserItem *pIServerUserItem, LPCTSTR lpszMessage, WORD wType);

    // 房间消息
    virtual bool SendRoomMessage(DWORD dwSocketID, LPCTSTR lpszMessage, WORD wType, bool bAndroid);

    // 网络接口
public:
    // 发送数据
    virtual bool SendData(BYTE cbSendMask, WORD wMainCmdID, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 发送数据
    virtual bool SendData(DWORD dwContextID, WORD wMainCmdID, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 发送数据
    virtual bool SendData(IServerUserItem *pIServerUserItem, WORD wMainCmdID, WORD wSubCmdID, VOID *pData,
                          WORD wDataSize);

    // 群发数据
    virtual bool SendDataBatchToMobileUser(WORD wCmdTable, WORD wMainCmdID, WORD wSubCmdID, VOID *pData,
                                           WORD wDataSize);

    // 功能接口
public:
    // 断开协调
    virtual bool DisconnectCorrespond();

    // 插入分配
    virtual bool InsertDistribute(IServerUserItem *pIServerUserItem);

    // 删除用户
    virtual bool DeleteDistribute(IServerUserItem *pIServerUserItem);

    // 敏感词过滤
    virtual void SensitiveWordFilter(LPCTSTR pMsg, LPTSTR pszFiltered, int nMaxLen);

    // 解锁机器人
    virtual VOID UnLockAndroidUser(WORD wServerID, WORD wBatchID);

    // 解散私人桌子
    virtual VOID DismissPersonalTable(WORD wServerID, WORD wTableID);

    // 取消创建
    virtual VOID CancelCreateTable(DWORD dwUserID, DWORD dwDrawCountLimit, DWORD dwDrawTimeLimit, DWORD dwReason,
                                   WORD wTableID, TCHAR *szRoomID);

    // 开始游戏写入参与信息
    virtual VOID PersonalRoomWriteJoinInfo(DWORD dwUserID, WORD wTableID, WORD wChairID, DWORD dwKindID,
                                           TCHAR *szRoomID, TCHAR *szPersonalRoomGUID);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserItem INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserItem = {0xb1faa2f4, 0x9804, 0x4c6f, 0x9d, 0xfc, 0xb1, 0x0a, 0x08, 0x8a, 0x22, 0x69};

// 机器人接口
interface IAndroidUserItem : public IUnknownEx {
    // 信息接口
public:
    // 获取 I D
    virtual DWORD GetUserID();

    // 桌子号码
    virtual WORD GetTableID();

    // 椅子号码
    virtual WORD GetChairID();

    // 状态函数
public:
    // 获取状态
    virtual BYTE GetGameStatus();

    // 设置状态
    virtual VOID SetGameStatus(BYTE cbGameStatus);

    // 配置信息
public:
    // 获取状态
    virtual tagAndroidService *GetAndroidService();

    // 获取配置
    virtual tagAndroidParameter *GetAndroidParameter();

    // 功能接口
public:
    // 获取自己
    virtual IServerUserItem *GetMeUserItem();

    // 游戏用户
    virtual IServerUserItem *GetTableUserItem(WORD wChariID);

    // 银行接口
public:
    // 存入游戏币
    virtual bool PerformSaveScore(SCORE lScore);

    // 提取游戏币
    virtual bool PerformTakeScore(SCORE lScore);

    // 网络接口
public:
    // 发送函数
    virtual bool SendSocketData(WORD wSubCmdID);

    // 发送函数
    virtual bool SendSocketData(WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 动作接口
public:
    // 机器动作
    virtual bool JudgeAndroidActionAndRemove(WORD wAction);

    // 功能接口
public:
    // 删除时间
    virtual bool KillGameTimer(UINT nTimerID);

    // 设置时间
    virtual bool SetGameTimer(UINT nTimerID, UINT nElapse);

    // 发送准备
    virtual bool SendUserReady(VOID *pData, WORD wDataSize);

    // 发送聊天
    virtual bool SendChatMessage(DWORD dwTargetUserID, LPCTSTR pszChatString, COLORREF crFontColor);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserItemSink INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserItemSink = {
    0x1e8a1918, 0x572b, 0x453b, 0xbc, 0x0b, 0x6b, 0x61, 0x70, 0xa3, 0x3c, 0xca
};

// 机器人接口
interface IAndroidUserItemSink : public IUnknownEx {
    // 控制接口
public:
    // 重置接口
    virtual bool RepositionSink();

    // 初始接口
    virtual bool Initialization(IUnknownEx *pIUnknownEx);

    // 游戏事件
public:
    // 时间消息
    virtual bool OnEventTimer(UINT nTimerID);

    // 游戏消息
    virtual bool OnEventGameMessage(WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 游戏消息
    virtual bool OnEventFrameMessage(WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 场景消息
    virtual bool OnEventSceneMessage(BYTE cbGameStatus, bool bLookonOther, VOID *pData, WORD wDataSize);

    // 用户事件
public:
    // 用户进入
    virtual VOID OnEventUserEnter(IAndroidUserItem *pIAndroidUserItem, bool bLookonUser);

    // 用户离开
    virtual VOID OnEventUserLeave(IAndroidUserItem *pIAndroidUserItem, bool bLookonUser);

    // 用户积分
    virtual VOID OnEventUserScore(IAndroidUserItem *pIAndroidUserItem, bool bLookonUser);

    // 用户状态
    virtual VOID OnEventUserStatus(IAndroidUserItem *pIAndroidUserItem, bool bLookonUser);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IUserTaskManagerSink INTERFACE_VERSION(1, 1)
static const GUID IID_IUserTaskManagerSink = {
    0x1c721508, 0x81f8, 0x403f, 0xb5, 0xc9, 0x3e, 0xb3, 0xe3, 0xe5, 0xcd, 0xa9
};

// 任务接口
interface IUserTaskManagerSink : public IUnknownEx {
    // 任务参数
public:
    // 移除参数
    virtual VOID RemoveTaskParameter();

    // 查找参数
    virtual tagTaskParameter *SearchTaskParameter(WORD wTaskID);

    // 枚举参数
    virtual tagTaskParameter *EnumTaskParameter(CTaskParameterMap::iterator &Position);

    // 添加参数
    virtual bool AddTaskParameter(tagTaskParameter TaskParameter[], WORD wPatemterCount);

    // 获取参数数目
    virtual WORD GetTaskParameterCount();

    // 用户任务
public:
    // 移除任务
    virtual VOID RemoveUserTask(DWORD dwUserID);

    // 获取任务
    virtual tagUserTaskEntry *GetUserTaskEntry(DWORD dwUserID);

    // 获取任务
    virtual tagUserTaskEntry *GetUserTaskEntry(DWORD dwUserID, BYTE cbTaskStatus);

    // 设置任务
    virtual VOID SetUserTaskInfo(DWORD dwUserID, tagUserTaskInfo UserTaskInfo[], WORD wTaskCount);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAndroidUserManager INTERFACE_VERSION(1, 1)
static const GUID IID_IAndroidUserManager = {
    0xba43054d, 0x924b, 0x4013, 0xb2, 0x6d, 0xa6, 0x91, 0xb6, 0x20, 0x23, 0xb2
};

// 机器人接口
interface IAndroidUserManager : public IUnknownEx {
    // 控制接口
public:
    // 启动服务
    virtual bool StartService();

    // 停止服务
    virtual bool ConcludeService();

    // 配置接口
public:
    // 配置组件
    virtual bool InitAndroidUser(tagAndroidUserParameter &AndroidUserParameter);

    // 移除参数
    virtual bool RemoveAndroidParameter(DWORD dwBatchID);

    // 添加参数
    virtual bool AddAndroidParameter(tagAndroidParameter AndroidParameter[], WORD wParemeterCount);

    // 插入机器
    virtual bool InsertAndroidInfo(tagAndroidAccountsInfo AndroidAccountsInfo[], WORD wAndroidCount, DWORD dwBatchID);

    // 管理接口
public:
    // 删除机器
    virtual bool DeleteAndroidUserItem(DWORD dwAndroidID, bool bStockRetrieve);

    // 查找机器
    virtual IAndroidUserItem *SearchAndroidUserItem(DWORD dwUserID, DWORD dwContextID);

    // 创建机器
    virtual IAndroidUserItem *CreateAndroidUserItem(tagAndroidItemConfig &AndroidItemConfig);

    // 事件接口
public:
    // 脉冲事件
    virtual bool OnEventTimerPulse(DWORD dwTimerID, WPARAM dwBindParameter);

    // 状态接口
public:
    // 机器数目
    virtual WORD GetAndroidCount();

    // 加载机器
    virtual bool GetAndroidLoadInfo(DWORD &dwBatchID, DWORD &dwLoadCount);

    // 用户状况
    virtual WORD GetAndroidUserInfo(tagAndroidUserInfo &AndroidUserInfo, DWORD dwServiceMode);

    // 获取房间配置
    virtual tagGameServiceOption *GetGameServiceOption();

    // 获取游戏属性
    virtual tagGameServiceAttrib *GetGameServiceAttrib();

    // 获取比赛配置
    virtual tagGameMatchOption *GetGameMatchOption();

    // 网络接口
public:
    // 发送数据
    virtual bool SendDataToClient(WORD wMainCmdID, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 发送数据
    virtual bool SendDataToClient(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID *pData, WORD wDataSize);

    // 发送数据
    virtual bool SendDataToServer(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID *pData, WORD wDataSize);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IQueryServiceSink INTERFACE_VERSION(1, 1)
static const GUID IID_IQueryServiceSink = {0xba43054d, 0x924b, 0x4013, 0xb2, 0x6d, 0xa6, 0x96, 0xb6, 0x26, 0x26, 0xb2};

// 查询接口
interface IQueryServiceSink : public IUnknownEx {
    // 查询是否扣服务费
    virtual bool QueryBuckleServiceCharge(WORD wChairID);
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDBCorrespondManager INTERFACE_VERSION(1, 1)
static const GUID IID_IDBCorrespondManager = {
    0xba43154d, 0x924b, 0x4018, 0xb2, 0x6d, 0xa6, 0x96, 0xb6, 0x28, 0x26, 0xb2
};

// 查询接口
interface IDBCorrespondManager : public IServiceModule {
    // 配置接口
public:
    // 配置模块
    virtual bool InitDBCorrespondManager(IDataBaseEngine *pIDataBaseEngine);

    // 控制事件
public:
    // 请求事件
    virtual bool PostDataBaseRequest(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID *pData, WORD wDataSize,
                                     BYTE cbCache = FALSE);

    // 同步事件
public:
    // 请求完成
    virtual bool OnPostRequestComplete(DWORD dwUserID, bool bSucceed);

    // 定时事件
public:
    // 定时事件
    virtual bool OnTimerNotify();
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ICompilationSink INTERFACE_VERSION(1, 1)
static const GUID IID_IGobalLogFile = {0xa62cf097, 0xcb15, 0x43a9, 0xad, 0x1a, 0xb4, 0x08, 0xb0, 0xb9, 0x32, 0x9a};

// 日志接口
interface IGobalLogFile : public IUnknownEx {
public:
    // 游戏日志
    virtual void WriteGameLog(LPCTSTR pszLogString);
};


// 游戏服务
DECLARE_MODULE_DYNAMIC(GameServiceManager)

DECLARE_MODULE_DYNAMIC(AndroidUserItemSink)

//////////////////////////////////////////////////////////////////////////////////
