#include "GameServiceManager.h"
#include "TableFrameSink.h"

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CGameServiceManager::CGameServiceManager() {
  // 内核属性
  m_GameServiceAttrib.wKindID = KIND_ID;
  m_GameServiceAttrib.wChairCount = GAME_PLAYER;
  m_GameServiceAttrib.wSupporType = (GAME_GENRE_GOLD | GAME_GENRE_SCORE | GAME_GENRE_MATCH | GAME_GENRE_EDUCATE);

  // 功能标志
  m_GameServiceAttrib.cbDynamicJoin = FALSE;
  m_GameServiceAttrib.cbAndroidUser = TRUE;
  m_GameServiceAttrib.cbOffLineTrustee = TRUE;

  // 服务属性
  m_GameServiceAttrib.dwServerVersion = VERSION_SERVER;
  m_GameServiceAttrib.dwClientVersion = VERSION_CLIENT;
  lstrcpyn(m_GameServiceAttrib.szGameName, GAME_NAME, CountArray(m_GameServiceAttrib.szGameName));
  lstrcpyn(m_GameServiceAttrib.szDataBaseName, szTreasureDB, CountArray(m_GameServiceAttrib.szDataBaseName));
  lstrcpyn(m_GameServiceAttrib.szServerDLLName, GAME_COMPONENT_DLL_NAME, CountArray(m_GameServiceAttrib.szServerDLLName));
}

// 接口查询
VOID* CGameServiceManager::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IGameServiceManager, Guid, dwQueryVer);
  QUERYINTERFACE(IGameServiceCustomRule, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IGameServiceManager, Guid, dwQueryVer);
  return NULL;
}

// 创建桌子
VOID* CGameServiceManager::CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer) {
  // 变量定义
  CTableFrameSink* pTableFrameSink = new (std::nothrow) CTableFrameSink();
  if (pTableFrameSink == NULL) {
    CLogger::Error(TEXT("创建桌子失败"));
    return nullptr;
  }

  // 查询接口
  VOID* pObject = pTableFrameSink->QueryInterface(Guid, dwQueryVer);
  if (pObject == NULL) {
    CLogger::Error(TEXT("接口查询失败"));
    return nullptr;
  }

  return pTableFrameSink;
}

// 创建机器
VOID* CGameServiceManager::CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer) {
  // 创建组件
  if (m_AndroidServiceHelper.GetInterface() == NULL) {
    m_AndroidServiceHelper.SetModuleCreateInfo(ANDROID_SERVICE_DLL_NAME, "CreateGameServiceManager");

    if (!m_AndroidServiceHelper.CreateInstance()) {
      CLogger::Error(TEXT("创建机器人服务失败"));
      return nullptr;
    }
    m_AndroidServiceHelper->RectifyParameter(m_GameServiceOption);
  }

  // 创建机器人
  VOID* pAndroidObject = m_AndroidServiceHelper->CreateAndroidUserItemSink(Guid, dwQueryVer);
  if (pAndroidObject == NULL) {
    CLogger::Error(TEXT("创建机器人接收器失败"));
    return nullptr;
  }

  return pAndroidObject;
}

// 创建数据
VOID* CGameServiceManager::CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer) {
  return NULL;
}

// 组件属性
bool CGameServiceManager::GetServiceAttrib(tagGameServiceAttrib& GameServiceAttrib) {
  // 设置变量
  GameServiceAttrib = m_GameServiceAttrib;

  return true;
}

// 调整参数
bool CGameServiceManager::RectifyParameter(tagGameServiceOption& GameServiceOption) {
  // 保存房间选项
  m_GameServiceOption = GameServiceOption;

  // 单元积分
  GameServiceOption.lCellScore = std::max((LONG)1L, GameServiceOption.lCellScore);

  tagCustomRule* pCustomRule = (tagCustomRule*) GameServiceOption.cbCustomRule;

  // 金币游戏
  if (GameServiceOption.wServerType & (GAME_GENRE_GOLD | GAME_GENRE_EDUCATE | SCORE_GENRE_POSITIVE)) {
    GameServiceOption.lMinTableScore = std::max((SCORE)(GameServiceOption.lCellScore * pCustomRule->wMaxScoreTimes), GameServiceOption.lMinTableScore);
  }

  // 最大倍数
  if (pCustomRule->wMaxScoreTimes < 32 || pCustomRule->wMaxScoreTimes > 512) {
    pCustomRule->wMaxScoreTimes = 32;
  }

  // 逃跑倍数
  if (pCustomRule->wFleeScoreTimes < 2 || pCustomRule->wFleeScoreTimes > 512) {
    pCustomRule->wFleeScoreTimes = 12;
  }
  // 出牌时间
  if (pCustomRule->cbTimeOutCard < 5 || pCustomRule->cbTimeOutCard > 60) {
    pCustomRule->cbTimeOutCard = 20;
  }
  // 叫分时间
  if (pCustomRule->cbTimeCallScore < 5 || pCustomRule->cbTimeCallScore > 60) {
    pCustomRule->cbTimeCallScore = 20;
  }
  // 开始时间
  if (pCustomRule->cbTimeStartGame < 5 || pCustomRule->cbTimeStartGame > 60) {
    pCustomRule->cbTimeStartGame = 30;
  }
  // 首出时间
  if (pCustomRule->cbTimeHeadOutCard < 5 || pCustomRule->cbTimeHeadOutCard > 60) {
    pCustomRule->cbTimeHeadOutCard = 30;
  }

  return true;
}

// 获取配置
bool CGameServiceManager::SaveCustomRule(LPBYTE pcbCustomRule, WORD wCustonSize) {
  // 变量定义
  ASSERT(wCustonSize >= sizeof(tagCustomRule));
  tagCustomRule* pCustomRule = (tagCustomRule*) pcbCustomRule;
  m_GameCustomRule = *pCustomRule;
  return true;
}

// 默认配置
bool CGameServiceManager::DefaultCustomRule(LPBYTE pcbCustomRule, WORD wCustonSize) {
  // 变量定义
  ASSERT(wCustonSize >= sizeof(tagCustomRule));
  tagCustomRule* pCustomRule = (tagCustomRule*) pcbCustomRule;

  // 设置变量
  pCustomRule->cbTimeOutCard = 20;
  pCustomRule->cbTimeStartGame = 30;
  pCustomRule->cbTimeCallScore = 20;
  pCustomRule->cbTimeHeadOutCard = 30;

  // 游戏控制
  pCustomRule->wMaxScoreTimes = 32;
  pCustomRule->wFleeScoreTimes = 12;
  pCustomRule->cbFleeScorePatch = FALSE;

  return true;
}

//////////////////////////////////////////////////////////////////////////////////

// 建立对象函数
extern "C" PLATFORM_EXPORT VOID* CreateGameServiceManager(const GUID& Guid, DWORD dwInterfaceVer) {
  static CGameServiceManager GameServiceManager;
  return GameServiceManager.QueryInterface(Guid, dwInterfaceVer);
}

// 游戏组件版本
extern "C" PLATFORM_EXPORT uint32_t GetFileVersion() { return VERSION_SERVER; }

//////////////////////////////////////////////////////////////////////////////////
