#include "InitParameter.h"
#include "CorrespondHeader.h"

//////////////////////////////////////////////////////////////////////////////////
// 初始化
VOID CInitParameter::InitParameter() {
  // 系统配置
  max_connect_ = MAX_CONTENT;
  service_port_ = PORT_CENTER;

  // 配置信息
  ZeroMemory(server_name_, sizeof(server_name_));
}

// 加载配置
void CInitParameter::LoadInitParameter() {
  // 重置参数
  InitParameter();

  // 获取路径
  TCHAR work_dir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(work_dir, std::size(work_dir));

  // 构造路径
  const auto filename = std::filesystem::path(StringT(work_dir)) / TEXT("ServerParameter.ini");

  // 读取配置
  CWHIniData ini_data;
  ini_data.SetIniFilePath(filename);

  // 读取配置
  max_connect_ = ini_data.ReadInt(TEXT("Correspond"), TEXT("ConnectMax"), max_connect_);
  service_port_ = ini_data.ReadInt(TEXT("Correspond"), TEXT("ServicePort"), service_port_);
  ini_data.ReadEncryptString(TEXT("ServerInfo"), TEXT("ServiceName"), nullptr, server_name_, std::size(server_name_));
}

//////////////////////////////////////////////////////////////////////////////////
