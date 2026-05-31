#include "DistributeManager.h"

//////////////////////////////////////////////////////////////////////////////////

// 内存管理
tagDistributeNode* CDistributeNodePool::m_pHeadOfFreeList = NULL;
const int CDistributeNodePool::BLOCK_SIZE = 20;

// 常量定义
#define DISTRIBUTE_WAIT_TIMESTAMP 10 // 等待时间戳

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CDistributeNodePool::CDistributeNodePool() {}

// 析构函数
CDistributeNodePool::~CDistributeNodePool() {
  if (m_pHeadOfFreeList != NULL) {
    tagDistributeNode* pDistributeNode = m_pHeadOfFreeList;
    while (pDistributeNode != NULL) {
      // 安全释放
      m_pHeadOfFreeList = pDistributeNode->pNextDistributeNode;
      SafeDelete(pDistributeNode);
      pDistributeNode = m_pHeadOfFreeList;
    }
  }
}

// 分配结点
tagDistributeNode* CDistributeNodePool::AllocNode() {
  // 获取头结点
  tagDistributeNode* pDistributeNode = m_pHeadOfFreeList;
  if (pDistributeNode != NULL) {
    m_pHeadOfFreeList = pDistributeNode->pNextDistributeNode;
  } else {
    // 分配大块内存
    for (int nIndex = 0; nIndex < BLOCK_SIZE; nIndex++) {
      tagDistributeNode* pNewBlock = new tagDistributeNode;
      pNewBlock->pNextDistributeNode = m_pHeadOfFreeList;
      m_pHeadOfFreeList = pNewBlock;
    }

    // 设置结点
    pDistributeNode = m_pHeadOfFreeList;
    m_pHeadOfFreeList = pDistributeNode->pNextDistributeNode;
  }

  return pDistributeNode;
}

// 释放结点
VOID CDistributeNodePool::FreeNode(void* pNode) {
  // 归还结点
  tagDistributeNode* pDeadNode = static_cast<tagDistributeNode*>(pNode);
  pDeadNode->pNextDistributeNode = m_pHeadOfFreeList;
  m_pHeadOfFreeList = pDeadNode;

  return;
}

//////////////////////////////////////////////////////////////////////////////////

// 构造函数
CDistributeManager::CDistributeManager() {
  // 设置变量
  m_pHeadNode = NULL;
  m_wNodeCount = 0;
  m_wAndroidCount = 0;
  m_wRealCount = 0;
  m_cbDistributeRule = 0;

  // 设置字典
  m_SameTableMap.reserve(10003);
}

// 析构函数
CDistributeManager::~CDistributeManager() {
  // 移除节点
  RemoveAll();

  // 释放对象
  m_SameTableBuffer.insert(m_SameTableBuffer.end(), m_SameTableActive.begin(), m_SameTableActive.end());
  for (INT_PTR nIndex = 0; nIndex < m_SameTableBuffer.size(); nIndex++) {
    SafeDelete(m_SameTableBuffer[nIndex]);
  }

  // 移除元素
  m_SameTableBuffer.clear();
  m_SameTableActive.clear();
}


// 插入结点
BOOL CDistributeManager::InsertDistributeNode(const tagDistributeInfo& DistributeInfo) {
  // 查找用户
  if (SearchNode(DistributeInfo.pIServerUserItem) != NULL) {
    return true;
  }

  // 头部判断
  if (m_pHeadNode == NULL) {
    // 分配结点
    m_pHeadNode = m_DistributeNodePool.AllocNode();
    if (m_pHeadNode == NULL) {
      return false;
    }

    // 设置变量
    m_pHeadNode->pNextDistributeNode = NULL;
    m_pHeadNode->pPrepDistributeNode = NULL;
    CopyMemory(&m_pHeadNode->DistributeInfo, &DistributeInfo, sizeof(DistributeInfo));
    m_pHeadNode->DistributeInfo.pPertainNode = m_pHeadNode;
  } else {
    // 分配结点
    tagDistributeNode* pDistributeNode = m_DistributeNodePool.AllocNode();
    if (pDistributeNode == NULL) {
      return false;
    }

    // 设置结点
    pDistributeNode->pNextDistributeNode = NULL;
    pDistributeNode->pPrepDistributeNode = NULL;
    CopyMemory(&pDistributeNode->DistributeInfo, &DistributeInfo, sizeof(DistributeInfo));
    pDistributeNode->DistributeInfo.pPertainNode = pDistributeNode;

    // 表头结点
    if (m_pHeadNode->pNextDistributeNode != NULL) {
      m_pHeadNode->pNextDistributeNode->pPrepDistributeNode = pDistributeNode;
      pDistributeNode->pNextDistributeNode = m_pHeadNode->pNextDistributeNode;
    }

    pDistributeNode->pPrepDistributeNode = m_pHeadNode;
    m_pHeadNode->pNextDistributeNode = pDistributeNode;
  }

  // 更新数目
  if (DistributeInfo.pIServerUserItem->IsAndroidUser()) {
    ++m_wAndroidCount;
  } else {
    ++m_wRealCount;
  }

  ++m_wNodeCount;

  return true;
}

// 移除结点
VOID CDistributeManager::RemoveDistributeNode(IServerUserItem* pIServerUserItem) {
  // 查找结点
  tagDistributeNode* pDistributeNode = SearchNode(pIServerUserItem);
  if (pDistributeNode != NULL) {
    RemoveDistributeNode(pDistributeNode);
  }
}

// 移除结点
VOID CDistributeManager::RemoveDistributeNode(tagDistributeNode* pDistributeNode) {
  // 参数校验
  if (pDistributeNode == NULL) {
    return;
  }

  // 查找用户
  if (SearchNode(pDistributeNode->DistributeInfo.pIServerUserItem) == NULL) {
    return;
  }

  // 变量定义
  tagDistributeNode* pPrepNode = pDistributeNode->pPrepDistributeNode;
  tagDistributeNode* pNextNode = pDistributeNode->pNextDistributeNode;

  // 调整结点
  if (pPrepNode != NULL) {
    // 中间结点
    if (pNextNode != NULL) {
      pPrepNode->pNextDistributeNode = pNextNode;
      pNextNode->pPrepDistributeNode = pPrepNode;
    } else {
      // 尾部结点
      if (pPrepNode->pNextDistributeNode == pDistributeNode) {
        pPrepNode->pNextDistributeNode = NULL;
      }
    }
  } else {
    // 头部结点
    if (pNextNode != NULL) {
      pNextNode->pPrepDistributeNode = NULL;
      m_pHeadNode = pNextNode;
    } else {
      m_pHeadNode = NULL;
    }
  }

  // 更新数目
  if (pDistributeNode->DistributeInfo.pIServerUserItem->IsAndroidUser()) {
    --m_wAndroidCount;
  } else {
    --m_wRealCount;
  }
  --m_wNodeCount;

  // 安全释放
  m_DistributeNodePool.FreeNode(pDistributeNode);
}

// 移除结点
VOID CDistributeManager::RemoveAll() {
  // 释放内存
  while (m_pHeadNode != NULL) {
    RemoveDistributeNode(m_pHeadNode);
  }

  // 重置变量
  m_pHeadNode = NULL;
  m_wNodeCount = 0;
  m_wAndroidCount = 0;
  m_wRealCount = 0;

  return;
}

// 执行分组
WORD CDistributeManager::PerformDistribute(CDistributeInfoArray& DistributeInfoArray, WORD wNeedCount) {
  // 定义变量
  tagDistributeNode* pMoveNode = NULL;
  tagDistributeNode* pMoveStartNode = NULL;
  if (m_pHeadNode != NULL && m_wNodeCount > 1) {
    pMoveNode = m_pHeadNode;
    WORD wRandNodeIndex = rand() % m_wNodeCount;
    while (wRandNodeIndex-- > 0) {
      if (pMoveNode == NULL) {
        pMoveNode = m_pHeadNode;
        break;
      }
      pMoveNode = pMoveNode->pNextDistributeNode;
    }

    // 设置变量
    if (pMoveNode != NULL) {
      pMoveStartNode = pMoveNode;
    }
  }

  //
  if (pMoveNode == NULL) {
    return 0;
  }

  // 获取时间戳
  DWORD dwCurrentStamp = (DWORD) time(NULL);

  // 获取用户
  do {
    // 定义变量
    BOOL bFirstSuccess = TRUE;

    // 等待时间
    if (dwCurrentStamp - pMoveNode->DistributeInfo.dwInsertStamp < DISTRIBUTE_WAIT_TIMESTAMP) {
      bFirstSuccess = FALSE;
    }

    // 等级过滤
    if (DistributeInfoArray.size() > 0 && DistributeInfoArray[0].wDistribute != pMoveNode->DistributeInfo.wDistribute) {
      bFirstSuccess = FALSE;
    }

    // 机器过滤
    if (bFirstSuccess == TRUE && DistributeInfoArray.size() == wNeedCount - 1 &&
        FilterRuleIsAllAndroid(DistributeInfoArray, pMoveNode->DistributeInfo.pIServerUserItem)) {
      bFirstSuccess = FALSE;
    }

    // 同IP过滤
    if (bFirstSuccess == TRUE && (m_cbDistributeRule & DISTRIBUTE_SAME_ADDRESS) == 0 &&
        FilterRuleExitsIPAddr(DistributeInfoArray, pMoveNode->DistributeInfo.dwClientAddr) == TRUE) {
      bFirstSuccess = FALSE;
    }

    // 同桌过滤
    if (bFirstSuccess == TRUE && (m_cbDistributeRule & DISTRIBUTE_LAST_TABLE) == 0 &&
        FilterRuleIsLastSameTable(DistributeInfoArray, pMoveNode->DistributeInfo.pIServerUserItem->GetUserID()) == TRUE) {
      bFirstSuccess = FALSE;
    }

    // 获取成功
    if (bFirstSuccess == TRUE) {
      DistributeInfoArray.emplace_back(pMoveNode->DistributeInfo);
    }

    // 向前推进
    pMoveNode = pMoveNode->pNextDistributeNode;
    if (pMoveNode == NULL) {
      pMoveNode = m_pHeadNode;
    }

    // 成功判断
    if (DistributeInfoArray.size() == wNeedCount) {
      break;
    }

  } while (pMoveNode && pMoveNode != pMoveStartNode);

  return (WORD) DistributeInfoArray.size();
}

// 查找结点
tagDistributeNode* CDistributeManager::SearchNode(IServerUserItem* pIServerUserItem) {
  if (m_pHeadNode == NULL) {
    return NULL;
  }

  // 设置变量
  tagDistributeNode* pMoveNode = m_pHeadNode;

  // 查找结点
  while (pMoveNode != NULL) {
    // 接口判断
    if (pMoveNode->DistributeInfo.pIServerUserItem == pIServerUserItem) {
      return pMoveNode;
    }

    // 向前推进
    pMoveNode = pMoveNode->pNextDistributeNode;
  }

  return NULL;
}

// 获取信息
tagSameTableInfo* CDistributeManager::GetUserSameTableInfo(DWORD dwUserID) {
  // 变量定义
  tagSameTableInfo* pSameTableInfo;
  if (auto it = m_SameTableMap.find(dwUserID); it == m_SameTableMap.end()) {
    pSameTableInfo = ActiveSameTableInfo();
    m_SameTableMap[dwUserID] = pSameTableInfo;
  }

  return pSameTableInfo;
}

// 移除信息
VOID CDistributeManager::RemoveUserSameTableInfo(DWORD dwUserID) {
  // 变量定义
  if (auto it = m_SameTableMap.find(dwUserID); it != m_SameTableMap.end()) {
    // 移除信息
    m_SameTableMap.erase(it);
    RemoveSameTableInfo(it->second);
  }
}

// 激活对象
tagSameTableInfo* CDistributeManager::ActiveSameTableInfo() {
  // 查找缓冲
  if (m_SameTableBuffer.size() > 0) {
    tagSameTableInfo* pSameTableInfo = m_SameTableBuffer.front();
    m_SameTableBuffer.erase(m_SameTableBuffer.begin());
    m_SameTableActive.emplace_back(pSameTableInfo);
    return pSameTableInfo;
  }

  // 创建对象
  // 创建对象
  tagSameTableInfo* pSameTableInfo = new (std::nothrow) tagSameTableInfo();
  if (pSameTableInfo == NULL) {
    CLogger::Error(TEXT("{} → 内存不足,对象创建失败!"), TEXT(__FUNCTION__));
    return NULL;
  }

  // 设置对象
  ZeroMemory(pSameTableInfo, sizeof(tagSameTableInfo));
  m_SameTableActive.emplace_back(pSameTableInfo);

  return pSameTableInfo;
}

// 移除对象
VOID CDistributeManager::RemoveSameTableInfo(tagSameTableInfo* pSameTableInfo) {
  ZeroMemory(pSameTableInfo, sizeof(tagSameTableInfo));
  m_SameTableBuffer.emplace_back(pSameTableInfo);

  // 查找对象
  for (INT_PTR nIndex = 0; nIndex < m_SameTableActive.size(); nIndex++) {
    if (m_SameTableActive[nIndex] == pSameTableInfo) {
      m_SameTableActive.erase(m_SameTableActive.begin() + nIndex);
      break;
    }
  }
}

// IP同址
BOOL CDistributeManager::FilterRuleExitsIPAddr(const CDistributeInfoArray& DistributeInfoArray, DWORD dwClientAddr) {
  // 查找同IP
  for (INT_PTR nIndex = 0; nIndex < DistributeInfoArray.size(); nIndex++) {
    if (DistributeInfoArray[nIndex].dwClientAddr == dwClientAddr) {
      return TRUE;
    }
  }

  return FALSE;
}

// 机器过滤
BOOL CDistributeManager::FilterRuleIsAllAndroid(const CDistributeInfoArray& DistributeInfoArray, IServerUserItem* const pIServerUserItem) {
  // 参数校验
  if (pIServerUserItem == NULL || DistributeInfoArray.size() == 0) {
    return FALSE;
  }

  // 变量定义
  WORD wAndroidCount = 0;

  // 统计机器
  for (INT_PTR nIndex = 0; nIndex < DistributeInfoArray.size(); nIndex++) {
    if (DistributeInfoArray[nIndex].pIServerUserItem->IsAndroidUser() == true) {
      ++wAndroidCount;
    }
  }

  return (wAndroidCount == DistributeInfoArray.size()) && pIServerUserItem->IsAndroidUser();
}

// 上局同桌
BOOL CDistributeManager::FilterRuleIsLastSameTable(const CDistributeInfoArray& DistributeInfoArray, DWORD dwUserID) {
  // 参数校验
  if (DistributeInfoArray.size() == 0) {
    return FALSE;
  }

  // 变量定义
  tagSameTableInfo* pSameTableInfo = NULL;

  for (INT_PTR nIndex = 0; nIndex < DistributeInfoArray.size(); nIndex++) {
    pSameTableInfo = GetUserSameTableInfo(DistributeInfoArray[nIndex].pIServerUserItem->GetUserID());
    if (pSameTableInfo != NULL) {
      for (WORD i = 0; i < pSameTableInfo->wPlayerCount; i++) {
        if (pSameTableInfo->wPlayerIDSet[i] == dwUserID) {
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

//////////////////////////////////////////////////////////////////////////////////
