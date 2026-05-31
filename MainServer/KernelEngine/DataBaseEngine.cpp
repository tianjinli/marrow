#include "DataBaseEngine.h"

#include <sqlext.h>

#include "TraceService.h"

//////////////////////////////////////////////////////////////////////////////////

// // 动作定义
// #define ASYNCHRONISM_TIMER 11 // 时间事件
// #define ASYNCHRONISM_CONTROL 12 // 控制事件
// #define ASYNCHRONISM_DATABASE 13 // 数据事件

//////////////////////////////////////////////////////////////////////////////////
// 接口查询
VOID* CDataBaseException::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IDataBaseException, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDataBaseException, Guid, dwQueryVer);
  return nullptr;
}

// 设置错误
VOID CDataBaseException::SetExceptionInfo(enSQLException SQLException, const StringT& SQLDescribe) {
  // 设置信息
  exception_ = SQLException;
  describe_ = SQLDescribe;
}

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CDataBase::~CDataBase() {
  // 关闭连接
  CDataBase::CloseConnection();
}

// 接口查询
VOID* CDataBase::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IDataBase, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDataBase, Guid, dwQueryVer);
  return nullptr;
}

// 打开连接
VOID CDataBase::OpenConnection() {
  // 连接数据库
  try {
    // 关闭连接
    CloseConnection();

    // 构造连接
    const auto strConnect = std::format(TEXT("Driver={};DataBase={};Uid={};Pwd={};Server=tcp:{},{};TrustServerCertificate=yes;Charset=UTF-8;"),
                                        driver_name_, db_name_, user_, password_, host_ip_, host_port);

    connection_.connect(ToSimpleString(strConnect));
    CLogger::Info(TEXT("打开 {}@{} {}"), db_name_, host_ip_, connection_.connected() ? TEXT("成功") : TEXT("失败"));
  } catch (const std::exception& err) {
    // 设置变量
    connect_count_ = 0;
    connect_error_time_ = std::time(nullptr);

    // 抛出异常
    OnSQLException(SQLException_Connect, err);
  }
}

// 关闭连接
VOID CDataBase::CloseConnection() {
  try {
    // 设置变量
    connect_count_ = 0;
    connect_error_time_ = 0;

    // 关闭连接
    CloseRecordset();
    if (connection_.connected()) {
      connection_.disconnect();
    }
  } catch (const std::exception& err) {
    OnSQLException(SQLException_Syntax, err);
  }
}

// 连接信息
bool CDataBase::SetConnectionInfo(DWORD dwDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) {
  auto addr = asio::ip::make_address_v4(dwDBAddr);
#ifdef _UNICODE
  StringT strAddr = StringUtils::ToUtf16(addr.to_string());
#else
  StringT strAddr = ToSimpleUtf8(addr.to_string());
#endif
  return SetConnectionInfo(strAddr.c_str(), wPort, szDBName, szUser, szPassword);
}

// 连接信息
bool CDataBase::SetConnectionInfo(LPCTSTR szDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) {
  // 效验参数
  ASSERT(szDBAddr != nullptr);
  ASSERT(szDBName != nullptr);
  ASSERT(szUser != nullptr);
  ASSERT(szPassword != nullptr);

  GetDriverInfo(&driver_name_, &driver_ver_);
  host_ip_ = szDBAddr;
  host_port = wPort;

  // 字符转换
  ConvertToSQLSyntax(szDBName, &db_name_);
  ConvertToSQLSyntax(szUser, &user_);
  ConvertToSQLSyntax(szPassword, &password_);

  return true;
}

// 清除参数
VOID CDataBase::ClearParameters() {
  return_var_ = 0;
  // recordend_ = true;
  bind_keys_.clear();
  parameters_.clear();
}

// 获取参数
template<typename T>
VOID CDataBase::GetParameterT(LPCTSTR pszItem, T& DBVarValue) {
  auto itr = parameters_.find(pszItem);
  if (itr != parameters_.end()) {
    DBVarValue = std::get<T>(itr->second.DBVarValue);
  }
}

// 插入参数
template<typename T>
VOID CDataBase::AddParameterT(LPCTSTR pszParamName, ParamDirection Direction, const T& DBVarValue) {
  Parameter parameter{Direction, DBVarValue};
  bind_keys_.emplace_back(pszParamName);
  parameters_.emplace(pszParamName, std::move(parameter));
}

// 往下移动
VOID CDataBase::MoveToNext() {
  // try {
  //   recordset_.next();
  // } catch (const std::exception& ComError) {
  //   OnSQLException(SQLException_Syntax, ComError);
  // }

  ++cached_pos_;
}

// 移到开头
VOID CDataBase::MoveToFirst() {
  // try {
  //   recordset_.first();
  // } catch (const std::exception& ComError) {
  //   OnSQLException(SQLException_Syntax, ComError);
  // }

  cached_pos_ = cached_set_.begin();
}

// 是否结束
bool CDataBase::IsRecordsetEnd() {
  // try {
  //   return recordset_.at_end();
  // } catch (const std::exception &ComError) {
  //   OnSQLException(SQLException_Syntax, ComError);
  //   return false;
  // }

  return cached_pos_ == cached_set_.end(); // 检测是否有数据
}

// 获取数目
LONG CDataBase::GetRecordCount() {
  // try {
  //   return recordset_.rows();
  // } catch (const std::exception& ComError) {
  //   OnSQLException(SQLException_Syntax, ComError);
  // }

  return cached_set_.size();
}

// 返回数值
LONG CDataBase::GetReturnValue() {
  return return_var_;
}

// 关闭记录
VOID CDataBase::CloseRecordset() {
  // try {
  //   recordset_ = nanodbc::result();
  // } catch (const std::exception& ComError) {
  //   OnSQLException(SQLException_Syntax, ComError);
  // }

  cached_set_.clear();
  cached_pos_ = cached_set_.end();
}

// 获取数据
template<typename T>
inline VOID CDataBase::GetRecordsetValueT(LPCTSTR pszItem, T& DBVarValue) {
  try {
    auto value = cached_pos_->find(pszItem);
    if (value == cached_pos_->end()) { // 没找到
      return;
    }

    if constexpr (!std::is_same_v<T, std::monostate>) {
      // short column = recordset_.column(pszItem);
      // if (!recordset_.is_null(column)) {
      //   recordset_.get_ref(column, DBVarValue);
      if (auto pval = std::get_if<T>(&value->second)) {
        DBVarValue = *pval; // 复制出来
      }
    }
  } catch (const std::exception& ComError) {
    OnSQLException(SQLException_Syntax, ComError);
  }
}

VOID CDataBase::FillRecordset(nanodbc::result& result) {
  // while (result.next()) {
  //   int rows = result.rows();
  //   cachedset_.resize(rows);
  //   for (int i = 0; i < rows; i++) {
  //     int columns = result.columns();
  //   }
  // }
  const short col_count = result.columns();
  std::vector<StringT> col_names(col_count);
  for (short i = 0; i < col_count; ++i) {
    col_names[i] = ToSimpleString(result.column_name(i));
  }
  try {
    while (result.next()) {
      std::unordered_map<StringT, ParamVariant> row;

      for (short i = 0; i < col_count; ++i) {
        // 获取列名
        StringT col_name = col_names[i];

        // 判断列是否为空
        if (result.is_null(i)) {
          // 你可以选择用 std::monostate 或自定义 Null 类型
          row.emplace(std::move(col_name), std::monostate{});
          continue;
        }

        // 根据列类型读取数据
        switch (result.column_c_datatype(i)) {
          case SQL_C_BINARY: {
            std::vector<uint8_t> v;
            result.get_ref(i, v);
            row.emplace(std::move(col_name), ParamVariant{std::move(v)});
            break;
          }
          case SQL_C_CHAR:
          case SQL_C_WCHAR: {
            nanodbc::string v;
            result.get_ref(i, v);
            row.emplace(std::move(col_name), ParamVariant{std::move(v)});
            break;
          }

          case SQL_C_LONG:
          case SQL_C_SBIGINT: {
            int64_t v = 0;
            result.get_ref(i, v);
            row.emplace(std::move(col_name), ParamVariant{std::move(v)});
            break;
          }

          case SQL_C_FLOAT:
          case SQL_C_DOUBLE: {
            double v = 0.0;
            result.get_ref(i, v);
            row.emplace(std::move(col_name), ParamVariant{std::move(v)});
            break;
          }

          case SQL_C_DATE:
          case SQL_C_TIME:
          case SQL_C_TIMESTAMP: {
            nanodbc::timestamp v;
            result.get_ref(i, v);
            row.emplace(std::move(col_name), ParamVariant{std::move(v)});
            break;
          }
          default:
            CLogger::Warn("未知的 SQL 数据类型");
            break;
        }
      }

      cached_set_.push_back(std::move(row));
    }
    MoveToFirst();
  } catch (const std::exception& err) {
    // 没有结果集返回的时候第一个字段为 parameter_ordinal
    // 这里并不想这么判断，还是只记录异常不向外抛异常了
    OnSQLException(SQLException_None, err);
  }
}

// 存储过程
VOID CDataBase::ExecuteProcess(LPCTSTR pszSPName, bool bRecordset) {
  // 执行命令
  do {
    try {
      // 关闭记录
      CloseRecordset();

      // 设置名字
      int size = bind_keys_.size();
      StringT parameter;
      if (size > 0) {
        parameter.reserve(size * 2);
        while ((--size) > 0) {
          parameter += TEXT("?,");
        }
        parameter += TEXT("?");
      }
      // https://learn.microsoft.com/zh-cn/sql/relational-databases/native-client-odbc-how-to/running-stored-procedures-call-stored-procedures
      StringT query = std::format(TEXT("{{? = CALL {}({}) }}"), StringT(pszSPName), parameter);
      nanodbc::statement stmt(connection_, ToSimpleString(query));
      int index = 0;
      size = bind_keys_.size();
      return_var_ = -1;
      // stmt.bind(index++, &return_var_, nanodbc::statement::PARAM_RETURN);
      stmt.bind(index++, &return_var_, nanodbc::statement::PARAM_OUT);
      for (size_t i = 0; i < size; i++, index++) {
        auto& bindkey = bind_keys_[i];
        auto& parameter = parameters_[bindkey];
        std::visit(
            [&](auto&& arg) {
              using T = std::decay_t<decltype(arg)>;
              if constexpr (std::is_same_v<T, int64_t>) {
                auto& ref = std::get<int64_t>(parameter.DBVarValue);
                stmt.bind(index, &arg, parameter.Direction);
              } else if constexpr (std::is_same_v<T, double>) {
                auto& ref = std::get<double>(parameter.DBVarValue);
                stmt.bind(index, &arg, parameter.Direction);
              } else if constexpr (std::is_same_v<T, nanodbc::timestamp>) {
                auto& ref = std::get<nanodbc::timestamp>(parameter.DBVarValue);
                stmt.bind(index, &arg, parameter.Direction);
              } else if constexpr (std::is_same_v<T, nanodbc::string>) {
                auto& ref = std::get<nanodbc::string>(parameter.DBVarValue);
#ifdef _WIN32
                stmt.bind(index, arg.c_str(), arg.size(), parameter.Direction);
#else
                std::vector<uint8_t> data = StringUtils::ToNChar(arg);
                stmt.bind(index, {data}, parameter.Direction);
#endif
              } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
                auto& ref = std::get<std::vector<uint8_t>>(parameter.DBVarValue);
                stmt.bind(index, {arg}, parameter.Direction);
              } else {
                ASSERT(false);
                return;
              }
            },
            parameter.DBVarValue);
      }

      // 执行命令
      if (bRecordset) {
        auto result = stmt.execute();
        FillRecordset(result);
        while (result.next_result())
          ;
      } else {
        stmt.just_execute();
      }

      break;
    } catch (const std::exception& ComError) {
      // 22002: Indicator variable required but not supplied
      // 关于上述错误の解决方案系在存储过程里面添加如下脚本:
      // SET @strErrorDescribe = ISNULL(@strErrorDescribe, N'')
      // 连接判断
      if (IsConnectError()) {
        // 重新连接
        if (!TryConnectAgain(ComError)) {
          OnSQLException(SQLException_Connect, ComError);
        }
      } else {
        // 抛出异常
        OnSQLException(SQLException_Syntax, ComError);
      }
    }
  } while (TRUE);
}

// 执行语句
VOID CDataBase::ExecuteSentence(LPCTSTR pszCommand, bool bRecordset) {
  // 执行命令
  do {
    try {
      // 执行命令
      nanodbc::statement stmt(connection_, ToSimpleString(pszCommand));
      if (bRecordset) {
        auto result = stmt.execute();
        FillRecordset(result);
        // recordend_ = !recordset_.next(); // 检测是否有数据
      } else {
        stmt.just_execute();
      }

      break;
    } catch (const std::exception& ComError) {
      // 连接判断
      if (IsConnectError()) {
        // 重新连接
        if (!TryConnectAgain(ComError)) {
          OnSQLException(SQLException_Connect, ComError);
        }
      } else {
        // 抛出异常
        OnSQLException(SQLException_Syntax, ComError);
      }
    }
  } while (TRUE);
}

// 连接错误
bool CDataBase::IsConnectError() {
  try {
    // 状态判断
    if (!connection_.connected()) {
      return true;
    }

    return false;
  } catch (const std::exception& ComError) {
    OnSQLException(SQLException_Connect, ComError);
  }

  return false;
}

// 是否打开
bool CDataBase::IsRecordsetOpened() {
  return true;
}

// 重新连接
bool CDataBase::TryConnectAgain(const std::exception& exception) {
  // 设置变量
  connect_count_++;

  // 重连判断
  time_t dwNowTime = std::time(nullptr);
  if ((connect_count_ >= resume_connect_count_) || ((connect_error_time_ + resume_connect_time_) <= dwNowTime)) {
    try {
      // 打开连接
      OpenConnection();

      // 提示消息
      CLogger::Warn(TEXT("成功重新连接数据库"));

      return true;
    } catch (IDataBaseException*) {
      OnSQLException(SQLException_Connect, exception);
    }
  } else {
    OnSQLException(SQLException_Connect, exception);
  }

  return false;
}

// 转换字符
VOID CDataBase::ConvertToSQLSyntax(LPCTSTR pszString, StringT* strResult) {
  strResult->clear();

  // 替换字符
  while (*pszString != TEXT('\0')) {
    strResult->append(pszString, 1);
    if (*pszString == TEXT('\'')) {
      strResult->append(pszString, 1); // \' 转义 \'\'
    }
    ++pszString;
  }
}

// 错误处理
VOID CDataBase::OnSQLException(const enSQLException SQLException, const std::exception& exception) {
  // 设置异常
#if defined(_WIN32)
#if defined(_UNICODE)
  auto strDescribe = StringUtils::ToUtf16Ex(exception.what());
#else
  auto strDescribe = StringUtils::ToLocal(StringUtils::ToUtf16Ex(exception.what()));
#endif
#else
  auto strDescribe = exception.what();
#endif
  db_exception_.SetExceptionInfo(SQLException, strDescribe);

  // 抛出异常
  if (SQLException != SQLException_None) {
    throw &db_exception_;
  }
}

constexpr std::string prefix{"ODBC Driver "};
constexpr std::string suffix(" for SQL Server");

int ParseDriverVersion(const std::string& name) {
  auto pos1 = name.find(prefix);
  if (pos1 == std::string::npos) {
    return -1;
  }

  pos1 += prefix.size();
  const auto pos2 = name.find(suffix, pos1);
  if (pos2 == std::string::npos) {
    return -1;
  }

  const auto num = name.substr(pos1, pos2 - pos1);
  return std::stoi(num);
}

bool CDataBase::GetDriverInfo(StringT* driver_name, int32_t* dirver_ver) {
  // 构造连接
  const auto& drivers = nanodbc::list_drivers();
  for (const auto& [name, _]: drivers) {
    int32_t ver = ParseDriverVersion(ToSimpleString(name));
    if (ver >= 18) {
      if (dirver_ver) {
        *dirver_ver = ver;
      }
      *driver_name = ToSimpleString(name);
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////////
// 接口查询
VOID* CDataBaseEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE(IDataBaseEngine, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDataBaseEngine, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CDataBaseEngine::StartService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(io_context != nullptr && !is_running_.load(std::memory_order_relaxed));
  if (io_context == nullptr || is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  io_context_ = std::move(io_context);
  strand_ = asio::make_strand(io_context_->get_executor());

  // 初始化事件通知总线的执行序列
  OnDataBaseEngineStart.Setup(strand_);
  OnDataBaseEngineConclude.Setup(strand_);
  OnDataBaseEngineRequest.Setup(strand_);
  OnDataBaseEngineResponse.Setup(strand_);

  OnDataBaseEngineStart(this);
  return is_running_.exchange(true, std::memory_order_relaxed) == false;
}

// 停止服务
bool CDataBaseEngine::ConcludeService() {
  // 设置变量
  if (!is_running_.exchange(false, std::memory_order_relaxed)) {
    return true;
  }

  OnDataBaseEngineConclude(this);

  // 释放外部绑定的多态事件
  OnDataBaseEngineStart.Clear();
  OnDataBaseEngineConclude.Clear();
  OnDataBaseEngineRequest.Clear();
  OnDataBaseEngineResponse.Clear();

  // 重置内核资产生命周期
  io_context_.reset();
  return true;
}

// // 引擎负荷
// bool CDataBaseEngine::GetBurthenInfo(tagBurthenInfo& BurthenInfo) {
//   return true;
// }

// 配置模块
bool CDataBaseEngine::SetDataBaseEngineSink(void* object_ptr) {
  // 查询接口
  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IDataBaseEngineSink) != nullptr);
  const auto sink_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IDataBaseEngineSink);

  if (sink_ptr == nullptr) {
    return false;
  }

  OnDataBaseEngineStart += MEMBER_DELEGATE(&IDataBaseEngineSink::OnDataBaseEngineStart, sink_ptr);
  OnDataBaseEngineConclude += MEMBER_DELEGATE(&IDataBaseEngineSink::OnDataBaseEngineConclude, sink_ptr);
  OnDataBaseEngineRequest += MEMBER_DELEGATE(&IDataBaseEngineSink::OnDataBaseEngineRequestInternal, sink_ptr);
  return true;
}

// 配置模块
bool CDataBaseEngine::SetDataBaseEngineEvent(void* object_ptr) {
  // 查询接口
  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IDataBaseEngineEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IDataBaseEngineEvent);

  if (event_ptr == nullptr) {
    return false;
  }

  OnDataBaseEngineResponse += MEMBER_DELEGATE(&IDataBaseEngineEvent::OnEventDataBaseResultInternal, event_ptr);
  return true;
}

// 处理事件
bool CDataBaseEngine::PostDataBaseRequest(WORD request_id, DWORD context_id, VOID* data, WORD data_size) {
  // 状态效验
  ASSERT(is_running_.load(std::memory_order_relaxed));
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  auto post_data = ConvertToBytes(data, data_size);
  OnDataBaseEngineRequest(request_id, context_id, std::move(post_data));
  return true;
}

bool CDataBaseEngine::OnEventDataBaseResult(WORD request_id, DWORD context_id, VOID* data, WORD data_size) {
  auto post_data = ConvertToBytes(data, data_size);
  OnDataBaseEngineResponse(request_id, context_id, std::move(post_data));
  return true;
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(DataBase);
DECLARE_CREATE_MODULE(DataBaseEngine);

//////////////////////////////////////////////////////////////////////////////////
