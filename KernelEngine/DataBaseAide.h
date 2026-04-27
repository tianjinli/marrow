/// https://docs.microsoft.com/zh-cn/sql/linux/quickstart-install-connect-red-hat
#pragma once

#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 数据库助手
class KERNEL_ENGINE_CLASS CDataBaseAide {
  // 变量定义
protected:
  IDataBase* db_engine_; // 数据对象

  // 函数定义
public:
  // 构造函数
  CDataBaseAide(IUnknownEx* pIUnknownEx = nullptr);
  // 析构函数
  virtual ~CDataBaseAide() = default;

  // 管理函数
public:
  // 设置对象
  bool SetDataBase(IUnknownEx* pIUnknownEx);
  // 获取对象
  VOID* GetDataBase(REFGUID Guid, DWORD dwQueryVer);

  // 获取数据
public:
  // 获取数据
  INT GetValue_INT(LPCTSTR pszItem);
  // 获取数据
  UINT GetValue_UINT(LPCTSTR pszItem);
  // 获取数据
  LONG GetValue_LONG(LPCTSTR pszItem);
  // 获取数据
  BYTE GetValue_BYTE(LPCTSTR pszItem);
  // 获取数据
  WORD GetValue_WORD(LPCTSTR pszItem);
  // 获取数据
  DWORD GetValue_DWORD(LPCTSTR pszItem);
  // 获取数据
  FLOAT GetValue_FLOAT(LPCTSTR pszItem);
  // 获取数据
  DOUBLE GetValue_DOUBLE(LPCTSTR pszItem);
  // 获取数据
  LONGLONG GetValue_LONGLONG(LPCTSTR pszItem);
  // 获取数据
  VOID GetValue_VarValue(LPCTSTR pszItem, StringT& DBVarValue);
  // 获取数据
  VOID GetValue_SystemTime(LPCTSTR pszItem, SYSTEMTIME& SystemTime);
  // 获取字符
  VOID GetValue_String(LPCTSTR pszItem, LPTSTR pszString, UINT uMaxCount);

  // 参数函数
public:
  // 重置参数
  VOID ResetParameter();
  // 获取参数
  VOID GetParameter(LPCTSTR pszItem, StringT& DBVarValue);
  // 获取参数
  VOID GetParameter(LPCTSTR pszItem, LPTSTR pszString, UINT uSize);

  // 插入参数
public:
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, int32_t nValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, uint32_t uValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
#ifdef _WIN32
  inline VOID AddParameter(LPCTSTR pszItem, long nValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  inline VOID AddParameter(LPCTSTR pszItem, unsigned long uValue,
                           nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
#endif
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, int64_t lValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, uint64_t lValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, uint8_t cbValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, int16_t wValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, uint16_t dwValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, FLOAT fValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, DOUBLE dValue, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, LPCTSTR pszString, nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameter(LPCTSTR pszItem, const SYSTEMTIME& SystemTime,
                    nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_IN);
  // 插入参数
  VOID AddParameterOutput(LPCTSTR pszItem, LPTSTR pszString, UINT uSize,
                          nanodbc::statement::param_direction ParameterDirection = nanodbc::statement::PARAM_INOUT);

  // 执行辅助
public:
  // 返回数值
  LONG GetReturnValue();
  // 存储过程
  LONG ExecuteProcess(LPCTSTR pszSPName, bool bRecordset);
};

//////////////////////////////////////////////////////////////////////////////////
