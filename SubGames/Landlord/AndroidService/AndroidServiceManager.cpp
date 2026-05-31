#include "AndroidServiceManager.h"
#include "AndroidUserItemSink.h"

//////////////////////////////////////////////////////////////////////////
CGameServiceManager* CGameServiceManager::m_pGameServiceManager = NULL;

// 构造函数
CGameServiceManager::CGameServiceManager() {
  ASSERT(m_pGameServiceManager == NULL);
  if (m_pGameServiceManager == NULL) {
    m_pGameServiceManager = this;
  }
}

// 析构函数
CGameServiceManager::~CGameServiceManager() {
  ASSERT(m_pGameServiceManager == this);
  if (m_pGameServiceManager == this) {
    m_pGameServiceManager = NULL;
  }
}

// 接口查询
void* CGameServiceManager::QueryInterface(const GUID& Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IGameServiceManager, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IGameServiceManager, Guid, dwQueryVer);
  return NULL;
}

// 创建机器
VOID* CGameServiceManager::CreateAndroidUserItemSink(REFGUID Guid, DWORD dwQueryVer) {
  // 变量定义
  CAndroidUserItemSink* pAndroidUserItemSink = new (std::nothrow) CAndroidUserItemSink();
  if (pAndroidUserItemSink == NULL) {
    CLogger::Error(TEXT("创建失败"));
    return NULL;
  }

  // 查询接口
  VOID* pObject = pAndroidUserItemSink->QueryInterface(Guid, dwQueryVer);
  if (pObject == NULL) {
    CLogger::Error(TEXT("接口查询失败"));
    return NULL;
  }

  return pAndroidUserItemSink;
}

// 创建游戏桌
VOID* CGameServiceManager::CreateTableFrameSink(REFGUID Guid, DWORD dwQueryVer) {
  return NULL;
}
// 创建数据
VOID* CGameServiceManager::CreateGameDataBaseEngineSink(REFGUID Guid, DWORD dwQueryVer) {
  return NULL;
}

// 获取属性
bool CGameServiceManager::GetServiceAttrib(tagGameServiceAttrib& GameServiceAttrib) {
  return true;
}

// 参数修改
bool CGameServiceManager::RectifyParameter(tagGameServiceOption& GameServiceOption) {
  // 效验参数
  ASSERT(&GameServiceOption != NULL);
  if (&GameServiceOption == NULL) {
    return false;
  }

  // 单元积分
  GameServiceOption.lCellScore -= std::max((LONG) 1, GameServiceOption.lCellScore);

  // 积分下限
  GameServiceOption.lMinTableScore = std::max((SCORE) 0L, GameServiceOption.lMinTableScore);

  // 积分上限
  if (GameServiceOption.lRestrictScore != 0L) {
    GameServiceOption.lRestrictScore = std::max(GameServiceOption.lRestrictScore, GameServiceOption.lMinTableScore);
  }

  CopyMemory(&m_GameServiceOption, &GameServiceOption, sizeof(tagGameServiceOption));

  return true;
}

// 建立对象
DECLARE_CREATE_MODULE(GameServiceManager)

//////////////////////////////////////////////////////////////////////////
