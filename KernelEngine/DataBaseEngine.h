#pragma once

//#include "AsynchronismEngine.h"
#include "KernelEngineHead.h"

#define ODBC_DRIVER_VERSION 18
//////////////////////////////////////////////////////////////////////////////////
// ADO 错误类
class CDataBaseException : public IDataBaseException {
  // 错误类型
protected:
  [[maybe_unused]] HRESULT result_ = 0; // 异常代码
  enSQLException exception_ = SQLException_None; // 异常类型

  // 辅助变量
protected:
  StringT describe_; // 异常信息

  // 函数定义
public:
  // 构造函数
  CDataBaseException() = default;
  // 析构函数
  virtual ~CDataBaseException() = default;

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() { return; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 功能接口
public:
  // 错误代号
  virtual HRESULT GetExceptionResult() { return result_; }
  // 错误描述
  virtual LPCTSTR GetExceptionDescribe() { return describe_.c_str(); }
  // 错误类型
  virtual enSQLException GetExceptionType() { return exception_; }

  // 功能函数
public:
  // 设置错误
  VOID SetExceptionInfo(enSQLException SQLException, const StringT& SQLDescribe);
};

//////////////////////////////////////////////////////////////////////////////////

// 数据库对象
class CDataBase : public IDataBase {
  struct Parameter {
    ParamDirection Direction;
    ParamVariant DBVarValue;
  };

  // 连接信息
protected:
  StringT driver_name_ = TEXT("ODBC Driver 18 for SQL Server"); // 驱动名称
  int32_t driver_ver_ = ODBC_DRIVER_VERSION; // 驱动版本
  StringT db_name_; // DB 名称
  StringT host_ip_; // IP 地址
  uint16_t host_port; // 端口
  StringT user_; // 用户名
  StringT password_; // 密码
  CDataBaseException db_exception_; // 错误对象

  // 状态变量
protected:
  DWORD connect_count_ = 0; // 重试次数
  time_t connect_error_time_ = 0; // 错误时间
  const DWORD resume_connect_time_ = 30; // 恢复时间
  const DWORD resume_connect_count_ = 10; // 恢复次数

  // 内核变量
protected:
  nanodbc::connection connection_;
  // nanodbc::result recordset_;
  std::vector<StringT> bind_keys_; // 用于顺序绑定参数(由于 statement_.bind_name(...) 系鸡肋)
  std::unordered_map<StringT, Parameter> parameters_;
  LONG return_var_ = -1; // 返回值
  // bool recordend_ = true;                       // 无数据了
  std::vector<std::unordered_map<StringT, ParamVariant>> cached_set_;
  std::vector<std::unordered_map<StringT, ParamVariant>>::iterator cached_pos_;

  std::atomic_uint64_t pending_count_ = 0;

  // 函数定义
public:
  // 构造函数
  CDataBase() = default;
  // 析构函数
  virtual ~CDataBase();

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() { delete this; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 连接接口
public:
  // 打开连接
  virtual VOID OpenConnection();
  // 关闭连接
  virtual VOID CloseConnection();
  // 连接信息
  virtual bool SetConnectionInfo(DWORD dwDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword);
  // 连接信息
  virtual bool SetConnectionInfo(LPCTSTR szDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword);

  // 参数接口
public:
  // 清除参数
  virtual VOID ClearParameters();
  // 获取参数
  template<typename T>
  VOID GetParameterT(LPCTSTR pszItem, T& DBVarValue);
  VOID GetParameter(LPCTSTR pszItem, int64_t& DBVarValue) { GetParameterT(pszItem, DBVarValue); }
  VOID GetParameter(LPCTSTR pszItem, double& DBVarValue) { GetParameterT(pszItem, DBVarValue); }
  VOID GetParameter(LPCTSTR pszItem, StringT& DBVarValue) { GetParameterT(pszItem, DBVarValue); }
  VOID GetParameter(LPCTSTR pszItem, nanodbc::timestamp& DBVarValue) { GetParameterT(pszItem, DBVarValue); }

  // 插入参数
  template<typename T>
  VOID AddParameterT(LPCTSTR pszParamName, ParamDirection Direction, const T& DBVarValue);

  VOID AddParameter(LPCTSTR pszParamName, ParamDirection Direction, const int64_t& DBVarValue) { AddParameterT(pszParamName, Direction, DBVarValue); }
  VOID AddParameter(LPCTSTR pszParamName, ParamDirection Direction, const double& DBVarValue) { AddParameterT(pszParamName, Direction, DBVarValue); }
  VOID AddParameter(LPCTSTR pszParamName, ParamDirection Direction, const StringT& DBVarValue) { AddParameterT(pszParamName, Direction, DBVarValue); }
  VOID AddParameter(LPCTSTR pszParamName, ParamDirection Direction, const nanodbc::timestamp& DBVarValue) {
    AddParameterT(pszParamName, Direction, DBVarValue);
  }

  // 控制接口
public:
  // 关闭记录
  virtual VOID CloseRecordset();

  // 记录接口
public:
  // 往下移动
  virtual VOID MoveToNext();
  // 移到开头
  virtual VOID MoveToFirst();
  // 是否结束
  virtual bool IsRecordsetEnd();
  // 获取数目
  virtual LONG GetRecordCount();
  // 返回数值
  virtual LONG GetReturnValue();
  // 获取数据
  template<typename T>
  VOID GetRecordsetValueT(LPCTSTR pszItem, T& DBVarValue);

  VOID GetRecordsetValue(LPCTSTR pszItem, int64_t& DBVarValue) { GetRecordsetValueT(pszItem, DBVarValue); }
  VOID GetRecordsetValue(LPCTSTR pszItem, double& DBVarValue) { GetRecordsetValueT(pszItem, DBVarValue); }
  VOID GetRecordsetValue(LPCTSTR pszItem, StringT& DBVarValue) { GetRecordsetValueT(pszItem, DBVarValue); }
  VOID GetRecordsetValue(LPCTSTR pszItem, nanodbc::timestamp& DBVarValue) { GetRecordsetValueT(pszItem, DBVarValue); }

  VOID FillRecordset(nanodbc::result& result);

  // 控制接口
public:
  // 存储过程
  virtual VOID ExecuteProcess(LPCTSTR pszSPName, bool bRecordset);
  // 执行语句
  virtual VOID ExecuteSentence(LPCTSTR pszCommand, bool bRecordset);

  // 内部函数
protected:
  // 连接错误
  bool IsConnectError();
  // 是否打开
  bool IsRecordsetOpened();
  // 重新连接
  bool TryConnectAgain(const std::exception& exception);
  // 转换字符
  VOID ConvertToSQLSyntax(LPCTSTR pszString, StringT* strResult);
  // 错误处理
  VOID OnSQLException(enSQLException SQLException, const std::exception& exception);
  // 获取驱动名称
  static bool GetDriverInfo(StringT* driver_name, int32_t* dirver_ver = nullptr);
};

//////////////////////////////////////////////////////////////////////////////////

// 数据库引擎类
class CDataBaseEngine : public IDataBaseEngine/*, public IAsynchronismEngineSink*/ {
  // 组件变量
protected:
  // CAsynchronismEngine asynchronism_engine_; // 异步引擎

  // 接口变量
protected:
  // IDataBaseEngineSink* event_handler_ = nullptr; // 钩子接口

  // 函数定义
public:
  // 构造函数
  explicit CDataBaseEngine() = default;
  // 析构函数
  virtual ~CDataBaseEngine() = default;

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() { delete this; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 服务接口
public:
  // 启动服务
  virtual bool InitiateService(std::shared_ptr<asio::io_context> io_context);
  // 停止服务
  virtual bool ConcludeService();

  // 引擎负荷
  virtual bool GetBurthenInfo(tagBurdenInfo& BurthenInfo);

  // 配置接口
public:
  // 配置模块
  virtual bool SetDataBaseEngineSink(IUnknownEx* sink_any);
  // 配置模块
  virtual bool SetDataBaseEngineSink(IUnknownEx* sink_anys[], WORD sink_count);

  // 控制事件
public:
  // 控制事件
  virtual bool PostDataBaseControl(WORD control_id, VOID* data, WORD data_size);
  // 请求事件
  virtual bool PostDataBaseRequest(WORD request_id, DWORD context_id, VOID* data, WORD data_size);
  // 延期请求
  virtual bool DeferDataBaseRequest(WORD request_id, DWORD context_id, VOID* data, WORD data_size);

  // 异步接口
public:
  // 启动事件
  virtual bool OnAsynchronismEngineStart();
  // 停止事件
  virtual bool OnAsynchronismEngineConclude();
  // // 异步数据
  // virtual bool OnAsynchronismEngineData(WORD identifier, nf::BufferPtr data);
  // // 异步数据
  // virtual bool OnAsynchronismEngineData(WORD identifier, VOID* data, WORD data_size);
};

//////////////////////////////////////////////////////////////////////////////////
