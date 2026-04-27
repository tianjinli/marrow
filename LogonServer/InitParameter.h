#pragma once

#include "GlobalDefine/Platform.h"

//////////////////////////////////////////////////////////////////////////////////

// 服务时间
#define TIME_CONNECT 30 // 重连时间
#define TIME_COLLECT 300 // 统计时间
#define TIME_LOAD_LIST 120 // 加载列表
#define TIME_RELOAD_LIST 600 // 加载列表

// 客户时间
#define TIME_INTERMITTENT 0 // 中断时间
#define TIME_ONLINE_COUNT 600 // 人数时间

//////////////////////////////////////////////////////////////////////////////////

// 配置参数
class CInitParameter {
  // 系统配置
public:
  BYTE delay_list_ = TRUE; // 延时列表
  WORD max_connect_ = MAX_CONTENT; // 最大连接
  WORD service_port_ = PORT_LOGON; // 服务端口
  TCHAR server_name_[LEN_SERVER]; // 服务器名

  // 组件时间
public:
  WORD connect_time_ = TIME_CONNECT; // 重连时间
  WORD collect_time_ = TIME_COLLECT; // 统计时间
  WORD load_list_time_ = TIME_LOAD_LIST; // 列表时间
  WORD m_wReLoadListTime = TIME_RELOAD_LIST; // 列表时间

  // 客户时间
public:
  WORD intermittent_time_ = TIME_INTERMITTENT; // 中断时间
  WORD online_count_time_ = TIME_ONLINE_COUNT; // 人数时间

  // 协调信息
public:
  WORD correspond_port_ = PORT_CENTER; // 协调端口
  tagAddressInfo correspond_address_{.szAddress = TEXT("127.0.0.1")}; // 协调地址

  // 约战信息
public:
  WORD personal_port_ = PORT_PERSONAL_ROOM; // 约战端口
  tagAddressInfo personal_address_{.szAddress = TEXT("127.0.0.1")}; // 约战地址

  // 服务地址
public:
  tagAddressInfo service_address_{.szAddress = TEXT("127.0.0.1")}; // 服务地址
  tagDataBaseParameter accounts_db_parameter_{}; // 连接地址
  tagDataBaseParameter treasure_db_parameter_{}; // 连接地址
  tagDataBaseParameter platform_db_parameter_{}; // 连接地址

  // 函数定义
public:
  // 构造函数
  explicit CInitParameter() = default;
  // 析构函数
  virtual ~CInitParameter() = default;

  // 功能函数
public:
  // 加载配置
  void LoadInitParameter();
};

//////////////////////////////////////////////////////////////////////////////////
