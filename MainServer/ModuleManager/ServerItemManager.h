#pragma once

#include "ModuleManagerHead.h"
#include "ServerInfoManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 主对话框
class MODULE_MANAGER_CLASS CServerItemManager {
  // 列表变量
protected:
  CServerInfoBuffer m_ServerInfoBuffer; // 模块信息

  // 配置变量
public:
  tagModuleInitParameter m_ModuleInitParameter{}; // 配置参数

  // 函数定义
public:
  // 构造函数
  CServerItemManager() = default;
  // 析构函数
  virtual ~CServerItemManager() = default;

  // 配置函数
public:
  // 打开房间
  bool OpenGameServer(WORD wServerID);

  // 功能函数
protected:
  // 获取参数
  bool GetModuleInitParameter(tagGameServerInfo* pGameServerInfo, bool bAutoMode);
};

//////////////////////////////////////////////////////////////////////////////////
