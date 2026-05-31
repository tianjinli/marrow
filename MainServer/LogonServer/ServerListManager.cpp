#include "ServerListManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameTypeItem::CGameTypeItem() {
  // 设置变量
  ZeroMemory(&m_GameType, sizeof(m_GameType));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameKindItem::CGameKindItem() {
  // 设置变量
  ZeroMemory(&m_GameKind, sizeof(m_GameKind));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameNodeItem::CGameNodeItem() {
  // 设置变量
  ZeroMemory(&m_GameNode, sizeof(m_GameNode));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameServerItem::CGameServerItem() {
  // 设置变量
  ZeroMemory(&m_GameServer, sizeof(m_GameServer));
  ZeroMemory(&m_GameMatch, sizeof(m_GameMatch));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePageItem::CGamePageItem() {
  // 设置变量
  ZeroMemory(&m_GamePage, sizeof(m_GamePage));
}

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
  // 删除种类
  for (auto& pGameTypeItem: m_TypeItemMap | std::views::values) {
    SafeDelete(pGameTypeItem);
  }
  for (auto& pGameTypeItem: m_TypeItemBuffer) {
    SafeDelete(pGameTypeItem);
  }
  m_TypeItemMap.clear();
  m_TypeItemBuffer.clear();

  // 删除类型
  for (auto& [wKey, pGameKindItem]: m_KindItemMap) {
    SafeDelete(pGameKindItem);
  }
  for (auto& pGameKindItem: m_KindItemBuffer) {
    SafeDelete(pGameKindItem);
  }
  m_KindItemMap.clear();
  m_KindItemBuffer.clear();

  // 删除节点
  for (auto& [wKey, pGameNodeItem]: m_NodeItemMap) {
    SafeDelete(pGameNodeItem);
  }
  for (auto& pGameNodeItem: m_NodeItemBuffer) {
    SafeDelete(pGameNodeItem);
  }
  m_NodeItemMap.clear();
  m_NodeItemBuffer.clear();

  // 删除房间
  for (auto& [wKey, pGameServerItem]: m_ServerItemMap) {
    SafeDelete(pGameServerItem);
  }
  for (auto& pGameServerItem: m_ServerItemBuffer) {
    SafeDelete(pGameServerItem);
  }
  m_ServerItemMap.clear();
  m_ServerItemBuffer.clear();

  // 删除定制
  for (auto& [wKey, pGamePageItem]: m_PageItemMap) {
    SafeDelete(pGamePageItem);
  }
  for (auto& pGamePageItem: m_PageItemBuffer) {
    SafeDelete(pGamePageItem);
  }
  m_PageItemMap.clear();
  m_PageItemBuffer.clear();
}

// 重置列表
VOID CServerListManager::ResetServerList() {
  // 废弃列表
  DisuseKernelItem();
  DisuseServerItem();

  // 清理列表
  CleanKernelItem();
  CleanServerItem();

  return;
}

// 清理内核
VOID CServerListManager::CleanKernelItem() {
  // 清理种类
  std::erase_if(m_TypeItemMap, [this](const auto& pair) {
    auto* pGameTypeItem = pair.second;
    if (pGameTypeItem->m_bDisuse) {
      m_TypeItemBuffer.emplace_back(pGameTypeItem); // 移入 Buffer
      return true; // 返回 true 代表从 Map 中删掉它
    }
    return false; // 返回 false 代表保留
  });

  // 清理类型
  std::erase_if(m_KindItemMap, [this](const auto& pair) {
    auto* pGameKindItem = pair.second;
    if (pGameKindItem->m_bDisuse) {
      m_KindItemBuffer.emplace_back(pGameKindItem); // 移入 Buffer
      return true; // 返回 true 代表从 Map 中删掉它
    }
    return false; // 返回 false 代表保留
  });

  // 清理节点
  std::erase_if(m_NodeItemMap, [this](const auto& pair) {
    auto* pGameNodeItem = pair.second;
    if (pGameNodeItem->m_bDisuse) {
      m_NodeItemBuffer.emplace_back(pGameNodeItem); // 移入 Buffer
      return true; // 返回 true 代表从 Map 中删掉它
    }
    return false; // 返回 false 代表保留
  });

  // 清理定制
  std::erase_if(m_PageItemMap, [this](const auto& pair) {
    auto* pGamePageItem = pair.second;
    if (pGamePageItem->m_bDisuse) {
      m_PageItemBuffer.emplace_back(pGamePageItem); // 移入 Buffer
      return true; // 返回 true 代表从 Map 中删掉它
    }
    return false; // 返回 false 代表保留
  });
}

// 清理房间
VOID CServerListManager::CleanServerItem() {
  std::vector<CGameServerItem*> pGameServerItems;
  // 删除房间
  std::erase_if(m_ServerItemMap, [this, &pGameServerItems](const auto& pair) {
    auto* pGameServerItem = pair.second;
    if (pGameServerItem->m_bDisuse == true) {
      m_ServerItemBuffer.emplace_back(pGameServerItem); // 移入 Buffer
      return true; // 返回 true 代表从 Map 中删掉它
    }
    return false; // 返回 false 代表保留
  });

  // 设置人数
  for (auto& pGameServerItem: pGameServerItems) {
    if (auto itr = m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID); itr != m_KindItemMap.end()) {
      CGameKindItem* pGameKindItem = itr->second;
      tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
      pGameKindItem->m_GameKind.dwOnLineCount = std::max(pGameKindItem->m_GameKind.dwOnLineCount - pGameServer->dwOnLineCount, (DWORD) 0);
      pGameKindItem->m_GameKind.dwFullCount = std::max(pGameKindItem->m_GameKind.dwFullCount - pGameServer->dwFullCount, (DWORD) 0);
    }
  }
}

// 废弃内核
VOID CServerListManager::DisuseKernelItem() {
  // 废弃种类
  for (auto& pGameTypeItem: m_TypeItemMap | std::views::values) {
    pGameTypeItem->m_bDisuse = true;
  }

  // 废弃类型
  for (auto& pGameKindItem: m_KindItemMap | std::views::values) {
    pGameKindItem->m_bDisuse = true;
  }

  // 废弃节点
  for (auto& pGameNodeItem: m_NodeItemMap | std::views::values) {
    pGameNodeItem->m_bDisuse = true;
  }

  // 废弃定制
  for (auto& pGamePageItem: m_PageItemMap | std::views::values) {
    pGamePageItem->m_bDisuse = true;
  }
}

// 废弃房间
VOID CServerListManager::DisuseServerItem() {
  // 废弃房间
  for (auto& pGameServerItem: m_ServerItemMap | std::views::values) {
    pGameServerItem->m_bDisuse = true;
  }
}

// 统计人数
DWORD CServerListManager::CollectOnlineInfo(bool bAndroid) {
  // 变量定义
  DWORD dwOnLineCount = 0L;
  for (auto& pGameKindItem: m_KindItemMap | std::views::values) {
    // 统计人数
    if (pGameKindItem != NULL) {
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
  for (auto& pGameServerItem: m_ServerItemMap | std::views::values) {
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

// 插入种类
bool CServerListManager::InsertGameType(tagGameType* pGameType) {
  // 效验参数
  ASSERT(pGameType != NULL);
  if (pGameType == NULL) {
    return false;
  }

  // 查找现存
  CGameTypeItem* pGameTypeItem = SearchGameType(pGameType->wTypeID);
  if (pGameTypeItem == NULL) {
    // 创建对象
    if (!m_TypeItemBuffer.empty()) {
      // pGameTypeItem = m_TypeItemBuffer[nStroeCount - 1];
      // m_TypeItemBuffer.erase(m_TypeItemBuffer.end() - 1);
      pGameTypeItem = m_TypeItemBuffer.back();
      m_TypeItemBuffer.pop_back();
    } else {
      // 使用 std::nothrow 彻底告别 try-catch，如果内存不足直接返回 nullptr，不会抛出异常
      pGameTypeItem = new (std::nothrow) CGameTypeItem;
      if (pGameTypeItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGameTypeItem, sizeof(CGameTypeItem));
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
  ASSERT(pGameKind != NULL);
  if (pGameKind == NULL) {
    return false;
  }

  // 查找现存
  CGameKindItem* pGameKindItem = SearchGameKind(pGameKind->wKindID);
  if (pGameKindItem == NULL) {
    // 创建对象
    if (!m_KindItemBuffer.empty()) {
      pGameKindItem = m_KindItemBuffer.back();
      m_KindItemBuffer.pop_back();
    } else {
      // 使用 std::nothrow 彻底告别 try-catch，如果内存不足直接返回 nullptr，不会抛出异常
      pGameKindItem = new (std::nothrow) CGameKindItem;
      if (pGameKindItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGameKindItem, sizeof(CGameKindItem));
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
  ASSERT(pGameNode != NULL);
  if (pGameNode == NULL) {
    return false;
  }

  // 查找现存
  CGameNodeItem* pGameNodeItem = SearchGameNode(pGameNode->wNodeID);
  if (pGameNodeItem == NULL) {
    // 创建对象
    if (!m_NodeItemBuffer.empty()) {
      pGameNodeItem = m_NodeItemBuffer.back();
      m_NodeItemBuffer.pop_back();
    } else {
      // 使用 std::nothrow 彻底告别 try-catch，如果内存不足直接返回 nullptr，不会抛出异常
      pGameNodeItem = new (std::nothrow) CGameNodeItem;
      if (pGameNodeItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGameNodeItem, sizeof(CGameNodeItem));
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
  ASSERT(pGamePage != NULL);
  if (pGamePage == NULL) {
    return false;
  }

  // 获取子项
  CGamePageItem* pGamePageItem = SearchGamePage(pGamePage->wPageID);
  if (pGamePageItem == NULL) {
    // 创建对象
    if (!m_PageItemBuffer.empty()) {
      pGamePageItem = m_PageItemBuffer.back();
      m_PageItemBuffer.pop_back();
    } else {
      // 使用 std::nothrow 彻底告别 try-catch，如果内存不足直接返回 nullptr，不会抛出异常
      pGamePageItem = new (std::nothrow) CGamePageItem;
      if (pGamePageItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGamePageItem, sizeof(CGamePageItem));
  }

  // 设置数据
  pGamePageItem->m_bDisuse = false;
  CopyMemory(&pGamePageItem->m_GamePage, pGamePage, sizeof(tagGamePage));

  // 设置索引
  m_PageItemMap[pGamePage->wPageID] = pGamePageItem;
  return true;
}

// 插入房间
bool CServerListManager::InsertGameServer(tagGameServer* pGameServer) {
  // 效验参数
  ASSERT(pGameServer != NULL);
  if (pGameServer == NULL) {
    return false;
  }

  // 查找房间
  WORD wKindID = 0;
  DWORD dwOnLineCount = 0L;
  DWORD dwAndroidCount = 0L;
  DWORD dwMaxPlayer = 0L;
  CGameServerItem* pGameServerItem = NULL;

  // 获取子项
  if (auto itr = m_ServerItemMap.find(pGameServer->wServerID); itr == m_ServerItemMap.end()) {
    // 创建对象
    if (!m_ServerItemBuffer.empty()) {
      pGameServerItem = m_ServerItemBuffer.back();
      m_ServerItemBuffer.pop_back();
    } else {
      pGameServerItem = new (std::nothrow) CGameServerItem;
      if (pGameServerItem == NULL) {
        return false;
      }
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
    CGameKindItem* pGameKindItemLast = NULL;
    CGameKindItem* pGameKindItemCurrent = NULL;

    // 历史人数
    if (auto itr = m_KindItemMap.find(wKindID); itr != m_KindItemMap.end()) {
      pGameKindItemLast = itr->second;
      pGameKindItemLast->m_GameKind.dwOnLineCount -= dwOnLineCount;
      pGameKindItemLast->m_GameKind.dwAndroidCount -= dwAndroidCount;
      pGameKindItemLast->m_GameKind.dwFullCount -= dwMaxPlayer;
    }

    // 当前人数
    if (auto itr = m_KindItemMap.find(pGameServer->wKindID); itr != m_KindItemMap.end()) {
      pGameKindItemCurrent = itr->second;
      pGameKindItemCurrent->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;
      pGameKindItemCurrent->m_GameKind.dwAndroidCount += pGameServer->dwAndroidCount;
      pGameKindItemCurrent->m_GameKind.dwFullCount += pGameServer->dwFullCount;
    }
  } else {
    // 查找类型
    CGameKindItem* pGameKindItem = NULL;
    if (auto itr = m_KindItemMap.find(wKindID); itr != m_KindItemMap.end()) {
      pGameKindItem = itr->second;
    }

    // 设置人数
    if (pGameKindItem != NULL) {
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
  CGameTypeItem* pGameTypeItem = SearchGameType(wTypeID);
  if (pGameTypeItem == NULL) {
    return false;
  }

  // 删除数据
  m_TypeItemMap.erase(wTypeID);
  m_TypeItemBuffer.emplace_back(pGameTypeItem);

  // 设置变量
  // ZeroMemory(pGameTypeItem, sizeof(CGameTypeItem));
  return true;
}

// 删除类型
bool CServerListManager::DeleteGameKind(WORD wKindID) {
  // 查找类型
  CGameKindItem* pGameKindItem = SearchGameKind(wKindID);
  if (pGameKindItem == NULL) {
    return false;
  }

  // 删除数据
  m_KindItemMap.erase(wKindID);
  m_KindItemBuffer.emplace_back(pGameKindItem);

  // 设置变量
  ZeroMemory(pGameKindItem, sizeof(CGameKindItem));
  return true;
}

// 删除节点
bool CServerListManager::DeleteGameNode(WORD wNodeID) {
  // 查找节点
  CGameNodeItem* pGameNodeItem = SearchGameNode(wNodeID);
  if (pGameNodeItem == NULL) {
    return false;
  }

  // 删除数据
  m_NodeItemMap.erase(wNodeID);
  m_NodeItemBuffer.emplace_back(pGameNodeItem);

  // 设置变量
  ZeroMemory(pGameNodeItem, sizeof(CGameNodeItem));
  return true; // !!! return false;
}

// 删除房间
bool CServerListManager::DeleteGameServer(WORD wServerID) {
  // 查找房间
  CGameServerItem* pGameServerItem = SearchGameServer(wServerID);
  if (pGameServerItem == NULL) {
    return false;
  }

  // 删除数据
  m_ServerItemMap.erase(wServerID);
  m_ServerItemBuffer.emplace_back(pGameServerItem);

  // 设置人数
  if (auto itr = m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID); itr != m_KindItemMap.end()) {
    CGameKindItem* pGameKindItem = itr->second;
    tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
    pGameKindItem->m_GameKind.dwOnLineCount = std::max(pGameKindItem->m_GameKind.dwOnLineCount - pGameServer->dwOnLineCount, (DWORD) 0);
    pGameKindItem->m_GameKind.dwFullCount = std::max(pGameKindItem->m_GameKind.dwFullCount - pGameServer->dwFullCount, (DWORD) 0);
    pGameKindItem->m_GameKind.dwAndroidCount = std::max(pGameKindItem->m_GameKind.dwAndroidCount - pGameServer->dwAndroidCount, (DWORD) 0);
  }

  return true; // !!! return false;
}

// 删除定制
bool CServerListManager::DeleteGamePage(WORD wPageID) {
  // 查找类型
  CGamePageItem* pGamePageItem = SearchGamePage(wPageID);
  if (pGamePageItem == NULL) {
    return false;
  }

  // 删除数据
  m_PageItemMap.erase(wPageID);
  m_PageItemBuffer.emplace_back(pGamePageItem);

  // 设置变量
  ZeroMemory(pGamePageItem, sizeof(CGamePageItem));
  return true;
}

// 枚举种类
CGameTypeItem* CServerListManager::EmunGameTypeItem(POSITION& Position) {
  // 变量定义
  CGameTypeItem* pGameTypeItem = NULL;
  auto itr = Position.empty() ? m_TypeItemMap.begin() : Position.get<CTypeItemMap::iterator>();

  // 获取对象
  if (itr != m_TypeItemMap.end()) {
    pGameTypeItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGameTypeItem;
}

// 枚举类型
CGameKindItem* CServerListManager::EmunGameKindItem(POSITION& Position) {
  // 变量定义
  CGameKindItem* pGameKindItem = NULL;
  auto itr = Position.empty() ? m_KindItemMap.begin() : Position.get<CKindItemMap::iterator>();

  // 获取对象
  if (itr != m_KindItemMap.end()) {
    pGameKindItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGameKindItem;
}

// 枚举节点
CGameNodeItem* CServerListManager::EmunGameNodeItem(POSITION& Position) {
  // 变量定义
  CGameNodeItem* pGameNodeItem = NULL;
  auto itr = Position.empty() ? m_NodeItemMap.begin() : Position.get<CNodeItemMap::iterator>();

  // 获取对象
  if (itr != m_NodeItemMap.end()) {
    pGameNodeItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGameNodeItem;
}

// 枚举房间
CGameServerItem* CServerListManager::EmunGameServerItem(POSITION& Position) {
  // 变量定义
  CGameServerItem* pGameServerItem = NULL;
  auto itr = Position.empty() ? m_ServerItemMap.begin() : Position.get<CServerItemMap::iterator>();

  // 获取对象
  if (itr != m_ServerItemMap.end()) {
    pGameServerItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGameServerItem;
}

// 枚举定制
CGamePageItem* CServerListManager::EmunGamePageItem(POSITION& Position) {
  // 变量定义
  CGamePageItem* pGamePageItem = NULL;
  auto itr = Position.empty() ? m_PageItemMap.begin() : Position.get<CPageItemMap::iterator>();

  // 获取对象
  if (itr != m_PageItemMap.end()) {
    pGamePageItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGamePageItem;
}

// 查找种类
CGameTypeItem* CServerListManager::SearchGameType(WORD wTypeID) {
  auto itr = m_TypeItemMap.find(wTypeID);
  return itr != m_TypeItemMap.end() ? itr->second : nullptr;
}

// 查找类型
CGameKindItem* CServerListManager::SearchGameKind(WORD wKindID) {
  auto itr = m_KindItemMap.find(wKindID);
  return itr != m_KindItemMap.end() ? itr->second : nullptr;
}

// 查找节点
CGameNodeItem* CServerListManager::SearchGameNode(WORD wNodeID) {
  auto itr = m_NodeItemMap.find(wNodeID);
  return itr != m_NodeItemMap.end() ? itr->second : nullptr;
}

// 查找房间
CGameServerItem* CServerListManager::SearchGameServer(WORD wServerID) {
  auto itr = m_ServerItemMap.find(wServerID);
  return itr != m_ServerItemMap.end() ? itr->second : nullptr;
}

// 查找定制
CGamePageItem* CServerListManager::SearchGamePage(WORD wPageID) {
  auto itr = m_PageItemMap.find(wPageID);
  return itr != m_PageItemMap.end() ? itr->second : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
