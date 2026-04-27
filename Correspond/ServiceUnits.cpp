#include "ServiceUnits.h"
#include "CorrespondHeader.h"
#include "KernelEngine/AsyncEventHub.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CServiceUnits::~CServiceUnits() {
  DoStopService();
}

// 启动服务
bool CServiceUnits::DoStartService() {
  // 效验状态
  ASSERT(service_status_ == ServiceStatus_Stop);
  if (service_status_ != ServiceStatus_Stop)
    return false;

  // 设置状态
  SetServiceStatus(ServiceStatus_Config);

  work_guard_.emplace(io_context_->get_executor());
  if (io_context_->stopped()) {
    io_context_->restart();
  }
  StartThreadPool([this] {
    static std::hash<std::thread::id> hash;
    auto thread_id = hash(std::this_thread::get_id());
    try {
      io_context_->run();
    } catch (const std::exception& ex) {
      CLogger::Error(FMT_STRING("Thread {:#x} crashed: {}"), thread_id, ToSimpleUtf8(ex.what()));
    }
    CLogger::Warn(FMT_STRING("Thread {:#x} exited!!!"), thread_id);
  });

  // 订阅事件
  GlobalEventBus::Init(*io_context_);
  if (!SubscribeEvents()) {
    DoStopService();
    return false;
  }

  // 配置服务
  if (!InitializeService()) {
    DoStopService();
    return false;
  }

  // 启动内核
  if (!StartKernelService()) {
    DoStopService();
    return false;
  }

  // 模拟异步开始事件
  dispatch_engine_sink_.OnAttemperEngineStart(nullptr);

  // 设置状态
  SetServiceStatus(ServiceStatus_Service);

  return true;
}

// 停止服务
bool CServiceUnits::DoStopService() {
  // 停止服务
  if (timer_engine_.GetInterface() != nullptr)
    timer_engine_->ConcludeService();
  // if (attemper_engine_.GetInterface() != nullptr)
  //   attemper_engine_->ConcludeService();
  if (network_engine_.GetInterface() != nullptr)
    network_engine_->ConcludeService();

  // 任务队列为空时 run() 可退出
  work_guard_->reset();
  ConcludeThreadPool();
  GlobalEventBus::Shutdown();
  io_context_->stop();

  // 模拟异步停止事件
  dispatch_engine_sink_.OnAttemperEngineConclude(nullptr);

  // 设置变量
  SetServiceStatus(ServiceStatus_Stop);
  return true;
}

// 设置接口
bool CServiceUnits::SetServiceUnitsSink(IServiceUnitsSink* pIServiceUnitsSink) {
  // 设置变量
  service_units_sink_ = pIServiceUnitsSink;

  return true;
}

// 配置组件
bool CServiceUnits::InitializeService() {
  // 加载参数
  init_parameter_.LoadInitParameter();

  // 创建组件
  if ((timer_engine_.GetInterface() == nullptr) && !timer_engine_.CreateInstance())
    return false;
  // if ((attemper_engine_.GetInterface() == nullptr) && !attemper_engine_.CreateInstance())
  //   return false;
  if ((network_engine_.GetInterface() == nullptr) && !network_engine_.CreateInstance())
    return false;

  // // 组件接口
  // IUnknownEx* attemper_engine = attemper_engine_.GetInterface();
  // IUnknownEx* network_engine = network_engine_.GetInterface();
  //
  // // 回调接口
  // IUnknownEx* attemper_engine_sink = QUERY_OBJECT_INTERFACE(attemper_engine_sink_, IUnknownEx);
  //
  // // 绑定接口
  // if (!attemper_engine_->SetAttemperEngineSink(attemper_engine_sink))
  //   return false;
  //
  // // 内核组件
  // if (!timer_engine_->SetTimerEngineEvent(attemper_engine))
  //   return false;
  // if (!attemper_engine_->SetNetworkEngine(network_engine))
  //   return false;
  // if (!network_engine_->SetTCPNetworkEngineEvent(attemper_engine))
  //   return false;
  //
  // // 调度回调
  // attemper_engine_sink_.init_parameter_ = &init_parameter_;
  // attemper_engine_sink_.timer_engine_ = timer_engine_.GetInterface();
  // attemper_engine_sink_.network_engine_ = network_engine_.GetInterface();

  // 调度回调
  dispatch_engine_sink_.init_parameter_ = &init_parameter_;
  dispatch_engine_sink_.timer_engine_ = timer_engine_.GetInterface();
  dispatch_engine_sink_.network_engine_ = network_engine_.GetInterface();

  // 配置网络
  WORD wMaxConnect = init_parameter_.max_connect_;
  WORD wServicePort = init_parameter_.service_port_;
  return network_engine_->SetServiceParameter(wServicePort, wMaxConnect, szCompilation);
}

// 启动内核
bool CServiceUnits::StartKernelService() {
  // 时间引擎
  if (!timer_engine_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  // // 调度引擎
  // if (!attemper_engine_->InitiateService(io_context_)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 网络引擎
  if (!network_engine_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  return true;
}

// 设置状态
bool CServiceUnits::SetServiceStatus(enServiceStatus ServiceStatus) {
  // 状态判断
  if (service_status_ != ServiceStatus) {
    // 错误通知
    if ((service_status_ != ServiceStatus_Service) && (ServiceStatus == ServiceStatus_Stop)) {
      CLogger::Error(TEXT("服务启动失败"));
    }

    // 设置变量
    service_status_ = ServiceStatus;

    // 状态通知
    ASSERT(service_units_sink_ != nullptr);
    if (service_units_sink_ != nullptr)
      service_units_sink_->OnServiceUnitsStatus(service_status_);
  }

  return true;
}

bool CServiceUnits::SubscribeEvents() {
  auto timer_engine_sink = QUERY_OBJECT_INTERFACE(dispatch_engine_sink_, ITimerEngineEvent);
  GlobalEventBus::Get()->Subscribe<AsyncTimerEventTag>([timer_engine_sink](DWORD timer_id, WPARAM bind_parameter) {
    return timer_engine_sink->OnEventTimer(timer_id, bind_parameter);
  });

  auto control_engine_sink = QUERY_OBJECT_INTERFACE(dispatch_engine_sink_, IControlEngineEvent);
  GlobalEventBus::Get()->Subscribe<AsyncControlEventTag>([control_engine_sink](WORD control_id, nf::BufferPtr data) {
    return control_engine_sink->OnEventControl(control_id, data->data(), data->size());
  });

  auto network_engine_sink = QUERY_OBJECT_INTERFACE(dispatch_engine_sink_, ITCPNetworkEngineEvent);
  GlobalEventBus::Get()->Subscribe<TCPNetworkBindEventTag>([network_engine_sink](DWORD socket_id, DWORD client_addr) {
    return network_engine_sink->OnEventTCPNetworkBind(socket_id, client_addr);
  });
  GlobalEventBus::Get()->Subscribe<TCPNetworkShutEventTag>([network_engine_sink, this](DWORD socket_id, DWORD client_addr, DWORD active_time) {
    return network_engine_sink->OnEventTCPNetworkShut(socket_id, client_addr, active_time);
  });
  GlobalEventBus::Get()->Subscribe<TCPNetworkReadEventTag>([network_engine_sink](DWORD socket_id, TCP_Command tcp_cmd, nf::BufferPtr body) {
    return network_engine_sink->OnEventTCPNetworkRead(socket_id, tcp_cmd, body->data(), body->size());
  });
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
