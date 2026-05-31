#include "DBCorrespondManager.h"

//////////////////////////////////////////////////////////////////////////
// 构造函数
CDBCorrespondManager::CDBCorrespondManager(void) {
  m_bService = false;
  m_pIKernelDataBaseEngine = NULL;
}

// 析构函数
CDBCorrespondManager::~CDBCorrespondManager(void) {
  m_pIKernelDataBaseEngine = NULL;

  ClearAmortizeData();

  m_DBRequestUserArray.clear();
}

// 接口查询
VOID* CDBCorrespondManager::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IDBCorrespondManager, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IDBCorrespondManager, Guid, dwQueryVer);
  return NULL;
}

// 启动服务
bool CDBCorrespondManager::StartService(std::shared_ptr<asio::io_context> io_context) {
  ASSERT(!m_bService);
  m_bService = true;
  return true;
}

// 停止服务
bool CDBCorrespondManager::ConcludeService() {
  std::lock_guard lock(m_AsyncCritical);

  m_bService = false;

  ClearAmortizeData();

  m_DBRequestUserArray.clear();

  return true;
}

// 配置模块
bool CDBCorrespondManager::InitDBCorrespondManager(IDataBaseEngine* pIDataBaseEngine) {
  m_pIKernelDataBaseEngine = pIDataBaseEngine;
  return true;
}

// 请求事件
bool CDBCorrespondManager::PostDataBaseRequest(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize, BYTE cbCache) {
  // 设置映射
  std::lock_guard lock(m_AsyncCritical);

  // 缓存处理
  AmortizeSyncData(dwUserID, wRequestID, dwContextID, pData, wDataSize, cbCache);

  // 执行缓冲
  PerformAmortisation();

  return true;
}

// 请求完成
bool CDBCorrespondManager::OnPostRequestComplete(DWORD dwUserID, bool bSucceed) {
  // 设置映射
  std::lock_guard lock(m_AsyncCritical);

  // 清除投递记录
  INT_PTR nIndex = GetUserArrayIndex(dwUserID);
  if (nIndex != -1) {
    m_DBRequestUserArray.erase(m_DBRequestUserArray.begin() + nIndex);
  }

  // 清除数据
  for (auto itr = m_DBRequestList.begin(); itr != m_DBRequestList.end(); ) {
    tagDBRequestHead* pDBRequestHead = *itr;
    if (pDBRequestHead->dwUserID == dwUserID && (pDBRequestHead->cbCache == FALSE || bSucceed)) {
      itr = m_DBRequestList.erase(itr); // 返回下一个有效迭代器
      delete[] ((BYTE*) pDBRequestHead);
    } else {
      ++itr;
    }
  }

  // 执行缓冲
  PerformAmortisation();

  return true;
}

// 定时事件
bool CDBCorrespondManager::OnTimerNotify() {
  // 设置映射
  std::lock_guard lock(m_AsyncCritical);

  // 执行缓冲
  PerformAmortisation();

  return true;
}

// 已经提交请求
bool CDBCorrespondManager::IsPostDBRequest(DWORD dwUserID) {
  for (INT_PTR i = 0; i < m_DBRequestUserArray.size(); i++) {
    if (dwUserID == m_DBRequestUserArray[i]) {
      return true;
    }
  }

  return false;
}

// 获得索引
INT_PTR CDBCorrespondManager::GetUserArrayIndex(DWORD dwUserID) {
  for (INT_PTR i = 0; i < m_DBRequestUserArray.size(); i++) {
    if (dwUserID == m_DBRequestUserArray[i]) {
      return i;
    }
  }

  return -1;
}

// 缓冲请求
bool CDBCorrespondManager::AmortizeSyncData(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize, BYTE cbCache) {
  // 定义变量
  WORD wBuffSize = sizeof(tagDBRequestHead) + wDataSize;
  BYTE* pBuff = new BYTE[wBuffSize];
  ZeroMemory(pBuff, wBuffSize);
  tagDBRequestHead* pDBRequestHead = (tagDBRequestHead*) pBuff;

  // 填充数据
  pDBRequestHead->cbCache = cbCache;
  pDBRequestHead->dwUserID = dwUserID;
  pDBRequestHead->dwContextID = dwContextID;
  pDBRequestHead->wRequestID = wRequestID;
  pDBRequestHead->wDataSize = wDataSize;
  CopyMemory(((VOID*) (pDBRequestHead + 1)), pData, wDataSize);

  // 加入链表
  m_DBRequestList.emplace_back(pDBRequestHead);

  return true;
}

// 执行缓冲
VOID CDBCorrespondManager::PerformAmortisation() {
  // 遍历数据
  for (auto& pDBRequestHead: m_DBRequestList) {
    // 投递请求
    if (!IsPostDBRequest(pDBRequestHead->dwUserID)) {
      if (m_pIKernelDataBaseEngine) {
        m_DBRequestUserArray.emplace_back(pDBRequestHead->dwUserID);
        m_pIKernelDataBaseEngine->PostDataBaseRequest(pDBRequestHead->wRequestID, pDBRequestHead->dwContextID, (VOID*) (pDBRequestHead + 1),
                                                      pDBRequestHead->wDataSize);
      }
    }
  }
}

// 清除缓存
VOID CDBCorrespondManager::ClearAmortizeData() {
  while (!m_DBRequestList.empty()) {
    tagDBRequestHead* pRqHead = m_DBRequestList.front();
    m_DBRequestList.pop_front();
    if (pRqHead) {
      delete[] ((BYTE*) pRqHead);
    }
  }
}
