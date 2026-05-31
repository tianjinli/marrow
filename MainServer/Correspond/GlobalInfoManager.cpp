#include "GlobalInfoManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGlobalUserItem::CGlobalUserItem() {
  // 用户属性
  m_dwUserID = 0L;
  m_dwGameID = 0L;
  m_szNickName[0] = 0;

  // 等级信息
  m_cbMemberOrder = 0;
  m_cbMasterOrder = 0;

  // 链表属性
  m_pNextUserItemPtr = NULL;
}

// 析构函数
CGlobalUserItem::~CGlobalUserItem() {}

void CGlobalUserItem::UpdateStatus(const WORD wTableID, const WORD wChairID, const BYTE cbUserStatus) {
  m_UserInfo.wTableID = wTableID;
  m_UserInfo.wChairID = wChairID;
  m_UserInfo.cbUserStatus = cbUserStatus;
}

// 枚举房间
CGlobalServerItem* CGlobalUserItem::EnumServerItem(WORD wIndex) {
  if (wIndex >= m_GlobalServerItemArray.size()) {
    return NULL;
  }
  return m_GlobalServerItemArray[wIndex];
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGlobalPlazaItem::CGlobalPlazaItem() {
  // 设置变量
  m_wIndex = 0;
  ZeroMemory(&m_GamePlaza, sizeof(m_GamePlaza));

  // 链表属性
  m_pNextPlazaItemPtr = NULL;
}

// 析构函数
CGlobalPlazaItem::~CGlobalPlazaItem() {}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGlobalChatItem::CGlobalChatItem() {
  // 设置变量
  m_wIndex = 0;
  ZeroMemory(&m_ChatServer, sizeof(m_ChatServer));

  // 链表属性
  m_pNextChatServerPtr = NULL;
}

// 析构函数
CGlobalChatItem::~CGlobalChatItem() {}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGlobalServerItem::CGlobalServerItem() {
  // 设置变量
  m_wIndex = 0;
  ZeroMemory(&m_GameServer, sizeof(m_GameServer));

  // 链表属性
  m_pNextServerItemPtr = NULL;

  // 设置质数
  m_MapUserID.reserve(PRIME_SERVER_USER);
}

// 析构函数
CGlobalServerItem::~CGlobalServerItem() {}

// 寻找用户
CGlobalUserItem* CGlobalServerItem::SearchUserItem(DWORD dwUserID) {
  // 搜索用户
  auto itr = m_MapUserID.find(dwUserID);
  return itr != m_MapUserID.end() ? itr->second : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGlobalInfoManager::CGlobalInfoManager() {
  // 存储变量
  m_pGlobalUserItem = NULL;
  m_pGlobalPlazaItem = NULL;
  m_pGlobalServerItem = NULL;
  m_pGlobalChatItem = NULL;

  // 设置索引
  m_MapPlazaID.reserve(PRIME_SERVER);
  m_MapServerID.reserve(PRIME_SERVER);
  m_MapUserID.reserve(PRIME_PLATFORM_USER);
  m_MapGameID.reserve(PRIME_PLATFORM_USER);
  m_MapNickName.reserve(PRIME_PLATFORM_USER);
  m_MapChatID.reserve(PRIME_SERVER);
  // 约战房
  m_MapPersonalTableInfo.reserve(MAX_SERVER);
  m_ServerTableCountArray.clear();
}

// 析构函数
CGlobalInfoManager::~CGlobalInfoManager() {
  // 删除用户
  for (auto& pGlobalUserItem: m_MapUserID | std::views::values) {
    SafeDelete(pGlobalUserItem);
  }
  m_MapUserID.clear();

  // 删除广场
  for (auto& pGlobalPlazaItem: m_MapPlazaID | std::views::values) {
    SafeDelete(pGlobalPlazaItem);
  }
  m_MapPlazaID.clear();

  // 删除房间
  for (auto& pGlobalServerItem: m_MapServerID | std::views::values) {
    SafeDelete(pGlobalServerItem);
  }
  m_MapServerID.clear();

  // 删除聊天
  for (auto& pGlobalChatItem: m_MapChatID | std::views::values) {
    SafeDelete(pGlobalChatItem);
  }
  m_MapChatID.clear();

  // 删除用户
  CGlobalUserItem* pGlobalUserItem = nullptr;
  while (m_pGlobalUserItem != nullptr) {
    pGlobalUserItem = m_pGlobalUserItem;
    m_pGlobalUserItem = m_pGlobalUserItem->m_pNextUserItemPtr;
    SafeDelete(pGlobalUserItem);
  }

  // 删除广场
  CGlobalPlazaItem* pGlobalPlazaItem = nullptr;
  while (m_pGlobalPlazaItem != nullptr) {
    pGlobalPlazaItem = m_pGlobalPlazaItem;
    m_pGlobalPlazaItem = m_pGlobalPlazaItem->m_pNextPlazaItemPtr;
    SafeDelete(pGlobalPlazaItem);
  }

  // 删除房间
  CGlobalServerItem* pGlobalServerItem = nullptr;
  while (m_pGlobalServerItem != nullptr) {
    pGlobalServerItem = m_pGlobalServerItem;
    m_pGlobalServerItem = m_pGlobalServerItem->m_pNextServerItemPtr;
    SafeDelete(pGlobalServerItem);
  }

  // 删除聊天
  CGlobalChatItem* pGlobalChatItem = nullptr;
  while (m_pGlobalChatItem != nullptr) {
    pGlobalChatItem = m_pGlobalChatItem;
    m_pGlobalChatItem = m_pGlobalChatItem->m_pNextChatServerPtr;
    SafeDelete(pGlobalChatItem);
  }
}

// 重置数据
VOID CGlobalInfoManager::ResetData() {
  // 删除用户
  for (auto& pGlobalUserItem: m_MapUserID | std::views::values) {
    if (pGlobalUserItem != nullptr) {
      FreeGlobalUserItem(pGlobalUserItem);
    }
  }

  // 删除广场
  for (auto& pGlobalPlazaItem: m_MapPlazaID | std::views::values) {
    if (pGlobalPlazaItem != nullptr) {
      FreeGlobalPlazaItem(pGlobalPlazaItem);
    }
  }

  // 删除房间
  for (auto& pGlobalServerItem: m_MapServerID | std::views::values) {
    if (pGlobalServerItem != nullptr) {
      FreeGlobalServerItem(pGlobalServerItem);
    }
  }

  // 删除聊天
  for (auto& pGlobalChatItem: m_MapChatID | std::views::values) {
    if (pGlobalChatItem != nullptr) {
      FreeGlobalChatItem(pGlobalChatItem);
    }
  }

  // 删除索引
  m_MapUserID.clear();
  m_MapGameID.clear();
  m_MapPlazaID.clear();
  m_MapServerID.clear();
  m_MapNickName.clear();
  m_MapChatID.clear();
}

// 查找桌子
tagPersonalTableInfo* CGlobalInfoManager::SearchTableByID(LPCTSTR lpszTableID) {
  // 定义变量
  auto itr = m_MapPersonalTableInfo.find(lpszTableID);
  return itr != m_MapPersonalTableInfo.end() ? itr->second : nullptr;
}

tagPersonalTableInfo* CGlobalInfoManager::SearchTableByTableIDAndServerID(DWORD dwServerID, DWORD dwTableID) {
  // 移除桌子
  for (auto& pPersonalTable: m_MapPersonalTableInfo | std::views::values) {
    if (pPersonalTable != NULL && pPersonalTable->dwServerID == dwServerID && pPersonalTable->dwTableID == dwTableID) {
      return pPersonalTable;
    }
  }

  return nullptr;
}

// 添加桌子
bool CGlobalInfoManager::AddFreeServerTable(DWORD dwServerID) {
  // 查找房间
  CGlobalServerItem* pServerItem = SearchServerItem(static_cast<WORD>(dwServerID));
  if (pServerItem == NULL) {
    return true;
  }

  for (auto& pServerTableCount: m_ServerTableCountArray) {
    if (pServerTableCount->dwServerID == dwServerID) {
      // pServerTableCount->dwTableCount += (pServerItem->m_GameServer.wTableCount < pServerTableCount->dwTableCount + 1) ? 0 : 1;
      // 上面是精简之后的原始代码，存在一个桌子数永远不可能增加到服务器定义的最大数目的“漏洞”
      if (pServerTableCount->dwTableCount + 1 < pServerItem->m_GameServer.wTableCount) {
        pServerTableCount->dwTableCount += 1;
      }
      break;
    }
  }

  return true;
}

// 添加桌子
bool CGlobalInfoManager::AddServerTable(StringT strServerID, tagPersonalTableInfo PersonalTable) {
  // 添加桌子
  tagPersonalTableInfo* pPersonalTable = NULL;

  // 检查房主创建约战房间的数目，如果创建的数目大于最大数目则不允许创建
  if (GetHostCreatePersonalRoomCount(PersonalTable.dwUserID) > MAX_CREATE_PERSONAL_ROOM) {
    return false;
  }

  // 添加桌子
  pPersonalTable = NULL;
  if (m_MapPersonalTableInfo.find(strServerID) == m_MapPersonalTableInfo.end()) {
    pPersonalTable = new (std::nothrow) tagPersonalTableInfo;

    m_MapPersonalTableInfo[strServerID] = pPersonalTable;
    CopyMemory(pPersonalTable, &PersonalTable, sizeof(tagPersonalTableInfo));
  } else {
    return false;
  }

  return true;
}

// 移除空闲桌子
bool CGlobalInfoManager::RemoveFreeServerTable(DWORD dwServerID) {
  // 查找房间
  CGlobalServerItem* pServerItem = SearchServerItem(static_cast<WORD>(dwServerID));
  if (pServerItem == NULL) {
    return true;
  }

  for (auto& pServerTableCount: m_ServerTableCountArray) {
    if (pServerTableCount->dwServerID == dwServerID) {
      if (pServerTableCount->dwTableCount >= 1) {
        pServerTableCount->dwTableCount -= 1;
      }
      break;
    }
  }

  return true;
}

// 移除桌子
bool CGlobalInfoManager::RemoveServerTable(DWORD dwServerID, DWORD dwTableID) {
  // 移除桌子
  auto itr = std::ranges::find_if(m_MapPersonalTableInfo, [dwServerID, dwTableID](auto& kv) {
    auto pPersonalTable = kv.second;
    return pPersonalTable != NULL && pPersonalTable->dwServerID == dwServerID && pPersonalTable->dwTableID == dwTableID;
  });

  if (itr != m_MapPersonalTableInfo.end()) {
    auto pPersonalTable = itr->second;
    // 将被解散的坐桌放入解散桌子集合 约战房
    // 统计同一个房主被解散的房间数量
    int nOneHostDissumeCount = 0;
    for (int i = 0; i < m_VecDissumePersonalTableInfo.size(); i++) {
      if (m_VecDissumePersonalTableInfo[i].dwUserID == pPersonalTable->dwUserID) {
        nOneHostDissumeCount++;
      }
    }

    // 删除最先被解散的约战房
    if (nOneHostDissumeCount > MAX_CREATE_PERSONAL_ROOM) {
      int nOneHostDissumeCount = 0;
      for (int i = 0; i < m_VecDissumePersonalTableInfo.size(); i++) {
        if (m_VecDissumePersonalTableInfo[i].dwUserID == pPersonalTable->dwUserID) {
          nOneHostDissumeCount = i;
          m_VecDissumePersonalTableInfo.erase(m_VecDissumePersonalTableInfo.begin() + i);
          break;
        }
      }
    }
    m_VecDissumePersonalTableInfo.push_back(*pPersonalTable);

    SafeDelete(pPersonalTable);
    m_MapPersonalTableInfo.erase(itr);
    return true;
  }
  return false;
}

// 移除桌子
bool CGlobalInfoManager::RemoveServerTable(DWORD dwServerID) {
  // 移除桌子
  std::vector<tagPersonalTableInfo*> pPersonalTables;
  std::erase_if(m_MapPersonalTableInfo, [&pPersonalTables, dwServerID](auto& kv) {
    auto pPersonalTable = kv.second;
    if (pPersonalTable != NULL && pPersonalTable->dwServerID == dwServerID) {
      pPersonalTables.emplace_back(pPersonalTable);
      return true;
    }
    return false;
  });

  for (auto& pPersonalTable: pPersonalTables) {
    SafeDelete(pPersonalTable);
  }
  return true;
}


// 获取房主创建的所有房间
VOID CGlobalInfoManager::GetHostCreatePersonalRoom(tagHostCreatRoomInfo& HostCreatRoomInfo) {
  // 移除桌子
  int iHostCreateRoomCount = 0;
  for (auto& pPersonalTable: m_MapPersonalTableInfo | std::views::values) {
    if (pPersonalTable != NULL && pPersonalTable->dwUserID == HostCreatRoomInfo.dwUserID && pPersonalTable->dwKindID == HostCreatRoomInfo.dwKindID) {
      lstrcpyn(HostCreatRoomInfo.szRoomID[iHostCreateRoomCount], pPersonalTable->szRoomID, CountArray(pPersonalTable->szRoomID));
      iHostCreateRoomCount++;
      // 大于最大房间数目返回
      if (iHostCreateRoomCount >= MAX_CREATE_PERSONAL_ROOM) {
        break;
      }
    }
  }

  // 加上已经解散的桌子
  for (int i = m_VecDissumePersonalTableInfo.size() - 1; i >= 0; i--) {
    if (m_VecDissumePersonalTableInfo[i].dwUserID == HostCreatRoomInfo.dwUserID &&
        m_VecDissumePersonalTableInfo[i].dwKindID == HostCreatRoomInfo.dwKindID) {
      lstrcpyn(HostCreatRoomInfo.szRoomID[iHostCreateRoomCount], m_VecDissumePersonalTableInfo[i].szRoomID,
               CountArray(m_VecDissumePersonalTableInfo[i].szRoomID));
      iHostCreateRoomCount++;
      if (iHostCreateRoomCount >= MAX_CREATE_PERSONAL_ROOM) {
        break;
      }
    }
  }
}


// 获取房主创建的房间的数量
INT CGlobalInfoManager::GetHostCreatePersonalRoomCount(DWORD dwUserID) {
  return std::ranges::count_if(m_MapPersonalTableInfo | std::views::values, [dwUserID](const auto& info) {
    return info != nullptr && info->dwUserID == dwUserID;
  });
}

// 获取房间
DWORD CGlobalInfoManager::GetFreeServer(DWORD dwUserID, DWORD dwKindID, BYTE cbIsJoinGame) {
  // 变量定义
  bool bExit = false;
  StringT strServerID;

  // 如果房主参与游戏
  if (cbIsJoinGame) {
    for (auto& pPersonalTableInfo: m_MapPersonalTableInfo | std::views::values) {
      if (pPersonalTableInfo != NULL && pPersonalTableInfo->dwUserID == dwUserID) {
        return 0;
      }
    }
  }

  // 查找房间
  INT_PTR nSize = m_ServerTableCountArray.size();
  for (INT_PTR i = 0; i < nSize; ++i) {
    tagServerTableCount* pServerTableCount = m_ServerTableCountArray[i];
    if (pServerTableCount->dwKindID == dwKindID && pServerTableCount->dwTableCount > 0) {
      return pServerTableCount->dwServerID;
    }
  }
  return 0;
}

// 生成房间ID
VOID CGlobalInfoManager::RandServerID(LPTSTR pszServerID, WORD wMaxCount) {
  // 定义变量
  TCHAR szScource[11] = TEXT("0123456789");
  std::vector<TCHAR> lpszTmp(wMaxCount + 1);

  bool bExit = true;
  while (bExit) {
    // 生成ID
    for (int i = 0; i < wMaxCount; ++i) {
      lpszTmp[i] = szScource[rand() % 10];
    }

    // 查找ID
    bExit = (m_MapPersonalTableInfo.find(lpszTmp.data()) != m_MapPersonalTableInfo.end());
  }

  // 字符转换
  lstrcpyn(pszServerID, lpszTmp.data(), wMaxCount);
}

// 删除用户
bool CGlobalInfoManager::DeleteUserItem(DWORD dwUserID, WORD wServerID) {
  // 寻找用户
  CGlobalUserItem* pGlobalUserItem = SearchUserItemByUserID(dwUserID);
  if (pGlobalUserItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 变量定义
  CGlobalServerItem* pGlobalServerItem = nullptr;
  INT_PTR nServerCount = pGlobalUserItem->m_GlobalServerItemArray.size();

  // 退出房间
  for (INT_PTR i = 0; i < nServerCount; i++) {
    // 获取房间
    pGlobalServerItem = pGlobalUserItem->m_GlobalServerItemArray[i];

    // 房间判断
    if (pGlobalServerItem->GetServerID() == wServerID) {
      // 删除关联
      pGlobalServerItem->m_MapUserID.erase(dwUserID);
      pGlobalUserItem->m_GlobalServerItemArray.erase(pGlobalUserItem->m_GlobalServerItemArray.begin() + i);

      // 释放用户
      if (pGlobalUserItem->m_GlobalServerItemArray.size() == 0) {
        // 释放索引
        m_MapUserID.erase(dwUserID);
        m_MapGameID.erase(pGlobalUserItem->GetGameID());
        m_MapNickName.erase(pGlobalUserItem->GetNickName());

        // 释放对象
        FreeGlobalUserItem(pGlobalUserItem);
      }

      return true;
    }
  }

  // 错误断言
  ASSERT(FALSE);

  return false;
}

// 激活用户
bool CGlobalInfoManager::ActiveUserItem(tagGlobalUserInfo& GlobalUserInfo, WORD wServerID) {
  // 寻找房间
  CGlobalServerItem* pGlobalServerItem = SearchServerItem(wServerID);
  if (pGlobalServerItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 寻找用户
  CGlobalUserItem* pGlobalUserItem = SearchUserItemByUserID(GlobalUserInfo.dwUserID);
  if (pGlobalUserItem == nullptr) {
    // 创建用户
    pGlobalUserItem = CreateGlobalUserItem();
    if (pGlobalUserItem == nullptr) {
      ASSERT(FALSE);
      return false;
    }

    // 构造昵称
    TCHAR szNickName[LEN_NICKNAME] = TEXT("");
    lstrcpyn(szNickName, GlobalUserInfo.szNickName, CountArray(szNickName));

    // 设置用户
    pGlobalUserItem->m_dwUserID = GlobalUserInfo.dwUserID;
    pGlobalUserItem->m_dwGameID = GlobalUserInfo.dwGameID;
    lstrcpyn(pGlobalUserItem->m_szNickName, GlobalUserInfo.szNickName, CountArray(pGlobalUserItem->m_szNickName));

    // 辅助信息
    pGlobalUserItem->m_cbGender = GlobalUserInfo.cbGender;
    pGlobalUserItem->m_cbMemberOrder = GlobalUserInfo.cbMemberOrder;
    pGlobalUserItem->m_cbMasterOrder = GlobalUserInfo.cbMasterOrder;
    memcpy(&pGlobalUserItem->m_UserInfo, &GlobalUserInfo.userInfo, sizeof(tagUserInfo));

    // 昵称索引
    m_MapNickName[szNickName] = pGlobalUserItem;

    // 设置索引
    m_MapUserID[GlobalUserInfo.dwUserID] = pGlobalUserItem;
    m_MapGameID[GlobalUserInfo.dwGameID] = pGlobalUserItem;
  } else {
    // 重复判断
    for (INT_PTR i = 0; i < pGlobalUserItem->m_GlobalServerItemArray.size(); i++) {
      if (pGlobalUserItem->m_GlobalServerItemArray[i]->GetServerID() == wServerID) {
        ASSERT(FALSE);
        return false;
      }
    }
  }

  // 设置关联
  pGlobalUserItem->m_GlobalServerItemArray.emplace_back(pGlobalServerItem);
  pGlobalServerItem->m_MapUserID[GlobalUserInfo.dwUserID] = pGlobalUserItem;

  return true;
}

// 删除广场
bool CGlobalInfoManager::DeletePlazaItem(WORD wPlazaID) {
  // 寻找广场
  CGlobalPlazaItem* pGlobalPlazaItem = SearchPlazaItem(wPlazaID);
  if (pGlobalPlazaItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 释放广场
  m_MapPlazaID.erase(wPlazaID);
  FreeGlobalPlazaItem(pGlobalPlazaItem);
  return true;
}

// 激活广场
bool CGlobalInfoManager::ActivePlazaItem(WORD wBindIndex, tagGamePlaza& GamePlaza) {
  // 寻找广场
  CGlobalPlazaItem* pGlobalPlazaItem = SearchPlazaItem(GamePlaza.wPlazaID);
  if (pGlobalPlazaItem != nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 创建广场
  pGlobalPlazaItem = CreateGlobalPlazaItem();
  if (pGlobalPlazaItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 设置广场
  pGlobalPlazaItem->m_wIndex = wBindIndex;
  pGlobalPlazaItem->m_GamePlaza = GamePlaza;

  // 设置索引
  m_MapPlazaID[GamePlaza.wPlazaID] = pGlobalPlazaItem;

  return true;
}

// 删除房间
bool CGlobalInfoManager::DeleteServerItem(WORD wServerID) {
  // 寻找房间
  CGlobalServerItem* pGlobalServerItem = SearchServerItem(wServerID);
  if (pGlobalServerItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }


  // 删除用户
  for (auto& [dwUserKey, pGlobalUserItem]: pGlobalServerItem->m_MapUserID) {
    // 房间关联
    for (INT_PTR i = 0; i < pGlobalUserItem->m_GlobalServerItemArray.size(); i++) {
      // 获取房间
      CGlobalServerItem* pTempServerItem = pGlobalUserItem->m_GlobalServerItemArray[i];

      // 房间判断
      if (pTempServerItem->GetServerID() == wServerID) {
        pGlobalUserItem->m_GlobalServerItemArray.erase(pGlobalUserItem->m_GlobalServerItemArray.begin() + i);
        break;
      }
    }

    // 释放用户
    if (pGlobalUserItem->m_GlobalServerItemArray.size() == 0) {
      m_MapUserID.erase(dwUserKey);
      FreeGlobalUserItem(pGlobalUserItem);
    }
  }

  // 释放房间
  m_MapServerID.erase(wServerID);
  FreeGlobalServerItem(pGlobalServerItem);

  // 查找房间
  auto itr = std::ranges::find_if(m_ServerTableCountArray, [wServerID](const auto& pServerTableCount) {
    return pServerTableCount != nullptr && pServerTableCount->dwServerID == wServerID;
  });
  if (itr != m_ServerTableCountArray.end()) {
    SafeDelete(*itr);
    m_ServerTableCountArray.erase(itr);
  }

  // 删除用户信息
  std::vector<tagPersonalTableInfo*> pPersonalTableInfos;
  std::erase_if(m_MapPersonalTableInfo, [&pPersonalTableInfos, wServerID](auto& pair) {
    auto pPersonalTableInfo = pair.second;
    if (pPersonalTableInfo != NULL && pPersonalTableInfo->dwServerID == wServerID) {
      pPersonalTableInfos.emplace_back(pPersonalTableInfo);
      return true;
    }
    return false;
  });
  for (auto& pPersonalTableInfo: pPersonalTableInfos) {
    SafeDelete(pPersonalTableInfo);
  }

  // 删除用户信息
  RemoveServerTable(wServerID);
  return true;
}

// 激活房间
bool CGlobalInfoManager::ActiveServerItem(WORD wBindIndex, tagGameServer& GameServer) {
  // 寻找房间
  CGlobalServerItem* pGlobalServerItem = SearchServerItem(GameServer.wServerID);
  if (pGlobalServerItem != nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 创建房间
  pGlobalServerItem = CreateGlobalServerItem();
  if (pGlobalServerItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 设置房间
  pGlobalServerItem->m_wIndex = wBindIndex;
  pGlobalServerItem->m_GameServer = GameServer;

  // 设置索引
  m_MapServerID[GameServer.wServerID] = pGlobalServerItem;

  // 插入约战房间
  DWORD dwServerID = pGlobalServerItem->GetServerID();
  DWORD dwTableCount = pGlobalServerItem->GetTabelCount();

  // 查找房间
  INT_PTR nSize = m_ServerTableCountArray.size();
  bool bExit = false;
  for (INT_PTR i = 0; i < nSize; ++i) {
    tagServerTableCount* pServerTableCount = m_ServerTableCountArray[i];
    if (pServerTableCount != NULL && pServerTableCount->dwServerID == dwServerID) {
      bExit = true;
      break;
    }
  }

  if (bExit == false && pGlobalServerItem->m_GameServer.wServerType == GAME_GENRE_PERSONAL) {
    tagServerTableCount* pServerTableCount = new tagServerTableCount;
    pServerTableCount->dwKindID = pGlobalServerItem->GetKindID();
    pServerTableCount->dwServerID = dwServerID;
    pServerTableCount->dwTableCount = dwTableCount;

    m_ServerTableCountArray.emplace_back(pServerTableCount);
  }

  return true;
}

// 删除聊天
bool CGlobalInfoManager::DeleteChatItem(WORD wChatID) {
  // 寻找广场
  CGlobalChatItem* pGlobalChatItem = SearchChatItem(wChatID);
  if (pGlobalChatItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 释放广场
  m_MapChatID.erase(wChatID);
  FreeGlobalChatItem(pGlobalChatItem);

  return true;
}

// 激活聊天
bool CGlobalInfoManager::ActiveChatItem(WORD wBindIndex, tagChatServer& ChatServer) {
  // 寻找房间
  CGlobalChatItem* pGlobalChatItem = SearchChatItem(ChatServer.wChatID);
  if (pGlobalChatItem != nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 创建房间
  pGlobalChatItem = CreateGlobalChatItem();
  if (pGlobalChatItem == nullptr) {
    ASSERT(FALSE);
    return false;
  }

  // 设置房间
  pGlobalChatItem->m_wIndex = wBindIndex;
  pGlobalChatItem->m_ChatServer = ChatServer;

  // 设置索引
  m_MapChatID[ChatServer.wChatID] = pGlobalChatItem;

  return true;
}

// 寻找广场
CGlobalPlazaItem* CGlobalInfoManager::SearchPlazaItem(WORD wPlazaID) {
  // 寻找房间
  auto itr = m_MapPlazaID.find(wPlazaID);
  return itr != m_MapPlazaID.end() ? itr->second : nullptr;
}

// 寻找房间
CGlobalServerItem* CGlobalInfoManager::SearchServerItem(WORD wServerID) {
  // 寻找房间
  auto itr = m_MapServerID.find(wServerID);
  return itr != m_MapServerID.end() ? itr->second : nullptr;
}

// 寻找聊天
CGlobalChatItem* CGlobalInfoManager::SearchChatItem(WORD wChatID) {
  // 寻找房间
  auto itr = m_MapChatID.find(wChatID);
  return itr != m_MapChatID.end() ? itr->second : nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByUserID(DWORD dwUserID) {
  // 寻找用户
  auto itr = m_MapUserID.find(dwUserID);
  return itr != m_MapUserID.end() ? itr->second : nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByGameID(DWORD dwGameID) {
  // 寻找用户
  auto itr = m_MapGameID.find(dwGameID);
  return itr != m_MapGameID.end() ? itr->second : nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByNickName(LPCTSTR pszNickName) {
  // 寻找用户
  auto itr = m_MapNickName.find(pszNickName);
  return itr != m_MapNickName.end() ? itr->second : nullptr;
}

// 枚举用户
CGlobalUserItem* CGlobalInfoManager::EnumUserItem(POSITION& Position) {
  // 变量定义
  CGlobalUserItem* pGlobalUserItem = NULL;
  auto itr = Position.empty() ? m_MapUserID.begin() : Position.get<CMapUserID::iterator>();

  // 获取对象
  if (itr != m_MapUserID.end()) {
    pGlobalUserItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGlobalUserItem;
}

// 枚举广场
CGlobalPlazaItem* CGlobalInfoManager::EnumPlazaItem(POSITION& Position) {
  // 变量定义
  CGlobalPlazaItem* pGlobalPlazaItem = NULL;
  auto itr = Position.empty() ? m_MapPlazaID.begin() : Position.get<CMapPlazaID::iterator>();

  // 获取对象
  if (itr != m_MapPlazaID.end()) {
    pGlobalPlazaItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGlobalPlazaItem;
}

// 枚举房间
CGlobalServerItem* CGlobalInfoManager::EnumServerItem(POSITION& Position) {
  // 变量定义
  CGlobalServerItem* pGlobalServerItem = NULL;
  auto itr = Position.empty() ? m_MapServerID.begin() : Position.get<CMapServerID::iterator>();

  // 获取对象
  if (itr != m_MapServerID.end()) {
    pGlobalServerItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGlobalServerItem;
}

// 枚举聊天
CGlobalChatItem* CGlobalInfoManager::EnumChatItem(POSITION& Position) {
  // 变量定义
  CGlobalChatItem* pGlobalChatItem = NULL;
  auto itr = Position.empty() ? m_MapChatID.begin() : Position.get<CMapChatID::iterator>();

  // 获取对象
  if (itr != m_MapChatID.end()) {
    pGlobalChatItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGlobalChatItem;
}

// 创建用户
CGlobalUserItem* CGlobalInfoManager::CreateGlobalUserItem() {
  // 使用存储
  if (m_pGlobalUserItem != NULL) {
    CGlobalUserItem* pGlobalUserItem = m_pGlobalUserItem;
    m_pGlobalUserItem = m_pGlobalUserItem->m_pNextUserItemPtr;
    pGlobalUserItem->m_pNextUserItemPtr = NULL;
    return pGlobalUserItem;
  }

  // 创建对象
  CGlobalUserItem* pGlobalUserItem = new (std::nothrow) CGlobalUserItem();
  return pGlobalUserItem;
}

// 创建广场
CGlobalPlazaItem* CGlobalInfoManager::CreateGlobalPlazaItem() {
  // 使用存储
  if (m_pGlobalPlazaItem != NULL) {
    CGlobalPlazaItem* pGlobalPlazaItem = m_pGlobalPlazaItem;
    m_pGlobalPlazaItem = m_pGlobalPlazaItem->m_pNextPlazaItemPtr;
    pGlobalPlazaItem->m_pNextPlazaItemPtr = NULL;
    return pGlobalPlazaItem;
  }

  // 创建对象
  CGlobalPlazaItem* pGlobalPlazaItem = new (std::nothrow) CGlobalPlazaItem;
  return pGlobalPlazaItem;
}

// 创建房间
CGlobalServerItem* CGlobalInfoManager::CreateGlobalServerItem() {
  // 使用存储
  if (m_pGlobalServerItem != NULL) {
    CGlobalServerItem* pGlobalServerItem = m_pGlobalServerItem;
    m_pGlobalServerItem = m_pGlobalServerItem->m_pNextServerItemPtr;
    pGlobalServerItem->m_pNextServerItemPtr = NULL;
    return pGlobalServerItem;
  }

  // 创建对象
  CGlobalServerItem* pGlobalServerItem = new (std::nothrow) CGlobalServerItem();
  return pGlobalServerItem;
}

// 创建聊天
CGlobalChatItem* CGlobalInfoManager::CreateGlobalChatItem() {
  // 使用存储
  if (m_pGlobalChatItem != NULL) {
    CGlobalChatItem* pGlobalChatItem = m_pGlobalChatItem;
    m_pGlobalChatItem = m_pGlobalChatItem->m_pNextChatServerPtr;
    pGlobalChatItem->m_pNextChatServerPtr = NULL;
    return pGlobalChatItem;
  }

  // 创建对象
  CGlobalChatItem* pGlobalChatItem = new (std::nothrow) CGlobalChatItem;
  return pGlobalChatItem;
}

// 释放用户
bool CGlobalInfoManager::FreeGlobalUserItem(CGlobalUserItem* pGlobalUserItem) {
  // 效验参数
  ASSERT(pGlobalUserItem != NULL);
  if (pGlobalUserItem == NULL) {
    return false;
  }

  // 设置变量
  pGlobalUserItem->m_dwUserID = 0L;
  pGlobalUserItem->m_dwGameID = 0L;
  pGlobalUserItem->m_szNickName[0] = 0;
  ZeroMemory(&pGlobalUserItem->m_UserInfo, sizeof(tagUserInfo));
  pGlobalUserItem->m_GlobalServerItemArray.clear();

  // 加入存储
  pGlobalUserItem->m_pNextUserItemPtr = m_pGlobalUserItem;
  m_pGlobalUserItem = pGlobalUserItem;

  return true;
}

// 释放广场
bool CGlobalInfoManager::FreeGlobalPlazaItem(CGlobalPlazaItem* pGlobalPlazaItem) {
  // 效验参数
  ASSERT(pGlobalPlazaItem != NULL);
  if (pGlobalPlazaItem == NULL) {
    return false;
  }

  // 设置变量
  pGlobalPlazaItem->m_wIndex = 0;
  ZeroMemory(&pGlobalPlazaItem->m_GamePlaza, sizeof(pGlobalPlazaItem->m_GamePlaza));

  // 加入存储
  pGlobalPlazaItem->m_pNextPlazaItemPtr = m_pGlobalPlazaItem;
  m_pGlobalPlazaItem = pGlobalPlazaItem;

  return true;
}

// 释放房间
bool CGlobalInfoManager::FreeGlobalServerItem(CGlobalServerItem* pGlobalServerItem) {
  // 效验参数
  ASSERT(pGlobalServerItem != NULL);
  if (pGlobalServerItem == NULL) {
    return false;
  }

  // 设置索引
  pGlobalServerItem->m_MapUserID.clear();

  // 设置变量
  pGlobalServerItem->m_wIndex = 0;
  ZeroMemory(&pGlobalServerItem->m_GameServer, sizeof(pGlobalServerItem->m_GameServer));

  // 加入存储
  pGlobalServerItem->m_pNextServerItemPtr = m_pGlobalServerItem;
  m_pGlobalServerItem = pGlobalServerItem;

  return true;
}

// 释放聊天
bool CGlobalInfoManager::FreeGlobalChatItem(CGlobalChatItem* pGlobalChatItem) {
  // 效验参数
  ASSERT(pGlobalChatItem != NULL);
  if (pGlobalChatItem == NULL) {
    return false;
  }

  // 设置变量
  pGlobalChatItem->m_wIndex = 0;
  ZeroMemory(&pGlobalChatItem->m_ChatServer, sizeof(pGlobalChatItem->m_ChatServer));

  // 加入存储
  pGlobalChatItem->m_pNextChatServerPtr = m_pGlobalChatItem;
  m_pGlobalChatItem = pGlobalChatItem;

  return true;
}

// 添加一种游戏最多创建约战房间的数目
bool CGlobalInfoManager::AddPersonalMaxCreate(CMD_CS_S_RegisterPersonal RegisterPersonal) {
  int iCount = m_vecPersonalRoomMaxCreate.size();

  // 遍历，如果不存在则加入， 如果存在则修改
  bool bIsExist = false;
  for (int i = 0; i < iCount; i++) {
    if (m_vecPersonalRoomMaxCreate[i].dwKindID == RegisterPersonal.dwKindID) {
      m_vecPersonalRoomMaxCreate[i].dwMaxCreate = RegisterPersonal.dwMaxCreate;
      bIsExist = true;
      break;
    }
  }

  if (!bIsExist) {
    m_vecPersonalRoomMaxCreate.push_back(RegisterPersonal);
  }

  return true;
}

// 是否可以再创建房间
bool CGlobalInfoManager::CanCreatePersonalRoom(DWORD dwKindID, DWORD dwUserID) {
  // 获取可以创建房间的最大数目
  int iCount = m_vecPersonalRoomMaxCreate.size();
  int iMaxCreateCount = 0;
  for (int i = 0; i < iCount; i++) {
    if (m_vecPersonalRoomMaxCreate[i].dwKindID == dwKindID) {
      iMaxCreateCount = m_vecPersonalRoomMaxCreate[i].dwMaxCreate;
      break;
    }
  }

  if (iMaxCreateCount == 0) {
    iMaxCreateCount = MAX_CREATE_COUNT;
  }

  // 获得约战房间数目
  int iHaveCreate = GetHostCreatePersonalRoomCount(dwUserID);
  if (iHaveCreate < iMaxCreateCount) {
    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////
