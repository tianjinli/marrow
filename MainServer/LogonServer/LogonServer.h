#pragma once

#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 程序对象
class CLogonServer : public IServiceUnitsSink {
protected:
  CServiceUnits m_ServiceUnits; // 服务单元
  // 函数定义
public:
  // 构造函数
  explicit CLogonServer() { m_ServiceUnits.SetServiceUnitsSink(this); }
  // 析构函数
  virtual ~CLogonServer() = default;

  // 重载函数
public:
  virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus);
  // 启动服务
  VOID StartService();
  // 停止服务
  VOID ConcludeService();

  // 消息映射
public:
  // 关闭询问
  bool OnQueryEndSession();
};

//////////////////////////////////////////////////////////////////////////////////
