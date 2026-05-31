#include "GamePropertyListManager.h"

//////////////////////////////////////////////////////////////////////////////////


CGamePropertyListItem::CGamePropertyListItem() {
  m_bDisuse = false;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePropertyTypeItem::CGamePropertyTypeItem() {
  // 设置变量
  ZeroMemory(&m_PropertyTypeItem, sizeof(m_PropertyTypeItem));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePropertyRelatItem::CGamePropertyRelatItem() {
  // 设置变量
  ZeroMemory(&m_PropertyRelatItem, sizeof(m_PropertyRelatItem));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePropertyItem::CGamePropertyItem() {
  // 设置变量
  ZeroMemory(&m_PropertyItem, sizeof(m_PropertyItem));
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGamePropertySubItem::CGamePropertySubItem() {
  // 设置变量
  ZeroMemory(&m_PropertySubItem, sizeof(m_PropertySubItem));
}

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
  for (auto& pGamePropertyTypeItem: m_GamePropertyTypeItemMap | std::views::values) {
    SafeDelete(pGamePropertyTypeItem);
  }
  for (auto& pGamePropertyTypeItem: m_GamePropertyTypeItemArray) {
    SafeDelete(pGamePropertyTypeItem);
  }
  m_GamePropertyTypeItemMap.clear();
  m_GamePropertyTypeItemArray.clear();

  // 删除关系
  for (auto& pGamePropertyRelatItem: m_GamePropertyRelatItemMap | std::views::values) {
    SafeDelete(pGamePropertyRelatItem);
  }
  for (auto& pGamePropertyRelatItem: m_GamePropertyRelatItemArray) {
    SafeDelete(pGamePropertyRelatItem);
  }
  m_GamePropertyRelatItemMap.clear();
  m_GamePropertyRelatItemArray.clear();

  // 删除道具
  for (auto& pGamePropertyItem: m_GamePropertyItemMap | std::views::values) {
    SafeDelete(pGamePropertyItem);
  }
  for (auto& pGamePropertyItem: m_GamePropertyItemArray) {
    SafeDelete(pGamePropertyItem);
  }
  m_GamePropertyItemMap.clear();
  m_GamePropertyItemArray.clear();

  // 删除子道具
  for (auto& pGamePropertySubItem: m_GamePropertySubItemMap | std::views::values) {
    SafeDelete(pGamePropertySubItem);
  }
  for (auto& pGamePropertySubItem: m_GamePropertySubItemArray) {
    SafeDelete(pGamePropertySubItem);
  }
  m_GamePropertySubItemMap.clear();
  m_GamePropertySubItemArray.clear();
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
    if (pGamePropertyTypeItem && pGamePropertyTypeItem->m_bDisuse == true) {
      m_GamePropertyTypeItemArray.emplace_back(pGamePropertyTypeItem);
      itr = m_GamePropertyTypeItemMap.erase(itr);
    } else {
      ++itr;
    }
  }
}

// 清理关系
VOID CGamePropertyListManager::CleanPropertyRelatItem() {
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end();) {
    CGamePropertyRelatItem* pGamePropertyRelatItem = itr->second;
    if (pGamePropertyRelatItem && pGamePropertyRelatItem->m_bDisuse == true) {
      m_GamePropertyRelatItemArray.emplace_back(pGamePropertyRelatItem);
      itr = m_GamePropertyRelatItemMap.erase(itr);
    } else {
      ++itr;
    }
  }
}

// 清理道具
VOID CGamePropertyListManager::CleanPropertyItem() {
  for (auto itr = m_GamePropertyItemMap.begin(); itr != m_GamePropertyItemMap.end();) {
    CGamePropertyItem* pGamePropertyItem = itr->second;
    if (pGamePropertyItem && pGamePropertyItem->m_bDisuse == true) {
      m_GamePropertyItemArray.emplace_back(pGamePropertyItem);
      itr = m_GamePropertyItemMap.erase(itr);
    } else {
      ++itr;
    }
  }
}

// 清理子道具
VOID CGamePropertyListManager::CleanPropertySubItem() {
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end();) {
    CGamePropertySubItem* pGamePropertySubItem = itr->second;
    if (pGamePropertySubItem && pGamePropertySubItem->m_bDisuse == true) {
      m_GamePropertySubItemArray.emplace_back(pGamePropertySubItem);
      itr = m_GamePropertySubItemMap.erase(itr);
    } else {
      ++itr;
    }
  }
}

// 废弃种类
VOID CGamePropertyListManager::DisusePropertyTypeItem() {
  for (auto& pGamePropertyTypeItem: m_GamePropertyTypeItemMap | std::views::values) {
    pGamePropertyTypeItem->m_bDisuse = true;
  }
}

// 废弃关系
VOID CGamePropertyListManager::DisusePropertyRelatItem() {
  for (auto& pGamePropertyRelatItem: m_GamePropertyRelatItemMap | std::views::values) {
    pGamePropertyRelatItem->m_bDisuse = true;
  }
}

// 废弃道具
VOID CGamePropertyListManager::DisusePropertyItem() {
  for (auto& pGamePropertyItem: m_GamePropertyItemMap | std::views::values) {
    pGamePropertyItem->m_bDisuse = true;
  }
}

// 废弃子道具
VOID CGamePropertyListManager::DisusePropertySubItem() {
  for (auto& pGamePropertySubItem: m_GamePropertySubItemMap | std::views::values) {
    pGamePropertySubItem->m_bDisuse = true;
  }
}

// 插入种类
bool CGamePropertyListManager::InsertGamePropertyTypeItem(tagPropertyTypeItem* ptagPropertyTypeItem) {
  // 效验参数
  ASSERT(ptagPropertyTypeItem != NULL);
  if (ptagPropertyTypeItem == NULL) {
    return false;
  }

  // 查找现存
  CGamePropertyTypeItem* pGamePropertyTypeItem = NULL;
  if (auto itr = m_GamePropertyTypeItemMap.find(ptagPropertyTypeItem->dwTypeID); itr != m_GamePropertyTypeItemMap.end()) {
    pGamePropertyTypeItem = itr->second;
  } else {
    // 创建对象
    if (!m_GamePropertyTypeItemArray.empty()) {
      pGamePropertyTypeItem = m_GamePropertyTypeItemArray.back();
      m_GamePropertyTypeItemArray.pop_back();
    } else {
      pGamePropertyTypeItem = new (std::nothrow) CGamePropertyTypeItem;
      if (pGamePropertyTypeItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGamePropertyTypeItem, sizeof(CGamePropertyTypeItem));
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
  ASSERT(ptagPropertyRelatItem != NULL);
  if (ptagPropertyRelatItem == NULL) {
    return false;
  }

  // 查找现存
  bool bFinder = false;
  DWORD dwKey = 0;
  DWORD dwCurrKey = 0;
  CGamePropertyRelatItem* pPropertyRelatItem = NULL;
  for (auto itr = m_GamePropertyRelatItemMap.begin(); itr != m_GamePropertyRelatItemMap.end(); ++itr) {
    pPropertyRelatItem = itr->second;
    if (pPropertyRelatItem == nullptr) {
      break; // 遇到 nullptr 立即中断
    }
    if (pPropertyRelatItem->m_PropertyRelatItem.dwPropertyID == ptagPropertyRelatItem->dwPropertyID &&
        pPropertyRelatItem->m_PropertyRelatItem.dwTypeID == ptagPropertyRelatItem->dwTypeID) {
      bFinder = true;
      dwKey = itr->first;
      break;
    }
    dwCurrKey++;
  }

  // 获取子项
  if (bFinder == false) {
    // 创建对象
    pPropertyRelatItem = new (std::nothrow) CGamePropertyRelatItem;
    if (pPropertyRelatItem == NULL) {
      return false;
    }

    // 设置变量
    ZeroMemory(pPropertyRelatItem, sizeof(CGamePropertyRelatItem));
    dwCurrKey++;
  } else {
    dwCurrKey = dwKey;
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
  ASSERT(ptagPropertyItem != NULL);
  if (ptagPropertyItem == NULL) {
    return false;
  }

  // 查找现存
  CGamePropertyItem* pGamePropertyItem = NULL;

  // 获取子项
  if (auto itr = m_GamePropertyItemMap.find(ptagPropertyItem->dwPropertyID); itr != m_GamePropertyItemMap.end()) {
    pGamePropertyItem = itr->second;
  } else {
    // 创建对象
    if (!m_GamePropertyItemArray.empty()) {
      pGamePropertyItem = m_GamePropertyItemArray.back();
      m_GamePropertyItemArray.pop_back();
    } else {
      pGamePropertyItem = new (std::nothrow) CGamePropertyItem;
      if (pGamePropertyItem == NULL) {
        return false;
      }
    }

    // 设置变量
    ZeroMemory(pGamePropertyItem, sizeof(CGamePropertyItem));
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
  ASSERT(ptagPropertySubItem != NULL);
  if (ptagPropertySubItem == NULL) {
    return false;
  }

  // 查找现存
  bool bFinder = false;
  DWORD dwKey = 0;
  DWORD dwCurrKey = 0;
  CGamePropertySubItem* pGamePropertySubItem = NULL;
  for (auto itr = m_GamePropertySubItemMap.begin(); itr != m_GamePropertySubItemMap.end(); ++itr) {
    pGamePropertySubItem = itr->second;
    if (pGamePropertySubItem == nullptr) {
      break; // 遇到 nullptr 立即中断
    }
    if (pGamePropertySubItem->m_PropertySubItem.dwPropertyID == ptagPropertySubItem->dwPropertyID &&
        pGamePropertySubItem->m_PropertySubItem.dwOwnerPropertyID == ptagPropertySubItem->dwOwnerPropertyID) {
      bFinder = true;
      dwKey = itr->first;
      break;
    }
    dwCurrKey++;
  }

  // 获取子项
  if (bFinder == false) {
    // 创建对象
    pGamePropertySubItem = new (std::nothrow) CGamePropertySubItem;
    if (pGamePropertySubItem == NULL) {
      return false;
    }

    // 设置变量
    ZeroMemory(pGamePropertySubItem, sizeof(CGamePropertySubItem));
    dwCurrKey++;
  } else {
    dwCurrKey = dwKey;
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
  if (itr == m_GamePropertyTypeItemMap.end()) {
    return false;
  }

  // 删除数据
  m_GamePropertyTypeItemMap.erase(dwTypeID);

  return true;
}

// 删除道具
bool CGamePropertyListManager::DeleteGamePropertyItem(DWORD dwPropertyID) {
  // 查找道具
  auto itr = m_GamePropertyItemMap.find(dwPropertyID);
  if (itr == m_GamePropertyItemMap.end()) {
    return false;
  }

  CGamePropertyItem* pGamePropertyItem = itr->second;

  // 删除数据
  m_GamePropertyItemMap.erase(dwPropertyID);
  m_GamePropertyItemArray.emplace_back(pGamePropertyItem);

  return true;
}

// 枚举种类
CGamePropertyTypeItem* CGamePropertyListManager::EmunGamePropertyTypeItem(POSITION& Position) {
  // 变量定义
  CGamePropertyTypeItem* pGamePropertyTypeItem = NULL;
  auto itr = Position.empty() ? m_GamePropertyTypeItemMap.begin() : Position.get<CGamePropertyTypeItemMap::iterator>();

  // 获取对象
  if (itr != m_GamePropertyTypeItemMap.end()) {
    pGamePropertyTypeItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGamePropertyTypeItem;
}

// 枚举道具
CGamePropertyRelatItem* CGamePropertyListManager::EmunGamePropertyRelatItem(POSITION& Position) {
  // 变量定义
  CGamePropertyRelatItem* pGamePropertyRelatItem = NULL;
  auto itr = Position.empty() ? m_GamePropertyRelatItemMap.begin() : Position.get<CGamePropertyRelatItemMap::iterator>();

  // 获取对象
  if (itr != m_GamePropertyRelatItemMap.end()) {
    pGamePropertyRelatItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGamePropertyRelatItem;
}

// 枚举道具
CGamePropertyItem* CGamePropertyListManager::EmunGamePropertyItem(POSITION& Position) {
  // 变量定义
  CGamePropertyItem* pGamePropertyItem = NULL;
  auto itr = Position.empty() ? m_GamePropertyItemMap.begin() : Position.get<CGamePropertyItemMap::iterator>();

  // 获取对象
  if (itr != m_GamePropertyItemMap.end()) {
    pGamePropertyItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGamePropertyItem;
}

// 枚举子道具
CGamePropertySubItem* CGamePropertyListManager::EmunGamePropertySubItem(POSITION& Position) {
  // 变量定义
  CGamePropertySubItem* pGamePropertySubItem = NULL;
  auto itr = Position.empty() ? m_GamePropertySubItemMap.begin() : Position.get<CGamePropertySubItemMap::iterator>();

  // 获取对象
  if (itr != m_GamePropertySubItemMap.end()) {
    pGamePropertySubItem = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pGamePropertySubItem;
}

// 查找道具
CGamePropertyItem* CGamePropertyListManager::SearchGamePropertyItem(DWORD dwPropertyID) {
  auto itr = m_GamePropertyItemMap.find(dwPropertyID);
  return itr != m_GamePropertyItemMap.end() ? itr->second : nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
