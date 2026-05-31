#include "UserTaskManager.h"

//////////////////////////////////////////////////////////////////////////////////
// 构造函数
CUserTaskManager::CUserTaskManager() {
  // 初始化表
  m_TaskParameterMap.reserve(TASK_MAX_COUNT + 1);
}

// 析构函数
CUserTaskManager::~CUserTaskManager() {
  // 删除缓冲
  for (INT_PTR nIndex = 0; nIndex < m_UserTaskEntryBuffer.size(); nIndex++) {
    SafeDelete(m_UserTaskEntryBuffer[nIndex]);
  }

  // 删除缓冲
  for (INT_PTR nIndex = 0; nIndex < m_TaskParameterBuffer.size(); nIndex++) {
    SafeDelete(m_TaskParameterBuffer[nIndex]);
  }

  // 删除参数
  for (auto& pTaskParameter: m_TaskParameterMap | std::views::values) {
    SafeDelete(pTaskParameter);
  }

  // 删除任务
  for (auto& pUserTaskEntry: m_UserTaskEntryMap | std::views::values) {
    SafeDelete(pUserTaskEntry);
  }

  // 移除元素
  m_TaskParameterMap.clear();
  m_UserTaskEntryMap.clear();
  m_TaskParameterBuffer.clear();
  m_UserTaskEntryBuffer.clear();
}

// 接口查询
VOID* CUserTaskManager::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IUserTaskManagerSink, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IUserTaskManagerSink, Guid, dwQueryVer);
  return NULL;
}

// 添加参数
bool CUserTaskManager::AddTaskParameter(tagTaskParameter TaskParameter[], WORD wPatemterCount) {
  // 参数校验
  if (wPatemterCount == 0 || TaskParameter == NULL) {
    return false;
  }

  // 变量定义
  for (WORD i = 0; i < wPatemterCount; i++) {
    // 变量定义
    tagTaskParameter* pTaskParameter = NULL;

    // 查找现存
    if (m_TaskParameterBuffer.size() > 0) {
      // 查找任务
      for (INT_PTR nIndex = 0; nIndex < m_TaskParameterBuffer.size(); nIndex++) {
        if (m_TaskParameterBuffer[nIndex]->wTaskID == TaskParameter[i].wTaskID) {
          pTaskParameter = m_TaskParameterBuffer[nIndex];
          m_TaskParameterBuffer.erase(m_TaskParameterBuffer.begin() + nIndex);
          break;
        }
      }
    }

    // 创建对象
    if (pTaskParameter == NULL) {
      pTaskParameter = new (std::nothrow) tagTaskParameter();
      if (pTaskParameter == NULL) {
        CLogger::Error(TEXT("内存不足！"));
        ASSERT(FALSE);
        break;
      }
    }

    // 拷贝数据
    CopyMemory(pTaskParameter, &TaskParameter[i], sizeof(tagTaskParameter));

    // 保存任务
    m_TaskParameterMap[pTaskParameter->wTaskID] = pTaskParameter;
  }

  return true;
}

// 获取参数数目
WORD CUserTaskManager::GetTaskParameterCount() {
  return (WORD) m_TaskParameterMap.size();
}

// 移除参数
VOID CUserTaskManager::RemoveTaskParameter() {
  // 移除参数
  for (auto& pTaskParameter: m_TaskParameterMap | std::views::values) {
    m_TaskParameterBuffer.emplace_back(pTaskParameter);
  }

  // 移除严肃
  m_TaskParameterMap.clear();
}

// 查找参数
tagTaskParameter* CUserTaskManager::SearchTaskParameter(WORD wTaskID) {
  // 变量定义
  auto it = m_TaskParameterMap.find(wTaskID);
  return it != m_TaskParameterMap.end() ? it->second : NULL;
}

// 枚举参数
tagTaskParameter* CUserTaskManager::EnumTaskParameter(POSITION& Position) {
  // 变量定义
  tagTaskParameter* pTaskParameter = NULL;
  auto itr = Position.empty() ? m_TaskParameterMap.begin() : Position.get<CTaskParameterMap::iterator>();

  // 获取对象
  if (itr != m_TaskParameterMap.end()) {
    pTaskParameter = itr->second;
    Position = ++itr; // 没到头，更新指针内的迭代器位数据
  } else {
    Position = nullptr;
    return nullptr;
  }

  return pTaskParameter;
}

// 设置任务
VOID CUserTaskManager::SetUserTaskInfo(DWORD dwUserID, tagUserTaskInfo UserTaskInfo[], WORD wTaskCount) {
  // 参数校验
  ASSERT(wTaskCount > 0 && UserTaskInfo != NULL);
  if (wTaskCount == 0 || UserTaskInfo == NULL) {
    return;
  }

  // 变量定义
  tagUserTaskEntry* pUserTaskEntry = NULL;
  tagUserTaskEntry* pUserTaskEntryHead = NULL;

  // 查找对象
  if (auto itr = m_UserTaskEntryMap.find(dwUserID); itr != m_UserTaskEntryMap.end()) {
    pUserTaskEntryHead = itr->second;
  }

  // 查找链表尾部
  while (pUserTaskEntryHead != NULL && pUserTaskEntryHead->pNextTaskEntry) {
    pUserTaskEntryHead = pUserTaskEntryHead->pNextTaskEntry;
  }

  // 变量定义
  for (WORD i = 0; i < wTaskCount; i++) {
    // 创建对象
    pUserTaskEntry = CreateUserTaskEntry();
    if (pUserTaskEntry == NULL) {
      break;
    }

    // 设置对象
    pUserTaskEntry->cbTaskStatus = UserTaskInfo[i].cbTaskStatus;
    pUserTaskEntry->wTaskProgress = UserTaskInfo[i].wTaskProgress;
    pUserTaskEntry->dwResidueTime = UserTaskInfo[i].dwResidueTime;
    pUserTaskEntry->dwLastUpdateTime = UserTaskInfo[i].dwLastUpdateTime;

    // 查找参数
    if (auto itr = m_TaskParameterMap.find(UserTaskInfo[i].wTaskID); itr != m_TaskParameterMap.end()) {
      pUserTaskEntry->pTaskParameter = itr->second;
    }

    // 链接任务
    if (pUserTaskEntryHead != NULL) {
      pUserTaskEntryHead->pNextTaskEntry = pUserTaskEntry;
      pUserTaskEntryHead = pUserTaskEntryHead->pNextTaskEntry;
    }

    // 设置链表头
    if (i == 0 && pUserTaskEntryHead == NULL) {
      pUserTaskEntryHead = pUserTaskEntry;
      m_UserTaskEntryMap[dwUserID] = pUserTaskEntryHead;
    }
  }
}

// 移除任务
VOID CUserTaskManager::RemoveUserTask(DWORD dwUserID) {
  // 查找对象
  if (auto itr = m_UserTaskEntryMap.find(dwUserID); itr != m_UserTaskEntryMap.end()) {
    // 添加到缓冲
    tagUserTaskEntry* pUserTaskEntry = itr->second;
    while (pUserTaskEntry != NULL) {
      m_UserTaskEntryBuffer.emplace_back(pUserTaskEntry);
      pUserTaskEntry = pUserTaskEntry->pNextTaskEntry;
    }

    // 移除对象
    m_UserTaskEntryMap.erase(itr);
  }
}

// 获取任务
tagUserTaskEntry* CUserTaskManager::GetUserTaskEntry(DWORD dwUserID) {
  // 变量定义
  auto itr = m_UserTaskEntryMap.find(dwUserID);
  return itr != m_UserTaskEntryMap.end() ? itr->second : NULL;
}

// 获取任务
tagUserTaskEntry* CUserTaskManager::GetUserTaskEntry(DWORD dwUserID, BYTE cbTaskStatus) {
  // 变量定义
  tagUserTaskEntry* pUserTaskEntry = NULL;
  if (auto itr = m_UserTaskEntryMap.find(dwUserID); itr != m_UserTaskEntryMap.end()) {
    pUserTaskEntry = itr->second;
  }
  if (pUserTaskEntry == NULL) {
    return NULL;
  }

  // 变量定义
  tagUserTaskEntry* pUserTaskEntryHead = NULL;
  tagUserTaskEntry* pUserTaskEntryLink = NULL;

  // 遍历链表
  while (pUserTaskEntry != NULL) {
    if (pUserTaskEntry->cbTaskStatus == cbTaskStatus) {
      if (pUserTaskEntryHead == NULL) {
        pUserTaskEntryHead = pUserTaskEntry;
        pUserTaskEntryLink = pUserTaskEntryHead;
      } else {
        pUserTaskEntryLink->pNextStatusEntry = pUserTaskEntry;
        pUserTaskEntryLink = pUserTaskEntryLink->pNextStatusEntry;
      }

      pUserTaskEntryLink->pNextStatusEntry = NULL;
    }

    pUserTaskEntry = pUserTaskEntry->pNextTaskEntry;
  }

  return pUserTaskEntryHead;
}

// 创建任务
tagUserTaskEntry* CUserTaskManager::CreateUserTaskEntry() {
  // 变量定义
  tagUserTaskEntry* pUserTaskEntry = NULL;

  // 查找缓冲
  if (!m_UserTaskEntryBuffer.empty()) {
    pUserTaskEntry = m_UserTaskEntryBuffer.front();
    m_UserTaskEntryBuffer.erase(m_UserTaskEntryBuffer.begin());
  }

  // 创建任务
  if (pUserTaskEntry == NULL) {
    pUserTaskEntry = new (std::nothrow) tagUserTaskEntry;
    if (pUserTaskEntry == NULL) {
      CLogger::Error("内存不足，资源申请失败！");
    }
  }

  // 初始内存
  ZeroMemory(pUserTaskEntry, sizeof(tagUserTaskEntry));

  return pUserTaskEntry;
}

// 重置对象
VOID CUserTaskManager::ResetTaskManager() {
  // 移除参数
  RemoveTaskParameter();

  // 移除任务
  // 预分配内存，防止 vector 扩容带来的堆内存二次分配
  m_UserTaskEntryBuffer.reserve(m_UserTaskEntryBuffer.size() + m_UserTaskEntryMap.size());

  // 管道视图：直接解构出字典里所有的 Value (指针)
  for (auto* pUserTaskEntry: m_UserTaskEntryMap | std::views::values) {
    if (pUserTaskEntry != nullptr) {
      m_UserTaskEntryBuffer.emplace_back(pUserTaskEntry);
    }
  }

  // 移除元素
  m_TaskParameterMap.clear();
  m_UserTaskEntryMap.clear();
}

//////////////////////////////////////////////////////////////////////////////////
