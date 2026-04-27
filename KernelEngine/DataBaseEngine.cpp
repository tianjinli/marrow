#include "DataBaseEngine.h"

#include <sqlext.h>

#include "AsyncEventHub.h"
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
    const auto strConnect = fmt::format(TEXT("Driver={};DataBase={};Uid={};Pwd={};Server=tcp:{},{};TrustServerCertificate=yes;Charset=UTF-8;"),
                                        driver_name_, db_name_, user_, password_, host_ip_, host_port);

    connection_.connect(strConnect);
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
    if (connection_.connected())
      connection_.disconnect();
  } catch (const std::exception& err) {
    OnSQLException(SQLException_Syntax, err);
  }
}

// 连接信息
bool CDataBase::SetConnectionInfo(DWORD dwDBAddr, WORD wPort, LPCTSTR szDBName, LPCTSTR szUser, LPCTSTR szPassword) {
  auto addr = asio::ip::make_address_v4(dwDBAddr);
  StringT strAddr = ToSimpleStringT(addr.to_string());
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
    if (value == cached_pos_->end()) // 没找到
      return;

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
    col_names[i] = result.column_name(i);
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
          case SQL_C_CHAR:
          case SQL_C_BINARY:
          case SQL_C_WCHAR: {
            StringT v;
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
        while ((--size) > 0) {
          parameter += TEXT("?, ");
        }
        parameter += TEXT("?");
      }
      // https://learn.microsoft.com/zh-cn/sql/relational-databases/native-client-odbc-how-to/running-stored-procedures-call-stored-procedures
      StringT query = fmt::format(TEXT("{{? = call {}({}) }}"), StringT(pszSPName), parameter);
      nanodbc::statement stmt(connection_, query);
      int index = 0;
      size = bind_keys_.size();
      return_var_ = -1;
      stmt.bind(index++, &return_var_, nanodbc::statement::PARAM_RETURN);
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
              } else if constexpr (std::is_same_v<T, StringT>) {
                auto& ref = std::get<StringT>(parameter.DBVarValue);
                stmt.bind(index, arg.c_str(), arg.size(), parameter.Direction);
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
      nanodbc::statement stmt(connection_, pszCommand);
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
    if (!connection_.connected())
      return true;

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
  if (SQLException != SQLException_None)
    throw &db_exception_;
}

bool CDataBase::GetDriverInfo(StringT* driver_name, int32_t* dirver_ver) {
  // 构造连接
  const auto& drivers = nanodbc::list_drivers();
  for (const auto& [name, _]: drivers) {
    int32_t ver = 0;
    SscanfT(name.c_str(), TEXT("ODBC Driver %d for SQL Server"), &ver);
    if (ver >= 18) {
      if (dirver_ver)
        *dirver_ver = ver;
      *driver_name = name;
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
  // QUERYINTERFACE(IAsynchronismEngineSink, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDataBaseEngine, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CDataBaseEngine::InitiateService(std::shared_ptr<asio::io_context> io_context) {
  // 注册对象
  // IUnknownEx* pIAsynchronismEngineSink = QUERY_ME_INTERFACE(IUnknownEx);
  // if (!asynchronism_engine_.SetAsynchronismSink(pIAsynchronismEngineSink)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 异步引擎
  // if (!asynchronism_engine_.InitiateService(io_context)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  if (GlobalEventBus::Get())
    GlobalEventBus::Get()->Publish<DataBaseStartEventTag>(QUERY_ME_INTERFACE(IUnknownEx));
  return true;
}

// 停止服务
bool CDataBaseEngine::ConcludeService() {
  // // 异步引擎
  // asynchronism_engine_.ConcludeService();

  if (GlobalEventBus::Get())
    GlobalEventBus::Get()->Publish<DataBaseStopEventTag>(QUERY_ME_INTERFACE(IUnknownEx));
  return true;
}

// 引擎负荷
bool CDataBaseEngine::GetBurthenInfo(tagBurdenInfo& BurthenInfo) {
  return true;
}

// 配置模块
bool CDataBaseEngine::SetDataBaseEngineSink(IUnknownEx* sink_any) {
  // 查询接口
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(sink_any, IDataBaseEngineSink) != nullptr);
  // event_handler_ = QUERY_OBJECT_PTR_INTERFACE(sink_any, IDataBaseEngineSink);
  //
  // // 结果判断
  // if (event_handler_ == nullptr) {
  //   ASSERT(FALSE);
  //   return false;
  // }
  return true;
}

// 配置模块
bool CDataBaseEngine::SetDataBaseEngineSink(IUnknownEx* sink_anys[], WORD sink_count) {
  //  暂时性处理

  // 效验参数
  ASSERT(sink_anys != nullptr);
  if (sink_anys == nullptr)
    return false;

  ASSERT(sink_count > 0);
  if (sink_count == 0)
    return false;

  // // 查询接口
  // event_handler_ = QUERY_OBJECT_PTR_INTERFACE(sink_objs[0], IDataBaseEngineSink);
  // if (event_handler_ == nullptr) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // for (WORD i = 0; i < sink_count; i++) {
  //   SetDataBaseEngineSink(sink_anys[i]);
  // }
  SetDataBaseEngineSink(sink_anys[0]);
  return true;
}

// 延期请求
bool CDataBaseEngine::DeferDataBaseRequest(WORD request_id, DWORD context_id, VOID* data, WORD data_size) {
  return PostDataBaseRequest(request_id, context_id, data, data_size);
}

// 控制事件
bool CDataBaseEngine::PostDataBaseControl(WORD control_id, VOID*, WORD) {
  // const NTY_ControlEvent control_event{.control_id = control_id};
  // auto post_data = ConvertToBytes(control_event);

  // 投递请求
  // asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_CONTROL, std::move(post_data));
  ASSERT(FALSE); // 目前没有被使用
  return true;
}

// 处理事件
bool CDataBaseEngine::PostDataBaseRequest(WORD request_id, DWORD context_id, VOID* data, WORD data_size) {
  // // 效验参数
  // ASSERT((data_size + sizeof(NTY_DataBaseEvent)) <= MAX_ASYNCHRONISM_DATA);
  // if ((data_size + sizeof(NTY_DataBaseEvent)) > MAX_ASYNCHRONISM_DATA)
  //   return false;

  // const NTY_DataBaseEvent database_event{.request_id = request_id, .context_id = context_id};
  //
  // auto post_data = std::make_shared<std::vector<uint8_t>>();
  // // 分配内存（不进行清零操作）
  // post_data->reserve(sizeof(NTY_DataBaseEvent) + data_size);
  // // 构造数据
  // AppendToBytes(post_data, database_event);
  //
  // // 附加数据
  // if (data_size > 0) {
  //   AppendToBytes(post_data, data, data_size);
  // }
  //
  // // 投递请求
  // asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_DATABASE, std::move(post_data));

  auto post_data = ConvertToBytes(data, data_size);
  GlobalEventBus::Get()->Publish<DataBaseRequestEventTag>(request_id, context_id, std::move(post_data));
  return true;
}

// 启动事件
bool CDataBaseEngine::OnAsynchronismEngineStart() {
  // // 效验参数
  // ASSERT(event_handler_ != nullptr);
  // if (event_handler_ == nullptr)
  //   return false;

  // // 事件通知
  // return event_handler_->OnDataBaseEngineStart(database_engine);
  return true;
}

// 停止事件
bool CDataBaseEngine::OnAsynchronismEngineConclude() {
  // // 效验参数
  // ASSERT(event_handler_ != nullptr);
  // if (event_handler_ == nullptr)
  //   return false;

  // // 事件通知
  // return event_handler_->OnDataBaseEngineConclude(database_engine);
  return true;
}

// bool CDataBaseEngine::OnAsynchronismEngineData(WORD identifier, nf::BufferPtr data) {
//   switch (identifier) {
//     case ASYNCHRONISM_TIMER: // 时间事件
//     {
//       // 效验参数
//       ASSERT(data->size() == sizeof(NTY_TimerEvent));
//       if (data->size() != sizeof(NTY_TimerEvent))
//         return false;
//
//       // 变量定义
//       const auto timer_event = reinterpret_cast<NTY_TimerEvent*>(data->data());
//
//       // 处理数据
//       try {
//         // ASSERT(event_handler_ != nullptr);
//         // return event_handler_->OnDataBaseEngineTimer(timer_event->timer_id, timer_event->bind_parameter);
//       } catch (...) {
//         ASSERT(FALSE);
//         return false;
//       }
//
//       return true;
//     }
//     case ASYNCHRONISM_CONTROL: // 控制事件
//     {
//       // 大小断言
//       ASSERT(data->size() >= sizeof(NTY_ControlEvent));
//       if (data->size() < sizeof(NTY_ControlEvent))
//         return false;
//
//       // 变量定义
//       const auto control_event = reinterpret_cast<NTY_ControlEvent*>(data->data());
//
//       // // 处理数据
//       // try {
//       //   ASSERT(event_handler_ != nullptr);
//       //   event_handler_->OnDataBaseEngineControl(control_event->control_id, control_event + 1, data->size() - sizeof(NTY_ControlEvent));
//       // } catch (...) {
//       //   ASSERT(FALSE);
//       //   return false;
//       // }
//
//       return true;
//     }
//     case ASYNCHRONISM_DATABASE: // 数据事件
//     {
//       // 效验参数
//       ASSERT(data->size() >= sizeof(NTY_DataBaseEvent));
//       if (data->size() < sizeof(NTY_DataBaseEvent))
//         return false;
//
//       // 变量定义
//       const auto pDataBaseEvent = reinterpret_cast<NTY_DataBaseEvent*>(data->data());
//
//       // 处理数据
//       try {
//         // ASSERT(event_handler_ != nullptr);
//         // return event_handler_->OnDataBaseEngineRequest(pDataBaseEvent->request_id, pDataBaseEvent->context_id, pDataBaseEvent + 1,
//         //                                                data->size() - sizeof(NTY_DataBaseEvent));
//       } catch (std::exception e) {
//         CLogger::Error(FMT_STRING("{} → {}"), __FUNCTION__, ToSimpleUtf8(e.what()));
//         return false;
//       } catch (...) {
//         ASSERT(FALSE);
//         return false;
//       }
//
//       return true;
//     }
//   }
//
//   // 错误断言
//   ASSERT(FALSE);
//
//   return false;
// }

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(DataBase);
DECLARE_CREATE_MODULE(DataBaseEngine);

//////////////////////////////////////////////////////////////////////////////////
