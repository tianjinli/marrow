#include "ServiceUnits.h"
#include "ControlPacket.h"
#include "KernelEngine/AsyncEventHub.h"

//////////////////////////////////////////////////////////////////////////////////

// 静态变量
extern CServiceUnits* service_units = nullptr; // 对象指针

//////////////////////////////////////////////////////////////////////////////////
// 构造函数
CServiceUnits::CServiceUnits() {
  // 设置对象
  ASSERT(service_units == nullptr);
  if (service_units == nullptr)
    service_units = this;
}

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

  GlobalEventBus::Init(*io_context_);
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

  // 获取列表
  SendControlPacket(CT_LOAD_DB_GAME_LIST, nullptr, 0);

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
  if (correspond_service_.GetInterface() != nullptr)
    correspond_service_->ConcludeService();
  if (personal_service_.GetInterface() != nullptr)
    personal_service_->ConcludeService();

  // 等待数据库任务结束
  while (active_requests_.load() > 0) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
  }
  if (database_engine_.GetInterface() != nullptr)
    database_engine_->ConcludeService();

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

  // // 创建组件
  // if ((timer_engine_.GetInterface() == nullptr) && (!timer_engine_.CreateInstance()))
  //   return false;
  // // if ((attemper_engine_.GetInterface() == nullptr) && (!attemper_engine_.CreateInstance()))
  // //   return false;
  // if ((database_engine_.GetInterface() == nullptr) && (!database_engine_.CreateInstance()))
  //   return false;
  // if ((network_engine_.GetInterface() == nullptr) && (!network_engine_.CreateInstance()))
  //   return false;
  // if ((correspond_service_.GetInterface() == nullptr) && (!correspond_service_.CreateInstance()))
  //   return false;
  // if ((personal_service_.GetInterface() == nullptr) && (!personal_service_.CreateInstance()))
  //   return false;
  //
  // // 组件接口
  // // IUnknownEx* attemper_engine = attemper_engine_.GetInterface();
  // IUnknownEx* network_engine = network_engine_.GetInterface();
  // IUnknownEx* attemper_engine_sink = QUERY_OBJECT_INTERFACE(attemper_engine_sink_, IUnknownEx);
  //
  // // 数据引擎
  // const size_t kDataBaseEngineSinkLen = sizeof(database_engine_sinks_) / sizeof(database_engine_sinks_[0]);
  // IUnknownEx* database_engine_sinks[kDataBaseEngineSinkLen];
  // for (WORD i = 0; i < kDataBaseEngineSinkLen; i++)
  //   database_engine_sinks[i] = QUERY_OBJECT_INTERFACE(m_DataBaseEngineSink[i], IUnknownEx);
  //
  // // 内核组件
  // if (!timer_engine_->SetTimerEngineEvent(attemper_engine))
  //   return false;
  // if (!attemper_engine_->SetNetworkEngine(network_engine))
  //   return false;
  // if (!attemper_engine_->SetAttemperEngineSink(attemper_engine_sink))
  //   return false;
  // if (!network_engine_->SetTCPNetworkEngineEvent(attemper_engine))
  //   return false;
  // if (!database_engine_->SetDataBaseEngineSink(database_engine_sinks_, std::size(database_engine_sinks_)))
  //   return false;
  //
  // // 协调服务
  // if (!correspond_service_->SetServiceID(NETWORK_CORRESPOND))
  //   return false;
  // if (!correspond_service_->SetTCPSocketEvent(attemper_engine))
  //   return false;
  //
  // // 约战服务
  // if (!personal_service_->SetServiceID(NETWORK_PERSONAL))
  //   return false;
  // if (!personal_service_->SetTCPSocketEvent(attemper_engine))
  //   return false;
  //
  // // 调度回调
  // attemper_engine_sink_.init_parameter_ = &init_parameter_;
  // attemper_engine_sink_.timer_engine_ = timer_engine_.GetInterface();
  // attemper_engine_sink_.database_engine_ = database_engine_.GetInterface();
  // attemper_engine_sink_.network_engine_ = network_engine_.GetInterface();
  // attemper_engine_sink_.correspond_service_ = correspond_service_.GetInterface();
  // attemper_engine_sink_.personal_service_ = personal_service_.GetInterface();
  //
  // // 数据库回调
  // for (INT i = 0; i < std::size(database_engine_sinks_); i++) {
  //   database_engine_sinks_[i].init_parameter_ = &init_parameter_;
  //   database_engine_sinks_[i].database_engine_event_ = QUERY_OBJECT_INTERFACE(attemper_engine, IDataBaseEngineEvent);
  // }

  // 创建组件
  if ((timer_engine_.GetInterface() == nullptr) && (!timer_engine_.CreateInstance()))
    return false;
  if ((database_engine_.GetInterface() == nullptr) && (!database_engine_.CreateInstance()))
    return false;
  if ((network_engine_.GetInterface() == nullptr) && (!network_engine_.CreateInstance()))
    return false;
  if ((correspond_service_.GetInterface() == nullptr) && (!correspond_service_.CreateInstance()))
    return false;
  if ((personal_service_.GetInterface() == nullptr) && (!personal_service_.CreateInstance()))
    return false;

  // 协调服务
  if (!correspond_service_->SetServiceID(NETWORK_CORRESPOND))
    return false;
  // 约战服务
  if (!personal_service_->SetServiceID(NETWORK_PERSONAL))
    return false;

  // 调度回调
  dispatch_engine_sink_.init_parameter_ = &init_parameter_;
  dispatch_engine_sink_.timer_engine_ = timer_engine_.GetInterface();
  dispatch_engine_sink_.database_engine_ = database_engine_.GetInterface();
  dispatch_engine_sink_.network_engine_ = network_engine_.GetInterface();
  dispatch_engine_sink_.correspond_service_ = correspond_service_.GetInterface();
  dispatch_engine_sink_.personal_service_ = personal_service_.GetInterface();

  database_engine_sink_.init_parameter_ = &init_parameter_;

  // 配置网络
  WORD wMaxConnect = init_parameter_.max_connect_;
  WORD wServicePort = init_parameter_.service_port_;
  if (!network_engine_->SetServiceParameter(wServicePort, wMaxConnect, szCompilation))
    return false;

  return true;
}

// 启动内核
bool CServiceUnits::StartKernelService() {
  // 时间引擎
  if (!timer_engine_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  //// 调度引擎
  // if (!attemper_engine_->InitiateService(io_context_)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 数据引擎
  if (!database_engine_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  // 协调引擎
  if (!correspond_service_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  // 约战引擎
  if (!personal_service_->InitiateService(io_context_)) {
    ASSERT(FALSE);
    return false;
  }

  // 模拟异步开始事件
  dispatch_engine_sink_.OnAttemperEngineStart(nullptr);

  return true;
}

// 启动网络
bool CServiceUnits::StartNetworkService() {
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

// 发送控制
bool CServiceUnits::SendControlPacket(WORD control_id, VOID* data, WORD data_size) {
  // // 状态效验
  // ASSERT(attemper_engine_.GetInterface() != nullptr);
  // if (attemper_engine_.GetInterface() == nullptr)
  //   return false;

  // // 发送控制
  // attemper_engine_->OnEventControl(wControlID, pData, wDataSize);
  auto post_data = ConvertToBytes(data, data_size);
  GlobalEventBus::Get()->Publish<AsyncControlEventTag>(control_id, std::move(post_data));
  return true;
}

void CServiceUnits::OnLoadDbListResult(BYTE cbSuccess) {
  if (service_status_ != ServiceStatus_Service) {
    if (cbSuccess == ER_FAILURE) { // 失败处理
      DoStopService();
    } else if (cbSuccess == ER_SUCCESS) { // 成功处理
      // 连接协调
      SendControlPacket(CT_CONNECT_CORRESPOND, nullptr, 0);

      // 连接协调
      SendControlPacket(CT_CONNECT_PERSONAL_ROOM_CORRESPOND, nullptr, 0);
    }
  }
}

void CServiceUnits::OnCorrespondResult(BYTE cbSuccess) {
  if (service_status_ != ServiceStatus_Service) {
    if (cbSuccess == ER_FAILURE) { // 失败处理
      DoStopService();
    } else if (cbSuccess == ER_SUCCESS) { // 成功处理
      // 启动网络
      if (!StartNetworkService()) {
        DoStopService();
        return;
      }

      // 设置状态
      SetServiceStatus(ServiceStatus_Service);
    }
  }
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
  auto database_result_sink = QUERY_OBJECT_INTERFACE(dispatch_engine_sink_, IDataBaseEngineEvent);
  GlobalEventBus::Get()->Subscribe<DataBaseResultEventTag>([database_result_sink, this](WORD request_id, DWORD context_id, nf::BufferPtr data) {
    return database_result_sink->OnEventDataBase(request_id, context_id, data->data(), data->size());
  });

  auto socket_service_sink = QUERY_OBJECT_INTERFACE(dispatch_engine_sink_, ITCPSocketEvent);
  GlobalEventBus::Get()->Subscribe<TCPSocketLinkEventTag>([socket_service_sink](WORD service_id, INT error_code) {
    return socket_service_sink->OnEventTCPSocketLink(service_id, error_code);
  });
  GlobalEventBus::Get()->Subscribe<TCPSocketShutEventTag>([socket_service_sink](WORD service_id, INT error_code) {
    return socket_service_sink->OnEventTCPSocketShut(service_id, error_code);
  });
  GlobalEventBus::Get()->Subscribe<TCPSocketReadEventTag>([socket_service_sink, this](WORD service_id, TCP_Command tcp_cmd, nf::BufferPtr body) {
    return socket_service_sink->OnEventTCPSocketRead(service_id, tcp_cmd, body->data(), body->size());
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

  auto database_request_sink = QUERY_OBJECT_INTERFACE(database_engine_sink_, IDataBaseEngineSink);
  GlobalEventBus::Get()->Subscribe<DataBaseRequestEventTag>([database_request_sink, this](WORD request_id, DWORD context_id, nf::BufferPtr data) {
    if (service_status_ == ServiceStatus_Stop)
      return false;

    active_requests_.fetch_add(1);
    auto ok = database_request_sink->OnDataBaseEngineRequest(request_id, context_id, data->data(), data->size());
    active_requests_.fetch_sub(1);
    return ok;
  });
  GlobalEventBus::Get()->Subscribe<DataBaseStartEventTag>([database_request_sink, this](IUnknownEx* unused) {
    active_requests_ = 0;
    return database_request_sink->OnDataBaseEngineStart(unused);
  });
  GlobalEventBus::Get()->Subscribe<DataBaseStopEventTag>([database_request_sink, this](IUnknownEx* unused) {
    return database_request_sink->OnDataBaseEngineConclude(unused);
  });
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
