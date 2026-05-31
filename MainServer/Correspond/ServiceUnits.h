#pragma once

#include "asio.hpp"

#include "AttemperEngineSink.h"
#include "CorrespondHead.h"
#include "InitParameter.h"

//////////////////////////////////////////////////////////////////////////////////
// 枚举定义

// 服务状态
enum enServiceStatus {
  ServiceStatus_Stop, // 停止状态
  ServiceStatus_Config, // 配置状态
  ServiceStatus_Service, // 服务状态
};

//////////////////////////////////////////////////////////////////////////////////

// 状态接口
interface IServiceUnitsSink {
  // 接口定义
public:
  // 服务状态
  virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

// 服务单元
class CServiceUnits : public CWHThreadPool {
  // 状态变量
protected:
  enServiceStatus m_ServiceStatus = ServiceStatus_Stop; // 运行状态

  // 服务组件
protected:
  CInitParameter m_InitParameter; // 配置参数
  CAttemperEngineSink m_AttemperEngineSink; // 调度钩子

  // 内核组件
protected:
  CTimerEngineHelper m_TimerEngine; // 时间引擎
  CAttemperEngineHelper m_AttemperEngine; // 调度引擎
  CTCPNetworkEngineHelper m_TCPNetworkEngine; // 网络引擎

  // 接口变量
protected:
  IServiceUnitsSink* m_pIServiceUnitsSink; // 状态接口
  std::shared_ptr<asio::io_context> io_context_ = std::make_shared<asio::io_context>();
  std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;

  // 函数定义
public:
  // 构造函数
  CServiceUnits() = default;
  // 析构函数
  virtual ~CServiceUnits();

  // 信息函数
public:
  // 获取状态
  enServiceStatus GetServiceStatus() { return m_ServiceStatus; }

  // 服务控制
public:
  // 启动服务
  bool StartService();
  // 停止服务
  bool ConcludeService();
  // 设置接口
  bool SetServiceUnitsSink(IServiceUnitsSink* pIServiceUnitsSink);

  // 辅助函数
protected:
  // 配置组件
  bool InitializeService();
  // 启动内核
  bool StartKernelService();

  // 内部函数
private:
  // 设置状态
  bool SetServiceStatus(enServiceStatus ServiceStatus);
};

//////////////////////////////////////////////////////////////////////////////////
