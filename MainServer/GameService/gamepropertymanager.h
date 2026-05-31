#pragma once

#include "GameServiceHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 类型定义
using CPropertyInfoArray = std::vector<tagPropertyInfo>; // 道具数组
using CPropertyBuffMAP = std::map<DWORD, std::vector<tagPropertyBuff>>; // 玩家道具Buff

//////////////////////////////////////////////////////////////////////////////////

// 道具管理
class GAME_SERVICE_CLASS CGamePropertyManager {
  // 变量定义
protected:
  CPropertyInfoArray m_PropertyInfoArray; // 道具数组
  static CPropertyBuffMAP m_PropertyBuffMap; // 道具Buff

  // 函数定义
public:
  // 构造函数
  CGamePropertyManager();
  // 析构函数
  virtual ~CGamePropertyManager();

  // 配置函数
public:
  // 设置道具
  bool SetGamePropertyInfo(tagPropertyInfo PropertyInfo[], WORD wPropertyCount);
  bool SetGamePropertyBuff(const DWORD dwUserID, const tagPropertyBuff PropertyBuff[], const WORD wBuffCount);
  bool ClearUserBuff(const DWORD dwUserID);

  // 功能函数
public:
  // 查找道具
  tagPropertyInfo* SearchPropertyItem(WORD wPropertyIndex);
  static tagPropertyBuff* SearchValidPropertyBuff(const DWORD dwUserID, const PROP_KIND Kind);
};

//////////////////////////////////////////////////////////////////////////////////
