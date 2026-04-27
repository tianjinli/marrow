#pragma once

#include <map>
#include <vector>

#include "GameServiceHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 类型定义
typedef CWHArray<tagPropertyInfo> CPropertyInfoArray; // 道具数组
typedef std::map<DWORD, std::vector<tagPropertyBuff> > CPropertyBuffMAP; // 玩家道具Buff

//////////////////////////////////////////////////////////////////////////////////

// 道具管理
class GAME_SERVICE_CLASS CGamePropertyManager {
  // 变量定义
protected:
  CPropertyInfoArray property_info_array_; // 道具数组
  static CPropertyBuffMAP property_buff_map_; // 道具Buff

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
