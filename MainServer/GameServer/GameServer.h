#pragma once

#include "GameServerHead.h"
#include "ServiceUnits.h"

//////////////////////////////////////////////////////////////////////////////////

// 程序对象
class CGameServer : public IServiceUnitsSink {
protected:
  CServiceUnits m_ServiceUnits; // 服务单元
protected:
  CModuleDBParameter m_ModuleDBParameter; // 模块参数
protected:
  bool m_bAutoControl = false; // 自动控制
  bool m_bOptionSuccess = false; // 配置标志
  tagModuleInitParameter m_ModuleInitParameter{}; // 配置参数
  // 函数定义
public:
  // 构造函数
  explicit CGameServer() { m_ServiceUnits.SetServiceUnitsSink(this); }
  virtual ~CGameServer() = default;

  // 重载函数
public:
  virtual VOID OnServiceUnitsStatus(enServiceStatus ServiceStatus);
  // 启动函数
  VOID StartService();
  VOID ConcludeService();

public:
  bool OnQueryEndSession();
  bool LoadConfigByID(uint16_t game_server_id);
  bool GetPlatformDBParameter(tagDataBaseParameter& DataBaseParameter);
};

//////////////////////////////////////////////////////////////////////////////////
