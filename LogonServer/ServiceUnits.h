#pragma once

#include "DataBaseEngineSink.h"
#include "DispatchEngineSink.h"
#include "InitParameter.h"
#include "LogonServerHeader.h"

//////////////////////////////////////////////////////////////////////////////////

// 网络标示
#define NETWORK_CORRESPOND 1 // 协调服务
#define NETWORK_PERSONAL 2 // 约战服务

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
  // 友元定义
  friend class CDispatchEngineSink;
  friend class CDataBaseEngineSink;

  // 状态变量
protected:
  enServiceStatus service_status_ = ServiceStatus_Stop; // 运行状态

  // 配置组件
public:
  CInitParameter init_parameter_; // 配置参数

  // 服务组件
protected:
  // CAttemperEngineSink attemper_engine_sink_; // 调度钩子
  // CDataBaseEngineSink database_engine_sinks_[4]; // 数据钩子
  CDispatchEngineSink dispatch_engine_sink_; // 调度钩子
  CDataBaseEngineSink database_engine_sink_; // 数据钩子

  // 内核组件
protected:
  CTimerEngineHelper timer_engine_; // 时间引擎
  // CAttemperEngineHelper attemper_engine_; // 调度引擎
  CDataBaseEngineHelper database_engine_; // 数据引擎
  CTCPNetworkEngineHelper network_engine_; // 网络引擎
  CTCPSocketServiceHelper correspond_service_; // 协调服务
  CTCPSocketServiceHelper personal_service_; // 约战服务

private:
  std::atomic_uint64_t active_requests_ = 0;

  // 接口变量
protected:
  IServiceUnitsSink* service_units_sink_ = nullptr; // 状态接口
  std::shared_ptr<asio::io_context> io_context_ = std::make_shared<asio::io_context>();
  std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;

  // 函数定义
public:
  // 构造函数
  CServiceUnits();
  // 析构函数
  virtual ~CServiceUnits();

  // 信息函数
public:
  // 获取状态
  enServiceStatus GetServiceStatus() {
    return service_status_;
  }

  // 服务控制
public:
  // 启动服务
  bool DoStartService();
  // 停止服务
  bool DoStopService();
  // 设置接口
  bool SetServiceUnitsSink(IServiceUnitsSink* pIServiceUnitsSink);

public:
  // 列表结果
  void OnLoadDbListResult(BYTE cbSuccess);
  // 协调结果
  void OnCorrespondResult(BYTE cbSuccess);

  // 辅助函数
protected:
  bool SubscribeEvents();
  // 配置组件
  bool InitializeService();
  // 启动内核
  bool StartKernelService();
  // 启动网络
  bool StartNetworkService();

  // 内部函数
private:
  // 设置状态
  bool SetServiceStatus(enServiceStatus ServiceStatus);
  // 发送控制
  bool SendControlPacket(WORD control_id, VOID* data, WORD data_size);
};

extern CServiceUnits* service_units; // 对象指针
