#include "InitParameter.h"

//////////////////////////////////////////////////////////////////////////////////
// 初始化
VOID CInitParameter::InitParameter() {
  // 系统配置
  m_wMaxConnect = MAX_CONTENT;
  m_wServicePort = PORT_CENTER;

  // 配置信息
  ZeroMemory(m_szServerName, sizeof(m_szServerName));
}

// 加载配置
VOID CInitParameter::LoadInitParameter() {
  // 重置参数
  InitParameter();

  // 获取路径
  TCHAR szWorkDir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(szWorkDir, CountArray(szWorkDir));

  // 构造路径
  const auto szIniFile = std::filesystem::path(StringT(szWorkDir)) / TEXT("ServerParameter.ini");

  // 读取配置
  CWHIniData IniData;
  IniData.SetIniFilePath(szIniFile);

  // 读取配置
  m_wMaxConnect = IniData.ReadInt(TEXT("Correspond"), TEXT("ConnectMax"), m_wMaxConnect);
  m_wServicePort = IniData.ReadInt(TEXT("Correspond"), TEXT("ServicePort"), m_wServicePort);
  IniData.ReadEncryptString(TEXT("ServerInfo"), TEXT("ServiceName"), NULL, m_szServerName, CountArray(m_szServerName));
}

//////////////////////////////////////////////////////////////////////////////////
