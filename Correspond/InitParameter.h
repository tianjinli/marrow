#pragma once

#include "CorrespondHeader.h"

//////////////////////////////////////////////////////////////////////////////////

// 配置参数
class CInitParameter {
  // 系统配置
public:
  WORD max_connect_ = MAX_CONTENT;  // 最大连接
  WORD service_port_ = PORT_CENTER; // 服务端口

  // 配置信息
public:
  TCHAR server_name_[LEN_SERVER] = {}; // 服务器名

  // 函数定义
public:
  // 构造函数
  CInitParameter() = default;
  // 析构函数
  virtual ~CInitParameter() = default;

  // 功能函数
public:
  // 初始化
  VOID InitParameter();
  // 加载配置
  VOID LoadInitParameter();
};

//////////////////////////////////////////////////////////////////////////////////
