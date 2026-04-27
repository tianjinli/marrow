#include "AndroidUserManager.h"

#include <ranges>

//////////////////////////////////////////////////////////////////////////////////

// 常量定义
#define ANDROID_LOGON_COUNT 32 // 登录数目
#define ADNDROID_PLAY_DRAW 10 // 游戏局数
#define ANDROID_UNLOAD_TIME 30 * 60 // 不加载时间
#define ANDROID_UNIN_TIME 10 * 60 // 不进入时间

// 时间标识
#define IDI_ANDROID_IN (IDI_REBOT_MODULE_START + 0) // 进入时间
#define IDI_ANDROID_OUT (IDI_REBOT_MODULE_START + MAX_BATCH) // 退出时间
#define IDI_ANDROID_PULSE (IDI_REBOT_MODULE_START + 2 * MAX_BATCH) // 脉冲时间

// 机器时间
#define TIME_ANDROID_INOUT 15L // 登录时间
#define TIME_ANDROID_PULSE 1L // 脉冲时间
#define TIME_LOAD_ANDROID_INFO 900L // 加载用户
#define TIME_ANDROID_REPOSE_TIME 1800L // 服务时间

//////////////////////////////////////////////////////////////////////////////////
#ifndef _WIN32
void GetLocalTime(SYSTEMTIME* system_time) {
  using namespace std::chrono;

  // 获取当前时间点
  auto now = system_clock::now();

  // 转换为 time_t（秒）
  auto now_time_t = system_clock::to_time_t(now);
      std::tm local_tm{};
  localtime_r(&now_time_t, &local_tm);
  // 计算毫秒
  auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  system_time->wYear = local_tm.tm_year + 1900;
  system_time->wMonth = local_tm.tm_mon + 1;
  system_time->wDay = local_tm.tm_mday;
  system_time->wHour = local_tm.tm_hour;
  system_time->wMinute = local_tm.tm_min;
  system_time->wSecond = local_tm.tm_sec;
  system_time->wMilliseconds = (int)ms.count();
}
#endif

// 构造函数
CAndroidUserManager::CAndroidUserManager() {
  // 系统参数
  m_pGameParameter = nullptr;
  m_pGameServiceAttrib = nullptr;
  m_pGameServiceOption = nullptr;
  m_pGameMatchOption = nullptr;

  // 组件接口
  m_pITimerEngine = nullptr;
  m_pIServerUserManager = nullptr;
  m_pIGameServiceManager = nullptr;
  m_pIGameServiceSustomTime = nullptr;
  m_pITCPNetworkEngineEvent = nullptr;
  m_pIMainServiceFrame = nullptr;

  // 配置变量
  m_dwMinSitInterval = 0;
  m_dwMaxSitInterval = 0;
  m_bServiceContinue = false;
  m_wAutoAndroidCount = ANDROID_LOGON_COUNT;

  // 库存变量
  m_wStockCount = 0;
  ZeroMemory(&m_AndroidItemConfig, sizeof(m_AndroidItemConfig));

  // 设置索引
  // std::unordered_map 无需 InitHashTable，自动管理
  // 如果你想预分配桶，可以加： task_parameter_map_.reserve(PRIME_ANDROID_USER);
}

// 析构函数
CAndroidUserManager::~CAndroidUserManager() {
  // 清理断言
  ASSERT(android_user_item_map_.size() == 0L);
  ASSERT(m_AndroidParameterArray.GetCount() == 0L);
  ASSERT(m_AndroidUserItemArray.GetCount() == 0L);
  ASSERT(m_AndroidUserItemBuffer.GetCount() == 0L);
}

// 接口查询
VOID* CAndroidUserManager::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IAndroidUserManager, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IAndroidUserManager, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CAndroidUserManager::StartService() {
  // 时间间隔
  DWORD dwTimeCell = TIME_CELL;
  DWORD dwElapse = TIME_ANDROID_PULSE * 1000L;

  // 调整时间
  if (m_pIGameServiceSustomTime != nullptr) {
    dwTimeCell = m_pIGameServiceSustomTime->GetTimerEngineTimeCell();
    if (dwTimeCell > TIME_CELL)
      dwTimeCell = TIME_CELL;

    dwElapse = m_pIGameServiceSustomTime->GetAndroidTimerPulse();
    if (dwElapse < dwTimeCell)
      dwElapse = dwTimeCell;
  }

  // 启动时间
  m_pITimerEngine->SetTimer(IDI_ANDROID_PULSE, dwElapse, TIMES_INFINITY, 0);

  return true;
}

// 停止服务
bool CAndroidUserManager::ConcludeService() {
  // 删除存储
  for (INT_PTR i = 0; i < m_AndroidUserItemArray.GetCount(); i++) {
    SafeRelease(m_AndroidUserItemArray[i]);
  }

  // 删除存储
  for (INT_PTR i = 0; i < m_AndroidParameterArray.GetCount(); i++) {
    SafeDelete(m_AndroidParameterArray[i]);
  }

  // 清理数组
  android_user_item_map_.clear();
  m_AndroidParameterArray.RemoveAll();
  m_AndroidUserItemArray.RemoveAll();
  m_AndroidUserItemBuffer.RemoveAll();

  // 库存用户
  m_wStockCount = 0;
  ZeroMemory(&m_AndroidItemConfig, sizeof(m_AndroidItemConfig));

  return true;
}

// 配置组件
bool CAndroidUserManager::InitAndroidUser(tagAndroidUserParameter& AndroidUserParameter) {
  // 服务配置
  m_bServiceContinue = AndroidUserParameter.bServiceContinue;
  m_dwMinSitInterval = AndroidUserParameter.dwMinSitInterval;
  m_dwMaxSitInterval = AndroidUserParameter.dwMaxSitInterval;

  // 设置变量
  m_pGameParameter = AndroidUserParameter.pGameParameter;
  m_pGameServiceAttrib = AndroidUserParameter.pGameServiceAttrib;
  m_pGameServiceOption = AndroidUserParameter.pGameServiceOption;
  m_pGameMatchOption = AndroidUserParameter.pGameMatchOption;

  // 组件接口
  m_pITimerEngine = AndroidUserParameter.pITimerEngine;
  m_pIServerUserManager = AndroidUserParameter.pIServerUserManager;
  m_pIGameServiceManager = AndroidUserParameter.pIGameServiceManager;
  m_pIGameServiceSustomTime = AndroidUserParameter.pIGameServiceSustomTime;
  m_pITCPNetworkEngineEvent = AndroidUserParameter.pITCPNetworkEngineEvent;

  return true;
}

// 移除参数
bool CAndroidUserManager::RemoveAndroidParameter(DWORD dwBatchID) {
  // 获取参数
  tagAndroidParameterEx* pAndroidParameter = GetAndroidParameter(dwBatchID);
  if (pAndroidParameter == nullptr)
    return false;

  // 获取时间
  SYSTEMTIME SystemTime;
  GetLocalTime(&SystemTime);
  DWORD dwTodayTickCount = GetTodayTickCount(SystemTime);

  // 调整参数
  pAndroidParameter->bIsValided = false;
  pAndroidParameter->AndroidParameter.dwLeaveTime = dwTodayTickCount;

  // 获取索引
  WORD wParameterIndex = GetAndroidParameterIndex(pAndroidParameter);
  if (wParameterIndex == INVALID_WORD)
    return true;

  // 关闭定时器
  m_pITimerEngine->KillTimer(IDI_ANDROID_IN + wParameterIndex);

  // 删掉库存
  for (int i = m_wStockCount - 1; i >= 0; i--) {
    // 批次判断
    if (m_AndroidItemConfig[i].pAndroidParameter != &pAndroidParameter->AndroidParameter)
      continue;

    // 删除存存
    m_wStockCount--;
    m_AndroidItemConfig[i] = m_AndroidItemConfig[m_wStockCount];
  }

  // 加载判断
  if (pAndroidParameter->bIsLoadAndroid == false) {
    // 释放资源
    SafeDelete(pAndroidParameter);
    m_AndroidParameterArray[wParameterIndex] = nullptr;

    return true;
  }

  // 开启定时器
  m_pITimerEngine->SetTimer(IDI_ANDROID_OUT + wParameterIndex, TIME_ANDROID_INOUT * 1000L, TIMES_INFINITY, (WPARAM) pAndroidParameter);

  return true;
}

// 设置参数
bool CAndroidUserManager::AddAndroidParameter(tagAndroidParameter AndroidParameter[], WORD wParameterCount) {
  // 变量定义
  tagAndroidParameterEx* pAndroidParameter = nullptr;

  for (WORD wIndex = 0; wIndex < wParameterCount; wIndex++) {
    // 数目限制
    if (m_AndroidParameterArray.GetCount() >= MAX_BATCH)
      break;

    // 设置资源
    pAndroidParameter = GetAndroidParameter(AndroidParameter[wIndex].dwBatchID);
    if (pAndroidParameter == nullptr) {
      // 申请资源
      pAndroidParameter = new tagAndroidParameterEx();
      if (pAndroidParameter == nullptr)
        return false;

      // 设置变量
      pAndroidParameter->bIsValided = true;
      pAndroidParameter->bIsLoadAndroid = false;

      // 变量定义
      bool bVacancyExist = false;

      // 查找空缺
      for (INT_PTR nIndex = 0; nIndex < m_AndroidParameterArray.GetCount(); nIndex++) {
        if (m_AndroidParameterArray[nIndex] == nullptr) {
          bVacancyExist = true;
          m_AndroidParameterArray[nIndex] = pAndroidParameter;
          break;
        }
      }

      // 不存在空缺
      if (bVacancyExist == false) {
        m_AndroidParameterArray.Add(pAndroidParameter);
      }
    }

    // 拷贝数据
    CopyMemory(pAndroidParameter, &AndroidParameter[wIndex], sizeof(tagAndroidParameter));
  }

  return true;
}

// 插入机器
bool CAndroidUserManager::InsertAndroidInfo(tagAndroidAccountsInfo AndroidAccountsInfo[], WORD wAndroidCount, DWORD dwBatchID) {
  // 获取参数
  tagAndroidParameterEx* pAndroidParameter = GetAndroidParameter(dwBatchID);
  if (pAndroidParameter == nullptr)
    return false;

  // 记录库存
  CAndroidUserItem* pAndroidUserItem = nullptr;
  WORD wStockCount = m_wStockCount;
  bool bAndroidExist = false;

  // 添加新机器
  for (WORD wIndex = 0; wIndex < wAndroidCount; wIndex++) {
    // 设置变量
    bAndroidExist = false;

    // 从库存中查找
    for (WORD wItem = 0; wItem < wStockCount; wItem++) {
      if (m_AndroidItemConfig[wItem].AndroidAccountsInfo.dwUserID == AndroidAccountsInfo[wIndex].dwUserID) {
        bAndroidExist = true;
        break;
      }
    }

    // 存在判断
    if (bAndroidExist == true)
      continue;

    // 从映射中查找
    if (android_user_item_map_.contains(AndroidAccountsInfo[wIndex].dwUserID)) {
      continue;
    }
    // 添加机器
    m_AndroidItemConfig[m_wStockCount].AndroidAccountsInfo = AndroidAccountsInfo[wIndex];
    m_AndroidItemConfig[m_wStockCount].pAndroidParameter = &pAndroidParameter->AndroidParameter;

    // 增加库存
    m_wStockCount++;
  }

  // 获取索引
  WORD wParameterIndex = GetAndroidParameterIndex(pAndroidParameter);
  if (wParameterIndex == INVALID_WORD)
    return true;

  // 开启定时器
  m_pITimerEngine->KillTimer(IDI_ANDROID_IN + wParameterIndex);
  m_pITimerEngine->SetTimer(IDI_ANDROID_IN + wParameterIndex, TIME_ANDROID_INOUT * 1000L, TIMES_INFINITY, (WPARAM) pAndroidParameter);

  return true;
}

// 删除机器
bool CAndroidUserManager::DeleteAndroidUserItem(DWORD dwAndroidID, bool bStockRetrieve) {
  // 获取对象
  WORD wIndex = LOWORD(dwAndroidID);
  CAndroidUserItem* pAndroidUserItem = GetAndroidUserItem(wIndex);

  // 对象效验
  // ASSERT((pAndroidUserItem!=nullptr)&&(pAndroidUserItem->m_wRoundID==HIWORD(dwAndroidID)));
  if ((pAndroidUserItem == nullptr) || (pAndroidUserItem->m_wRoundID != HIWORD(dwAndroidID)))
    return false;

  // 关闭事件
  try {
    m_pITCPNetworkEngineEvent->OnEventTCPNetworkShut(dwAndroidID, 0, 0L);
  } catch (...) {
    // 错误断言
    ASSERT(FALSE);
  }

  // 机器配置
  tagAndroidItemConfig AndroidItemConfig = pAndroidUserItem->m_AndroidItemConfig;

  // 删除对象
  FreeAndroidUserItem(pAndroidUserItem->GetUserID());

  ////库存回收
  // if(bStockRetrieve==true)
  //{
  //	InsertAndroidInfo(&AndroidItemConfig.AndroidAccountsInfo,1,AndroidItemConfig.pAndroidParameter->dwBatchID);
  // }

  return true;
}

// 查找机器
IAndroidUserItem* CAndroidUserManager::SearchAndroidUserItem(DWORD dwUserID, DWORD dwContextID) {
  // 查找机器
  CAndroidUserItem* pAndroidUserItem = nullptr;
  auto it = android_user_item_map_.find(dwUserID);
  if (it != android_user_item_map_.end()) {
    pAndroidUserItem = it->second;
  }
  // 机器判断
  if (pAndroidUserItem != nullptr) {
    WORD wRoundID = pAndroidUserItem->m_wRoundID;
    WORD wAndroidIndex = pAndroidUserItem->m_wAndroidIndex;
    if (MAKELONG(wAndroidIndex, wRoundID) == dwContextID)
      return pAndroidUserItem;
  }

  return nullptr;
}

// 创建机器
IAndroidUserItem* CAndroidUserManager::CreateAndroidUserItem(tagAndroidItemConfig& AndroidItemConfig) {
  // 效验参数
  ASSERT(AndroidItemConfig.AndroidAccountsInfo.dwUserID != 0L);
  if (AndroidItemConfig.AndroidAccountsInfo.dwUserID == 0L)
    return nullptr;

  // 创建对象
  CAndroidUserItem* pAndroidUserItem = ActiveAndroidUserItem(AndroidItemConfig);
  if (pAndroidUserItem == nullptr)
    return nullptr;

  // 属性变量
  WORD wRoundID = pAndroidUserItem->m_wRoundID;
  WORD wAndroidIndex = pAndroidUserItem->m_wAndroidIndex;

  // 连接模拟
  try {
    if (m_pITCPNetworkEngineEvent->OnEventTCPNetworkBind(MAKELONG(wAndroidIndex, wRoundID), 0L) == false) {
      throw 0;
    }
  } catch (...) {
    // 错误断言
    ASSERT(FALSE);

    // 释放用户
    FreeAndroidUserItem(AndroidItemConfig.AndroidAccountsInfo.dwUserID);

    return nullptr;
  }

  // 变量定义
  CMD_GR_LogonUserID LogonUserID;
  ZeroMemory(&LogonUserID, sizeof(LogonUserID));

  // 版本信息
  LogonUserID.dwPlazaVersion = VERSION_PLAZA;
  LogonUserID.dwFrameVersion = VERSION_FRAME;
  LogonUserID.dwProcessVersion = m_pGameServiceAttrib->dwClientVersion;

  // 用户信息
  LogonUserID.dwUserID = AndroidItemConfig.AndroidAccountsInfo.dwUserID;
  LSTRCPYN(LogonUserID.szPassword, AndroidItemConfig.AndroidAccountsInfo.szPassword, std::size(LogonUserID.szPassword));
  LogonUserID.wKindID = m_pGameServiceOption->wKindID;

  // 构造数据
  TCP_Command Command;
  Command.wMainCmdID = MDM_GR_LOGON;
  Command.wSubCmdID = SUB_GR_LOGON_USERID;

  // 消息处理
  try {
    // 发送数据
    if (m_pITCPNetworkEngineEvent->OnEventTCPNetworkRead(MAKELONG(wAndroidIndex, wRoundID), Command, &LogonUserID, sizeof(LogonUserID)) == false) {
      throw 0;
    }
  } catch (...) {
    // 错误断言
    ASSERT(FALSE);

    // 删除机器
    DeleteAndroidUserItem(MAKELONG(wAndroidIndex, wRoundID), false);

    return nullptr;
  }

  return pAndroidUserItem;
}

// 设置接口
VOID CAndroidUserManager::SetMainServiceFrame(IMainServiceFrame* pIMainServiceFrame) {
  m_pIMainServiceFrame = pIMainServiceFrame;

  return;
}

// 脉冲事件
bool CAndroidUserManager::OnEventTimerPulse(DWORD dwTimerID, WPARAM dwBindParameter) {
  // 进入处理
  if (dwTimerID >= IDI_ANDROID_IN && dwTimerID < IDI_ANDROID_IN + MAX_BATCH) {
    // 获取时间
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    DWORD dwTodayTickCount = GetTodayTickCount(SystemTime);

    // 获取参数
    tagAndroidParameter* pAndroidParameter = (tagAndroidParameter*) dwBindParameter;
    if (pAndroidParameter == nullptr) {
      // 关闭定时器
      m_pITimerEngine->KillTimer(dwTimerID);

      return true;
    }

    // 变量定义
    bool bAllowAndroidAttend = CServerRule::IsAllowAndroidAttend(m_pGameServiceOption->dwServerRule);
    bool bAllowAndroidSimulate = CServerRule::IsAllowAndroidSimulate(m_pGameServiceOption->dwServerRule);

    // 成功标识
    bool bLogonSuccessed = false;

    // 登录处理
    if ((bAllowAndroidAttend == true) || (bAllowAndroidSimulate == true)) {
      // 寻找机器
      for (WORD i = 0; i < m_wStockCount; i++) {
        // 批次判断
        if (m_AndroidItemConfig[i].pAndroidParameter != pAndroidParameter)
          continue;

        // 模拟判断
        if ((m_pGameServiceOption->wServerType & GAME_GENRE_MATCH) && (pAndroidParameter->dwServiceMode & ANDROID_SIMULATE) != 0 &&
            (pAndroidParameter->dwServiceMode & ANDROID_PASSIVITY) == 0 && (pAndroidParameter->dwServiceMode & ANDROID_INITIATIVE) == 0) {
          continue;
        }

        // 创建机器
        if (CreateAndroidUserItem(m_AndroidItemConfig[i]) != nullptr) {
          // 删除存存
          m_wStockCount--;
          bLogonSuccessed = true;
          m_AndroidItemConfig[i] = m_AndroidItemConfig[m_wStockCount];

          break;
        }
      }
    }

    // 关闭定时器
    m_pITimerEngine->KillTimer(dwTimerID);

    // 定时器处理
    if (bLogonSuccessed == true) {
      // 计算流逝时间
      DWORD dwElapse =
          pAndroidParameter->dwEnterMinInterval + rand() % (pAndroidParameter->dwEnterMaxInterval - pAndroidParameter->dwEnterMinInterval + 1);
      if (dwElapse == 0)
        dwElapse = TIME_ANDROID_INOUT;

      // 设置定时器
      m_pITimerEngine->SetTimer(dwTimerID, dwElapse * 1000L, TIMES_INFINITY, dwBindParameter);
    } else {
      // 剩余时间
      DWORD dwRemaindTime = BatchServiceRemaindTime(pAndroidParameter, dwTodayTickCount);
      if (dwRemaindTime <= ANDROID_UNIN_TIME) {
        // 设置定时器
        m_pITimerEngine->SetTimer(IDI_ANDROID_OUT + (dwTimerID - IDI_ANDROID_IN), TIME_ANDROID_INOUT * 1000L, TIMES_INFINITY, dwBindParameter);

        CLogger::Info(TEXT("本批次机器人开始退出, 批次ID={}"), pAndroidParameter->dwBatchID);
      } else {
        // 设置定时器
        m_pITimerEngine->SetTimer(dwTimerID, TIME_ANDROID_INOUT * 1000L, TIMES_INFINITY, dwBindParameter);
      }
    }

    return true;
  }

  // 退出处理
  if (dwTimerID >= IDI_ANDROID_OUT && dwTimerID < IDI_ANDROID_OUT + MAX_BATCH) {
    // 变量定义
    DWORD dwUserID = 0L;

    // 获取时间
    SYSTEMTIME SystemTime;
    GetLocalTime(&SystemTime);
    DWORD dwTodayTickCount = GetTodayTickCount(SystemTime);

    // 获取参数
    tagAndroidParameterEx* pAndroidParameter = (tagAndroidParameterEx*) dwBindParameter;
    if (pAndroidParameter == nullptr) {
      // 关闭定时器
      m_pITimerEngine->KillTimer(dwTimerID);

      return true;
    }

    // 时间判断
    if (BatchServiceRemaindTime(&pAndroidParameter->AndroidParameter, dwTodayTickCount) > 0) {
      return true;
    }

    // 变量定义
    bool bLogoutMarked = false;
    bool bLogoutFinished = true;

    // 退出处理
    for (const auto& value : android_user_item_map_ | std::views::values) {
      if (!value)
        continue;
      // 获取对象
      CAndroidUserItem* pAndroidUserItem = value;

      // 退出判断
      if (pAndroidUserItem->GetAndroidParameter() == &pAndroidParameter->AndroidParameter) {
        // 设置变量
        bLogoutFinished = false;

        // 变量定义
        IServerUserItem* pIServerUserItem = pAndroidUserItem->m_pIServerUserItem;
        if (pIServerUserItem != nullptr) {
          // 服务状态
          BYTE cbUserStatus = pIServerUserItem->GetUserStatus();
          BYTE cbMatchStatus = pIServerUserItem->GetUserMatchStatus();
          if ((cbUserStatus != US_FREE) && (cbUserStatus != US_SIT)) {
            if (m_pGameServiceOption->wServerType == GAME_GENRE_MATCH && m_pGameMatchOption->cbMatchType == MATCH_TYPE_IMMEDIATE) {
              // 防止中途换桌时UserStatus变为US_FREE导致卡赛，此时使用SignUpStatus判断可否被踢出
              if (cbMatchStatus != MUS_NULL && pAndroidUserItem->m_bWaitLeave == false) {
                pAndroidUserItem->m_bWaitLeave = true;
              }
            } else {
              // 标记动作
              if (bLogoutMarked == false && pAndroidUserItem->m_wAndroidAction == 0) {
                bLogoutMarked = true;
                pAndroidUserItem->m_wAndroidAction |= ANDROID_WAITLEAVE;
              }
            }
            continue;
          } else {
            if (m_pGameServiceOption->wServerType == GAME_GENRE_MATCH && m_pGameMatchOption->cbMatchType == MATCH_TYPE_IMMEDIATE) {
              WORD wTableID = pIServerUserItem->GetTableID();
              if ((cbMatchStatus != MUS_NULL || pAndroidUserItem->m_bWaitLeave == false) && wTableID != INVALID_TABLE) {
                pAndroidUserItem->m_bWaitLeave = true;
                continue;
              }
            }
          }
        }

        // 删除用户
        WORD wRoundID = pAndroidUserItem->m_wRoundID;
        WORD wAndroidIndex = pAndroidUserItem->m_wAndroidIndex;
        DeleteAndroidUserItem(MAKELONG(wAndroidIndex, wRoundID), false);

        // 设置标识
        if (pIServerUserItem != nullptr)
          break;
      }
    }

    // 关闭定时器
    m_pITimerEngine->KillTimer(dwTimerID);

    // 定时器处理
    if (bLogoutFinished == false) {
      // 计算流逝时间
      DWORD dwElapse = pAndroidParameter->AndroidParameter.dwLeaveMinInterval +
                       rand() % (pAndroidParameter->AndroidParameter.dwLeaveMaxInterval - pAndroidParameter->AndroidParameter.dwLeaveMinInterval + 1);
      if (dwElapse == 0)
        dwElapse = TIME_ANDROID_INOUT;

      // 设置定时器
      m_pITimerEngine->SetTimer(dwTimerID, dwElapse * 1000L, TIMES_INFINITY, dwBindParameter);
    } else {
      if (pAndroidParameter->bIsValided == false) {
        // 计算索引
        WORD wParameterIndex = (WORD) (dwTimerID - IDI_ANDROID_OUT);
        if (wParameterIndex == GetAndroidParameterIndex(pAndroidParameter)) {
          // 释放资源
          SafeDelete(m_AndroidParameterArray[wParameterIndex]);
          m_AndroidParameterArray[wParameterIndex] = nullptr;

          CLogger::Info(TEXT("本批次机器人全部退出,该批次将被删除, 批次ID={}"), pAndroidParameter->AndroidParameter.dwBatchID);

          // 解锁机器人
          if (m_pIMainServiceFrame != nullptr) {
            m_pIMainServiceFrame->UnLockAndroidUser(m_pGameServiceOption->wServerID, (WORD) pAndroidParameter->AndroidParameter.dwBatchID);
          }
        }
      } else {
        // 恢复标识
        pAndroidParameter->bIsLoadAndroid = false;

        CLogger::Info(TEXT("本批次机器人全部退出, 批次ID={}"), pAndroidParameter->AndroidParameter.dwBatchID);

        // 解锁机器人
        if (m_pIMainServiceFrame != nullptr) {
          m_pIMainServiceFrame->UnLockAndroidUser(m_pGameServiceOption->wServerID, (WORD) pAndroidParameter->AndroidParameter.dwBatchID);
        }
      }
    }

    return true;
  }

  // 用户脉冲
  if (dwTimerID == IDI_ANDROID_PULSE) {
    // 变量定义
    DWORD dwUserID = 0L;
    CAndroidUserItem* pAndroidUserItem = nullptr;

    // 用户处理
    for (const auto& value : android_user_item_map_ | std::views::values) {
      if (!value)
        continue;
      // 获取对象
      pAndroidUserItem = value;

      // 时间处理
      try {
        // ASSERT(pAndroidUserItem!=nullptr);
        if (pAndroidUserItem->m_pIServerUserItem != nullptr)
          pAndroidUserItem->OnTimerPulse(dwTimerID, dwBindParameter);
      } catch (...) {
        ASSERT(FALSE);
        DeleteAndroidUserItem(MAKELONG(pAndroidUserItem->m_wAndroidIndex, pAndroidUserItem->m_wRoundID), true);
      }
    }

    return true;
  }

  return false;
}

// 加载机器
bool CAndroidUserManager::GetAndroidLoadInfo(DWORD& dwBatchID, DWORD& dwLoadCount) {
  // 数目判断
  if (m_AndroidParameterArray.GetCount() == 0)
    return false;

  // 获取时间
  SYSTEMTIME SystemTime;
  GetLocalTime(&SystemTime);
  DWORD dwTodayTickCount = GetTodayTickCount(SystemTime);

  // 变量定义
  tagAndroidParameterEx* pAndroidParameter = nullptr;
  DWORD dwRemaindTime;

  // 退出处理
  for (INT_PTR nIndex = 0; nIndex < m_AndroidParameterArray.GetCount(); nIndex++) {
    // 获取对象
    pAndroidParameter = m_AndroidParameterArray[nIndex];
    if (pAndroidParameter == nullptr)
      continue;

    // 加载判断
    if (pAndroidParameter->bIsLoadAndroid == true)
      continue;

    // 剩余时间
    dwRemaindTime = BatchServiceRemaindTime(&pAndroidParameter->AndroidParameter, dwTodayTickCount);

    // 加载机器
    if (dwRemaindTime > ANDROID_UNLOAD_TIME) {
      // 设置标识
      pAndroidParameter->bIsLoadAndroid = true;

      // 设置参数
      dwBatchID = pAndroidParameter->AndroidParameter.dwBatchID;
      dwLoadCount = std::min(pAndroidParameter->AndroidParameter.dwAndroidCount, dwRemaindTime / pAndroidParameter->AndroidParameter.dwEnterMaxInterval);
      if (dwLoadCount == 0)
        continue;

      return true;
    }
  }

  return false;
}

// 用户状况
WORD CAndroidUserManager::GetAndroidUserInfo(tagAndroidUserInfo& AndroidUserInfo, DWORD dwServiceMode) {
  // 获取时间
  SYSTEMTIME SystemTime;
  GetLocalTime(&SystemTime);
  DWORD dwTodayTickCount = GetTodayTickCount(SystemTime);

  // 设置变量
  ZeroMemory(&AndroidUserInfo, sizeof(AndroidUserInfo));

  // 变量定义
  DWORD dwTimeMask = (1L << SystemTime.wHour);

  // 枚举对象
  for (const auto& value : android_user_item_map_ | std::views::values) {
    if (!value)
      continue;

    // 获取对象
    DWORD dwUserID = 0L;
    CAndroidUserItem* pAndroidUserItem = value;

    // 效验参数
    ASSERT((dwUserID != 0L) && (pAndroidUserItem != nullptr));
    if ((dwUserID == 0L) || (pAndroidUserItem == nullptr))
      break;

    // 绑定判断
    if (pAndroidUserItem->m_pIServerUserItem == nullptr)
      continue;

    // 变量定义
    IServerUserItem* pIServerUserItem = pAndroidUserItem->m_pIServerUserItem;
    tagAndroidParameter* pAndroidParameter = pAndroidUserItem->GetAndroidParameter();

    // 离开判断
    if (pAndroidUserItem->m_bWaitLeave)
      continue;

    // 模式判断
    if ((pAndroidParameter->dwServiceMode & dwServiceMode) == 0L)
      continue;

    // 离开判断
    if (!m_bServiceContinue && BatchServiceRemaindTime(pAndroidParameter, dwTodayTickCount) == 0)
      continue;

    // 休息时间
    DWORD dwCurrTime = (DWORD) time(nullptr);
    if ((pAndroidUserItem->m_dwStandupTickCount + pAndroidUserItem->m_dwReposeTickCount) > dwCurrTime)
      continue;

    // 状态判断
    switch (pIServerUserItem->GetUserStatus()) {
      case US_FREE: {
        ASSERT(AndroidUserInfo.wFreeUserCount < std::size(AndroidUserInfo.pIAndroidUserFree));
        AndroidUserInfo.pIAndroidUserFree[AndroidUserInfo.wFreeUserCount++] = pAndroidUserItem;
        break;
      }
      case US_SIT:
      case US_READY: {
        ASSERT(AndroidUserInfo.wSitdownUserCount < std::size(AndroidUserInfo.pIAndroidUserSitdown));
        AndroidUserInfo.pIAndroidUserSitdown[AndroidUserInfo.wSitdownUserCount++] = pAndroidUserItem;
        break;
      }
      case US_PLAYING:
      case US_OFFLINE: {
        ASSERT(AndroidUserInfo.wPlayUserCount < std::size(AndroidUserInfo.pIAndroidUserPlay));
        AndroidUserInfo.pIAndroidUserPlay[AndroidUserInfo.wPlayUserCount++] = pAndroidUserItem;
        break;
      }
    }
  };

  return AndroidUserInfo.wFreeUserCount + AndroidUserInfo.wPlayUserCount + AndroidUserInfo.wSitdownUserCount;
}

// 发送数据
bool CAndroidUserManager::SendDataToClient(WORD wMainCmdID, WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  // 发送数据
  for (INT_PTR i = 0; i < m_AndroidUserItemArray.GetCount(); i++) {
    // 获取对象
    CAndroidUserItem* pAndroidUserItem = m_AndroidUserItemArray[i];

    // 状态判断
    if (pAndroidUserItem->m_pIServerUserItem == nullptr)
      continue;
    if (pAndroidUserItem->m_AndroidItemConfig.AndroidAccountsInfo.dwUserID == 0L)
      continue;

    // 消息处理
    try {
      if (pAndroidUserItem->OnSocketRead(wMainCmdID, wSubCmdID, pData, wDataSize) == false) {
        throw 0;
      }
    } catch (...) {
      // 错误断言
      ASSERT(FALSE);

      // 断开用户
      WORD wRoundID = pAndroidUserItem->m_wRoundID;
      WORD wAndroidIndex = pAndroidUserItem->m_wAndroidIndex;
      DeleteAndroidUserItem(MAKELONG(wAndroidIndex, wRoundID), true);
    }
  }

  return true;
}

// 发送数据
bool CAndroidUserManager::SendDataToClient(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  // 获取对象
  WORD wIndex = LOWORD(dwAndroidID);
  CAndroidUserItem* pAndroidUserItem = GetAndroidUserItem(wIndex);

  // 对象效验
  // ASSERT((pAndroidUserItem!=nullptr)&&(pAndroidUserItem->m_wRoundID==HIWORD(dwAndroidID)));
  if ((pAndroidUserItem == nullptr) || (pAndroidUserItem->m_wRoundID != HIWORD(dwAndroidID)))
    return false;

  // 消息处理
  try {
    if (pAndroidUserItem->OnSocketRead(wMainCmdID, wSubCmdID, pData, wDataSize) == false) {
      ASSERT(FALSE);
      throw 0;
    }
  } catch (...) {
    ASSERT(FALSE);
    DeleteAndroidUserItem(dwAndroidID, true);
  }

  return true;
}

// 发送数据
bool CAndroidUserManager::SendDataToServer(DWORD dwAndroidID, WORD wMainCmdID, WORD wSubCmdID, VOID* pData, WORD wDataSize) {
  // 构造数据
  TCP_Command Command;
  Command.wSubCmdID = wSubCmdID;
  Command.wMainCmdID = wMainCmdID;

  // 消息处理
  try {
    if (m_pITCPNetworkEngineEvent->OnEventTCPNetworkRead(dwAndroidID, Command, pData, wDataSize) == false) {
      throw 0;
    }
  } catch (...) {
    CLogger::Error("SendDataToServer: MainID={},SubID={}", Command.wMainCmdID, Command.wSubCmdID);

    ASSERT(FALSE);
    DeleteAndroidUserItem(dwAndroidID, true);
  }

  return true;
}

// 获取对象
CAndroidUserItem* CAndroidUserManager::GetAndroidUserItem(WORD wIndex) {
  // 效验索引
  ASSERT(wIndex >= INDEX_ANDROID);
  if (wIndex < INDEX_ANDROID)
    return nullptr;

  // 效验索引
  ASSERT((wIndex - INDEX_ANDROID) < m_AndroidUserItemArray.GetCount());
  if (((wIndex - INDEX_ANDROID) >= m_AndroidUserItemArray.GetCount()))
    return nullptr;

  // 获取对象
  WORD wBufferIndex = wIndex - INDEX_ANDROID;
  CAndroidUserItem* pAndroidUserItem = m_AndroidUserItemArray[wBufferIndex];

  return pAndroidUserItem;
}

// 释放对象
VOID CAndroidUserManager::FreeAndroidUserItem(DWORD dwUserID) {
  // 效验参数
  ASSERT(dwUserID != 0);
  if (dwUserID == 0)
    return;

  // 变量定义
  CAndroidUserItem* pAndroidUserItem = nullptr;
  auto it = android_user_item_map_.find(dwUserID);
  if (it != android_user_item_map_.end()) {
    pAndroidUserItem = it->second;
  }
  // 对象判断
  if (pAndroidUserItem == nullptr) {
    ASSERT(FALSE);
    return;
  }

  // 复位对象
  pAndroidUserItem->RepositUserItem();

  // 设置索引
  android_user_item_map_.erase(dwUserID);
  m_AndroidUserItemBuffer.Add(pAndroidUserItem);

  return;
}

// 获取参数
tagAndroidParameterEx* CAndroidUserManager::GetAndroidParameter(DWORD dwBatchID) {
  // 查找数组
  for (INT_PTR nIndex = 0; nIndex < m_AndroidParameterArray.GetCount(); nIndex++) {
    if (m_AndroidParameterArray[nIndex] == nullptr)
      continue;
    if (m_AndroidParameterArray[nIndex]->AndroidParameter.dwBatchID == dwBatchID) {
      return m_AndroidParameterArray[nIndex];
    }
  }

  return nullptr;
}

// 获取索引
WORD CAndroidUserManager::GetAndroidParameterIndex(tagAndroidParameterEx* pAndroidParameter) {
  // 参数校验
  if (pAndroidParameter == nullptr)
    return INVALID_WORD;

  // 查找数组
  for (INT_PTR nIndex = 0; nIndex < m_AndroidParameterArray.GetCount(); nIndex++) {
    if (m_AndroidParameterArray[nIndex] == pAndroidParameter) {
      return (WORD) nIndex;
    }
  }

  return INVALID_WORD;
}

// 转换时间
DWORD CAndroidUserManager::GetTodayTickCount(SYSTEMTIME& SystemTime) {
  return SystemTime.wHour * 3600 + SystemTime.wMinute * 60 + SystemTime.wSecond;
}

// 剩余时间
DWORD CAndroidUserManager::BatchServiceRemaindTime(tagAndroidParameter* pAndroidParameter, DWORD dwTodayTickCount) {
  // 参数校验
  ASSERT(pAndroidParameter != nullptr);
  if (pAndroidParameter == nullptr)
    return 0;

  // 变量定义
  DWORD dwEnterTime, dwLeaveTime;
  DWORD dwRemaindTime = 0;

  // 设置变量
  dwEnterTime = pAndroidParameter->dwEnterTime;
  dwLeaveTime = pAndroidParameter->dwLeaveTime;

  // 计算时间
  // 进入时间和离开时间在同一天
  // 进入时间，离开时间都在同一天的 00:00:00 ~ 23:59:59 (0~86399)
  if (dwLeaveTime > dwEnterTime) {
    if (dwTodayTickCount >= dwEnterTime && dwTodayTickCount <= dwLeaveTime) {
      dwRemaindTime = dwLeaveTime - dwTodayTickCount;
    }
  }
  // 进入时间和离开时间不在同一天
  // 进入时间在第一天12:00:01 ~ 23:59:59 (43201~86399)
  // 离开时间在第二天 00:00:00 ~ 12:00:00 (0~43200)
  else if (dwLeaveTime < dwEnterTime) {
    // 第一天
    if (dwTodayTickCount >= dwEnterTime) {
      dwRemaindTime = dwLeaveTime + 24 * 3600 - dwTodayTickCount;
    }

    // 第二天
    if (dwTodayTickCount < dwLeaveTime) {
      dwRemaindTime = dwLeaveTime - dwTodayTickCount;
    }
  }

  return dwRemaindTime;
}

// 等待时间
DWORD CAndroidUserManager::BatchServiceAwaitTime(tagAndroidParameter* pAndroidParameter, DWORD dwTodayTickCount) {
  // 参数校验
  ASSERT(pAndroidParameter != nullptr);
  if (pAndroidParameter == nullptr)
    return 0;
  if (((tagAndroidParameterEx*) pAndroidParameter)->bIsLoadAndroid == false)
    return -1;

  // 变量定义
  DWORD dwEnterTime, dwLeaveTime;

  // 设置变量
  dwEnterTime = pAndroidParameter->dwEnterTime;
  dwLeaveTime = pAndroidParameter->dwLeaveTime;

  // 计算时间
  if (dwEnterTime < dwLeaveTime) {
    if (dwTodayTickCount < dwEnterTime) {
      return dwEnterTime - dwTodayTickCount;
    }
  } else {
    if (dwTodayTickCount < dwEnterTime && dwTodayTickCount > dwLeaveTime) {
      return dwEnterTime - dwTodayTickCount;
    }
  }

  return 0;
}

// 激活对象
CAndroidUserItem* CAndroidUserManager::ActiveAndroidUserItem(tagAndroidItemConfig& AndroidItemConfig) {
  // 变量定义
  CAndroidUserItem* pAndroidUserItem = nullptr;
  INT_PTR nFreeItemCount = m_AndroidUserItemBuffer.GetCount();

  // 获取对象
  if (nFreeItemCount > 0) {
    // 获取对象
    INT_PTR nItemPostion = nFreeItemCount - 1;
    pAndroidUserItem = m_AndroidUserItemBuffer[nItemPostion];

    // 删除数组
    m_AndroidUserItemBuffer.RemoveAt(nItemPostion);
  }

  // 创建对象
  if (pAndroidUserItem == nullptr) {
    // 数目判断
    if (m_AndroidUserItemArray.GetCount() >= MAX_ANDROID) {
      ASSERT(FALSE);
      return nullptr;
    }

    try {
      // 变量定义
      IAndroidUserItemSink* pIAndroidUserItemSink = nullptr;

      // 创建陪玩
      if (CServerRule::IsAllowAndroidAttend(m_pGameServiceOption->dwServerRule) == true) {
        pIAndroidUserItemSink =
            (IAndroidUserItemSink*) m_pIGameServiceManager->CreateAndroidUserItemSink(IID_IAndroidUserItemSink, VER_IAndroidUserItemSink);
      }

      // 创建对象
      try {
        pAndroidUserItem = new CAndroidUserItem;
      } catch (...) {
        // 错误断言
        ASSERT(FALSE);

        // 删除对象
        SafeRelease(pIAndroidUserItemSink);
      }

      // 错误判断
      if (pAndroidUserItem == nullptr) {
        // 错误断言
        ASSERT(FALSE);

        // 删除对象
        SafeRelease(pIAndroidUserItemSink);

        return nullptr;
      }

      // 设置用户
      if ((pIAndroidUserItemSink != nullptr) && (pIAndroidUserItemSink->Initialization(pAndroidUserItem) == false)) {
        // 错误断言
        ASSERT(FALSE);

        // 删除对象
        SafeRelease(pAndroidUserItem);
        SafeRelease(pIAndroidUserItemSink);

        return nullptr;
      }

      // 设置数组
      WORD wCurrentIndex = (WORD) m_AndroidUserItemArray.Add(pAndroidUserItem);

      // 配置变量
      pAndroidUserItem->m_dwMinSitInterval = m_dwMinSitInterval;
      pAndroidUserItem->m_dwMaxSitInterval = m_dwMaxSitInterval;

      // 设置接口
      pAndroidUserItem->m_wAndroidIndex = wCurrentIndex + INDEX_ANDROID;
      pAndroidUserItem->m_pIServerUserManager = m_pIServerUserManager;
      pAndroidUserItem->m_pIAndroidUserItemSink = pIAndroidUserItemSink;
      pAndroidUserItem->m_pIAndroidUserManager = QUERY_ME_INTERFACE(IAndroidUserManager);
    } catch (...) {
      ASSERT(FALSE);
      return nullptr;
    }
  }

  // 游戏局数
  if ((AndroidItemConfig.pAndroidParameter->dwSwitchMinInnings != 0L) && (AndroidItemConfig.pAndroidParameter->dwSwitchMaxInnings != 0L)) {
    // 变量定义
    DWORD dwSwitchMinInnings = AndroidItemConfig.pAndroidParameter->dwSwitchMinInnings;
    DWORD dwSwitchMaxInnings = AndroidItemConfig.pAndroidParameter->dwSwitchMaxInnings;

    // 调整局数
    if ((dwSwitchMaxInnings - dwSwitchMinInnings) > 0L)
      pAndroidUserItem->m_AndroidService.dwSwitchTableInnings = dwSwitchMinInnings + rand() % (dwSwitchMaxInnings - dwSwitchMinInnings);
    else
      pAndroidUserItem->m_AndroidService.dwSwitchTableInnings = dwSwitchMinInnings;
  } else
    pAndroidUserItem->m_AndroidService.dwSwitchTableInnings = 0;

  // 服务类型
  if (pAndroidUserItem->m_pIAndroidUserItemSink == nullptr && (AndroidItemConfig.pAndroidParameter->dwServiceMode & ANDROID_SIMULATE) != 0)
    AndroidItemConfig.pAndroidParameter->dwServiceMode = ANDROID_SIMULATE;

  // 设置变量
  pAndroidUserItem->m_AndroidItemConfig = AndroidItemConfig;

  // 设置索引
  android_user_item_map_[AndroidItemConfig.AndroidAccountsInfo.dwUserID] = pAndroidUserItem;

  return pAndroidUserItem;
}

// 强制起立
VOID CAndroidUserManager::ForceAndroidUserItemStandup(CAndroidUserItem* pAndroidUserItem) {
  // 校验参数
  ASSERT(pAndroidUserItem != nullptr);
  if (pAndroidUserItem == nullptr)
    return;

  // 属性变量
  WORD wRoundID = pAndroidUserItem->m_wRoundID;
  WORD wAndroidIndex = pAndroidUserItem->m_wAndroidIndex;

  // 构造结构
  CMD_GR_UserStandUp UserStandUp;
  UserStandUp.cbForceLeave = true;
  UserStandUp.wTableID = pAndroidUserItem->m_pIServerUserItem->GetTableID();
  UserStandUp.wChairID = pAndroidUserItem->m_pIServerUserItem->GetChairID();

  // 发送数据
  SendDataToServer(MAKELONG(wAndroidIndex, wRoundID), MDM_GR_USER, SUB_GR_USER_STANDUP, &UserStandUp, sizeof(UserStandUp));
}

//////////////////////////////////////////////////////////////////////////////////
