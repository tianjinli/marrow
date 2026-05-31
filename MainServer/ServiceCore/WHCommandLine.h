#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 命令解释
class [[deprecated]] SERVICE_CORE_CLASS CWHCommandLine {
public:
  // 构造函数
  CWHCommandLine() = default;
  // 析构函数
  virtual ~CWHCommandLine() = default;

  // 功能函数
  // 查询命令
  bool SearchCommandItem(LPCTSTR command_line, LPCTSTR command, TCHAR parameter[], WORD parameter_len);
};

//////////////////////////////////////////////////////////////////////////////////
