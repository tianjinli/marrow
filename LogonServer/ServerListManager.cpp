#include "ServerListManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CServerListManager::CServerListManager() {
  // 设置质数
  m_TypeItemMap.reserve(PRIME_TYPE);
  m_KindItemMap.reserve(PRIME_KIND);
  m_NodeItemMap.reserve(PRIME_NODE);
  m_PageItemMap.reserve(PRIME_PAGE);
  m_ServerItemMap.reserve(PRIME_SERVER);
}

// 析构函数
CServerListManager::~CServerListManager() {
  // 变量定义
  WORD wKey = 0;

  // 删除种类
  CGameTypeItem* pGameTypeItem = nullptr;
  for (auto itr = m_TypeItemMap.begin(); itr != m_TypeItemMap.end(); ++itr) {
    pGameTypeItem = itr->second;
    SafeDelete(pGameTypeItem);
  }
  for (INT_PTR i = 0; i < m_TypeItemBuffer.GetCount(); i++) {
    pGameTypeItem = m_TypeItemBuffer[i];
    SafeDelete(pGameTypeItem);
  }
  m_TypeItemMap.clear();
  m_TypeItemBuffer.RemoveAll();

  // 删除类型
  CGameKindItem* pGameKindItem = nullptr;
  for (auto itr = m_KindItemMap.begin(); itr != m_KindItemMap.end(); ++itr) {
    pGameKindItem = itr->second;
    SafeDelete(pGameKindItem);
  }
  for (INT_PTR i = 0; i < m_KindItemBuffer.GetCount(); i++) {
    pGameKindItem = m_KindItemBuffer[i];
    SafeDelete(pGameKindItem);
  }
  m_KindItemMap.clear();
  m_KindItemBuffer.RemoveAll();

  // 删除节点
  CGameNodeItem* pGameNodeItem = nullptr;
  for (auto itr = m_NodeItemMap.begin(); itr != m_NodeItemMap.end(); ++itr) {
    pGameNodeItem = itr->second;
    SafeDelete(pGameNodeItem);
  }
  for (INT_PTR i = 0; i < m_NodeItemBuffer.GetCount(); i++) {
    pGameNodeItem = m_NodeItemBuffer[i];
    SafeDelete(pGameNodeItem);
  }
  m_NodeItemMap.clear();
  m_NodeItemBuffer.RemoveAll();

  // 删除房间
  CGameServerItem* pGameServerItem = nullptr;
  for (auto itr = m_ServerItemMap.begin(); itr != m_ServerItemMap.end(); ++itr) {
    pGameServerItem = itr->second;
    SafeDelete(pGameServerItem);
  }
  for (INT_PTR i = 0; i < m_ServerItemBuffer.GetCount(); i++) {
    pGameServerItem = m_ServerItemBuffer[i];
    SafeDelete(pGameServerItem);
  }
  m_ServerItemMap.clear();
  m_ServerItemBuffer.RemoveAll();

  // 删除定制
  CGamePageItem* pGamePageItem = nullptr;
  for (auto itr = m_PageItemMap.begin(); itr != m_PageItemMap.end(); ++itr) {
    pGamePageItem = itr->second;
    SafeDelete(pGamePageItem);
  }
  for (INT_PTR i = 0; i < m_PageItemBuffer.GetCount(); i++) {
    pGamePageItem = m_PageItemBuffer[i];
    SafeDelete(pGamePageItem);
  }
  m_PageItemMap.clear();
  m_PageItemBuffer.RemoveAll();
}

// 重置列表
VOID CServerListManager::ResetServerList() {
  // 废弃列表
  DisuseKernelItem();
  DisuseServerItem();

  // 清理列表
  CleanKernelItem();
  CleanServerItem();
}

// 清理内核
VOID CServerListManager::CleanKernelItem() {
  // 清理种类
  for (auto itr = m_TypeItemMap.begin(); itr != m_TypeItemMap.end();) {
    // 索引房间
    CGameTypeItem* pGameTypeItem = itr->second;

    // 删除判断
    if (pGameTypeItem->m_bDisuse) {
      m_TypeItemMap.erase(itr++);
      m_TypeItemBuffer.Add(pGameTypeItem);
    } else {
      ++itr;
    }
  }

  // 清理类型
  for (auto itr = m_KindItemMap.begin(); itr != m_KindItemMap.end();) {
    // 索引房间
    CGameKindItem* pGameKindItem = itr->second;

    // 删除判断
    if (pGameKindItem->m_bDisuse) {
      m_KindItemMap.erase(itr++);
      m_KindItemBuffer.Add(pGameKindItem);
    } else {
      ++itr;
    }
  }

  // 清理节点
  for (auto itr = m_NodeItemMap.begin(); itr != m_NodeItemMap.end();) {
    // 索引房间
    WORD wNodeID = 0;
    CGameNodeItem* pGameNodeItem = itr->second;

    // 删除判断
    if (pGameNodeItem->m_bDisuse == true) {
      m_NodeItemMap.erase(itr++);
      m_NodeItemBuffer.Add(pGameNodeItem);
    } else {
      ++itr;
    }
  }

  // 清理定制
  for (auto itr = m_PageItemMap.begin(); itr != m_PageItemMap.end();) {
    // 索引房间
    CGamePageItem* pGamePageItem = itr->second;

    // 删除判断
    if (pGamePageItem->m_bDisuse) {
      m_PageItemMap.erase(itr++);
      m_PageItemBuffer.Add(pGamePageItem);
    } else {
      ++itr;
    }
  }
}

// 清理房间
VOID CServerListManager::CleanServerItem() {
  // 变量定义
  CGameServerItem* pGameServerItem = nullptr;
  // 删除房间
  for (auto itr = m_ServerItemMap.begin(); itr != m_ServerItemMap.end();) {
    // 索引房间
    pGameServerItem = itr->second;

    // 删除判断
    if (pGameServerItem->m_bDisuse) {
      // 删除数据
      m_ServerItemMap.erase(itr++); // itr = m_ServerItemMap.erase(itr);
      m_ServerItemBuffer.Add(pGameServerItem);

      // 设置人数
      CGameKindItem* pGameKindItem = nullptr;
      if (auto itr = m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID); itr != m_KindItemMap.end()) {
        pGameKindItem = itr->second;
        tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
        pGameKindItem->m_GameKind.dwOnLineCount = std::max(pGameKindItem->m_GameKind.dwOnLineCount - pGameServer->dwOnLineCount, (DWORD)0);
        pGameKindItem->m_GameKind.dwFullCount = std::max(pGameKindItem->m_GameKind.dwFullCount - pGameServer->dwFullCount, (DWORD)0);
      }
    } else {
      ++itr;
    }
  }
}

// 废弃内核
VOID CServerListManager::DisuseKernelItem() {
  // 变量定义
  WORD wKey = 0;
  DWORD dwKey = 0;

  // 废弃种类
  CGameTypeItem* pGameTypeItem = nullptr;
  for (auto itr = m_TypeItemMap.begin(); itr != m_TypeItemMap.end(); ++itr) {
    pGameTypeItem = itr->second;
    pGameTypeItem->m_bDisuse = true;
  }

  // 废弃类型
  CGameKindItem* pGameKindItem = nullptr;
  for (auto itr = m_KindItemMap.begin(); itr != m_KindItemMap.end(); ++itr) {
    pGameKindItem = itr->second;
    pGameKindItem->m_bDisuse = true;
  }

  // 废弃节点
  CGameNodeItem* pGameNodeItem = nullptr;
  for (auto itr = m_NodeItemMap.begin(); itr != m_NodeItemMap.end(); ++itr) {
    pGameNodeItem = itr->second;
    pGameNodeItem->m_bDisuse = true;
  }

  // 废弃定制
  CGamePageItem* pGamePageItem = nullptr;
  for (auto itr = m_PageItemMap.begin(); itr != m_PageItemMap.end(); ++itr) {
    pGamePageItem = itr->second;
    pGamePageItem->m_bDisuse = true;
  }
}

// 废弃房间
VOID CServerListManager::DisuseServerItem() {
  // 变量定义
  WORD wKey = 0;
  CGameServerItem* pGameServerItem = nullptr;

  // 废弃房间
  for (auto itr = m_ServerItemMap.begin(); itr != m_ServerItemMap.end(); ++itr) {
    pGameServerItem = itr->second;
    pGameServerItem->m_bDisuse = true;
  }
}

// 统计人数
DWORD CServerListManager::CollectOnlineInfo(bool bAndroid) {
  // 变量定义
  DWORD dwOnLineCount = 0L;

  // 统计人数
  for (auto itr = m_KindItemMap.begin(); itr != m_KindItemMap.end(); ++itr) {
    // 索引类型
    WORD wKindID = 0;
    CGameKindItem* pGameKindItem = itr->second;

    // 统计人数
    if (pGameKindItem != nullptr) {
      if (bAndroid == false) {
        dwOnLineCount += pGameKindItem->m_GameKind.dwOnLineCount;
      } else {
        dwOnLineCount += pGameKindItem->m_GameKind.dwAndroidCount;
      }
    }
  }

  return dwOnLineCount;
}

// 类型在线
DWORD CServerListManager::CollectOnlineInfo(WORD wKindID, bool bAndroid) {
  // 变量定义
  DWORD dwOnLineCount = 0L;

  // 枚举房间
  for (auto itr = m_ServerItemMap.begin(); itr != m_ServerItemMap.end(); ++itr) {
    // 索引房间
    WORD wServerID = 0;
    CGameServerItem* pGameServerItem = itr->second;

    // 统计人数
    if (pGameServerItem->m_GameServer.wKindID == wKindID) {
      if (bAndroid == false) {
        dwOnLineCount += pGameServerItem->m_GameServer.dwOnLineCount;
      } else {
        dwOnLineCount += pGameServerItem->m_GameServer.dwAndroidCount;
      }
    }
  }

  return dwOnLineCount;
}
// 统计设置人数
DWORD CServerListManager::CollectSetPlayer(WORD wKindID) {
  bool bAllGameServerItem = (wKindID == INVALID_WORD);
  // 变量定义
  DWORD dwOnLineCount = 0L;

  // 枚举房间
  for (auto itr = m_ServerItemMap.begin(); itr != m_ServerItemMap.end(); ++itr) {
    // 索引房间
    WORD wServerID = 0;
    CGameServerItem* pGameServerItem = itr->second;

    // 统计人数
    if (bAllGameServerItem) {
      dwOnLineCount += pGameServerItem->m_GameServer.dwSetPlayerCount;
    } else if (pGameServerItem->m_GameServer.wKindID == wKindID) {
      dwOnLineCount += pGameServerItem->m_GameServer.dwSetPlayerCount;
    }
  }

  return dwOnLineCount;
}
// 插入种类
bool CServerListManager::InsertGameType(tagGameType* pGameType) {
  // 效验参数
  ASSERT(pGameType != nullptr);
  if (pGameType == nullptr)
    return false;

  // 查找现存
  CGameTypeItem* pGameTypeItem = nullptr;
  auto itr = m_TypeItemMap.find(pGameType->wTypeID);
  if (itr == m_TypeItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_TypeItemBuffer.GetCount();
      if (nStroeCount > 0) {
        pGameTypeItem = m_TypeItemBuffer[nStroeCount - 1];
        m_TypeItemBuffer.RemoveAt(nStroeCount - 1);
      } else {
        pGameTypeItem = new CGameTypeItem;
        if (pGameTypeItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGameTypeItem, sizeof(CGameTypeItem));
  } else {
    pGameTypeItem = itr->second;
  }

  // 设置数据
  pGameTypeItem->m_bDisuse = false;
  CopyMemory(&pGameTypeItem->m_GameType, pGameType, sizeof(tagGameType));

  // 设置索引
  m_TypeItemMap[pGameType->wTypeID] = pGameTypeItem;

  return true;
}

// 插入类型
bool CServerListManager::InsertGameKind(tagGameKind* pGameKind) {
  // 效验参数
  ASSERT(pGameKind != nullptr);
  if (pGameKind == nullptr)
    return false;

  // 查找现存
  CGameKindItem* pGameKindItem = nullptr;
  auto itr = m_KindItemMap.find(pGameKind->wKindID);
  if (itr == m_KindItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_KindItemBuffer.GetCount();
      if (nStroeCount > 0) {
        pGameKindItem = m_KindItemBuffer[nStroeCount - 1];
        m_KindItemBuffer.RemoveAt(nStroeCount - 1);
      } else {
        pGameKindItem = new CGameKindItem;
        if (pGameKindItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGameKindItem, sizeof(CGameKindItem));
  } else {
    pGameKindItem = itr->second;
  }

  // 设置数据
  pGameKindItem->m_bDisuse = false;
  CopyMemory(&pGameKindItem->m_GameKind, pGameKind, sizeof(tagGameKind));

  // 设置索引
  m_KindItemMap[pGameKind->wKindID] = pGameKindItem;

  return true;
}

// 插入节点
bool CServerListManager::InsertGameNode(tagGameNode* pGameNode) {
  // 效验参数
  ASSERT(pGameNode != nullptr);
  if (pGameNode == nullptr)
    return false;

  // 查找现存
  CGameNodeItem* pGameNodeItem = nullptr;
  auto itr = m_NodeItemMap.find(pGameNode->wNodeID);
  if (itr == m_NodeItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_NodeItemBuffer.GetCount();
      if (nStroeCount > 0) {
        pGameNodeItem = m_NodeItemBuffer[nStroeCount - 1];
        m_NodeItemBuffer.RemoveAt(nStroeCount - 1);
      } else {
        pGameNodeItem = new CGameNodeItem;
        if (pGameNodeItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGameNodeItem, sizeof(CGameNodeItem));
  } else {
    pGameNodeItem = itr->second;
  }

  // 插入数据
  pGameNodeItem->m_bDisuse = false;
  CopyMemory(&pGameNodeItem->m_GameNode, pGameNode, sizeof(tagGameNode));

  // 设置索引
  m_NodeItemMap[pGameNode->wNodeID] = pGameNodeItem;

  return true;
}

// 插入定制
bool CServerListManager::InsertGamePage(tagGamePage* pGamePage) {
  // 效验参数
  ASSERT(pGamePage != nullptr);
  if (pGamePage == nullptr)
    return false;

  // 获取子项
  CGamePageItem* pGamePageItem = nullptr;
  auto itr = m_PageItemMap.find(pGamePage->wPageID);
  if (itr == m_PageItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_PageItemBuffer.GetCount();
      if (nStroeCount > 0) {
        pGamePageItem = m_PageItemBuffer[nStroeCount - 1];
        m_PageItemBuffer.RemoveAt(nStroeCount - 1);
      } else {
        pGamePageItem = new CGamePageItem;
        if (pGamePageItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGamePageItem, sizeof(CGamePageItem));
  } else {
    pGamePageItem = itr->second;
  }

  // 设置数据
  pGamePageItem = itr->second;
  pGamePageItem->m_bDisuse = false;
  CopyMemory(&pGamePageItem->m_GamePage, pGamePage, sizeof(tagGamePage));

  // 设置索引
  m_PageItemMap[pGamePage->wPageID] = pGamePageItem;

  return true;
}

// 插入房间
bool CServerListManager::InsertGameServer(tagGameServer* pGameServer) {
  // 效验参数
  ASSERT(pGameServer != nullptr);
  if (pGameServer == nullptr)
    return false;

  // 查找房间
  WORD wKindID = 0;
  DWORD dwOnLineCount = 0L;
  DWORD dwAndroidCount = 0L;
  DWORD dwMaxPlayer = 0L;
  CGameServerItem* pGameServerItem = nullptr;

  // 获取子项
  if (auto itr = m_ServerItemMap.find(pGameServer->wServerID); itr == m_ServerItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_ServerItemBuffer.GetCount();
      if (nStroeCount > 0) {
        pGameServerItem = m_ServerItemBuffer[nStroeCount - 1];
        m_ServerItemBuffer.RemoveAt(nStroeCount - 1);
      } else {
        pGameServerItem = new CGameServerItem;
        if (pGameServerItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGameServerItem, sizeof(CGameServerItem));
  } else {
    // 保存变量
    pGameServerItem = itr->second;
    wKindID = pGameServerItem->m_GameServer.wKindID;
    dwOnLineCount = pGameServerItem->m_GameServer.dwOnLineCount;
    dwAndroidCount = pGameServerItem->m_GameServer.dwAndroidCount;
    dwMaxPlayer = pGameServerItem->m_GameServer.dwFullCount;
  }

  // 设置数据
  pGameServerItem->m_bDisuse = false;
  CopyMemory(&pGameServerItem->m_GameServer, pGameServer, sizeof(tagGameServer));

  // 设置人数
  if (wKindID != pGameServerItem->m_GameServer.wKindID) {
    // 变量定义
    CGameKindItem* pGameKindItemLast = nullptr;

    // 历史人数
    if (auto itr = m_KindItemMap.find(wKindID); itr != m_KindItemMap.end()) {
      pGameKindItemLast->m_GameKind.dwOnLineCount -= dwOnLineCount;
      pGameKindItemLast->m_GameKind.dwAndroidCount -= dwAndroidCount;
      pGameKindItemLast->m_GameKind.dwFullCount -= dwMaxPlayer;
    }

    // 当前人数
    if (auto itr = m_KindItemMap.find(pGameServer->wKindID); itr != m_KindItemMap.end()) {
      CGameKindItem* pGameKindItemCurrent = itr->second;

      pGameKindItemCurrent->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;
      pGameKindItemCurrent->m_GameKind.dwAndroidCount += pGameServer->dwAndroidCount;
      pGameKindItemCurrent->m_GameKind.dwFullCount += pGameServer->dwFullCount;
    }
  } else {
    // 查找类型
    auto itr = m_KindItemMap.find(wKindID);
    if (itr != m_KindItemMap.end()) {
      // 设置人数
      CGameKindItem* pGameKindItem = itr->second;

      pGameKindItem->m_GameKind.dwOnLineCount -= dwOnLineCount;
      pGameKindItem->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;

      pGameKindItem->m_GameKind.dwAndroidCount -= dwAndroidCount;
      pGameKindItem->m_GameKind.dwAndroidCount += pGameServer->dwAndroidCount;

      pGameKindItem->m_GameKind.dwFullCount -= dwMaxPlayer;
      pGameKindItem->m_GameKind.dwFullCount += pGameServer->dwFullCount;
    }
  }

  // 设置索引
  m_ServerItemMap[pGameServer->wServerID] = pGameServerItem;

  return true;
}

// 删除种类
bool CServerListManager::DeleteGameType(WORD wTypeID) {
  // 查找种类
  auto itr = m_TypeItemMap.find(wTypeID);
  if (itr == m_TypeItemMap.end())
    return false;

  CGameTypeItem* pGameTypeItem = itr->second;

  // 删除数据
  m_TypeItemMap.erase(itr);
  m_TypeItemBuffer.Add(pGameTypeItem);

  return true;
}

// 删除类型
bool CServerListManager::DeleteGameKind(WORD wKindID) {
  // 查找类型
  auto itr = m_KindItemMap.find(wKindID);
  if (itr == m_KindItemMap.end())
    return false;

  CGameKindItem* pGameKindItem = itr->second;

  // 删除数据
  m_KindItemMap.erase(itr);
  m_KindItemBuffer.Add(pGameKindItem);

  // 设置变量
  ZeroMemory(pGameKindItem, sizeof(CGameKindItem));

  return true;
}

// 删除节点
bool CServerListManager::DeleteGameNode(WORD wNodeID) {
  // 查找节点
  auto itr = m_NodeItemMap.find(wNodeID);
  if (itr == m_NodeItemMap.end())
    return false;

  CGameNodeItem* pGameNodeItem = itr->second;

  // 删除数据
  m_NodeItemMap.erase(itr);
  m_NodeItemBuffer.Add(pGameNodeItem);

  // 设置变量
  ZeroMemory(pGameNodeItem, sizeof(CGameNodeItem));

  return true;
}

// 删除房间
bool CServerListManager::DeleteGameServer(WORD wServerID) {
  // 查找房间
  auto itr = m_ServerItemMap.find(wServerID);
  if (itr == m_ServerItemMap.end())
    return false;

  CGameServerItem* pGameServerItem = itr->second;

  // 删除数据
  m_ServerItemMap.erase(itr);
  m_ServerItemBuffer.Add(pGameServerItem);

  // 设置人数
  if (auto itr = m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID); itr != m_KindItemMap.end()) {
    CGameKindItem* pGameKindItem = itr->second;

    tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
    pGameKindItem->m_GameKind.dwOnLineCount = std::max(pGameKindItem->m_GameKind.dwOnLineCount - pGameServer->dwOnLineCount, (DWORD)0);
    pGameKindItem->m_GameKind.dwFullCount = std::max(pGameKindItem->m_GameKind.dwFullCount - pGameServer->dwFullCount, (DWORD)0);
    pGameKindItem->m_GameKind.dwAndroidCount = std::max(pGameKindItem->m_GameKind.dwAndroidCount - pGameServer->dwAndroidCount, (DWORD)0);
  }

  return true;
}

// 删除定制
bool CServerListManager::DeleteGamePage(WORD wPageID) {
  // 查找类型
  auto itr = m_PageItemMap.find(wPageID);
  if (itr == m_PageItemMap.end())
    return false;

  CGamePageItem* pGamePageItem = itr->second;

  // 删除数据
  m_PageItemMap.erase(itr);
  m_PageItemBuffer.Add(pGamePageItem);

  // 设置变量
  ZeroMemory(pGamePageItem, sizeof(CGamePageItem));

  return true;
}

// 枚举种类
CGameTypeItem* CServerListManager::EmunGameTypeItem(CTypeItemMap::iterator* itr) {
  auto cur = m_TypeItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_TypeItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举类型
CGameKindItem* CServerListManager::EmunGameKindItem(CKindItemMap::iterator* itr) {
  auto cur = m_KindItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_KindItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举节点
CGameNodeItem* CServerListManager::EmunGameNodeItem(CNodeItemMap::iterator* itr) {
  auto cur = m_NodeItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_NodeItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举房间
CGameServerItem* CServerListManager::EmunGameServerItem(CServerItemMap::iterator* itr) {
  auto cur = m_ServerItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_ServerItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举定制
CGamePageItem* CServerListManager::EmunGamePageItem(CPageItemMap::iterator* itr) {
  auto cur = m_PageItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_PageItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 查找种类
CGameTypeItem* CServerListManager::SearchGameType(WORD wTypeID) {
  auto itr = m_TypeItemMap.find(wTypeID);
  if (itr != m_TypeItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

// 查找类型
CGameKindItem* CServerListManager::SearchGameKind(WORD wKindID) {
  auto itr = m_KindItemMap.find(wKindID);
  if (itr != m_KindItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

// 查找节点
CGameNodeItem* CServerListManager::SearchGameNode(WORD wNodeID) {
  auto itr = m_NodeItemMap.find(wNodeID);
  if (itr != m_NodeItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

// 查找房间
CGameServerItem* CServerListManager::SearchGameServer(WORD wServerID) {
  auto itr = m_ServerItemMap.find(wServerID);
  if (itr != m_ServerItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

// 查找定制
CGamePageItem* CServerListManager::SearchGamePage(WORD wPageID) {
  auto itr = m_PageItemMap.find(wPageID);
  if (itr != m_PageItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
