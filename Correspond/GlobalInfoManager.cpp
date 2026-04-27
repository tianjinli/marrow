#include "GlobalInfoManager.h"
#include "CorrespondHeader.h"

//////////////////////////////////////////////////////////////////////////////////
void CGlobalUserItem::UpdateStatus(const WORD wTableID, const WORD wChairID, const BYTE cbUserStatus) {
  m_UserInfo.wTableID = wTableID;
  m_UserInfo.wChairID = wChairID;
  m_UserInfo.cbUserStatus = cbUserStatus;
}

// 枚举房间
CGlobalServerItem* CGlobalUserItem::EnumServerItem(WORD wIndex) {
  if (wIndex >= m_GlobalServerItemArray.GetCount())
    return nullptr;
  return m_GlobalServerItemArray[wIndex];
}

//////////////////////////////////////////////////////////////////////////////////
// 寻找用户
CGlobalUserItem* CGlobalServerItem::SearchUserItem(DWORD dwUserID) {
  // 搜索用户
  auto itr = m_MapUserID.find(dwUserID);
  if (itr != m_MapUserID.end()) {
    return itr->second;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CGlobalInfoManager::~CGlobalInfoManager() {
  // 删除用户
  for (auto item : m_MapUserID) {
    SafeDelete(item.second);
  }
  m_MapUserID.clear();

  // 删除广场
  for (auto item : m_MapPlazaID) {
    SafeDelete(item.second);
  }
  m_MapPlazaID.clear();

  // 删除房间
  for (auto item : m_MapServerID) {
    SafeDelete(item.second);
  }
  m_MapServerID.clear();

  // 删除聊天
  for (auto item : m_MapChatID) {
    SafeDelete(item.second);
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
  for (auto item : m_MapUserID) {
    if (item.second != nullptr)
      FreeGlobalUserItem(item.second);
  }

  // 删除广场
  for (auto item : m_MapPlazaID) {
    if (item.second != nullptr)
      FreeGlobalPlazaItem(item.second);
  }

  // 删除房间
  for (auto item : m_MapServerID) {
    if (item.second != nullptr)
      FreeGlobalServerItem(item.second);
  }

  // 删除聊天
  for (auto item : m_MapChatID) {
    if (item.second != nullptr)
      FreeGlobalChatItem(item.second);
  }

  // 删除索引
  m_MapUserID.clear();
  m_MapGameID.clear();
  m_MapPlazaID.clear();
  m_MapServerID.clear();
  m_MapNickName.clear();
  m_MapChatID.clear();
}

// 删除用户
bool CGlobalInfoManager::DeleteUserItem(DWORD dwUserID, WORD wServerID) {
  // 寻找用户
  auto itr = m_MapUserID.find(dwUserID);
  if (itr == m_MapUserID.end()) {
    ASSERT(FALSE);
    return false;
  }

  // 变量定义
  CGlobalServerItem* pGlobalServerItem = nullptr;
  CGlobalUserItem* pGlobalUserItem = itr->second;
  INT_PTR nServerCount = pGlobalUserItem->m_GlobalServerItemArray.GetCount();

  // 退出房间
  for (INT_PTR i = 0; i < nServerCount; i++) {
    // 获取房间
    pGlobalServerItem = pGlobalUserItem->m_GlobalServerItemArray[i];

    // 房间判断
    if (pGlobalServerItem->GetServerID() == wServerID) {
      // 删除关联
      pGlobalServerItem->m_MapUserID.erase(dwUserID);
      pGlobalUserItem->m_GlobalServerItemArray.RemoveAt(i);

      // 释放用户
      if (pGlobalUserItem->m_GlobalServerItemArray.GetCount() == 0) {
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
  CGlobalServerItem* pGlobalServerItem = nullptr;
  auto it = m_MapServerID.find(wServerID);
  if (it == m_MapServerID.end()) {
    ASSERT(FALSE);
    return false;
  }

  // 寻找用户
  CGlobalUserItem* pGlobalUserItem = nullptr;
  auto itr = m_MapUserID.find(GlobalUserInfo.dwUserID);
  if (itr == m_MapUserID.end()) {
    // 创建用户
    pGlobalUserItem = CreateGlobalUserItem();
    if (pGlobalUserItem == nullptr) {
      ASSERT(FALSE);
      return false;
    }

    // 构造昵称
    TCHAR szNickName[LEN_NICKNAME] = TEXT("");
    LSTRCPYN(szNickName, GlobalUserInfo.szNickName, std::size(szNickName));

    // 设置用户
    pGlobalUserItem->m_dwUserID = GlobalUserInfo.dwUserID;
    pGlobalUserItem->m_dwGameID = GlobalUserInfo.dwGameID;
    LSTRCPYN(pGlobalUserItem->m_szNickName, GlobalUserInfo.szNickName, std::size(pGlobalUserItem->m_szNickName));

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
    for (INT_PTR i = 0; i < pGlobalUserItem->m_GlobalServerItemArray.GetCount(); i++) {
      if (pGlobalUserItem->m_GlobalServerItemArray[i]->GetServerID() == wServerID) {
        ASSERT(FALSE);
        return false;
      }
    }
  }

  // 设置关联
  pGlobalUserItem->m_GlobalServerItemArray.Add(pGlobalServerItem);
  pGlobalServerItem->m_MapUserID[GlobalUserInfo.dwUserID] = pGlobalUserItem;

  return true;
}

// 删除广场
bool CGlobalInfoManager::DeletePlazaItem(WORD wPlazaID) {
  // 寻找广场
  auto itr = m_MapPlazaID.find(wPlazaID);
  if (itr == m_MapPlazaID.end()) {
    ASSERT(FALSE);
    return false;
  }

  // 释放广场
  CGlobalPlazaItem* pGlobalPlazaItem = itr->second;
  m_MapPlazaID.erase(wPlazaID);
  FreeGlobalPlazaItem(pGlobalPlazaItem);

  return true;
}

// 激活广场
bool CGlobalInfoManager::ActivePlazaItem(WORD wBindIndex, tagGamePlaza& GamePlaza) {
  // 寻找广场
  CGlobalPlazaItem* pGlobalPlazaItem = nullptr;
  auto itr = m_MapPlazaID.find(GamePlaza.wPlazaID);
  if (itr != m_MapPlazaID.end()) {
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
  auto itr = m_MapServerID.find(wServerID);
  if (itr == m_MapServerID.end()) {
    ASSERT(FALSE);
    return false;
  }

  // 变量定义
  DWORD dwUserKey = 0;
  CGlobalUserItem* pGlobalUserItem = nullptr;
  CGlobalServerItem* pGlobalServerItem = itr->second;

  // 删除用户
  for (auto itr = pGlobalServerItem->m_MapUserID.begin(); itr != pGlobalServerItem->m_MapUserID.end(); ++itr) {
    // 获取数据
    dwUserKey = itr->first;
    pGlobalUserItem = itr->second;

    // 房间关联
    for (INT_PTR i = 0; i < pGlobalUserItem->m_GlobalServerItemArray.GetCount(); i++) {
      // 获取房间
      CGlobalServerItem* pTempServerItem = pGlobalUserItem->m_GlobalServerItemArray[i];

      // 房间判断
      if (pTempServerItem->GetServerID() == wServerID) {
        pGlobalUserItem->m_GlobalServerItemArray.RemoveAt(i);
        break;
      }
    }

    // 释放用户
    if (pGlobalUserItem->m_GlobalServerItemArray.GetCount() == 0) {
      m_MapUserID.erase(dwUserKey);
      FreeGlobalUserItem(pGlobalUserItem);
    }
  }

  // 释放房间
  m_MapServerID.erase(wServerID);
  FreeGlobalServerItem(pGlobalServerItem);

  return true;
}

// 激活房间
bool CGlobalInfoManager::ActiveServerItem(WORD wBindIndex, tagGameServer& GameServer) {
  // 寻找房间
  CGlobalServerItem* pGlobalServerItem = nullptr;
  auto itr = m_MapServerID.find(GameServer.wServerID);
  if (itr != m_MapServerID.end()) {
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

  return true;
}

// 删除聊天
bool CGlobalInfoManager::DeleteChatItem(WORD wChatID) {
  // 寻找广场
  auto itr = m_MapChatID.find(wChatID);
  if (itr == m_MapChatID.end()) {
    ASSERT(FALSE);
    return false;
  }

  // 释放广场
  CGlobalChatItem* pGlobalChatItem = itr->second;
  m_MapChatID.erase(wChatID);
  FreeGlobalChatItem(pGlobalChatItem);

  return true;
}

// 激活聊天
bool CGlobalInfoManager::ActiveChatItem(WORD wBindIndex, tagChatServer& ChatServer) {
  // 寻找房间
  CGlobalChatItem* pGlobalChatItem = nullptr;
  auto itr = m_MapChatID.find(ChatServer.wChatID);
  if (itr != m_MapChatID.end()) {
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
  if (itr != m_MapPlazaID.end()) {
    return itr->second;
  }

  return nullptr;
}

// 寻找房间
CGlobalServerItem* CGlobalInfoManager::SearchServerItem(WORD wServerID) {
  // 寻找房间
  auto itr = m_MapServerID.find(wServerID);
  if (itr != m_MapServerID.end()) {
    return itr->second;
  }

  return nullptr;
}

// 寻找聊天
CGlobalChatItem* CGlobalInfoManager::SearchChatItem(WORD wChatID) {
  // 寻找房间
  auto itr = m_MapChatID.find(wChatID);
  if (itr != m_MapChatID.end()) {
    return itr->second;
  }

  return nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByUserID(DWORD dwUserID) {
  // 寻找用户
  auto itr = m_MapUserID.find(dwUserID);
  if (itr != m_MapUserID.end()) {
    return itr->second;
  }

  return nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByGameID(DWORD dwGameID) {
  // 寻找用户
  auto itr = m_MapGameID.find(dwGameID);
  if (itr != m_MapGameID.end()) {
    return itr->second;
  }

  return nullptr;
}

// 寻找用户
CGlobalUserItem* CGlobalInfoManager::SearchUserItemByNickName(LPCTSTR pszNickName) {
  // 寻找用户
  auto itr = m_MapNickName.find(pszNickName);
  if (itr != m_MapNickName.end()) {
    return itr->second;
  }

  return nullptr;
}

// 枚举用户
CGlobalUserItem* CGlobalInfoManager::EnumUserItem(CMapUserID::iterator* itr) {
  // 变量定义
  DWORD dwUserID = 0;

  auto cur = m_MapUserID.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_MapUserID.end()) {
    dwUserID = cur->first;
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举广场
CGlobalPlazaItem* CGlobalInfoManager::EnumPlazaItem(CMapPlazaID::iterator* itr) {
  // 变量定义
  WORD wPlazaID = 0;

  auto cur = m_MapPlazaID.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_MapPlazaID.end()) {
    wPlazaID = cur->first;
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举房间
CGlobalServerItem* CGlobalInfoManager::EnumServerItem(CMapServerID::iterator* itr) {
  // 变量定义
  WORD wServerID = 0;

  auto cur = m_MapServerID.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_MapServerID.end()) {
    wServerID = cur->first;
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举聊天
CGlobalChatItem* CGlobalInfoManager::EnumChatItem(CMapChatID::iterator* itr) {
  // 变量定义
  WORD wChatID = 0;

  auto cur = m_MapChatID.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  // 获取对象
  if (cur != m_MapChatID.end()) {
    wChatID = cur->first;
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 创建用户
CGlobalUserItem* CGlobalInfoManager::CreateGlobalUserItem() {
  // 使用存储
  if (m_pGlobalUserItem != nullptr) {
    CGlobalUserItem* pGlobalUserItem = m_pGlobalUserItem;
    m_pGlobalUserItem = m_pGlobalUserItem->m_pNextUserItemPtr;
    pGlobalUserItem->m_pNextUserItemPtr = nullptr;
    return pGlobalUserItem;
  }

  // 创建对象
  try {
    CGlobalUserItem* pGlobalUserItem = new CGlobalUserItem;
    return pGlobalUserItem;
  } catch (...) {
  }

  return nullptr;
}

// 创建广场
CGlobalPlazaItem* CGlobalInfoManager::CreateGlobalPlazaItem() {
  // 使用存储
  if (m_pGlobalPlazaItem != nullptr) {
    CGlobalPlazaItem* pGlobalPlazaItem = m_pGlobalPlazaItem;
    m_pGlobalPlazaItem = m_pGlobalPlazaItem->m_pNextPlazaItemPtr;
    pGlobalPlazaItem->m_pNextPlazaItemPtr = nullptr;
    return pGlobalPlazaItem;
  }

  // 创建对象
  try {
    CGlobalPlazaItem* pGlobalPlazaItem = new CGlobalPlazaItem;
    return pGlobalPlazaItem;
  } catch (...) {
  }

  return nullptr;
}

// 创建房间
CGlobalServerItem* CGlobalInfoManager::CreateGlobalServerItem() {
  // 使用存储
  if (m_pGlobalServerItem != nullptr) {
    CGlobalServerItem* pGlobalServerItem = m_pGlobalServerItem;
    m_pGlobalServerItem = m_pGlobalServerItem->m_pNextServerItemPtr;
    pGlobalServerItem->m_pNextServerItemPtr = nullptr;
    return pGlobalServerItem;
  }

  // 创建对象
  try {
    CGlobalServerItem* pGlobalServerItem = new CGlobalServerItem;
    return pGlobalServerItem;
  } catch (...) {
  }

  return nullptr;
}

// 创建聊天
CGlobalChatItem* CGlobalInfoManager::CreateGlobalChatItem() {
  // 使用存储
  if (m_pGlobalChatItem != nullptr) {
    CGlobalChatItem* pGlobalChatItem = m_pGlobalChatItem;
    m_pGlobalChatItem = m_pGlobalChatItem->m_pNextChatServerPtr;
    pGlobalChatItem->m_pNextChatServerPtr = nullptr;
    return pGlobalChatItem;
  }

  // 创建对象
  try {
    CGlobalChatItem* pGlobalChatItem = new CGlobalChatItem;
    return pGlobalChatItem;
  } catch (...) {
  }

  return nullptr;
}

// 释放用户
bool CGlobalInfoManager::FreeGlobalUserItem(CGlobalUserItem* pGlobalUserItem) {
  // 效验参数
  ASSERT(pGlobalUserItem != nullptr);
  if (pGlobalUserItem == nullptr)
    return false;

  // 设置变量
  pGlobalUserItem->m_dwUserID = 0;
  pGlobalUserItem->m_dwGameID = 0;
  pGlobalUserItem->m_szNickName[0] = 0;
  ZeroMemory(&pGlobalUserItem->m_UserInfo, sizeof(tagUserInfo));
  pGlobalUserItem->m_GlobalServerItemArray.RemoveAll();

  // 加入存储
  pGlobalUserItem->m_pNextUserItemPtr = m_pGlobalUserItem;
  m_pGlobalUserItem = pGlobalUserItem;

  return true;
}

// 释放广场
bool CGlobalInfoManager::FreeGlobalPlazaItem(CGlobalPlazaItem* pGlobalPlazaItem) {
  // 效验参数
  ASSERT(pGlobalPlazaItem != nullptr);
  if (pGlobalPlazaItem == nullptr)
    return false;

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
  ASSERT(pGlobalServerItem != nullptr);
  if (pGlobalServerItem == nullptr)
    return false;

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
  ASSERT(pGlobalChatItem != nullptr);
  if (pGlobalChatItem == nullptr)
    return false;

  // 设置变量
  pGlobalChatItem->m_wIndex = 0;
  ZeroMemory(&pGlobalChatItem->m_ChatServer, sizeof(pGlobalChatItem->m_ChatServer));

  // 加入存储
  pGlobalChatItem->m_pNextChatServerPtr = m_pGlobalChatItem;
  m_pGlobalChatItem = pGlobalChatItem;

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
