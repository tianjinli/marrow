#pragma once

#include "CorrespondHeader.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 主对话框
class CCorrespondServer : public IServiceUnitsSink {
  // 组件变量
protected:
  CServiceUnits m_ServiceUnits;               // 服务单元

  // 函数定义
public:
  // 构造函数
  CCorrespondServer() { m_ServiceUnits.SetServiceUnitsSink(this); }
  // 析构函数
  virtual ~CCorrespondServer() = default;

  // 服务接口
public:
  // 服务状态
  VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus) override;

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
