#include "GameServer.h"

//////////////////////////////////////////////////////////////////////////////////

// 服务状态
VOID CGameServer::OnServiceUnitsStatus(enServiceStatus ServiceStatus) {
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
VOID CGameServer::StartService() {
  // 启动服务
  try {
    m_ServiceUnits.StartService();
  } catch (...) {
    ASSERT(FALSE);
  }
}

// 停止服务
VOID CGameServer::ConcludeService() {
  // 停止服务
  try {
    m_ServiceUnits.ConcludeService();
  } catch (...) {
    ASSERT(FALSE);
  }
}

// 关闭询问
bool CGameServer::OnQueryEndSession() {
  // 提示消息
  if (m_ServiceUnits.GetServiceStatus() != ServiceStatus_Stop) {
    CLogger::Warn(TEXT("服务正在运行中，系统要求注销回话请求失败"));
    return FALSE;
  }

  return TRUE;
}

bool CGameServer::LoadConfigByID(uint16_t game_server_id) {
  // 变量定义
  tagDataBaseParameter DataBaseParameter;
  ZeroMemory(&DataBaseParameter, sizeof(DataBaseParameter));

  // 设置参数
  GetPlatformDBParameter(DataBaseParameter);
  m_ModuleDBParameter.SetPlatformDBParameter(DataBaseParameter);

  // 读取配置
  CServerItemManager DlgServerItem;
  if (DlgServerItem.OpenGameServer(game_server_id) == false) {
    CLogger::Error(TEXT("房间配置参数不存在或者加载失败"));
    return false;
  }
  m_ModuleInitParameter = DlgServerItem.m_ModuleInitParameter;

  // 设置模块
  LPCTSTR pszServerDLLName = m_ModuleInitParameter.GameServiceAttrib.szServerDLLName;
  m_ServiceUnits.CollocateService(pszServerDLLName, m_ModuleInitParameter.GameServiceOption);

  CLogger::Info(TEXT("[ {} ] 房间参数加载成功"), m_ModuleInitParameter.GameServiceOption.szServerName);

  return true;
}

// 获取连接
bool CGameServer::GetPlatformDBParameter(tagDataBaseParameter& DataBaseParameter) {
  // 获取路径
  TCHAR szWorkDir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(szWorkDir, CountArray(szWorkDir));

  // 构造路径
  const auto szIniFile = std::filesystem::path(StringT(szWorkDir)) / TEXT("ServerParameter.ini");

  // 读取配置
  CWHIniData IniData;
  IniData.SetIniFilePath(szIniFile);

  // 连接信息
  DataBaseParameter.wDataBasePort = (WORD) IniData.ReadInt(TEXT("PlatformDB"), TEXT("DBPort"), 1433);
  IniData.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBAddr"), NULL, DataBaseParameter.szDataBaseAddr, CountArray(DataBaseParameter.szDataBaseAddr));
  IniData.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBUser"), NULL, DataBaseParameter.szDataBaseUser, CountArray(DataBaseParameter.szDataBaseUser));
  IniData.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBPass"), NULL, DataBaseParameter.szDataBasePass, CountArray(DataBaseParameter.szDataBasePass));
  IniData.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBName"), szPlatformDB, DataBaseParameter.szDataBaseName,
                            CountArray(DataBaseParameter.szDataBaseName));

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
