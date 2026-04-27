#include "DataBaseAide.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CDataBaseAide::CDataBaseAide(IUnknownEx* pIUnknownEx) {
  // 查询接口
  db_engine_ = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IDataBase);
}

// 设置对象
bool CDataBaseAide::SetDataBase(IUnknownEx* pIUnknownEx) {
  if (pIUnknownEx != nullptr) {
    // 设置接口
    ASSERT(QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IDataBase) != nullptr);
    db_engine_ = QUERY_OBJECT_PTR_INTERFACE(pIUnknownEx, IDataBase);
    if (db_engine_ == nullptr) {
      ASSERT(FALSE);
      return false;
    }
  } else {
    db_engine_ = nullptr;
  }

  return true;
}

// 获取对象
VOID* CDataBaseAide::GetDataBase(REFGUID Guid, DWORD dwQueryVer) {
  if (db_engine_ == nullptr)
    return nullptr;
  return db_engine_->QueryInterface(Guid, dwQueryVer);
}

// 获取数据
INT CDataBaseAide::GetValue_INT(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<INT>(DBVarValue);
}

// 获取数据
UINT CDataBaseAide::GetValue_UINT(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<UINT>(DBVarValue);
}

// 获取数据
LONG CDataBaseAide::GetValue_LONG(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<LONG>(DBVarValue);
}

// 获取数据
BYTE CDataBaseAide::GetValue_BYTE(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<BYTE>(DBVarValue);
}

// 获取数据
WORD CDataBaseAide::GetValue_WORD(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<WORD>(DBVarValue);
}

// 获取数据
DWORD CDataBaseAide::GetValue_DWORD(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<DWORD>(DBVarValue);
}

// 获取数据
FLOAT CDataBaseAide::GetValue_FLOAT(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  double DBVarValue = 0.0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<FLOAT>(DBVarValue);
}

// 获取数据
DOUBLE CDataBaseAide::GetValue_DOUBLE(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  double DBVarValue = 0.0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<DOUBLE>(DBVarValue);
}

// 获取数据
LONGLONG CDataBaseAide::GetValue_LONGLONG(LPCTSTR pszItem) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  int64_t DBVarValue = 0;
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  return static_cast<LONGLONG>(DBVarValue);
}

// 获取数据
VOID CDataBaseAide::GetValue_VarValue(LPCTSTR pszItem, StringT& DBVarValue) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);
}

// 获取数据
VOID CDataBaseAide::GetValue_SystemTime(LPCTSTR pszItem, SYSTEMTIME& SystemTime) {
  // 效验参数
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  nanodbc::timestamp timestamp = {};
  ZeroMemory(&SystemTime, sizeof(SystemTime));
  db_engine_->GetRecordsetValue(pszItem, timestamp);

  // 转换变量
  SystemTime.wYear = timestamp.year;
  SystemTime.wMonth = timestamp.month;
  SystemTime.wDayOfWeek = 0;
  SystemTime.wDay = timestamp.day;
  SystemTime.wHour = timestamp.hour;
  SystemTime.wMinute = timestamp.min;
  SystemTime.wSecond = timestamp.sec;
  SystemTime.wMilliseconds = 0;
  // SystemTime.wMilliseconds = DBVarValue.fract;
}

// 获取字符
VOID CDataBaseAide::GetValue_String(LPCTSTR pszItem, LPTSTR pszString, UINT uMaxCount) {
  // 效验参数
  ASSERT(pszString != nullptr);
  ASSERT(db_engine_ != nullptr);

  // 读取变量
  StringT DBVarValue;
  DBVarValue.reserve(uMaxCount);
  db_engine_->GetRecordsetValue(pszItem, DBVarValue);

  // 设置结果
  LSTRCPYN(pszString, DBVarValue.data(), uMaxCount);
}

// 重置参数
VOID CDataBaseAide::ResetParameter() {
  ASSERT(db_engine_ != nullptr);
  db_engine_->ClearParameters();
  // m_pIDataBase->AddParameter(TEXT("RETURN_VALUE"), ParamDirection::PARAM_RETURN, static_cast<int64_t>(0));
}

// 获取参数
VOID CDataBaseAide::GetParameter(LPCTSTR pszItem, StringT& DBVarValue) {
  ASSERT(db_engine_ != nullptr);
  db_engine_->GetParameter(pszItem, DBVarValue);
}

// 获取参数
VOID CDataBaseAide::GetParameter(LPCTSTR pszItem, LPTSTR pszString, UINT uSize) {
  // 变量定义
  StringT DBVarValue;

  // 获取参数
  ASSERT(db_engine_ != nullptr);
  db_engine_->GetParameter(pszItem, DBVarValue);

  // 设置结果
  LSTRCPYN(pszString, DBVarValue.data(), uSize);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, int32_t nValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = nValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, uint32_t uValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = uValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

#ifdef _WIN32
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, long nValue, nanodbc::statement::param_direction ParameterDirection) {
  AddParameter(pszItem, (int32_t)nValue, ParameterDirection);
}

VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, unsigned long uValue, nanodbc::statement::param_direction ParameterDirection) {
  AddParameter(pszItem, (uint32_t)uValue, ParameterDirection);
}
#endif

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, int64_t lValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = lValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, uint64_t lValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = lValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, uint8_t cbValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = cbValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, int16_t wValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = wValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, uint16_t dwValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  int64_t DBVarValue = dwValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, FLOAT fValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  double DBVarValue = fValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, DOUBLE dValue, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  double DBVarValue = dValue;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, LPCTSTR pszString, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  StringT DBVarValue(pszString);
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameter(LPCTSTR pszItem, const SYSTEMTIME& SystemTime, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);

  nanodbc::timestamp timestamp;
  timestamp.year = SystemTime.wYear;
  timestamp.month = SystemTime.wMonth;
  timestamp.day = SystemTime.wDay;
  timestamp.hour = SystemTime.wHour;
  timestamp.min = SystemTime.wMinute;
  timestamp.sec = SystemTime.wSecond;
  timestamp.fract = 0;
  // DBVarValue.fract = SystemTime.wMilliseconds;
  nanodbc::timestamp& DBVarValue = timestamp;
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

// 插入参数
VOID CDataBaseAide::AddParameterOutput(LPCTSTR pszItem, LPTSTR pszString, UINT uSize, nanodbc::statement::param_direction ParameterDirection) {
  ASSERT(db_engine_ != nullptr);
  StringT DBVarValue(pszString, pszString + uSize);
  db_engine_->AddParameter(pszItem, ParameterDirection, DBVarValue);
}

LONG CDataBaseAide::GetReturnValue() {
  ASSERT(db_engine_ != nullptr);
  return db_engine_->GetReturnValue();
}

// 存储过程
LONG CDataBaseAide::ExecuteProcess(LPCTSTR pszSPName, bool bRecordset) {
  ASSERT(db_engine_ != nullptr);
  db_engine_->ExecuteProcess(pszSPName, bRecordset);
  return db_engine_->GetReturnValue();
}

//////////////////////////////////////////////////////////////////////////////////
