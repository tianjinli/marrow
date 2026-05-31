#include "ServiceUnits.h"
#include "ControlPacket.h"

//////////////////////////////////////////////////////////////////////////////////

// 静态变量
CServiceUnits* CServiceUnits::g_pServiceUnits = NULL; // 对象指针

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CServiceUnits::CServiceUnits() {
  // 设置对象
  ASSERT(g_pServiceUnits == NULL);
  if (g_pServiceUnits == NULL) {
    g_pServiceUnits = this;
  }
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

  // 启动线程池
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
  // 创建模块
  if (CreateServiceDLL() == false) {
    ConcludeService();
    return false;
  }

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

  // 加载配置
  SendControlPacket(CT_LOAD_SERVICE_CONFIG, NULL, 0);

  return true;
}

// 停止服务
bool CServiceUnits::ConcludeService() {
  // 设置变量
  SetServiceStatus(ServiceStatus_Stop);

  // 内核组件
  if (m_TimerEngine.GetInterface() != NULL) {
    m_TimerEngine->ConcludeService();
  }
  if (m_AttemperEngine.GetInterface() != NULL) {
    m_AttemperEngine->ConcludeService();
  }
  if (m_DataBaseEngine.GetInterface() != NULL) {
    m_DataBaseEngine->ConcludeService();
  }
  if (m_TCPSocketService.GetInterface() != NULL) {
    m_TCPSocketService->ConcludeService();
  }
  if (m_TCPNetworkEngine.GetInterface() != NULL) {
    m_TCPNetworkEngine->ConcludeService();
  }

  // 任务队列为空时 run() 可退出
  work_guard_->reset();
  ConcludeThreadPool();
  io_context_->stop();

  return true;
}

// 设置接口
bool CServiceUnits::SetServiceUnitsSink(IServiceUnitsSink* pIServiceUnitsSink) {
  // 设置变量
  m_pIServiceUnitsSink = pIServiceUnitsSink;

  return true;
}

// 投递请求
bool CServiceUnits::PostControlRequest(WORD wIdentifier, VOID* pData, WORD wDataSize) {
  // ⚡直接处理消息
  switch (wIdentifier) {
    case UI_CORRESPOND_RESULT: // 协调成功
    {
      // 效验消息
      ASSERT(wDataSize == sizeof(CP_ControlResult));
      if (wDataSize != sizeof(CP_ControlResult)) {
        return false;
      }

      // 变量定义
      CP_ControlResult* pControlResult = (CP_ControlResult*) pData;
      if (m_ServiceStatus != ServiceStatus_Service) {
        if (pControlResult->cbSuccess == ER_SUCCESS) {
          // 连接协调
          SendControlPacket(CT_CONNECT_CORRESPOND, NULL, 0);
        } else {
          // 失败处理
          ConcludeService();
        }
      }
    } break;
    case UI_SERVICE_CONFIG_RESULT: // 配置结果
    {
      // 效验消息
      ASSERT(wDataSize == sizeof(CP_ControlResult));
      if (wDataSize != sizeof(CP_ControlResult)) {
        return false;
      }

      // 变量定义
      CP_ControlResult* pControlResult = (CP_ControlResult*) pData;
      if (m_ServiceStatus != ServiceStatus_Service) {
        if (pControlResult->cbSuccess == ER_FAILURE) {
          // 失败处理
          ConcludeService();
        } else if (pControlResult->cbSuccess == ER_SUCCESS) {
          // 启动网络
          if (StartNetworkService() == false) {
            ConcludeService();
          } else {
            // 连接协调
            SendControlPacket(CT_CONNECT_CORRESPOND, NULL, 0);
          }
        }
      }

      return 0;
    }
  }
  return true;
}

// 创建模块
bool CServiceUnits::CreateServiceDLL() {
  // 时间引擎
  if ((m_TimerEngine.GetInterface() == NULL) && (m_TimerEngine.CreateInstance() == false)) {
    CTraceService::TraceString(m_TimerEngine.GetErrorDescribe(), TraceLevel_Exception);
    return false;
  }

  // 调度引擎
  // if ((m_AttemperEngine.GetInterface() == NULL) && (m_AttemperEngine.CreateInstance() == false)) {
  //   CTraceService::TraceString(m_AttemperEngine.GetErrorDescribe(), TraceLevel_Exception);
  //   return false;
  // }

  // 网络组件
  if ((m_TCPSocketService.GetInterface() == NULL) && (m_TCPSocketService.CreateInstance() == false)) {
    CTraceService::TraceString(m_TCPSocketService.GetErrorDescribe(), TraceLevel_Exception);
    return false;
  }

  // 网络引擎
  if ((m_TCPNetworkEngine.GetInterface() == NULL) && (m_TCPNetworkEngine.CreateInstance() == false)) {
    CTraceService::TraceString(m_TCPNetworkEngine.GetErrorDescribe(), TraceLevel_Exception);
    return false;
  }

  // 数据组件
  if ((m_DataBaseEngine.GetInterface() == NULL) && (m_DataBaseEngine.CreateInstance() == false)) {
    CTraceService::TraceString(m_DataBaseEngine.GetErrorDescribe(), TraceLevel_Exception);
    return false;
  }

  return true;
}

// 配置组件
bool CServiceUnits::InitializeService() {
  // 加载参数
  m_InitParameter.LoadInitParameter();

  // // 组件接口
  // IUnknownEx* pIAttemperEngine = m_AttemperEngine.GetInterface();
  // IUnknownEx* pITCPNetworkEngine = m_TCPNetworkEngine.GetInterface();
  // IUnknownEx* pIAttemperEngineSink = QUERY_OBJECT_INTERFACE(m_AttemperEngineSink, IUnknownEx);

  // // 数据引擎
  // IUnknownEx* pIDataBaseEngineSink;
  // pIDataBaseEngineSink = QUERY_OBJECT_INTERFACE(m_DataBaseEngineSink, IUnknownEx);

  // // 内核组件
  // if (m_TimerEngine->SetTimerEngineEvent(pIAttemperEngine) == false) {
  //   return false;
  // }
  // if (m_AttemperEngine->SetNetworkEngine(pITCPNetworkEngine) == false) {
  //   return false;
  // }
  // if (m_AttemperEngine->SetAttemperEngineSink(pIAttemperEngineSink) == false) {
  //   return false;
  // }
  // if (m_TCPNetworkEngine->SetTCPNetworkEngineEvent(pIAttemperEngine) == false) {
  //   return false;
  // }
  // if (m_DataBaseEngine->SetDataBaseEngineSink(pIDataBaseEngineSink) == false) {
  //   return false;
  // }

  // 协调服务
  if (m_TCPSocketService->SetServiceID(NETWORK_CORRESPOND) == false) {
    return false;
  }
  // if (m_TCPSocketService->SetTCPSocketEvent(pIAttemperEngine) == false) {
  //   return false;
  // }

  // 调度回调
  m_AttemperEngineSink.m_pInitParameter = &m_InitParameter;
  m_AttemperEngineSink.m_pITimerEngine = m_TimerEngine.GetInterface();
  // m_AttemperEngineSink.m_pIAttemperEngine = m_AttemperEngine.GetInterface();
  m_AttemperEngineSink.m_pITCPSocketService = m_TCPSocketService.GetInterface();
  m_AttemperEngineSink.m_pITCPNetworkEngine = m_TCPNetworkEngine.GetInterface();
  m_AttemperEngineSink.m_pIDataBaseEngine = m_DataBaseEngine.GetInterface();

  // 数据库回调
  m_DataBaseEngineSink.m_pInitParameter = &m_InitParameter;
  // m_DataBaseEngineSink.m_pIDataBaseEngineEvent = QUERY_OBJECT_PTR_INTERFACE(pIAttemperEngine, IDataBaseEngineEvent);

  // 配置网络
  m_TCPNetworkEngine->SetServiceParameter(m_InitParameter.m_wServicePort, m_InitParameter.m_wMaxPlayer, szCompilation);

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
  // if (m_AttemperEngine->StartService(io_context_) == false) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 数据引擎
  if (m_DataBaseEngine->StartService(io_context_) == false) {
    ASSERT(FALSE);
    return false;
  }

  // 协调引擎
  if (m_TCPSocketService->StartService(io_context_) == false) {
    ASSERT(FALSE);
    return false;
  }

  return true;
}

// 启动网络
bool CServiceUnits::StartNetworkService() {
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

// 发送控制
bool CServiceUnits::SendControlPacket(WORD wControlID, VOID* pData, WORD wDataSize) {
  // 状态效验
  ASSERT(m_AttemperEngine.GetInterface() != NULL);
  if (m_AttemperEngine.GetInterface() == NULL) {
    return false;
  }

  // 发送控制
  m_AttemperEngine->OnEventControl(wControlID, pData, wDataSize);
  return true;
}

//////////////////////////////////////////////////////////////////////////////////
