#pragma once

#include "CorrespondHeader.h"
//////////////////////////////////////////////////////////////////////////////////

// 类说明
class CGlobalUserItem;
class CGlobalPlazaItem;
class CGlobalServerItem;
class CGlobalChatItem;

// 数组定义
typedef CWHArray<CGlobalUserItem*> CGlobalUserItemArray;
typedef CWHArray<CGlobalPlazaItem*> CGlobalPlazaItemArray;
typedef CWHArray<CGlobalServerItem*> CGlobalServerItemArray;
typedef CWHArray<CGlobalChatItem*> CGlobalChatItemArray;

// 索引定义
typedef std::unordered_map<DWORD, CGlobalUserItem*> CMapUserID;
typedef std::unordered_map<DWORD, CGlobalUserItem*> CMapGameID;
typedef std::unordered_map<WORD, CGlobalPlazaItem*> CMapPlazaID;
typedef std::unordered_map<WORD, CGlobalServerItem*> CMapServerID;
typedef std::unordered_map<WORD, CGlobalChatItem*> CMapChatID;
typedef std::unordered_map<StringT, CGlobalUserItem*> CMapNickName;

//////////////////////////////////////////////////////////////////////////////////
// 结构定义

// 用户信息
struct tagGlobalUserInfo {
  // 用户信息
  DWORD dwUserID;                 // 用户标识
  DWORD dwGameID;                 // 游戏标识
  TCHAR szNickName[LEN_NICKNAME]; // 用户昵称

  // 辅助信息
  BYTE cbGender;      // 用户性别
  BYTE cbMemberOrder; // 会员等级
  BYTE cbMasterOrder; // 管理等级

  // 详细信息
  tagUserInfo userInfo; // 用户详细信息
};

// 私人房间信息
struct tagPersonalTable {
  DWORD dwServerID; // 房间I D
  DWORD dwTableID;  // 桌子I D
};

struct tagServerTableCount {
  DWORD dwKindID;     // 游戏I D
  DWORD dwServerID;   // 房间I D
  DWORD dwTableCount; // 桌子数目
};

typedef CWHArray<tagServerTableCount*> CServerTableCountArray;

//////////////////////////////////////////////////////////////////////////////////

// 用户子项
class CGlobalUserItem {
  // 友元定义
  friend class CGlobalInfoManager;

  // 用户属性
public:
  DWORD m_dwUserID = 0;                  // 用户标识
  DWORD m_dwGameID = 0;                  // 游戏标识
  TCHAR m_szNickName[LEN_NICKNAME] = {}; // 用户昵称

  // 辅助信息
public:
  BYTE m_cbGender = 0;      // 用户性别
  BYTE m_cbMemberOrder = 0; // 会员等级
  BYTE m_cbMasterOrder = 0; // 管理等级
  tagUserInfo m_UserInfo;   // 用户详细信息

  // 房间信息
public:
  CGlobalServerItemArray m_GlobalServerItemArray; // 房间数组

  // 链表属性
protected:
  CGlobalUserItem* m_pNextUserItemPtr = nullptr; // 对象指针

  // 函数定义
protected:
  // 构造函数
  CGlobalUserItem() = default;
  // 析构函数
  virtual ~CGlobalUserItem() = default;

  // 功能函数
public:
  // 用户标识
  DWORD GetUserID() { return m_dwUserID; }
  // 游戏标识
  DWORD GetGameID() { return m_dwGameID; }
  // 用户昵称
  LPCTSTR GetNickName() { return m_szNickName; }

  // 用户信息
public:
  // 用户性别
  BYTE GetGender() { return m_cbGender; }
  // 会员等级
  BYTE GetMemberOrder() { return m_cbMemberOrder; }
  // 会员等级
  BYTE GetMasterOrder() { return m_cbMasterOrder; }
  // 详细信息
  tagUserInfo* GetUserInfo() { return &m_UserInfo; }
  // 更新状态
  void UpdateStatus(const WORD wTableID, const WORD wChairID, const BYTE cbUserStatus);

  // 其他信息
public:
  // 枚举房间
  CGlobalServerItem* EnumServerItem(WORD wIndex);
};

//////////////////////////////////////////////////////////////////////////////////

// 广场子项
class CGlobalPlazaItem {
  // 友元定义
  friend class CGlobalInfoManager;

  // 变量定义
public:
  WORD m_wIndex = 0;             // 绑定索引
  tagGamePlaza m_GamePlaza = {}; // 游戏广场

  // 索引变量
public:
  CMapUserID m_MapUserID; // 用户索引

  // 链表属性
protected:
  CGlobalPlazaItem* m_pNextPlazaItemPtr = nullptr; // 对象指针

  // 函数定义
protected:
  // 构造函数
  CGlobalPlazaItem() = default;
  // 析构函数
  virtual ~CGlobalPlazaItem() = default;
};

//////////////////////////////////////////////////////////////////////////////////

// 房间子项
class CGlobalServerItem {
  // 友元定义
  friend class CGlobalInfoManager;

  // 变量定义
public:
  WORD m_wIndex = 0;               // 绑定索引
  tagGameServer m_GameServer = {}; // 游戏房间
  tagGameMatch m_GameMatch = {};

  // 索引变量
public:
  CMapUserID m_MapUserID{PRIME_SERVER_USER}; // 用户索引

  // 链表属性
protected:
  CGlobalServerItem* m_pNextServerItemPtr = nullptr; // 对象指针

  // 函数定义
protected:
  // 构造函数
  CGlobalServerItem() = default;
  // 析构函数
  virtual ~CGlobalServerItem() = default;

  // 功能函数
public:
  // 绑定索引
  WORD GetIndex() { return m_wIndex; }
  // 获取类型
  WORD GetKindID() { return m_GameServer.wKindID; }
  // 获取房间
  WORD GetServerID() { return m_GameServer.wServerID; }
  // 用户数目
  DWORD GetUserItemCount() { return (DWORD)m_MapUserID.size(); }
  // 桌子数目
  WORD GetTabelCount() { return m_GameServer.wTableCount; }
  // 比赛房间
  bool IsMatchServer() { return (m_GameMatch.wServerID == m_GameServer.wServerID) && m_GameMatch.dwMatchID != 0; }

  // 查找函数
public:
  // 寻找用户
  CGlobalUserItem* SearchUserItem(DWORD dwUserID);
};

//////////////////////////////////////////////////////////////////////////////////

// 聊天子项
class CGlobalChatItem {
  // 友元定义
  friend class CGlobalInfoManager;

  // 变量定义
public:
  WORD m_wIndex = 0;               // 绑定索引
  tagChatServer m_ChatServer = {}; // 游戏广场

  // 索引变量
public:
  CMapUserID m_MapUserID; // 用户索引

  // 链表属性
protected:
  CGlobalChatItem* m_pNextChatServerPtr = nullptr; // 对象指针

  // 函数定义
protected:
  // 构造函数
  CGlobalChatItem() = default;
  // 析构函数
  virtual ~CGlobalChatItem() = default;
};

//////////////////////////////////////////////////////////////////////////////////

// 全局信息
class CGlobalInfoManager {
  // 索引变量
protected:
  CMapUserID m_MapUserID{PRIME_PLATFORM_USER}; // 用户标识
  CMapPlazaID m_MapPlazaID{PRIME_SERVER};      // 广场标识
  CMapServerID m_MapServerID{PRIME_SERVER};    // 房间标识
  CMapChatID m_MapChatID{PRIME_SERVER};        // 聊天标识

  // 辅助索引
protected:
  CMapGameID m_MapGameID{PRIME_PLATFORM_USER};     // 标识索引
  CMapNickName m_MapNickName{PRIME_PLATFORM_USER}; // 昵称索引

  // 存储变量
protected:
  CGlobalUserItem* m_pGlobalUserItem = nullptr;     // 用户存储
  CGlobalPlazaItem* m_pGlobalPlazaItem = nullptr;   // 广场存储
  CGlobalServerItem* m_pGlobalServerItem = nullptr; // 房间存储
  CGlobalChatItem* m_pGlobalChatItem = nullptr;     // 聊天存储

  // 函数定义
public:
  // 构造函数
  CGlobalInfoManager() = default;
  // 析构函数
  virtual ~CGlobalInfoManager();

  // 管理函数
public:
  // 重置数据
  VOID ResetData();
  // 用户数目
  DWORD GetUserItemCount() { return (DWORD)m_MapUserID.size(); }
  // 大厅数目
  DWORD GetPlazaItemCount() { return (DWORD)m_MapPlazaID.size(); }
  // 房间数目
  DWORD GetServerItemCount() { return (DWORD)m_MapServerID.size(); }
  // 聊天数目
  DWORD GetChatItemCount() { return (DWORD)m_MapChatID.size(); }

  // 用户管理
public:
  // 删除用户
  bool DeleteUserItem(DWORD dwUserID, WORD wServerID);
  // 激活用户
  bool ActiveUserItem(tagGlobalUserInfo& GlobalUserInfo, WORD wServerID);

  // 广场管理
public:
  // 删除广场
  bool DeletePlazaItem(WORD wPlazaID);
  // 激活广场
  bool ActivePlazaItem(WORD wBindIndex, tagGamePlaza& GamePlaza);

  // 房间管理
public:
  // 删除房间
  bool DeleteServerItem(WORD wServerID);
  // 激活房间
  bool ActiveServerItem(WORD wBindIndex, tagGameServer& GameServer);

  // 聊天管理
public:
  // 删除聊天
  bool DeleteChatItem(WORD wChatID);
  // 激活聊天
  bool ActiveChatItem(WORD wBindIndex, tagChatServer& ChatServer);

  // 服务查找
public:
  // 寻找广场
  CGlobalPlazaItem* SearchPlazaItem(WORD wPlazaID);
  // 寻找房间
  CGlobalServerItem* SearchServerItem(WORD wServerID);
  // 寻找聊天
  CGlobalChatItem* SearchChatItem(WORD wChatID);

  // 用户查找
public:
  // 寻找用户
  CGlobalUserItem* SearchUserItemByUserID(DWORD dwUserID);
  // 寻找用户
  CGlobalUserItem* SearchUserItemByGameID(DWORD dwGameID);
  // 寻找用户
  CGlobalUserItem* SearchUserItemByNickName(LPCTSTR pszNickName);

  // 枚举函数
public:
  // 枚举用户
  CGlobalUserItem* EnumUserItem(CMapUserID::iterator* itr);
  // 枚举广场
  CGlobalPlazaItem* EnumPlazaItem(CMapPlazaID::iterator* itr);
  // 枚举房间
  CGlobalServerItem* EnumServerItem(CMapServerID::iterator* itr);
  // 枚举聊天
  CGlobalChatItem* EnumChatItem(CMapChatID::iterator* itr);

  // 创建函数
private:
  // 创建用户
  CGlobalUserItem* CreateGlobalUserItem();
  // 创建广场
  CGlobalPlazaItem* CreateGlobalPlazaItem();
  // 创建房间
  CGlobalServerItem* CreateGlobalServerItem();
  // 创建聊天
  CGlobalChatItem* CreateGlobalChatItem();

  // 释放函数
private:
  // 释放用户
  bool FreeGlobalUserItem(CGlobalUserItem* pGlobalUserItem);
  // 释放广场
  bool FreeGlobalPlazaItem(CGlobalPlazaItem* pGlobalPlazaItem);
  // 释放房间
  bool FreeGlobalServerItem(CGlobalServerItem* pGlobalServerItem);
  // 释放聊天
  bool FreeGlobalChatItem(CGlobalChatItem* pGlobalChatItem);
};

//////////////////////////////////////////////////////////////////////////////////
