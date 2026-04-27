#include "InitParameter.h"
#include "LogonServerHeader.h"

//////////////////////////////////////////////////////////////////////////////////
// 加载配置
void CInitParameter::LoadInitParameter() {
  // 获取路径
  TCHAR work_dir[MAX_PATH] = TEXT("");
  CWHService::GetWorkDirectory(work_dir, std::size(work_dir));

  // 构造路径
  const auto filename = std::filesystem::path(StringT(work_dir)) / TEXT("ServerParameter.ini");

  // 读取配置
  CWHIniData ini_data;
  ini_data.SetIniFilePath(filename);

  // 读取配置
  delay_list_ = ini_data.ReadInt(TEXT("LogonServer"), TEXT("DelayList"), delay_list_);
  max_connect_ = ini_data.ReadInt(TEXT("LogonServer"), TEXT("ConnectMax"), max_connect_);
  service_port_ = ini_data.ReadInt(TEXT("LogonServer"), TEXT("ServicePort"), service_port_);
  ini_data.ReadEncryptString(TEXT("ServerInfo"), TEXT("ServiceName"), NULL, server_name_, std::size(server_name_));
  ini_data.ReadEncryptString(TEXT("ServerInfo"), TEXT("ServiceAddr"), NULL, service_address_.szAddress, std::size(service_address_.szAddress));

  // 协调信息
  correspond_port_ = ini_data.ReadInt(TEXT("Correspond"), TEXT("ServicePort"), correspond_port_);
  ini_data.ReadEncryptString(TEXT("ServerInfo"), TEXT("CorrespondAddr"), NULL, correspond_address_.szAddress,
                            std::size(correspond_address_.szAddress));

  // 约战信息
  personal_port_ = ini_data.ReadInt(TEXT("Personal"), TEXT("ServicePort"), personal_port_);
  ini_data.ReadEncryptString(TEXT("ServerInfo"), TEXT("PersonalAddr"), correspond_address_.szAddress, personal_address_.szAddress,
                            std::size(personal_address_.szAddress));

  // 连接信息
  accounts_db_parameter_.wDataBasePort = (WORD)ini_data.ReadInt(TEXT("AccountsDB"), TEXT("DBPort"), 1433);
  ini_data.ReadEncryptString(TEXT("AccountsDB"), TEXT("DBAddr"), TEXT("localhost"), accounts_db_parameter_.szDataBaseAddr,
                            std::size(accounts_db_parameter_.szDataBaseAddr));
  ini_data.ReadEncryptString(TEXT("AccountsDB"), TEXT("DBUser"), TEXT("sa"), accounts_db_parameter_.szDataBaseUser,
                            std::size(accounts_db_parameter_.szDataBaseUser));
  ini_data.ReadEncryptString(TEXT("AccountsDB"), TEXT("DBPass"), NULL, accounts_db_parameter_.szDataBasePass,
                            std::size(accounts_db_parameter_.szDataBasePass));
  ini_data.ReadEncryptString(TEXT("AccountsDB"), TEXT("DBName"), szAccountsDB, accounts_db_parameter_.szDataBaseName,
                            std::size(accounts_db_parameter_.szDataBaseName));

  // 连接信息
  treasure_db_parameter_.wDataBasePort = (WORD)ini_data.ReadInt(TEXT("TreasureDB"), TEXT("DBPort"), 1433);
  ini_data.ReadEncryptString(TEXT("TreasureDB"), TEXT("DBAddr"), TEXT("localhost"), treasure_db_parameter_.szDataBaseAddr,
                            std::size(treasure_db_parameter_.szDataBaseAddr));
  ini_data.ReadEncryptString(TEXT("TreasureDB"), TEXT("DBUser"), TEXT("sa"), treasure_db_parameter_.szDataBaseUser,
                            std::size(treasure_db_parameter_.szDataBaseUser));
  ini_data.ReadEncryptString(TEXT("TreasureDB"), TEXT("DBPass"), NULL, treasure_db_parameter_.szDataBasePass,
                            std::size(treasure_db_parameter_.szDataBasePass));
  ini_data.ReadEncryptString(TEXT("TreasureDB"), TEXT("DBName"), szTreasureDB, treasure_db_parameter_.szDataBaseName,
                            std::size(treasure_db_parameter_.szDataBaseName));

  // 连接信息
  platform_db_parameter_.wDataBasePort = (WORD)ini_data.ReadInt(TEXT("PlatformDB"), TEXT("DBPort"), 1433);
  ini_data.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBAddr"), TEXT("localhost"), platform_db_parameter_.szDataBaseAddr,
                            std::size(platform_db_parameter_.szDataBaseAddr));
  ini_data.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBUser"), TEXT("sa"), platform_db_parameter_.szDataBaseUser,
                            std::size(platform_db_parameter_.szDataBaseUser));
  ini_data.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBPass"), NULL, platform_db_parameter_.szDataBasePass,
                            std::size(platform_db_parameter_.szDataBasePass));
  ini_data.ReadEncryptString(TEXT("PlatformDB"), TEXT("DBName"), szPlatformDB, platform_db_parameter_.szDataBaseName,
                            std::size(platform_db_parameter_.szDataBaseName));
}

//////////////////////////////////////////////////////////////////////////////////
