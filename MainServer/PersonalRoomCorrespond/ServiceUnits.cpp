#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CServiceUnits::CServiceUnits() {
  // 设置接口
  m_pIServiceUnitsSink = NULL;

  // 设置变量
  m_ServiceStatus = ServiceStatus_Stop;

  return;
}

// 析构函数
CServiceUnits::~CServiceUnits() {
  ConcludeService();
}

// 启动服务
bool CServiceUnits::StartService() {
  // 效验状态
  ASSERT(m_ServiceStatus == ServiceStatus_Stop);
  if (m_ServiceStatus != ServiceStatus_Stop) {
    return false;
  }

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
      CLogger::Error("Thread {:#x} crashed: {}", thread_id, ToSimpleUtf8(ex.what()));
    }
    CLogger::Warn("Thread {:#x} exited!!!", thread_id);
  });
  // 配置服务
  if (InitializeService() == false) {
    ConcludeService();
    return false;
  }

  // 启动内核
  if (StartKernelService() == false) {
    ConcludeService();
    return false;
  }

  // 设置状态
  SetServiceStatus(ServiceStatus_Service);

  return true;
}

// 停止服务
bool CServiceUnits::ConcludeService() {
  // 设置变量
  SetServiceStatus(ServiceStatus_Stop);

  // 停止服务
  if (m_TimerEngine.GetInterface() != NULL) {
    m_TimerEngine->ConcludeService();
  }
  if (m_AttemperEngine.GetInterface() != NULL) {
    m_AttemperEngine->ConcludeService();
  }
  if (m_TCPNetworkEngine.GetInterface() != NULL) {
    m_TCPNetworkEngine->ConcludeService();
  }
  // 任务队列为空时 run() 可退出
  work_guard_->reset();
  io_context_->stop();
  ConcludeThreadPool();

  return true;
}

// 设置接口
bool CServiceUnits::SetServiceUnitsSink(IServiceUnitsSink* pIServiceUnitsSink) {
  // 设置变量
  m_pIServiceUnitsSink = pIServiceUnitsSink;

  return true;
}

// 配置组件
bool CServiceUnits::InitializeService() {
  // 加载参数
  m_InitParameter.LoadInitParameter();

  // 创建组件
  if ((m_TimerEngine.GetInterface() == NULL) && (m_TimerEngine.CreateInstance() == false)) {
    return false;
  }
  if ((m_AttemperEngine.GetInterface() == NULL) && (m_AttemperEngine.CreateInstance() == false)) {
    return false;
  }
  if ((m_TCPNetworkEngine.GetInterface() == NULL) && (m_TCPNetworkEngine.CreateInstance() == false)) {
    return false;
  }

  // 内核组件
  if (m_TimerEngine->SetTimerEngineEvent(&m_AttemperEngineSink) == false) {
    return false;
  }
  if (m_AttemperEngine->SetControlEngineEvent(&m_AttemperEngineSink) == false) {
    return false;
  }
  if (m_AttemperEngine->SetAttemperEngineEvent(&m_AttemperEngineSink) == false) {
    return false;
  }
  if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(&m_AttemperEngineSink) == false) {
    return false;
  }

  // 调度回调
  m_AttemperEngineSink.m_pInitParameter = &m_InitParameter;
  m_AttemperEngineSink.m_pITimerEngine = m_TimerEngine.GetInterface();
  m_AttemperEngineSink.m_pITCPNetworkEngine = m_TCPNetworkEngine.GetInterface();

  // 配置网络
  WORD wMaxConnect = m_InitParameter.m_wMaxConnect;
  WORD wServicePort = m_InitParameter.m_wPrsnlServicePort;
  if (m_TCPNetworkEngine->SetServiceParameter(wServicePort, wMaxConnect, szCompilation) == false) {
    return false;
  }

  return true;
}

// 启动内核
bool CServiceUnits::StartKernelService() {
  // 时间引擎
  if (m_TimerEngine->StartService(io_context_) == false) {
    ASSERT(FALSE);
    return false;
  }

  // 调度引擎
  if (m_AttemperEngine->StartService(io_context_) == false) {
    ASSERT(FALSE);
    return false;
  }

  // 网络引擎
  if (m_TCPNetworkEngine->StartService(io_context_) == false) {
    ASSERT(FALSE);
    return false;
  }

  return true;
}

// 设置状态
bool CServiceUnits::SetServiceStatus(enServiceStatus ServiceStatus) {
  // 状态判断
  if (m_ServiceStatus != ServiceStatus) {
    // 错误通知
    if ((m_ServiceStatus != ServiceStatus_Service) && (ServiceStatus == ServiceStatus_Stop)) {
      CLogger::Error(TEXT("服务启动失败"));
    }

    // 设置变量
    m_ServiceStatus = ServiceStatus;

    // 状态通知
    ASSERT(m_pIServiceUnitsSink != NULL);
    if (m_pIServiceUnitsSink != NULL) {
      m_pIServiceUnitsSink->OnServiceUnitsStatus(m_ServiceStatus);
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
