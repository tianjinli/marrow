#include "ServerUserManager.h"

//////////////////////////////////////////////////////////////////////////////////
// 构造函数
CServerUserItem::CServerUserItem(enUserItemKind enUserKind) : m_enUserKind(enUserKind) {}

// 析构函数
CServerUserItem::~CServerUserItem() {}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CLocalUserItem::CLocalUserItem() : CServerUserItem(enLocalKind) {
  // 设置变量
  m_dwSocketID = 0L;
  m_dwLogonTime = 0;
  m_wServerID = FRIEND_INVALID_SERVERID;
  m_wTableID = FRIEND_INVALID_TABLEID;

  // 辅助变量
  ZeroMemory(m_szLogonPass, sizeof(m_szLogonPass));

  // 属性变量
  ZeroMemory(&m_UserInfo, sizeof(m_UserInfo));
}

// 析构函数
CLocalUserItem::~CLocalUserItem() {}

// 设置状态
VOID CLocalUserItem::SetGameStatus(BYTE cbGameStatus, WORD wServerID, WORD wTableID) {
  // 设置变量
  m_UserInfo.cbGameStatus = cbGameStatus;
  m_wServerID = wServerID;
  m_wTableID = wTableID;
}

// 对比帐号
bool CLocalUserItem::ContrastNickName(LPCTSTR pszNickName) {
  // 效验参数
  ASSERT(pszNickName != NULL);
  if (pszNickName == NULL) {
    return false;
  }

  StringT source_nick_name = m_UserInfo.szNickName;
  StringT target_nick_name = pszNickName;
  return (StringUtils::Compare(source_nick_name, target_nick_name, true) == 0);
}

// 对比密码
bool CLocalUserItem::ContrastLogonPass(LPCTSTR pszPassword) {
  // 效验参数
  ASSERT(pszPassword != NULL);
  if (pszPassword == NULL) {
    return false;
  }

  StringT source_logon_pass = m_szLogonPass;
  StringT target_logon_pass = pszPassword;
  return StringUtils::Compare(source_logon_pass, target_logon_pass, true) == 0;
}

// 重置数据
VOID CLocalUserItem::ResetUserItem() {
  // 设置变量
  m_dwSocketID = 0L;

  // 辅助变量
  ZeroMemory(m_szLogonPass, sizeof(m_szLogonPass));

  // 属性变量
  ZeroMemory(&m_UserInfo, sizeof(m_UserInfo));
}

// 修改密码
VOID CLocalUserItem::ModifyLogonPassword(LPCTSTR pszPassword) {
  // 拷贝字符
  lstrcpyn(m_szLogonPass, pszPassword, CountArray(m_szLogonPass));
}

//////////////////////////////////////////////////////////////////////////////////
// 构造函数
CUserFriendGroup::CUserFriendGroup() {
  // 变量定义
  m_dwOwnerUserID = NULL;
  m_wFriendCount = 0;
  m_wGroupContent = 0;
  m_pFriendInfo = NULL;
}

// 析构函数
CUserFriendGroup::~CUserFriendGroup() {
  // 释放资源
  if (m_pFriendInfo != NULL) {
    SafeDeleteArray(m_pFriendInfo);
  }
}

// 初始化分组
bool CUserFriendGroup::InitFriendGroup(WORD wGroupContent) {
  // 设置变量
  m_wGroupContent = wGroupContent;

  // 分配容量
  m_pFriendInfo = AllocateContent(m_wGroupContent);

  return m_pFriendInfo != NULL;
}

// 添加好友
bool CUserFriendGroup::AppendFriendInfo(tagServerFriendInfo& FriendInfo) {
  // 指针校验
  ASSERT(m_pFriendInfo != NULL);
  if (m_pFriendInfo == NULL) {
    return false;
  }

  // 拷贝数据
  CopyMemory((m_pFriendInfo + m_wFriendCount++), &FriendInfo, sizeof(tagServerFriendInfo));

  // 扩展容量
  if (m_wFriendCount == m_wGroupContent) {
    ExtendGroupContent(m_wFriendCount / 10);
  }

  return true;
}

// 移除好友
bool CUserFriendGroup::RemoveFriendInfo(DWORD dwUserID) {
  // 指针校验
  ASSERT(m_pFriendInfo != NULL);
  if (m_pFriendInfo == NULL) {
    return false;
  }

  // 变量定义
  tagServerFriendInfo* pFriendInfo = SearchFriendItem(dwUserID);

  // 移动内存
  if (pFriendInfo != NULL) {
    --m_wFriendCount;
    if (pFriendInfo != (tagServerFriendInfo*) (m_pFriendInfo + m_wGroupContent - 1)) {
      pFriendInfo = pFriendInfo++;
    }
  }

  return true;
}

// 查找好友
tagServerFriendInfo* CUserFriendGroup::SearchFriendItem(DWORD dwUserID) {
  // 变量定义
  tagServerFriendInfo* pFriendInfo = NULL;

  // 查找好友
  for (WORD wIndex = 0; wIndex < m_wFriendCount; ++wIndex) {
    pFriendInfo = (tagServerFriendInfo*) (m_pFriendInfo + wIndex);
    if (pFriendInfo->dwUserID == dwUserID) {
      return pFriendInfo;
    }
  }

  return NULL;
}

// 枚举好友
tagServerFriendInfo* CUserFriendGroup::EnumFriendItem(WORD wEnumIndex) {
  // 参数校验
  if (wEnumIndex >= m_wFriendCount) {
    return NULL;
  }

  return (tagServerFriendInfo*) (m_pFriendInfo + wEnumIndex);
}

// 分配容量
tagServerFriendInfo* CUserFriendGroup::AllocateContent(WORD wGroupContent) {
  // 变量定义
  tagServerFriendInfo* pFriendInfo = new (std::nothrow) tagServerFriendInfo[wGroupContent];

  ASSERT(pFriendInfo != NULL);
  if (pFriendInfo == NULL) {
    CLogger::Error(TEXT("系统内存资源不足,无法分配内存!"));
    return NULL;
  }

  // 初始化内存
  ZeroMemory(pFriendInfo, wGroupContent * sizeof(tagServerFriendInfo));
  return pFriendInfo;
}

// 扩展容量
bool CUserFriendGroup::ExtendGroupContent(WORD wExtendCount) {
  // 设置变量
  tagServerFriendInfo* pFriendInfo = AllocateContent(m_wGroupContent + wExtendCount);
  if (pFriendInfo == NULL) {
    return false;
  }

  // 移动数据
  MoveMemory(pFriendInfo, m_pFriendInfo, m_wGroupContent * sizeof(tagServerFriendInfo));

  // 释放内存
  SafeDeleteArray(m_pFriendInfo);

  // 设置变量
  m_pFriendInfo = pFriendInfo;
  m_wGroupContent += wExtendCount;

  return true;
}

// 重置分组
VOID CUserFriendGroup::ResetFriendGroup() {
  // 设置变量
  m_wFriendCount = 0;
  m_dwOwnerUserID = 0;
  if (m_pFriendInfo != NULL) {
    ZeroMemory(m_pFriendInfo, m_wGroupContent * sizeof(tagServerFriendInfo));
  }
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CServerUserManager::CServerUserManager() {}

// 析构函数
CServerUserManager::~CServerUserManager() {
  // 释放用户
  for (INT_PTR i = 0; i < m_UserItemStore.size(); i++) {
    CServerUserItem* pServerUserItem = m_UserItemStore[i];
    SafeDelete(pServerUserItem);
  }

  // 释放用户
  for (INT_PTR i = 0; i < m_UserItemArray.size(); i++) {
    CServerUserItem* pServerUserItem = m_UserItemArray[i];
    SafeDelete(pServerUserItem);
  }

  // 删除数据
  m_UserIDMap.clear();
  m_UserItemStore.clear();
  m_UserItemArray.clear();

  return;
}

// 枚举用户
CServerUserItem* CServerUserManager::EnumUserItem(WORD wEnumIndex) {
  if (wEnumIndex >= m_UserItemArray.size()) {
    return NULL;
  }
  return m_UserItemArray[wEnumIndex];
}

// 查找用户
CServerUserItem* CServerUserManager::SearchUserItem(DWORD dwUserID) {
  return m_UserIDMap[dwUserID];
}

// 删除用户
bool CServerUserManager::DeleteUserItem() {
  // 存储对象
  m_UserItemStore.insert(m_UserItemStore.end(), m_UserItemArray.begin(), m_UserItemArray.end());

  // 删除对象
  m_UserIDMap.clear();
  m_UserItemArray.clear();

  return true;
}

// 删除用户
bool CServerUserManager::DeleteUserItem(DWORD dwUserID) {
  // 变量定义
  CServerUserItem* pTempUserItem = NULL;

  // 寻找对象
  for (INT_PTR i = 0; i < m_UserItemArray.size(); i++) {
    // 获取用户
    pTempUserItem = m_UserItemArray[i];
    if (pTempUserItem->GetUserID() != dwUserID) {
      continue;
    }

    // 重置对象
    pTempUserItem->ResetUserItem();

    // 删除对象
    m_UserItemArray.erase(m_UserItemArray.begin() + i);
    m_UserIDMap.erase(dwUserID);
    m_UserItemStore.push_back(pTempUserItem);

    return true;
  }

  // 错误断言
  ASSERT(FALSE);

  return false;
}

// 插入用户
bool CServerUserManager::InsertLocalUserItem(tagInsertLocalUserInfo& InsertLocalUserInfo, tagFriendUserInfo& UserInfo, LPCTSTR pszPassword) {
  // 变量定义
  CLocalUserItem* pServerUserItem = NULL;

  // 获取指针
  if (m_UserItemStore.size() > 0) {
    INT_PTR nItemPostion = m_UserItemStore.size() - 1;
    pServerUserItem = (CLocalUserItem*) m_UserItemStore[nItemPostion];
    m_UserItemStore.erase(m_UserItemStore.begin() + nItemPostion);
  } else {
    pServerUserItem = new (std::nothrow) CLocalUserItem;
    if (pServerUserItem == NULL) {
      return false;
    }
  }

  // 属性变量
  CopyMemory(&pServerUserItem->m_UserInfo, &UserInfo, sizeof(UserInfo));

  // 辅助变量
  lstrcpyn(pServerUserItem->m_szLogonPass, pszPassword, CountArray(pServerUserItem->m_szLogonPass));

  // 插入用户
  m_UserItemArray.emplace_back(pServerUserItem);
  m_UserIDMap[UserInfo.dwUserID] = pServerUserItem;

  // 设置变量
  pServerUserItem->SetLogonTime(InsertLocalUserInfo.dwLogonTime);
  pServerUserItem->SetSocketID(InsertLocalUserInfo.dwSocketID);

  return true;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CFriendGroupManager::CFriendGroupManager() {}

// 析构函数
CFriendGroupManager::~CFriendGroupManager() {
  // 释放用户
  for (INT_PTR i = 0; i < m_UserFriendGroupArray.size(); i++) {
    CUserFriendGroup* pUserFriendGroup = m_UserFriendGroupArray[i];
    SafeDelete(pUserFriendGroup);
  }

  // 释放用户
  for (INT_PTR i = 0; i < m_UserFriendGroupStore.size(); i++) {
    CUserFriendGroup* pUserFriendGroup = m_UserFriendGroupStore[i];
    SafeDelete(pUserFriendGroup);
  }

  // 删除数据
  m_UserIDMap.clear();
  m_UserFriendGroupStore.clear();
  m_UserFriendGroupArray.clear();
}

// 枚举用户
CUserFriendGroup* CFriendGroupManager::EnumGroupItem(WORD wEnumIndex) {
  if (wEnumIndex >= m_UserFriendGroupArray.size()) {
    return NULL;
  }
  return m_UserFriendGroupArray[wEnumIndex];
}

// 查找用户
CUserFriendGroup* CFriendGroupManager::SearchGroupItem(DWORD dwUserID) {
  return m_UserIDMap[dwUserID];
}

// 删除用户
bool CFriendGroupManager::DeleteFriendGroup() {
  // 存储对象
  m_UserFriendGroupStore.insert(m_UserFriendGroupStore.end(), m_UserFriendGroupArray.begin(), m_UserFriendGroupArray.end());

  // 删除对象
  m_UserIDMap.clear();
  m_UserFriendGroupArray.clear();

  return true;
}

// 删除用户
bool CFriendGroupManager::DeleteFriendGroup(DWORD dwUserID) {
  // 变量定义
  CUserFriendGroup* pTempFriendGroup = NULL;

  // 寻找对象
  for (INT_PTR i = 0; i < m_UserFriendGroupArray.size(); i++) {
    // 获取用户
    pTempFriendGroup = m_UserFriendGroupArray[i];
    if (pTempFriendGroup == NULL || pTempFriendGroup->GetOwnerUserID() != dwUserID) {
      continue;
    }

    // 重置对象
    pTempFriendGroup->ResetFriendGroup();

    // 删除对象
    m_UserFriendGroupArray.erase(m_UserFriendGroupArray.begin() + i);
    m_UserIDMap.erase(dwUserID);
    m_UserFriendGroupStore.push_back(pTempFriendGroup);

    return true;
  }

  // 错误断言
  ASSERT(FALSE);

  return false;
}

// 插入好友
bool CFriendGroupManager::InsertFriendGroup(DWORD dwUserID, CUserFriendGroup* pUserFriendGroup) {
  // 设置标识
  pUserFriendGroup->SetOwnerUserID(dwUserID);

  // 插入用户
  m_UserFriendGroupArray.emplace_back(pUserFriendGroup);
  m_UserIDMap[dwUserID] = pUserFriendGroup;

  return true;
}

// 获取分组
CUserFriendGroup* CFriendGroupManager::ActiveFriendGroup(WORD wFriendCount) {
  // 变量定义
  CUserFriendGroup* pUserFriendGroup = NULL;
  WORD wGroupContent = std::min((int) (wFriendCount + wFriendCount / 10), MAX_FRIEND_COUNT);
  wGroupContent = std::max(MIN_FRIEND_CONTENT, (int) wGroupContent);

  // 获取指针
  if (m_UserFriendGroupStore.size() > 0) {
    // INT_PTR nItemPostion = m_UserFriendGroupStore.size() - 1;
    // for (INT_PTR nIndex = nItemPostion; nIndex >= 0; --nIndex) {
    //   pUserFriendGroup = (CUserFriendGroup*) m_UserFriendGroupStore[nIndex];
    //   if (pUserFriendGroup->GetGroupContent() >= wGroupContent) {
    //     pUserFriendGroup->ResetFriendGroup();
    //     m_UserFriendGroupStore.erase(m_UserFriendGroupStore.begin() + nItemPostion);
    //     break;
    //   }
    // }
    for (std::size_t i = m_UserFriendGroupStore.size(); i-- > 0;) {
      auto& group = *m_UserFriendGroupStore[i];
      if (group.GetGroupContent() >= wGroupContent) {
        group.ResetFriendGroup();
        m_UserFriendGroupStore.erase(m_UserFriendGroupStore.begin() + i);
        break;
      }
    }
  }

  if (pUserFriendGroup == NULL || pUserFriendGroup->GetGroupContent() < wGroupContent) {
    pUserFriendGroup = new (std::nothrow) CUserFriendGroup();
    if (pUserFriendGroup == NULL) {
      return NULL;
    }
    pUserFriendGroup->InitFriendGroup(wGroupContent);
  }

  return pUserFriendGroup;
}

//////////////////////////////////////////////////////////////////////////////////
