#pragma once

#include "CorrespondHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 配置参数
class CInitParameter {
  // 系统配置
public:
  WORD m_wMaxConnect; // 最大连接
  WORD m_wServicePort; // 服务端口

  // 配置信息
  TCHAR m_szServerName[LEN_SERVER]; // 服务器名

  // 构造函数
  CInitParameter() = default;
  // 析构函数
  virtual ~CInitParameter() = default;

  // 初始化
  VOID InitParameter();
  // 加载配置
  VOID LoadInitParameter();
};

//////////////////////////////////////////////////////////////////////////////////
