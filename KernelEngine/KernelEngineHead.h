#pragma once

// 第三方头文件
#include <asio.hpp>
#include <spdlog/spdlog.h>

// 本项目头文件
#include "nanodbc.h"

#include "ServiceCore/ServiceCoreHead.h"
#include "ServiceCore/StringUtils.h"

#if ASIO_VERSION >= 101800
using executor_type = asio::any_io_executor;
#else
using executor_type = asio::executor;
#endif

namespace nf {
  typedef std::shared_ptr<std::vector<uint8_t>> BufferPtr;
}

//////////////////////////////////////////////////////////////////////////////////
// 导出定义

#ifdef _WIN32
#ifdef KERNEL_ENGINE_DLL
#define KERNEL_ENGINE_CLASS _declspec(dllexport)
#else
#define KERNEL_ENGINE_CLASS _declspec(dllimport)
#endif
#define KERNEL_ENGINE_DLL_NAME TEXT("KernelEngine.dll") // 组件名字
#else
#define KERNEL_ENGINE_CLASS
#define KERNEL_ENGINE_DLL_NAME TEXT("KernelEngine.so") // 组件名字
#endif

//////////////////////////////////////////////////////////////////////////////////
// 系统常量

// 常量定义
#define TIME_CELL 200 // 时间单元
#define TIMES_INFINITY DWORD(-1) // 无限次数
#define MAX_ASYNCHRONISM_DATA 16384 // 异步数据

//////////////////////////////////////////////////////////////////////////////////
// 网络定义

// 连接错误
#define CONNECT_SUCCESS 0 // 连接成功
#define CONNECT_FAILURE 1 // 连接失败
#define CONNECT_EXCEPTION 2 // 参数异常

// 网络状态
#define SOCKET_STATUS_IDLE 0 // 空闲状态
#define SOCKET_STATUS_WAIT 1 // 等待状态
#define SOCKET_STATUS_CONNECT 2 // 连接状态

// 关闭原因
#define SHUT_REASON_INSIDE 0 // 内部原因
#define SHUT_REASON_NORMAL 1 // 正常关闭
#define SHUT_REASON_REMOTE 2 // 远程关闭
#define SHUT_REASON_TIME_OUT 3 // 网络超时
#define SHUT_REASON_EXCEPTION 4 // 异常关闭

//////////////////////////////////////////////////////////////////////////////////
// 枚举定义

// 输出等级
enum enTraceLevel {
  TraceLevel_Debug = spdlog::level::debug, // 调试消息
  TraceLevel_Info = spdlog::level::info, // 信息消息
  TraceLevel_Normal = TraceLevel_Info, // 普通消息
  TraceLevel_Warning = spdlog::level::warn, // 警告消息
  TraceLevel_Exception = spdlog::level::err, // 异常消息
};

// SQL 异常类型
enum enSQLException {
  SQLException_None = 0, // 没有异常
  SQLException_Connect = 1, // 连接错误
  SQLException_Syntax = 2, // 语法错误
};

//////////////////////////////////////////////////////////////////////////////////
// // 事件定义
//
// // 事件标识
// #define EVENT_TIMER 0x0001 // 时间事件
// #define EVENT_CONTROL 0x0002 // 控制事件
// #define EVENT_DATABASE 0x0003 // 数据库事件
//
// // 网络事件
// #define EVENT_TCP_SOCKET_READ 0x0004 // 读取事件
// #define EVENT_TCP_SOCKET_SHUT 0x0005 // 关闭事件
// #define EVENT_TCP_SOCKET_LINK 0x0006 // 连接事件
//
// // 网络事件
// #define EVENT_TCP_NETWORK_ACCEPT 0x0007 // 应答事件
// #define EVENT_TCP_NETWORK_READ 0x0008 // 读取事件
// #define EVENT_TCP_NETWORK_SHUT 0x0009 // 关闭事件
//
// // 事件掩码
// #define EVENT_MASK_KERNEL 0x00FF // 内核事件
// #define EVENT_MASK_CUSTOM 0xFF00 // 自定义事件

//////////////////////////////////////////////////////////////////////////////////

// 控制事件
struct NTY_ControlEvent {
  WORD control_id; // 控制标识
  // 下面存储额外数据
};

// 定时器事件
struct NTY_TimerEvent {
  DWORD timer_id; // 时间标识
  WPARAM bind_parameter; // 绑定参数
};

// 数据库事件
struct NTY_DataBaseEvent {
  WORD request_id; // 请求标识
  DWORD context_id; // 对象标识
  // 下面存储额外数据
};

// 读取事件
struct NTY_TCPSocketReadEvent {
  WORD service_id; // 服务标识
  TCP_Command command; // 命令信息
  WORD data_size; // 数据大小
  // 下面存储真实数据
};

// 关闭事件
struct NTY_TCPSocketShutEvent {
  WORD service_id; // 服务标识
  BYTE shut_reason; // 关闭原因
};

// 连接事件
struct NTY_TCPSocketLinkEvent {
  WORD service_id; // 服务标识
  INT error_code; // 错误代码
};

// 应答事件
struct NTY_TCPNetworkAcceptEvent {
  DWORD socket_id; // 网络标识
  DWORD client_addr; // 连接地址
};

// 读取事件
struct NTY_TCPNetworkReadEvent {
  DWORD socket_id; // 网络标识
  TCP_Command command; // 命令信息
  WORD data_size; // 数据大小
  // 下面存储真实数据
};

// 关闭事件
struct NTY_TCPNetworkShutEvent {
  DWORD socket_id; // 网络标识
  DWORD client_addr; // 连接地址
  DWORD active_time; // 连接时间
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseException INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseException = {0x428361ed, 0x9dfa, 0x43d7, 0x008f, 0x26, 0x17, 0x06, 0x47, 0x6b, 0x2a, 0x51};

// 数据库异常
interface IDataBaseException : virtual IUnknownEx {
  // 异常代码
  virtual HRESULT GetExceptionResult() = 0;
  // 异常描述
  virtual LPCTSTR GetExceptionDescribe() = 0;
  // 异常类型
  virtual enSQLException GetExceptionType() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITraceService INTERFACE_VERSION(1, 1)
static const GUID IID_ITraceService = {0xe5f636c6, 0xabb5, 0x4752, 0x00bb, 0xc8, 0xcd, 0xb1, 0x76, 0x58, 0xf5, 0x2d};

// 事件输出
interface ITraceService : virtual IUnknownEx {
  virtual void TraceDebug(const StringT& msg) = 0;
  virtual void TraceInfo(const StringT& msg) = 0;
  virtual void TraceWarn(const StringT& msg) = 0;
  virtual void TraceError(const StringT& msg) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServiceModule INTERFACE_VERSION(1, 1)
static const GUID IID_IServiceModule = {0x05980504, 0xa2f2, 0x4b0f, 0x009b, 0x54, 0x51, 0x54, 0x1e, 0x05, 0x5c, 0xff};

// 服务模块
interface IServiceModule : virtual IUnknownEx {
  // 启动服务(避免跟 Win32 的 StartService 重名)
  virtual bool InitiateService(std::shared_ptr<asio::io_context> io_context) = 0;
  // 停止服务
  virtual bool ConcludeService() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAsynchronismEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IAsynchronismEngine = {0xc7a13074, 0x75c5, 0x4b8e, 0xb5, 0x4b, 0xee, 0x0e, 0xec, 0xfe, 0xb9, 0xeb};

// 异步引擎
interface IAsynchronismEngine : IServiceModule {
  // 配置接口
public:
  // 队列负荷
  virtual bool GetBurdenInfo(tagBurdenInfo & BurdenInfo) = 0;
  // 设置模块
  virtual bool SetAsynchronismSink(IUnknownEx * pIUnknownEx) = 0;

  // 投递接口
public:
  // 投递数据
  // 1. PostAsynchronismData(1, std::vector<uint8_t>{1,2,3}); // 传右值给函数
  // 2. std::vector<uint8_t>{1,2,3}; PostAsynchronismData(1, std::move(data));
  virtual bool PostAsynchronismData(WORD identifier, nf::BufferPtr data) = 0;
  // virtual bool PostAsynchronismData(WORD wIdentifier, VOID* pData, WORD wDataSize) = 0;
  // 投递数据
  // virtual bool PostAsynchronismData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAsynchronismEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IAsynchronismEngineSink = {0x2edf5c9e, 0x2cac, 0x461d, 0x00a7, 0x82, 0x2e, 0x2f, 0xe1, 0x91, 0x80, 0xf8};

// 异步钩子
interface IAsynchronismEngineSink : virtual IUnknownEx {
  // 启动事件
  virtual bool OnAsynchronismEngineStart() = 0;
  // 停止事件
  virtual bool OnAsynchronismEngineConclude() = 0;
  // 异步数据
  virtual bool OnAsynchronismEngineData(WORD identifier, nf::BufferPtr data) = 0;
  // virtual bool OnAsynchronismEngineData(WORD wIdentifier, VOID* pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBase INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBase = {0xa2e38a78, 0x1e4f, 0x4de4, 0x00a5, 0xd1, 0xb9, 0x19, 0x9b, 0xce, 0x41, 0xae};

typedef nanodbc::statement::param_direction ParamDirection;
typedef std::variant<std::monostate, int64_t, double, StringT, nanodbc::timestamp> ParamVariant;
// template<class T>
// concept ParamVariantType = std::is_same_v<T, int64_t> || std::is_same_v<T, double> || std::is_same_v<T, StringT> ||
// std::is_same_v<T, nanodbc::timestamp>;

// 数据库接口
interface IDataBase : virtual IUnknownEx {
  // 连接接口
public:
  // 打开连接
  virtual VOID OpenConnection() = 0;
  // 关闭连接
  virtual VOID CloseConnection() = 0;
  // 连接信息
  virtual bool SetConnectionInfo(DWORD dwDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) = 0;
  // 连接信息
  virtual bool SetConnectionInfo(LPCTSTR szDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) = 0;

  // 参数接口
public:
  // 清除参数
  virtual VOID ClearParameters() = 0;
  // 获取参数
  virtual VOID GetParameter(LPCTSTR pszParamName, int64_t& DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, double& DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, StringT & DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, nanodbc::timestamp & DBVarValue) = 0;
  // 插入参数
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const int64_t& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const double& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const StringT& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const nanodbc::timestamp& DBVarValue) = 0;

  // 控制接口
public:
  // 关闭记录
  virtual VOID CloseRecordset() = 0;

  // 记录接口
public:
  // 往下移动
  virtual VOID MoveToNext() = 0;
  // 移到开头
  virtual VOID MoveToFirst() = 0;
  // 是否结束
  virtual bool IsRecordsetEnd() = 0;
  // 获取数目
  virtual LONG GetRecordCount() = 0;
  // 返回数值
  virtual LONG GetReturnValue() = 0;
  // 获取数据
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, int64_t& DBVarValue) = 0;
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, double& DBVarValue) = 0;
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, StringT & DBVarValue) = 0;
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, nanodbc::timestamp & DBVarValue) = 0;

  // 控制接口
public:
  // 存储过程
  virtual VOID ExecuteProcess(LPCTSTR pszSPName, bool bRecordset) = 0;
  // 执行语句
  virtual VOID ExecuteSentence(LPCTSTR pszCommand, bool bRecordset) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngine = {0x47b5a119, 0x1676, 0x49a3, 0xbe, 0xae, 0xca, 0x27, 0xeb, 0x59, 0x97, 0x22};

// 数据库引擎
interface IDataBaseEngine : IServiceModule {
  // 信息接口
public:
  // 引擎负荷
  virtual bool GetBurthenInfo(tagBurdenInfo & BurthenInfo) = 0;

  // 配置接口
public:
  // 配置模块
  virtual bool SetDataBaseEngineSink(IUnknownEx * pIUnknownEx) = 0;
  // 配置模块
  virtual bool SetDataBaseEngineSink(IUnknownEx * pIUnknownEx[], WORD wSinkCount) = 0;

  // 请求控制
public:
  // 控制事件
  virtual bool PostDataBaseControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
  // 请求事件
  virtual bool PostDataBaseRequest(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
  // 延期请求
  virtual bool DeferDataBaseRequest(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngineSink = {0x0ed26ed6, 0x69d7, 0x4f5b, 0x00b0, 0xca, 0x17, 0xae, 0xab, 0xba, 0x06, 0xdf};

// 数据库钩子
interface IDataBaseEngineSink : virtual IUnknownEx {
  // 系统事件
public:
  // 启动事件
  virtual bool OnDataBaseEngineStart(IUnknownEx * pIUnknownEx) = 0;
  // 停止事件
  virtual bool OnDataBaseEngineConclude(IUnknownEx * pIUnknownEx) = 0;

  // 内核事件
public:
  // 时间事件
  virtual bool OnDataBaseEngineTimer(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
  // 控制事件
  virtual bool OnDataBaseEngineControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
  // // 结果事件
  // virtual bool OnDataBaseEngineResult(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
  // 请求事件
  virtual bool OnDataBaseEngineRequest(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IUDPNetworkEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IUDPNetworkEngine = {0x8d138a9b, 0xa97d, 0x4d51, 0x9d, 0x6c, 0xd8, 0x6e, 0xa1, 0x84, 0x45, 0x2e};

// 网络引擎
interface IUDPNetworkEngine : IServiceModule {
  // 信息接口
public:
  // 配置端口
  virtual WORD GetServicePort() = 0;
  // 当前端口
  virtual WORD GetCurrentPort() = 0;

  // 配置接口
public:
  // 设置接口
  virtual bool SetUDPNetworkEngineEvent(IUnknownEx * pIUnknownEx) = 0;
  // 设置参数
  virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect) = 0;

  // 发送接口
public:
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID) = 0;
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;

  // 控制接口
public:
  // 关闭连接
  virtual bool CloseSocket(DWORD dwSocketID) = 0;
  // 设置关闭
  virtual bool ShutDownSocket(DWORD dwSocketID) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPNetworkEngine INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPNetworkEngine = {0x7747f683, 0xc0da, 0x4588, 0x89, 0xcc, 0x15, 0x93, 0xac, 0xc0, 0x44, 0xc8};

// 网络引擎
interface ITCPNetworkEngine : IServiceModule {
  // 信息接口
public:
  // 配置端口
  virtual WORD GetServicePort() = 0;
  // 当前端口
  virtual WORD GetCurrentPort() = 0;

  // 配置接口
public:
  // 设置接口
  virtual bool SetTCPNetworkEngineEvent(IUnknownEx * pIUnknownEx) = 0;
  // 设置参数
  virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation) = 0;

  // 发送接口
public:
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID) = 0;
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 批量发送
  virtual bool SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, BYTE cbBatchMask, VOID * pData, WORD wDataSize) = 0;

  // 控制接口
public:
  // 关闭连接
  virtual bool CloseSocket(DWORD dwSocketID) = 0;
  // 设置关闭
  virtual bool ShutDownSocket(DWORD dwSocketID) = 0;
  // 允许群发
  virtual bool AllowBatchSend(DWORD dwSocketID, bool bAllowBatch, BYTE cbBatchMask) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPSocketService INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPSocketService = {0x709a4449, 0xad77, 0x4b3d, 0xb4, 0xd6, 0x8d, 0x0b, 0x28, 0x65, 0xec, 0xae};

// 网络接口
interface ITCPSocketService : IServiceModule {
  // 配置接口
public:
  // 配置函数
  virtual bool SetServiceID(WORD wServiceID) = 0;
  // 设置接口
  virtual bool SetTCPSocketEvent(IUnknownEx * pIUnknownEx) = 0;

  // 功能接口
public:
  // 关闭连接
  virtual bool CloseSocket() = 0;
  // 连接地址
  virtual bool Connect(DWORD dwServerIP, WORD wPort) = 0;
  // 连接地址
  virtual bool Connect(LPCTSTR szServerIP, WORD wPort) = 0;
  // 发送函数
  virtual bool SendData(WORD wMainCmdID, WORD wSubCmdID) = 0;
  // 发送函数
  virtual bool SendData(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IWEBSocketService INTERFACE_VERSION(1, 1)
static const GUID IID_IWEBSocketService = {0x91052ff2, 0xeb29, 0x40b9, 0xb2, 0xa2, 0x8a, 0xa2, 0x98, 0x36, 0x2c, 0x84};

// 网络接口
interface IWEBSocketService : IServiceModule {
  // 配置接口
public:
  // 配置函数
  virtual bool SetServiceID(WORD wServiceID) = 0;
  // 设置接口
  virtual bool SetWEBSocketEvent(IUnknownEx * pIUnknownEx) = 0;

  // 功能接口
public:
  // 关闭连接
  virtual bool CloseSocket(BYTE cbShutReason) = 0;
  // 连接操作
  virtual bool ConnectServer(LPCTSTR pszURL, WORD wPort) = 0;
  // 发送请求
  virtual bool SendRequestData(VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITimerEngine INTERFACE_VERSION(1, 1)
static const GUID IID_ITimerEngine = {0x496401ae, 0x6fb0, 0x4e9f, 0x0090, 0x98, 0x44, 0x9d, 0x9c, 0xb2, 0xbd, 0x97};

// 定时器引擎
interface ITimerEngine : IServiceModule {
  // 配置接口
public:
  // 单元时间
  // virtual bool SetTimeCell(DWORD dwTimeCell) = 0;
  // 设置接口
  virtual bool SetTimerEngineEvent(IUnknownEx * pIUnknownEx) = 0;

  // 功能接口
public:
  // 设置定时器
  virtual bool SetTimer(DWORD dwTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM dwBindParameter) = 0;
  // 删除定时器
  virtual bool KillTimer(DWORD dwTimerID) = 0;
  // 删除定时器
  virtual bool KillAllTimers() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITimerEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITimerEngineEvent = {0xeb78a125, 0x62fc, 0x4811, {0x00b6, 0xf2, 0x59, 0x26, 0x88, 0x04, 0xc3, 0x02}};

// 定时器事件
interface ITimerEngineEvent : virtual IUnknownEx {
  // 接口定义
public:
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngineEvent = {0xc29c7131, 0xe84b, 0x4553, 0x00a8, 0x38, 0x12, 0xee, 0x07, 0xdd, 0x0e, 0xa3};

// 数据库事件
interface IDataBaseEngineEvent : virtual IUnknownEx {
  // 接口定义
public:
  // 数据库结果
  virtual bool OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPSocketEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPSocketEvent = {0x6f5bdb91, 0xf72a, 0x425d, 0x0087, 0x03, 0x39, 0xbc, 0xf7, 0x1e, 0x0b, 0x03};

// 网络事件
interface ITCPSocketEvent : virtual IUnknownEx {
  // 连接事件
  virtual bool OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode) = 0;
  // 关闭事件
  virtual bool OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason) = 0;
  // 读取事件
  virtual bool OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IWEBSocketEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IWEBSocketEvent = {0xabb2a528, 0xcc16, 0x4b67, 0xbd, 0x7b, 0x28, 0xa6, 0xce, 0x88, 0x8a, 0x33};

// 网络事件
interface IWEBSocketEvent : virtual IUnknownEx {
  // 状态接口
public:
  // 连接消息
  virtual bool OnEventWEBSocketLink(WORD wServiceID, WORD wRequestID, INT nErrorCode) = 0;
  // 关闭消息
  virtual bool OnEventWEBSocketShut(WORD wServiceID, WORD wRequestID, BYTE cbShutReason) = 0;

  // 数据接口
public:
  // 数据包流
  virtual bool OnEventWEBSocketMain(WORD wServiceID, WORD wRequestID, VOID * pcbMailData, WORD wStreamSize) = 0;
  // 数据包头
  virtual bool OnEventWEBSocketHead(WORD wServiceID, WORD wRequestID, VOID * pcbHeadData, WORD wHeadSize, INT nStatusCode) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPNetworkEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPNetworkEngineEvent = {0xb7e6da53, 0xfca5, 0x4d90, 0x0085, 0x48, 0xfe, 0x05, 0xf6, 0xb4, 0xc0, 0xef};

// 网络事件
interface ITCPNetworkEngineEvent : virtual IUnknownEx {
  // 接口定义
public:
  // 应答事件
  virtual bool OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr) = 0;
  // 关闭事件
  virtual bool OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime) = 0;
  // 读取事件
  virtual bool OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////
//
// #define VER_IAttemperEngine INTERFACE_VERSION(1, 1)
// static const GUID IID_IAttemperEngine = {0x0b070b2c, 0x9d72, 0x42d2, 0xa5, 0x70, 0xba, 0x2c, 0xbf, 0x6f, 0xbb, 0x1c};
//
// // 调度引擎
// interface IAttemperEngine : IServiceModule {
//   // 配置接口
// public:
//   // 网络接口
//   virtual bool SetNetworkEngine(IUnknownEx * pIUnknownEx) = 0;
//   // 回调接口
//   virtual bool SetAttemperEngineSink(IUnknownEx * pIUnknownEx) = 0;
//
//   // 控制事件
// public:
//   // 自定事件
//   virtual bool OnEventCustom(WORD wRequestID, VOID * pData, WORD wDataSize) = 0;
//   // 控制事件
//   virtual bool OnEventControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
// };

//////////////////////////////////////////////////////////////////////////////////
//
// #define VER_IAttemperEngineSink INTERFACE_VERSION(1, 1)
// static const GUID IID_IAttemperEngineSink = {0x831b9001, 0x4450, 0x45dd, 0x0091, 0x37, 0x0d, 0x26, 0x16, 0xe3, 0x75, 0x32};
//
// // 调度钩子
// interface IAttemperEngineSink : virtual IUnknownEx {
//   // 异步接口
// public:
//   // 启动事件
//   virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx) = 0;
//   // 停止事件
//   virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx) = 0;
//
//   // 事件接口
// public:
//   // 控制事件
//   virtual bool OnEventControl(WORD wIdentifier, VOID * pData, WORD wDataSize) = 0;
//   // 自定事件
//   virtual bool OnEventAttemperData(WORD wRequestID, VOID * pData, WORD wDataSize) = 0;
//
//   // 内核事件
// public:
//   // 时间事件
//   virtual bool OnEventTimer(DWORD dwTimerID, WPARAM wBindParam) = 0;
//   // 数据库事件 OnDataBaseEngineResult
//   virtual bool OnEventDataBase(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
//
//   // 连接事件
// public:
//   // 连接事件
//   virtual bool OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode) = 0;
//   // 关闭事件
//   virtual bool OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason) = 0;
//   // 读取事件
//   virtual bool OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
//
//   // 网络事件
// public:
//   // 应答事件
//   virtual bool OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr) = 0;
//   // 关闭事件
//   virtual bool OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, DWORD dwActiveTime) = 0;
//   // 读取事件
//   virtual bool OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
// };
//////////////////////////////////////////////////////////////////////////////////

// 控制事件
#define VER_IControlEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IControlEngineEvent = {0xd2489959, 0x5e9f, 0x4ef2, {0xb0, 0x73, 0x95, 0x7, 0xe4, 0x4c, 0xb0, 0x81}};

interface IControlEngineEvent : virtual IUnknownEx {
public:
  // 控制事件
  virtual bool OnEventControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
};

// 转发钩子
#define VER_IDispatchEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IDispatchEngineSink = {0xac0925f5, 0xdbfc, 0x45fb, {0x86, 0x22, 0xd5, 0xf4, 0x28, 0xe, 0x11, 0xf7}};

interface IDispatchEngineSink : virtual IUnknownEx,
                                ITimerEngineEvent,
                                IControlEngineEvent,
                                IDataBaseEngineEvent,
                                ITCPSocketEvent,
                                ITCPNetworkEngineEvent {
  // 异步接口(可以安全的删除)
public:
  // 启动事件
  virtual bool OnAttemperEngineStart(IUnknownEx * pIUnknownEx) = 0;
  // 停止事件
  virtual bool OnAttemperEngineConclude(IUnknownEx * pIUnknownEx) = 0;
};

//////////////////////////////////////////////////////////////////////////////////
// 组件辅助类

DECLARE_MODULE_HELPER(DataBase, KERNEL_ENGINE_DLL_NAME, "CreateDataBase")
DECLARE_MODULE_HELPER(TimerEngine, KERNEL_ENGINE_DLL_NAME, "CreateTimerEngine")
DECLARE_MODULE_HELPER(DataBaseEngine, KERNEL_ENGINE_DLL_NAME, "CreateDataBaseEngine")
// DECLARE_MODULE_HELPER(AttemperEngine, KERNEL_ENGINE_DLL_NAME, "CreateAttemperEngine")
DECLARE_MODULE_HELPER(TCPSocketService, KERNEL_ENGINE_DLL_NAME, "CreateTCPSocketService")
DECLARE_MODULE_HELPER(WEBSocketService, KERNEL_ENGINE_DLL_NAME, "CreateWEBSocketService")
DECLARE_MODULE_HELPER(TCPNetworkEngine, KERNEL_ENGINE_DLL_NAME, "CreateTCPNetworkEngine")
DECLARE_MODULE_HELPER(UDPNetworkEngine, KERNEL_ENGINE_DLL_NAME, "CreateUDPNetworkEngine")
DECLARE_MODULE_HELPER(AsynchronismEngine, KERNEL_ENGINE_DLL_NAME, "CreateAsynchronismEngine")
DECLARE_MODULE_HELPER(TraceService, KERNEL_ENGINE_DLL_NAME, "CreateTraceService")

//////////////////////////////////////////////////////////////////////////////////

// 导出文件
#ifndef KERNEL_ENGINE_DLL
#include "DataBaseAide.h"
#include "TraceService.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
