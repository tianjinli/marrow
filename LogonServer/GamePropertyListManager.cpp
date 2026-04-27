#include "GamePropertyListManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePropertyListManager::CGamePropertyListManager() {
  // 设置质数
  m_GamePropertyTypeItemMap.reserve(PRIME_TYPE);
  m_GamePropertyRelatItemMap.reserve(PRIME_KIND);
  m_GamePropertyItemMap.reserve(PRIME_SERVER);
  m_GamePropertySubItemMap.reserve(PRIME_KIND);
}

// 析构函数
CGamePropertyListManager::~CGamePropertyListManager() {
  // 删除种类
  CGamePropertyTypeItem* pGamePropertyTypeItem = nullptr;
  for (auto itr = m_GamePropertyTypeItemMap.begin(); itr != m_GamePropertyTypeItemMap.end(); ++itr) {
    pGamePropertyTypeItem = itr->second;
    SafeDelete(pGamePropertyTypeItem);
  }
  for (INT_PTR i = 0; i < m_GamePropertyTypeItemArray.GetCount(); i++) {
    pGamePropertyTypeItem = m_GamePropertyTypeItemArray[i];
    SafeDelete(pGamePropertyTypeItem);
  }
  m_GamePropertyTypeItemMap.clear();
  m_GamePropertyTypeItemArray.RemoveAll();

  // 删除关系
  CGamePropertyRelatItem* pGamePropertyRelatItem = nullptr;
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end(); ++itr) {
    pGamePropertyRelatItem = itr->second;
    SafeDelete(pGamePropertyRelatItem);
  }
  for (INT_PTR i = 0; i < m_GamePropertyRelatItemArray.GetCount(); i++) {
    pGamePropertyRelatItem = m_GamePropertyRelatItemArray[i];
    SafeDelete(pGamePropertyRelatItem);
  }
  m_GamePropertyRelatItemMap.clear();
  m_GamePropertyRelatItemArray.RemoveAll();

  // 删除道具
  CGamePropertyItem* pGamePropertyItem = nullptr;
  for (auto itr = m_GamePropertyItemMap.begin(); itr != m_GamePropertyItemMap.end(); ++itr) {
    pGamePropertyItem = itr->second;
    SafeDelete(pGamePropertyItem);
  }
  for (INT_PTR i = 0; i < m_GamePropertyItemArray.GetCount(); i++) {
    pGamePropertyItem = m_GamePropertyItemArray[i];
    SafeDelete(pGamePropertyItem);
  }
  m_GamePropertyItemMap.clear();
  m_GamePropertyItemArray.RemoveAll();

  // 删除子道具
  CGamePropertySubItem* pGamePropertySubItem = nullptr;
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end(); ++itr) {
    pGamePropertySubItem = itr->second;
    SafeDelete(pGamePropertySubItem);
  }
  for (INT_PTR i = 0; i < m_GamePropertySubItemArray.GetCount(); i++) {
    pGamePropertySubItem = m_GamePropertySubItemArray[i];
    SafeDelete(pGamePropertySubItem);
  }
  m_GamePropertySubItemMap.clear();
  m_GamePropertySubItemArray.RemoveAll();
}

// 重置列表
VOID CGamePropertyListManager::ResetPropertyListManager() {
  // 废弃种类
  DisusePropertyTypeItem();

  // 废弃关系
  DisusePropertyRelatItem();

  // 废弃道具
  DisusePropertySubItem();

  // 废弃道具
  DisusePropertyItem();

  // 清理种类
  CleanPropertyTypeItem();

  // 清理关系
  CleanPropertyRelatItem();

  // 清理道具
  CleanPropertyItem();

  // 清理道具
  CleanPropertySubItem();
}

// 清理种类
VOID CGamePropertyListManager::CleanPropertyTypeItem() {
  for (auto itr = m_GamePropertyTypeItemMap.begin(); itr != m_GamePropertyTypeItemMap.end();) {
    CGamePropertyTypeItem* pGamePropertyTypeItem = itr->second;
    if (pGamePropertyTypeItem->m_bDisuse) {
      m_GamePropertyTypeItemMap.erase(itr++);
      m_GamePropertyTypeItemArray.Add(pGamePropertyTypeItem);
    } else {
      ++itr;
    }
  }
}

// 清理关系
VOID CGamePropertyListManager::CleanPropertyRelatItem() {
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end();) {
    CGamePropertyRelatItem* pGamePropertyRelatItem = itr->second;

    if (pGamePropertyRelatItem->m_bDisuse) {
      m_GamePropertyRelatItemMap.erase(itr++);
      m_GamePropertyRelatItemArray.Add(pGamePropertyRelatItem);
    } else {
      ++itr;
    }
  }
}

// 清理道具
VOID CGamePropertyListManager::CleanPropertyItem() {
  for (auto itr = m_GamePropertyItemMap.begin(); itr != m_GamePropertyItemMap.end();) {
    CGamePropertyItem* pGamePropertyItem = itr->second;
    if (pGamePropertyItem->m_bDisuse) {
      m_GamePropertyItemMap.erase(itr++);
      m_GamePropertyItemArray.Add(pGamePropertyItem);
    } else {
      ++itr;
    }
  }
}

// 清理子道具
VOID CGamePropertyListManager::CleanPropertySubItem() {
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end();) {
    CGamePropertySubItem* pGamePropertySubItem = itr->second;
    if (pGamePropertySubItem->m_bDisuse == true) {
      m_GamePropertySubItemMap.erase(itr++);
      m_GamePropertySubItemArray.Add(pGamePropertySubItem);
    } else {
      ++itr;
    }
  }
}

// 废弃种类
VOID CGamePropertyListManager::DisusePropertyTypeItem() {
  for (auto itr = m_GamePropertyTypeItemMap.begin(); itr != m_GamePropertyTypeItemMap.end();) {
    CGamePropertyTypeItem* pGamePropertyTypeItem = itr->second;
    pGamePropertyTypeItem->m_bDisuse = true;
  }
}

// 废弃关系
VOID CGamePropertyListManager::DisusePropertyRelatItem() {
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end();) {
    CGamePropertyRelatItem* pGamePropertyRelatItem = itr->second;
    pGamePropertyRelatItem->m_bDisuse = true;
  }
}

// 废弃道具
VOID CGamePropertyListManager::DisusePropertyItem() {
  for (auto itr = m_GamePropertyItemMap.begin(); itr != m_GamePropertyItemMap.end();) {
    CGamePropertyItem* pGamePropertyItem = itr->second;
    pGamePropertyItem->m_bDisuse = true;
  }
}

// 废弃子道具
VOID CGamePropertyListManager::DisusePropertySubItem() {
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end();) {
    CGamePropertySubItem* pGamePropertySubItem = itr->second;
    pGamePropertySubItem->m_bDisuse = true;
  }
}

// 插入种类
bool CGamePropertyListManager::InsertGamePropertyTypeItem(tagPropertyTypeItem* ptagPropertyTypeItem) {
  // 效验参数
  ASSERT(ptagPropertyTypeItem != nullptr);
  if (ptagPropertyTypeItem == nullptr)
    return false;

  // 查找现存
  CGamePropertyTypeItem* pGamePropertyTypeItem = nullptr;
  if (auto itr = m_GamePropertyTypeItemMap.find(ptagPropertyTypeItem->dwTypeID); itr == m_GamePropertyTypeItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_GamePropertyTypeItemArray.GetCount();
      if (nStroeCount > 0) {
        pGamePropertyTypeItem = m_GamePropertyTypeItemArray[nStroeCount - 1];
        m_GamePropertyTypeItemArray.RemoveAt(nStroeCount - 1);
      } else {
        pGamePropertyTypeItem = new CGamePropertyTypeItem;
        if (pGamePropertyTypeItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGamePropertyTypeItem, sizeof(CGamePropertyTypeItem));
  } else {
    pGamePropertyTypeItem = itr->second;
  }

  // 设置数据
  CopyMemory(&pGamePropertyTypeItem->m_PropertyTypeItem, ptagPropertyTypeItem, sizeof(tagPropertyTypeItem));

  // 设置索引
  m_GamePropertyTypeItemMap[ptagPropertyTypeItem->dwTypeID] = pGamePropertyTypeItem;

  return true;
}

// 插入关系
bool CGamePropertyListManager::InsertGamePropertyRelatItem(tagPropertyRelatItem* ptagPropertyRelatItem) {
  // 效验参数
  ASSERT(ptagPropertyRelatItem != nullptr);
  if (ptagPropertyRelatItem == nullptr)
    return false;

  // 查找现存
  CGamePropertyRelatItem* pPropertyRelatItem = nullptr;

  bool bFinder = false;
  DWORD dwkey = 0;
  DWORD dwCurrKey = 0;
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end(); ++itr) {
    pPropertyRelatItem = itr->second;
    if (pPropertyRelatItem == nullptr)
      break;

    if (pPropertyRelatItem->m_PropertyRelatItem.dwPropertyID == ptagPropertyRelatItem->dwPropertyID &&
        pPropertyRelatItem->m_PropertyRelatItem.dwTypeID == ptagPropertyRelatItem->dwTypeID) {
      bFinder = true;
      break;
    }
    dwCurrKey++;
  }

  // 获取子项
  if (bFinder == false) {
    // 创建对象
    try {
      pPropertyRelatItem = new CGamePropertyRelatItem;
      if (pPropertyRelatItem == nullptr)
        return false;
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pPropertyRelatItem, sizeof(CGamePropertyRelatItem));
    dwCurrKey++;
  } else {
    dwCurrKey = dwkey;
  }

  // 设置数据
  CopyMemory(&pPropertyRelatItem->m_PropertyRelatItem, ptagPropertyRelatItem, sizeof(tagPropertyRelatItem));

  // 设置索引
  m_GamePropertyRelatItemMap[dwCurrKey] = pPropertyRelatItem;

  return true;
}

// 插入道具
bool CGamePropertyListManager::InsertGamePropertyItem(tagPropertyItem* ptagPropertyItem) {
  // 效验参数
  ASSERT(ptagPropertyItem != nullptr);
  if (ptagPropertyItem == nullptr)
    return false;

  // 查找现存
  CGamePropertyItem* pGamePropertyItem = nullptr;

  // 获取子项
  if (auto itr = m_GamePropertyItemMap.find(ptagPropertyItem->dwPropertyID); itr == m_GamePropertyItemMap.end()) {
    // 创建对象
    try {
      INT_PTR nStroeCount = m_GamePropertyItemArray.GetCount();
      if (nStroeCount > 0) {
        pGamePropertyItem = m_GamePropertyItemArray[nStroeCount - 1];
        m_GamePropertyItemArray.RemoveAt(nStroeCount - 1);
      } else {
        pGamePropertyItem = new CGamePropertyItem;
        if (pGamePropertyItem == nullptr)
          return false;
      }
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGamePropertyItem, sizeof(CGamePropertyItem));
  } else {
    pGamePropertyItem = itr->second;
  }

  // 设置数据
  CopyMemory(&pGamePropertyItem->m_PropertyItem, ptagPropertyItem, sizeof(tagPropertyItem));

  // 设置索引
  m_GamePropertyItemMap[ptagPropertyItem->dwPropertyID] = pGamePropertyItem;

  return true;
}

// 插入子道具
bool CGamePropertyListManager::InsertGamePropertySubItem(tagPropertySubItem* ptagPropertySubItem) {
  // 效验参数
  ASSERT(ptagPropertySubItem != nullptr);
  if (ptagPropertySubItem == nullptr)
    return false;

  // 查找现存
  CGamePropertySubItem* pGamePropertySubItem = nullptr;

  bool bFinder = false;
  DWORD dwkey = 0;
  DWORD dwCurrKey = 0;
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end(); ++itr) {
    pGamePropertySubItem = itr->second;
    if (pGamePropertySubItem == nullptr)
      break;

    if (pGamePropertySubItem->m_PropertySubItem.dwPropertyID == ptagPropertySubItem->dwPropertyID &&
        pGamePropertySubItem->m_PropertySubItem.dwOwnerPropertyID == ptagPropertySubItem->dwOwnerPropertyID) {
      bFinder = true;
      break;
    }
    dwCurrKey++;
  }

  // 获取子项
  if (bFinder == false) {
    // 创建对象
    try {
      pGamePropertySubItem = new CGamePropertySubItem;
      if (pGamePropertySubItem == nullptr)
        return false;
    } catch (...) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGamePropertySubItem, sizeof(CGamePropertySubItem));
    dwCurrKey++;
  } else {
    dwCurrKey = dwkey;
  }

  // 设置数据
  CopyMemory(&pGamePropertySubItem->m_PropertySubItem, ptagPropertySubItem, sizeof(tagPropertySubItem));

  // 设置索引
  m_GamePropertySubItemMap[dwCurrKey] = pGamePropertySubItem;

  return true;
}

// 删除种类
bool CGamePropertyListManager::DeleteGamePropertyTypeItem(DWORD dwTypeID) {
  // 查找种类
  auto itr = m_GamePropertyTypeItemMap.find(dwTypeID);
  if (itr == m_GamePropertyTypeItemMap.end())
    return false;

  // CGamePropertyTypeItem* pGamePropertyTypeItem = itr->second;

  // 删除数据
  m_GamePropertyTypeItemMap.erase(itr);
  // m_GamePropertyTypeItemArray.Add(pGamePropertyTypeItem);

  return true;
}

// 删除道具
bool CGamePropertyListManager::DeleteGamePropertyItem(DWORD dwPropertyID) {
  // 查找道具
  auto itr = m_GamePropertyItemMap.find(dwPropertyID);
  if (itr == m_GamePropertyItemMap.end())
    return false;

  CGamePropertyItem* pGamePropertyItem = itr->second;

  // 删除数据
  m_GamePropertyItemMap.erase(itr);
  m_GamePropertyItemArray.Add(pGamePropertyItem);

  return true;
}

// 枚举种类
CGamePropertyTypeItem* CGamePropertyListManager::EmunGamePropertyTypeItem(CGamePropertyTypeItemMap::iterator* itr) {
  // 获取对象
  auto cur = m_GamePropertyTypeItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  if (cur != m_GamePropertyTypeItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举道具
CGamePropertyRelatItem* CGamePropertyListManager::EmunGamePropertyRelatItem(CGamePropertyRelatItemMap::iterator* itr) {
  auto cur = m_GamePropertyRelatItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  if (cur != m_GamePropertyRelatItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举道具
CGamePropertyItem* CGamePropertyListManager::EmunGamePropertyItem(CGamePropertyItemMap::iterator* itr) {
  auto cur = m_GamePropertyItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  if (cur != m_GamePropertyItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 枚举子道具
CGamePropertySubItem* CGamePropertyListManager::EmunGamePropertySubItem(CGamePropertySubItemMap::iterator* itr) {
  auto cur = m_GamePropertySubItemMap.begin();
  if (itr != nullptr) {
    cur = ++(*itr);
  } else {
    itr = &cur;
  }

  if (cur != m_GamePropertySubItemMap.end()) {
    return cur->second;
  }

  itr = nullptr;
  return nullptr;
}

// 查找道具
CGamePropertyItem* CGamePropertyListManager::SearchGamePropertyItem(DWORD dwPropertyID) {
  if (auto itr = m_GamePropertyItemMap.find(dwPropertyID); itr != m_GamePropertyItemMap.end()) {
    return itr->second;
  }

  return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
