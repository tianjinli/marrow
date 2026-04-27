#include "GamePropertyManager.h"


bool IsValid(const tagPropertyBuff &property_buff) {
  return (property_buff.tUseTime + property_buff.UseResultsValidTime) > (DWORD) time(nullptr);
}

//////////////////////////////////////////////////////////////////////////////////

CPropertyBuffMAP CGamePropertyManager::property_buff_map_;

// 构造函数
CGamePropertyManager::CGamePropertyManager() {
}

// 析构函数
CGamePropertyManager::~CGamePropertyManager() {
}

// 设置道具
bool CGamePropertyManager::SetGamePropertyInfo(tagPropertyInfo PropertyInfo[], WORD wPropertyCount) {
  // 设置变量
  property_info_array_.SetSize(wPropertyCount);

  // 拷贝数组
  CopyMemory(property_info_array_.GetData(), PropertyInfo, sizeof(tagPropertyInfo) * wPropertyCount);

  return true;
}

bool CGamePropertyManager::SetGamePropertyBuff(const DWORD dwUserID, const tagPropertyBuff PropertyBuff[], const WORD wBuffCount) {
  if (dwUserID <= 0)
    return false;

  // 保存玩家Buff
  for (int i = 0; i < wBuffCount; i++) {
    property_buff_map_[dwUserID].push_back(PropertyBuff[i]);
  }
  return true;
}

bool CGamePropertyManager::ClearUserBuff(const DWORD dwUserID) {
  if (property_buff_map_[dwUserID].empty())
    return false;

  property_buff_map_[dwUserID].clear();
  return true;
}

// 查找道具
tagPropertyInfo* CGamePropertyManager::SearchPropertyItem(WORD wPropertyIndex) {
  // 查找道具
  for (INT_PTR i = 0; i < property_info_array_.GetCount(); i++) {
    if (property_info_array_[i].wIndex == wPropertyIndex) {
      return &property_info_array_[i];
    }
  }

  return nullptr;
}

tagPropertyBuff* CGamePropertyManager::SearchValidPropertyBuff(const DWORD dwUserID, const PROP_KIND Kind) {
  // 同种功能只返回 倍数最佳的道具Buff
  int index = -1;
  int dwMaxScoreMultiple = 0; // 最高倍数
  std::vector<tagPropertyBuff> vecBuff(property_buff_map_[dwUserID]);
  int nBuffCount = (int) vecBuff.size();
  for (int i = 0; i < nBuffCount; i++) {
    if ((PROP_KIND) vecBuff[i].dwKind == Kind && IsValid(vecBuff[i])) // 时间有效判断
    {
      int dwScoreMultiple = vecBuff[i].dwScoreMultiple;
      if (dwScoreMultiple >= dwMaxScoreMultiple) {
        dwMaxScoreMultiple = dwScoreMultiple;
        index = i;
      }
    }
  }
  if (index != -1)
    return &property_buff_map_[dwUserID][index];
  return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////
