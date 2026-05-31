#include "ServerItemManager.h"

// 打开房间
bool CServerItemManager::OpenGameServer(WORD wServerID) {
  // 变量定义
  tagGameServerInfo GameServerResult;
  ZeroMemory(&GameServerResult, sizeof(GameServerResult));

  // 机器标识
  TCHAR szMachineID[LEN_MACHINE_ID];
  CWHService::GetMachineID(szMachineID);

  // 加载房间
  CServerInfoManager ServerInfoManager;
  if (ServerInfoManager.LoadGameServerInfo(szMachineID, wServerID, GameServerResult) == false) {
    return false;
  }

  // 获取参数
  if (GetModuleInitParameter(&GameServerResult, true) == false) {
    return false;
  }

  return true;
}

// 获取参数
bool CServerItemManager::GetModuleInitParameter(tagGameServerInfo* pGameServerInfo, bool bAutoMode) {
  // 效验参数
  ASSERT(pGameServerInfo != NULL);
  if (pGameServerInfo == NULL) {
    return false;
  }

  // 游戏模块
  CGameServiceManagerHelper GameServiceManager;
  GameServiceManager.SetModuleCreateInfo(pGameServerInfo->szServerDLLName, GAME_SERVICE_CREATE_NAME);

  // 创建判断
  if (pGameServerInfo->dwNativeVersion == 0) {
    CLogger::Error(TEXT("[ {} ] 游戏服务器组件还没有安装，请先安装对应的游戏服务器"), pGameServerInfo->szGameName);

    return false;
  }

  // 更新判断
  if (pGameServerInfo->dwNativeVersion != pGameServerInfo->dwServerVersion) {
    CLogger::Error(TEXT("[ {} ] 游戏服务器组件已经更新了，不能继续用于启动房间"), pGameServerInfo->szGameName);

    return false;
  }

  // 加载模块
  if (GameServiceManager.CreateInstance() == false) {
    CLogger::Error(TEXT("[ {} ] 服务组件不存在或者加载失败，请重新安装服务组件"), pGameServerInfo->szGameName);

    return false;
  }

  // 模块属性
  GameServiceManager->GetServiceAttrib(m_ModuleInitParameter.GameServiceAttrib);

  // 挂接属性
  m_ModuleInitParameter.GameServiceOption.wKindID = pGameServerInfo->wKindID;
  m_ModuleInitParameter.GameServiceOption.wNodeID = pGameServerInfo->wNodeID;
  m_ModuleInitParameter.GameServiceOption.wSortID = pGameServerInfo->wSortID;
  m_ModuleInitParameter.GameServiceOption.wServerID = pGameServerInfo->wServerID;

  // 税收配置
  m_ModuleInitParameter.GameServiceOption.lCellScore = pGameServerInfo->lCellScore;
  m_ModuleInitParameter.GameServiceOption.wRevenueRatio = pGameServerInfo->wRevenueRatio;
  m_ModuleInitParameter.GameServiceOption.lServiceScore = pGameServerInfo->lServiceScore;

  // 房间配置
  m_ModuleInitParameter.GameServiceOption.lRestrictScore = pGameServerInfo->lRestrictScore;
  m_ModuleInitParameter.GameServiceOption.lMinTableScore = pGameServerInfo->lMinTableScore;
  m_ModuleInitParameter.GameServiceOption.lMinEnterScore = pGameServerInfo->lMinEnterScore;
  m_ModuleInitParameter.GameServiceOption.lMaxEnterScore = pGameServerInfo->lMaxEnterScore;

  // 会员限制
  m_ModuleInitParameter.GameServiceOption.cbMinEnterMember = pGameServerInfo->cbMinEnterMember;
  m_ModuleInitParameter.GameServiceOption.cbMaxEnterMember = pGameServerInfo->cbMaxEnterMember;

  // 房间配置
  m_ModuleInitParameter.GameServiceOption.dwServerRule = pGameServerInfo->dwServerRule;
  m_ModuleInitParameter.GameServiceOption.dwAttachUserRight = pGameServerInfo->dwAttachUserRight;

  // 房间属性
  m_ModuleInitParameter.GameServiceOption.wMaxPlayer = pGameServerInfo->wMaxPlayer;
  m_ModuleInitParameter.GameServiceOption.wTableCount = pGameServerInfo->wTableCount;
  m_ModuleInitParameter.GameServiceOption.wServerPort = pGameServerInfo->wServerPort;
  m_ModuleInitParameter.GameServiceOption.wServerKind = pGameServerInfo->wServerKind;
  m_ModuleInitParameter.GameServiceOption.wServerType = pGameServerInfo->wServerType;
  m_ModuleInitParameter.GameServiceOption.wServerLevel = pGameServerInfo->wServerLevel;
  lstrcpyn(m_ModuleInitParameter.GameServiceOption.szServerName, pGameServerInfo->szServerName, LEN_SERVER);
  lstrcpyn(m_ModuleInitParameter.GameServiceOption.szServerPasswd, pGameServerInfo->szServerPasswd, LEN_PASSWORD);

  // 分组属性
  m_ModuleInitParameter.GameServiceOption.cbDistributeRule = pGameServerInfo->cbDistributeRule;
  m_ModuleInitParameter.GameServiceOption.wMinDistributeUser = pGameServerInfo->wMinDistributeUser;
  m_ModuleInitParameter.GameServiceOption.wDistributeTimeSpace = pGameServerInfo->wDistributeTimeSpace;
  m_ModuleInitParameter.GameServiceOption.wDistributeDrawCount = pGameServerInfo->wDistributeDrawCount;
  m_ModuleInitParameter.GameServiceOption.wMinPartakeGameUser = pGameServerInfo->wMinPartakeGameUser;
  m_ModuleInitParameter.GameServiceOption.wMaxPartakeGameUser = pGameServerInfo->wMaxPartakeGameUser;

  // 连接信息
  lstrcpyn(m_ModuleInitParameter.GameServiceOption.szDataBaseName, pGameServerInfo->szDataBaseName,
           CountArray(m_ModuleInitParameter.GameServiceOption.szDataBaseName));
  lstrcpyn(m_ModuleInitParameter.GameServiceOption.szDataBaseAddr, pGameServerInfo->szDataBaseAddr,
           CountArray(m_ModuleInitParameter.GameServiceOption.szDataBaseAddr));

  // 数据设置
  UINT uCustomRuleSize = sizeof(m_ModuleInitParameter.GameServiceOption.cbCustomRule);
  CopyMemory(m_ModuleInitParameter.GameServiceOption.cbCustomRule, pGameServerInfo->cbCustomRule, uCustomRuleSize);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
