#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 命令解释
class [[deprecated]] SERVICE_CORE_CLASS CWHCommandLine {
  // 函数定义
public:
  // 构造函数
  CWHCommandLine() = default;
  // 析构函数
  virtual ~CWHCommandLine() = default;

  // 功能函数
public:
  // 查询命令
  bool SearchCommandItem(LPCTSTR pszCommandLine, LPCTSTR pszCommand, TCHAR szParameter[], WORD wParameterLen);
};

//////////////////////////////////////////////////////////////////////////////////
