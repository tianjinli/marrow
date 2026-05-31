#pragma once

#include "ChatServerHead.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 程序对象
class CChatServer : public IServiceUnitsSink {
  // 组件变量
protected:
  CServiceUnits m_ServiceUnits; // 服务单元

  // 配置参数
protected:
  bool m_bAutoControl = false; // 自动控制
  bool m_bOptionSuccess = false; // 配置标志

  // 函数定义
public:
  // 构造函数
  explicit CChatServer() { m_ServiceUnits.SetServiceUnitsSink(this); }
  // 析构函数
  virtual ~CChatServer() = default;

  // 服务接口
public:
  // 服务状态
  virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus);
  // 启动服务
  VOID StartService();
  // 停止服务
  VOID ConcludeService();

public:
  // 关闭询问
  bool OnQueryEndSession();
};

//////////////////////////////////////////////////////////////////////////////////
