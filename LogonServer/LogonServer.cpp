#include "LogonServer.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CLogonServer::CLogonServer() {
  m_ServiceUnits.SetServiceUnitsSink(this);
}

// 析构函数
CLogonServer::~CLogonServer() {
}

// 服务状态
VOID CLogonServer::OnServiceUnitsStatus(enServiceStatus ServiceStatus) {
  // 状态设置
  switch (ServiceStatus) {
    case ServiceStatus_Stop: // 停止状态
    {
      // 提示信息
      CLogger::Info(TEXT("服务停止成功"));

      break;
    }
    case ServiceStatus_Config: // 配置状态
    {
      // 提示信息
      CLogger::Info(TEXT("正在初始化组件..."));

      break;
    }
    case ServiceStatus_Service: // 服务状态
    {
      // 提示信息
      CLogger::Info(TEXT("服务启动成功"));

      break;
    }
  }
}

// 启动服务
VOID CLogonServer::DoStartService() {
  // 启动服务
  try {
    m_ServiceUnits.DoStartService();
  } catch (...) {
    ASSERT(FALSE);
  }
}

// 停止服务
VOID CLogonServer::DoStopService() {
  // 停止服务
  try {
    m_ServiceUnits.DoStopService();
  } catch (...) {
    ASSERT(FALSE);
  }
}

// 关闭询问
bool CLogonServer::OnQueryEndSession() {
  // 提示消息
  if (m_ServiceUnits.GetServiceStatus() != ServiceStatus_Stop) {
    CLogger::Warn(TEXT("服务正在运行中，系统要求注销回话请求失败"));
    return FALSE;
  }

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////////////
