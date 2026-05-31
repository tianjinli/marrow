#include "ServerListManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameTypeItem::CGameTypeItem() {
  // 设置变量
  ZeroMemory(&m_GameType, sizeof(m_GameType));

  return;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameKindItem::CGameKindItem() {
  // 设置变量
  ZeroMemory(&m_GameKind, sizeof(m_GameKind));

  return;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameNodeItem::CGameNodeItem() {
  // 设置变量
  ZeroMemory(&m_GameNode, sizeof(m_GameNode));

  return;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameServerItem::CGameServerItem() {
  // 设置变量
  ZeroMemory(&m_GameServer, sizeof(m_GameServer));

  return;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePageItem::CGamePageItem() {
  // 设置变量
  ZeroMemory(&m_GamePage, sizeof(m_GamePage));

  return;
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

  return;
}

// 析构函数
CServerListManager::~CServerListManager() {
  // 变量定义
  WORD wKey = 0;

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
  for (auto& pGameKindItem: m_KindItemMap | std::views::values) {
    SafeDelete(pGameKindItem);
  }
  for (auto& pGameKindItem: m_KindItemBuffer) {
    SafeDelete(pGameKindItem);
  }
  m_KindItemMap.clear();
  m_KindItemBuffer.clear();

  // 删除节点
  for (auto& pGameNodeItem: m_NodeItemMap | std::views::values) {
    SafeDelete(pGameNodeItem);
  }
  for (auto& pGameNodeItem: m_NodeItemBuffer) {
    SafeDelete(pGameNodeItem);
  }
  m_NodeItemMap.clear();
  m_NodeItemBuffer.clear();

  // 删除房间
  for (auto& pGameServerItem: m_ServerItemMap | std::views::values) {
    SafeDelete(pGameServerItem);
  }
  for (auto& pGameServerItem: m_ServerItemBuffer) {
    SafeDelete(pGameServerItem);
  }
  m_ServerItemMap.clear();
  m_ServerItemBuffer.clear();

  // 删除定制
  for (auto& pGamePageItem: m_PageItemMap | std::views::values) {
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
}

// 清理内核
VOID CServerListManager::CleanKernelItem() {
  // 清理种类
  std::erase_if(m_TypeItemMap, [this](auto& pair) {
    // 索引房间
    CGameTypeItem* pGameTypeItem = pair.second;

    // 删除判断
    if (pGameTypeItem->m_bDisuse == true) {
      m_TypeItemBuffer.emplace_back(pGameTypeItem);
      return true;
    }
    return false;
  });

  // 清理类型
  std::erase_if(m_KindItemMap, [this](auto& pair) {
    // 索引房间
    CGameKindItem* pGameKindItem = pair.second;

    // 删除判断
    if (pGameKindItem->m_bDisuse == true) {
      m_KindItemBuffer.emplace_back(pGameKindItem);
      return true;
    }
    return false;
  });

  // 清理节点
  std::erase_if(m_NodeItemMap, [this](auto& pair) {
    // 索引房间
    CGameNodeItem* pGameNodeItem = pair.second;

    // 删除判断
    if (pGameNodeItem->m_bDisuse == true) {
      m_NodeItemBuffer.emplace_back(pGameNodeItem);
      return true;
    }
    return false;
  });

  // 清理定制
  std::erase_if(m_PageItemMap, [this](auto& pair) {
    // 索引房间
    CGamePageItem* pGamePageItem = pair.second;

    // 删除判断
    if (pGamePageItem->m_bDisuse == true) {
      m_PageItemBuffer.emplace_back(pGamePageItem);
      return true;
    }
    return false;
  });
}

// 清理房间
VOID CServerListManager::CleanServerItem() {
  // 删除房间
  std::vector<CGameServerItem*> pGameServerItems;
  std::erase_if(m_ServerItemMap, [&pGameServerItems](auto& pair) {
    auto pGameServerItem = pair.second;
    if (pGameServerItem->m_bDisuse == true) {
      pGameServerItems.emplace_back(pGameServerItem);
      return true;
    }
    return false;
  });
  for (auto& pGameServerItem: pGameServerItems) {
    m_ServerItemBuffer.emplace_back(pGameServerItem);
    if (auto itr = m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID); itr != m_KindItemMap.end()) {
      CGameKindItem* pGameKindItem = itr->second;
      tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
      pGameKindItem->m_GameKind.dwOnLineCount -= pGameServer->dwOnLineCount;
      pGameKindItem->m_GameKind.dwFullCount -= pGameServer->dwFullCount;
    }
  }
}

// 废弃内核
VOID CServerListManager::DisuseKernelItem() {
  // 变量定义
  WORD wKey = 0;
  DWORD dwKey = 0;

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
  for (auto& pGamePageItem: m_PageItemBuffer) {
    pGamePageItem->m_bDisuse = true;
  }
}

// 废弃房间
VOID CServerListManager::DisuseServerItem() {
  for (auto& pGameServerItem: m_ServerItemMap | std::views::values) {
    pGameServerItem->m_bDisuse = true;
  }
}

// 统计人数
DWORD CServerListManager::CollectOnlineInfo() {
  // 变量定义
  DWORD dwOnLineCount = 0L;
  for (auto& pGameKindItem: m_KindItemMap | std::views::values) {
    // 统计人数
    if (pGameKindItem != NULL) {
      dwOnLineCount += pGameKindItem->m_GameKind.dwOnLineCount;
    }
  }

  return dwOnLineCount;
}

// 类型在线
DWORD CServerListManager::CollectOnlineInfo(WORD wKindID) {
  // 变量定义
  DWORD dwOnLineCount = 0L;
  for (auto& pGameServerItem: m_ServerItemMap | std::views::values) {
    // 统计人数
    if (pGameServerItem->m_GameServer.wKindID == wKindID) {
      dwOnLineCount += pGameServerItem->m_GameServer.dwOnLineCount;
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
  CGameTypeItem* pGameTypeItem = NULL;
  if (auto itr = m_TypeItemMap.find(pGameType->wTypeID); itr == m_TypeItemMap.end()) {
    // 创建对象
    if (!m_TypeItemBuffer.empty()) {
      pGameTypeItem = m_TypeItemBuffer.back();
      m_TypeItemBuffer.pop_back();
      } else {
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
  CGameKindItem* pGameKindItem = NULL;
  if (auto itr = m_KindItemMap.find(pGameKind->wKindID); itr == m_KindItemMap.end()) {
    // 创建对象
    if (!m_KindItemBuffer.empty()) {
      pGameKindItem = m_KindItemBuffer.back();
      m_KindItemBuffer.pop_back();
      } else {
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
  CGameNodeItem* pGameNodeItem = NULL;
  if (auto itr = m_NodeItemMap.find(pGameNode->wNodeID); itr == m_NodeItemMap.end()) {
    // 创建对象
    if (!m_NodeItemBuffer.empty()) {
      pGameNodeItem = m_NodeItemBuffer.back();
      m_NodeItemBuffer.pop_back();
      } else {
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
  CGamePageItem* pGamePageItem = NULL;
  if (auto itr = m_PageItemMap.find(pGamePage->wPageID); itr == m_PageItemMap.end()) {
    // 创建对象
    if (!m_PageItemBuffer.empty()) {
      pGamePageItem = m_PageItemBuffer.back();
      m_PageItemBuffer.pop_back();
      } else {
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
    if ((wKindID != 0) && (m_KindItemMap.find(wKindID) != m_KindItemMap.end())) {
      pGameKindItemLast->m_GameKind.dwOnLineCount -= dwOnLineCount;
      pGameKindItemLast->m_GameKind.dwFullCount -= dwMaxPlayer;
    }

    // 当前人数
    if (m_KindItemMap.find(pGameServer->wKindID) != m_KindItemMap.end()) {
      pGameKindItemCurrent->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;
      pGameKindItemCurrent->m_GameKind.dwFullCount += pGameServer->dwFullCount;
    }
  } else {
    // 查找类型
    CGameKindItem* pGameKindItem = NULL;
    if (m_KindItemMap.find(wKindID) != m_KindItemMap.end()) {
      pGameKindItem = m_KindItemMap.find(wKindID)->second;
    }

    // 设置人数
    if (pGameKindItem != NULL) {
      pGameKindItem->m_GameKind.dwOnLineCount -= dwOnLineCount;
      pGameKindItem->m_GameKind.dwOnLineCount += pGameServer->dwOnLineCount;

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
  CGameTypeItem* pGameTypeItem = NULL;
  if (m_TypeItemMap.find(wTypeID) == m_TypeItemMap.end()) {
    return false;
  }

  // 删除数据
  m_TypeItemMap.erase(wTypeID);
  m_TypeItemBuffer.emplace_back(pGameTypeItem);

  return true;
}

// 删除类型
bool CServerListManager::DeleteGameKind(WORD wKindID) {
  // 查找类型
  CGameKindItem* pGameKindItem = NULL;
  if (m_KindItemMap.find(wKindID) == m_KindItemMap.end()) {
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
  CGameNodeItem* pGameNodeItem = NULL;
  if (m_NodeItemMap.find(wNodeID) == m_NodeItemMap.end()) {
    return false;
  }

  // 删除数据
  m_NodeItemMap.erase(wNodeID);
  m_NodeItemBuffer.emplace_back(pGameNodeItem);

  // 设置变量
  ZeroMemory(pGameNodeItem, sizeof(CGameNodeItem));

  return false;
}

// 删除房间
bool CServerListManager::DeleteGameServer(WORD wServerID) {
  // 查找房间
  CGameServerItem* pGameServerItem = NULL;
  if (m_ServerItemMap.find(wServerID) == m_ServerItemMap.end()) {
    return false;
  }

  // 删除数据
  m_ServerItemMap.erase(wServerID);
  m_ServerItemBuffer.emplace_back(pGameServerItem);

  // 设置人数
  CGameKindItem* pGameKindItem = NULL;
  if (m_KindItemMap.find(pGameServerItem->m_GameServer.wKindID) != m_KindItemMap.end()) {
    tagGameServer* pGameServer = &pGameServerItem->m_GameServer;
    pGameKindItem->m_GameKind.dwOnLineCount -= pGameServer->dwOnLineCount;
    pGameKindItem->m_GameKind.dwFullCount -= pGameServer->dwFullCount;
  }

  return false;
}

// 删除定制
bool CServerListManager::DeleteGamePage(WORD wPageID) {
  // 查找类型
  CGamePageItem* pGamePageItem = NULL;
  if (m_PageItemMap.find(wPageID) == m_PageItemMap.end()) {
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
  CGameTypeItem* pGameTypeItem = NULL;
  if (auto itr = m_TypeItemMap.find(wTypeID); itr != m_TypeItemMap.end()) {
    pGameTypeItem = itr->second;
  }
  return pGameTypeItem;
}

// 查找类型
CGameKindItem* CServerListManager::SearchGameKind(WORD wKindID) {
  CGameKindItem* pGameKindItem = NULL;
  if (auto itr = m_KindItemMap.find(wKindID); itr != m_KindItemMap.end()) {
    pGameKindItem = itr->second;
  }
  return pGameKindItem;
}

// 查找节点
CGameNodeItem* CServerListManager::SearchGameNode(WORD wNodeID) {
  CGameNodeItem* pGameNodeItem = NULL;
  if (auto itr = m_NodeItemMap.find(wNodeID); itr != m_NodeItemMap.end()) {
    pGameNodeItem = itr->second;
  }
  return pGameNodeItem;
}

// 查找房间
CGameServerItem* CServerListManager::SearchGameServer(WORD wServerID) {
  CGameServerItem* pGameServerItem = NULL;
  if (auto itr = m_ServerItemMap.find(wServerID); itr != m_ServerItemMap.end()) {
    pGameServerItem = itr->second;
  }
  return pGameServerItem;
}

// 查找定制
CGamePageItem* CServerListManager::SearchGamePage(WORD wPageID) {
  CGamePageItem* pGamePageItem = NULL;
  if (auto itr = m_PageItemMap.find(wPageID); itr != m_PageItemMap.end()) {
    pGamePageItem = itr->second;
  }
  return pGamePageItem;
}

//////////////////////////////////////////////////////////////////////////////////
