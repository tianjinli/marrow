#pragma once

// 本项目头文件
#include "nanodbc.h"

#include "ServiceCore/ServiceCoreHead.h"
#include "ServiceCore/StringUtils.h"
#include "ServiceCore/TimeHelper.h"

namespace asio {
  class io_context;
}

namespace nf {
  using BufferPtr = std::shared_ptr<std::vector<uint8_t>>;

  // 使用 inline (C++17) 确保该常量在被多个头文件包含时，全局只有唯一一份物理实例，绝不重定义
  inline const auto kEmptyBuffer = std::make_shared<std::vector<std::uint8_t>>();
} // namespace nf

//////////////////////////////////////////////////////////////////////////////////

// 控制事件
struct NTY_ControlEvent {
  WORD wControlID; // 控制标识
};

// 定时器事件
struct NTY_TimerEvent {
  DWORD dwTimerID; // 时间标识
  WPARAM dwBindParameter; // 绑定参数
};

// 数据库事件
struct NTY_DataBaseEvent {
  WORD wRequestID; // 请求标识
  DWORD dwContextID; // 对象标识
};

// 读取事件
struct NTY_TCPSocketReadEvent {
  WORD wDataSize; // 数据大小
  WORD wServiceID; // 服务标识
  TCP_Command Command; // 命令信息
};

// 关闭事件
struct NTY_TCPSocketShutEvent {
  WORD wServiceID; // 服务标识
  BYTE cbShutReason; // 关闭原因
};

// 连接事件
struct NTY_TCPSocketLinkEvent {
  INT nErrorCode; // 错误代码
  WORD wServiceID; // 服务标识
};

// 应答事件
struct NTY_TCPNetworkAcceptEvent {
  DWORD dwSocketID; // 网络标识
  DWORD dwClientAddr; // 连接地址
};

// 读取事件
struct NTY_TCPNetworkReadEvent {
  WORD wDataSize; // 数据大小
  DWORD dwSocketID; // 网络标识
  TCP_Command Command; // 命令信息
};

// 关闭事件
struct NTY_TCPNetworkShutEvent {
  DWORD dwSocketID; // 网络标识
  DWORD dwClientAddr; // 连接地址
  DWORD dwActiveTime; // 连接时间
};

//////////////////////////////////////////////////////////////////////////////////
// 导出定义

#ifdef _WIN32
#ifdef KERNEL_ENGINE_DLL
#define KERNEL_ENGINE_CLASS __declspec(dllexport)
#else
#define KERNEL_ENGINE_CLASS __declspec(dllimport)
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
  TraceLevel_Debug = 1, // spdlog::level::debug, // 调试消息
  TraceLevel_Info = 2, // spdlog::level::info, // 信息消息
  TraceLevel_Normal = TraceLevel_Info, // 普通消息
  TraceLevel_Warning = 3, // spdlog::level::warn, // 警告消息
  TraceLevel_Exception = 4, // spdlog::level::err, // 异常消息
};

// SQL 异常类型
enum enSQLException {
  SQLException_None = 0, // 没有异常
  SQLException_Connect = 1, // 连接错误
  SQLException_Syntax = 2, // 语法错误
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseException INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseException = {0x428361ed, 0x9dfa, 0x43d7, 0x008f, 0x26, 0x17, 0x06, 0x47, 0x6b, 0x2a, 0x51};

// 数据库异常
interface IDataBaseException : IUnknownEx {
  // 异常代码
  virtual HRESULT GetExceptionResult() = 0;
  // 异常描述
  virtual LPCTSTR GetExceptionDescribe() = 0;
  // 异常类型
  virtual enSQLException GetExceptionType() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IServiceModule INTERFACE_VERSION(1, 1)
static const GUID IID_IServiceModule = {0x05980504, 0xa2f2, 0x4b0f, 0x009b, 0x54, 0x51, 0x54, 0x1e, 0x05, 0x5c, 0xff};

// 服务模块
interface IServiceModule : IUnknownEx {
  // 启动服务
  virtual bool StartService(std::shared_ptr<asio::io_context> io_context) = 0;
  // 停止服务
  virtual bool ConcludeService() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBase INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBase = {0xa2e38a78, 0x1e4f, 0x4de4, 0x00a5, 0xd1, 0xb9, 0x19, 0x9b, 0xce, 0x41, 0xae};

typedef nanodbc::statement::param_direction ParamDirection;
typedef std::variant<std::monostate, int64_t, double, nanodbc::string, std::vector<uint8_t>, nanodbc::timestamp> ParamVariant;
// template<class T>
// concept ParamVariantType = std::is_same_v<T, int64_t> || std::is_same_v<T, double> || std::is_same_v<T, StringT> ||
// std::is_same_v<T, nanodbc::timestamp>;

// 数据库接口
interface IDataBase : IUnknownEx {
  // ------------------------------- 连接接口 -------------------------------
  // 打开连接
  virtual VOID OpenConnection() = 0;
  // 关闭连接
  virtual VOID CloseConnection() = 0;
  // 连接信息
  virtual bool SetConnectionInfo(DWORD dwDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) = 0;
  // 连接信息
  virtual bool SetConnectionInfo(LPCTSTR szDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) = 0;

  // ------------------------------- 参数接口 -------------------------------
  // 清除参数
  virtual VOID ClearParameters() = 0;
  // 获取参数
  virtual VOID GetParameter(LPCTSTR pszParamName, int64_t& DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, double& DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, nanodbc::string & DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, std::vector<uint8_t> & DBVarValue) = 0;
  virtual VOID GetParameter(LPCTSTR pszParamName, nanodbc::timestamp & DBVarValue) = 0;
  // 插入参数
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const int64_t& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const double& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const nanodbc::string& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const std::vector<uint8_t>& DBVarValue) = 0;
  virtual VOID AddParameter(LPCTSTR pszParamName, nanodbc::statement::param_direction Direction, const nanodbc::timestamp& DBVarValue) = 0;

  // ------------------------------- 记录接口 -------------------------------
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
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, std::vector<uint8_t> & DBVarValue) = 0;
  virtual VOID GetRecordsetValue(LPCTSTR pszItem, nanodbc::timestamp & DBVarValue) = 0;

  // ------------------------------- 控制接口 -------------------------------
  // 存储过程
  virtual VOID ExecuteProcess(LPCTSTR pszSPName, bool bRecordset) = 0;
  // 执行语句
  virtual VOID ExecuteSentence(LPCTSTR pszCommand, bool bRecordset) = 0;
  // 关闭记录
  virtual VOID CloseRecordset() = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngine = {0x47b5a119, 0x1676, 0x49a3, 0xbe, 0xae, 0xca, 0x27, 0xeb, 0x59, 0x97, 0x22};

// 数据库引擎
interface IDataBaseEngine : IServiceModule {
  // ------------------------------- 信息接口 -------------------------------
  // // 引擎负荷
  // virtual bool GetBurthenInfo(tagBurthenInfo & BurthenInfo) = 0;

  // ------------------------------- 配置接口 -------------------------------
  // 配置模块 (IDataBaseEngineSink)
  virtual bool SetDataBaseEngineSink(void* pIUnknownEx) = 0;

  // ------------------------------- 请求控制 -------------------------------
  // 请求事件 (IDataBaseEngineSink::OnDataBaseEngineRequest)
  virtual bool PostDataBaseRequest(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngineSink INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngineSink = {0x0ed26ed6, 0x69d7, 0x4f5b, 0x00b0, 0xca, 0x17, 0xae, 0xab, 0xba, 0x06, 0xdf};

// 数据库钩子
interface IDataBaseEngineSink : IUnknownEx {
  // ------------------------------- 系统事件 -------------------------------
  // 启动事件 (IDataBaseEngineSink 子类)
  virtual bool OnDataBaseEngineStart(IUnknownEx * pIUnknownEx) = 0;
  // 停止事件 (IDataBaseEngineSink 子类)
  virtual bool OnDataBaseEngineConclude(IUnknownEx * pIUnknownEx) = 0;

  // ------------------------------- 内核事件 -------------------------------
  // // 结果事件
  // virtual bool OnDataBaseEngineResult(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
  // 请求事件
  virtual bool OnDataBaseEngineRequest(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
  // 请求事件 (向下兼容)
  bool OnDataBaseEngineRequestInternal(WORD request_id, DWORD context_id, nf::BufferPtr data) {
    return OnDataBaseEngineRequest(request_id, context_id, data->data(), data->size());
  }
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPNetworkEngine INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPNetworkEngine = {0x7747f683, 0xc0da, 0x4588, 0x89, 0xcc, 0x15, 0x93, 0xac, 0xc0, 0x44, 0xc8};

// 网络引擎
interface ITCPNetworkEngine : IServiceModule {
  // ------------------------------- 信息接口 -------------------------------
  // 当前端口
  virtual WORD GetCurrentPort() = 0;

  // ------------------------------- 配置接口 -------------------------------
  // 设置接口 (ITCPNetworkEngineEvent)
  virtual bool SetTCPNetworkEngineEvent(void* pIUnknownEx) = 0;
  // 设置参数
  virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation) = 0;

  // ------------------------------- 发送接口 -------------------------------
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID) = 0;
  // 发送函数
  virtual bool SendData(DWORD dwSocketID, WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize) = 0;
  // 批量发送
  virtual bool SendDataBatch(WORD wMainCmdID, WORD wSubCmdID, VOID * pData, WORD wDataSize, BYTE cbBatchMask) = 0;

  // ------------------------------- 控制接口 -------------------------------
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
  // ------------------------------- 配置接口 -------------------------------
  // 配置函数
  virtual bool SetServiceID(WORD wServiceID) = 0;
  // 设置接口 (ITCPSocketEvent)
  virtual bool SetTCPSocketEvent(void* pIUnknownEx) = 0;

  // ------------------------------- 功能接口 -------------------------------
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

#define VER_ITimerEngine INTERFACE_VERSION(1, 1)
static const GUID IID_ITimerEngine = {0x496401ae, 0x6fb0, 0x4e9f, 0x0090, 0x98, 0x44, 0x9d, 0x9c, 0xb2, 0xbd, 0x97};

// 定时器引擎
interface ITimerEngine : IServiceModule {
  // ------------------------------- 配置接口 -------------------------------
  // 单元时间
  virtual bool SetTimeCell(DWORD dwTimeCell) = 0;
  // 设置接口 (ITimerEngineEvent)
  virtual bool SetTimerEngineEvent(void* pIUnknownEx) = 0;

  // ------------------------------- 功能接口 -------------------------------
  // 设置定时器
  virtual bool SetTimer(DWORD dwTimerID, DWORD dwElapse, DWORD dwRepeat, WPARAM dwBindParameter) = 0;
  // 删除定时器
  virtual bool KillTimer(DWORD dwTimerID) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITimerEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITimerEngineEvent = {0xeb78a125, 0x62fc, 0x4811, {0x00b6, 0xf2, 0x59, 0x26, 0x88, 0x04, 0xc3, 0x02}};

// 定时器事件
interface ITimerEngineEvent : IUnknownEx {
  // 时间事件
  virtual bool OnEventTimer(DWORD dwTimerID, WPARAM dwBindParameter) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IDataBaseEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IDataBaseEngineEvent = {0xc29c7131, 0xe84b, 0x4553, 0x00a8, 0x38, 0x12, 0xee, 0x07, 0xdd, 0x0e, 0xa3};

// 数据库事件
interface IDataBaseEngineEvent : IUnknownEx {
  // 数据库结果
  virtual bool OnEventDataBaseResult(WORD wRequestID, DWORD dwContextID, VOID * pData, WORD wDataSize) = 0;
  // 数据库结果 (向下兼容)
  bool OnEventDataBaseResultInternal(WORD wRequestID, DWORD dwContextID, nf::BufferPtr buffer) {
    return OnEventDataBaseResult(wRequestID, dwContextID, buffer->data(), buffer->size());
  }
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPSocketEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPSocketEvent = {0x6f5bdb91, 0xf72a, 0x425d, 0x0087, 0x03, 0x39, 0xbc, 0xf7, 0x1e, 0x0b, 0x03};

// 网络事件
interface ITCPSocketEvent : IUnknownEx {
  // 连接事件
  virtual bool OnEventTCPSocketLink(WORD wServiceID, INT nErrorCode) = 0;
  // 关闭事件
  virtual bool OnEventTCPSocketShut(WORD wServiceID, BYTE cbShutReason) = 0;
  // 读取事件
  virtual bool OnEventTCPSocketRead(WORD wServiceID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
  // 读取事件 (向下兼容)
  bool OnEventTCPSocketReadInternal(WORD service_id, TCP_Command command, nf::BufferPtr buffer) {
    return OnEventTCPSocketRead(service_id, command, buffer->data(), buffer->size());
  }
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_ITCPNetworkEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_ITCPNetworkEngineEvent = {0xb7e6da53, 0xfca5, 0x4d90, 0x0085, 0x48, 0xfe, 0x05, 0xf6, 0xb4, 0xc0, 0xef};

// 网络事件
interface ITCPNetworkEngineEvent : IUnknownEx {
  // 应答事件
  virtual bool OnEventTCPNetworkBind(DWORD dwSocketID, DWORD dwClientAddr) = 0;
  // 关闭事件
  virtual bool OnEventTCPNetworkShut(DWORD dwSocketID, DWORD dwClientAddr, time_t dwActiveTime) = 0;
  // 读取事件
  virtual bool OnEventTCPNetworkRead(DWORD dwSocketID, TCP_Command Command, VOID * pData, WORD wDataSize) = 0;
  bool OnEventTCPNetworkReadInternal(DWORD dwSocketID, TCP_Command command, nf::BufferPtr buffer) {
    return OnEventTCPNetworkRead(dwSocketID, command, buffer->data(), buffer->size());
  }
};

//////////////////////////////////////////////////////////////////////////

#define VER_IAttemperEngine INTERFACE_VERSION(1, 1)
static const GUID IID_IAttemperEngine = {0x4d5d2424, 0x40fd, 0x4747, 0x86, 0xd8, 0x8f, 0xca, 0x6b, 0x96, 0xea, 0x0b};

// 调度引擎
interface IAttemperEngine : public IServiceModule {
  // ------------------------------- 配置接口 -------------------------------
  // 调度事件 (IAttemperEngineEvent)
  virtual bool SetAttemperEngineEvent(void* pIUnknownEx) = 0;
  // 控制事件 (IControlEngineEvent)
  virtual bool SetControlEngineEvent(void* pIUnknownEx) = 0;

  // ------------------------------- 控制事件 -------------------------------
  // 控制事件
  virtual bool OnEventControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IControlEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IControlEngineEvent = {0xd2489959, 0x5e9f, 0x4ef2, 0xb0, 0x73, 0x95, 0x7, 0xe4, 0x4c, 0xb0, 0x81};

// 控制事件
interface IControlEngineEvent : IUnknownEx {
  // 控制事件
  virtual bool OnEventControl(WORD wControlID, VOID * pData, WORD wDataSize) = 0;
  // 控制事件 (向下兼容)
  bool OnEventControlInternal(WORD wControlID, nf::BufferPtr buffer) {
    return OnEventControl(wControlID, buffer->data(), buffer->size());
  }
};

//////////////////////////////////////////////////////////////////////////////////

#define VER_IAttemperEngineEvent INTERFACE_VERSION(1, 1)
static const GUID IID_IAttemperEngineEvent = {0x9273c1c8, 0xccb5, 0x48f5, 0x98, 0x74, 0x97, 0x71, 0x8e, 0xb1, 0xd0, 0x4c};

// 调度事件
interface IAttemperEngineEvent : IUnknownEx {
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
DECLARE_MODULE_HELPER(AttemperEngine, KERNEL_ENGINE_DLL_NAME, "CreateAttemperEngine")
DECLARE_MODULE_HELPER(TCPSocketService, KERNEL_ENGINE_DLL_NAME, "CreateTCPSocketService")
DECLARE_MODULE_HELPER(TCPNetworkEngine, KERNEL_ENGINE_DLL_NAME, "CreateTCPNetworkEngine")

//////////////////////////////////////////////////////////////////////////////////

// 导出文件
#ifndef KERNEL_ENGINE_DLL
#include "DataBaseAide.h"
#include "TraceService.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
