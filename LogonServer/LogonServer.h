#pragma once

#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 主对话框
class CLogonServer : public IServiceUnitsSink {
  // 组件变量
protected:
  CServiceUnits m_ServiceUnits;               // 服务单元

  // 函数定义
public:
  // 构造函数
  CLogonServer();
  // 析构函数
  virtual ~CLogonServer();

  // 服务接口
public:
  // 服务状态
  virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus);

  // 启动服务
  VOID DoStartService();
  // 停止服务
  VOID DoStopService();

  // 消息映射
public:
  // 关闭询问
  bool OnQueryEndSession();
};

//////////////////////////////////////////////////////////////////////////////////
