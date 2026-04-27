#include "UserTaskManager.h"

//////////////////////////////////////////////////////////////////////////////////
// 构造函数
CUserTaskManager::CUserTaskManager() {
  // std::unordered_map 无需 InitHashTable，自动管理
  // 如果你想预分配桶，可以加： task_parameter_map_.reserve(TASK_MAX_COUNT + 1);
}

// 析构函数
CUserTaskManager::~CUserTaskManager() {
  // 删除缓冲
  for (INT_PTR index = 0; index < user_task_entry_buffer_.GetCount(); index++) {
    SafeDelete(user_task_entry_buffer_[index]);
  }

  // 删除缓冲
  for (INT_PTR index = 0; index < task_parameter_buffer_.GetCount(); index++) {
    SafeDelete(task_parameter_buffer_[index]);
  }

  // 删除 task_parameter_map_ 中的对象
  for (auto& pair: task_parameter_map_) {
    SafeDelete(pair.second);
  }

  // 删除 user_task_entry_map_ 中的链表对象
  for (auto& pair: user_task_entry_map_) {
    tagUserTaskEntry* task_entry = pair.second;
    while (task_entry != nullptr) {
      tagUserTaskEntry* pNext = task_entry->pNextTaskEntry;
      SafeDelete(task_entry);
      task_entry = pNext;
    }
  }

  // 清空 map（std::unordered_map 的 RemoveAll 等价于 clear()）
  task_parameter_map_.clear();
  user_task_entry_map_.clear();
  task_parameter_buffer_.RemoveAll();
  user_task_entry_buffer_.RemoveAll();
}

// 接口查询
VOID* CUserTaskManager::QueryInterface(REFGUID guid, DWORD query_ver) {
  QUERYINTERFACE(IUserTaskManagerSink, guid, query_ver);
  QUERYINTERFACE_IUNKNOWNEX(IUserTaskManagerSink, guid, query_ver);
  return nullptr;
}

// 添加参数
bool CUserTaskManager::AddTaskParameter(tagTaskParameter task_parameters[], WORD task_count) {
  // 参数校验
  if (task_count == 0 || task_parameters == nullptr)
    return false;

  // 变量定义
  for (WORD i = 0; i < task_count; i++) {
    // 变量定义
    tagTaskParameter* task_parameter = nullptr;

    // 查找现存
    if (task_parameter_buffer_.GetCount() > 0) {
      // 查找任务
      for (INT_PTR index = 0; index < task_parameter_buffer_.GetCount(); index++) {
        if (task_parameter_buffer_[index]->wTaskID == task_parameters[i].wTaskID) {
          task_parameter = task_parameter_buffer_[index];
          task_parameter_buffer_.RemoveAt(index);
          break;
        }
      }
    }

    // 创建对象
    if (task_parameter == nullptr) {
      try {
        task_parameter = new tagTaskParameter;
        if (task_parameter == nullptr)
          throw(TEXT("内存不足！"));
      } catch (...) {
        ASSERT(FALSE);
        break;
      }
    }

    // 拷贝数据
    CopyMemory(task_parameter, &task_parameters[i], sizeof(tagTaskParameter));

    // 保存任务
    task_parameter_map_[task_parameter->wTaskID] = task_parameter;
  }

  return true;
}

// 获取参数数目
WORD CUserTaskManager::GetTaskParameterCount() {
  return (WORD) task_parameter_map_.size(); // CMap::GetCount() → size()
}

// 移除所有参数（把指针放回缓冲）
VOID CUserTaskManager::RemoveTaskParameter() {
  for (auto& pair: task_parameter_map_) {
    task_parameter_buffer_.Add(pair.second);
  }
  task_parameter_map_.clear();
}

// 查找参数
tagTaskParameter* CUserTaskManager::SearchTaskParameter(WORD task_id) {
  auto it = task_parameter_map_.find(task_id);
  return (it != task_parameter_map_.end()) ? it->second : nullptr;
}

// 枚举参数
tagTaskParameter* CUserTaskManager::EnumTaskParameter(CTaskParameterMap::iterator& position) {
  if (position == task_parameter_map_.end()) {
    position = task_parameter_map_.begin();
  } else {
    ++position;
  }

  if (position == task_parameter_map_.end())
    return nullptr;

  return position->second;
}

// 设置任务
VOID CUserTaskManager::SetUserTaskInfo(DWORD user_id, tagUserTaskInfo user_task_infos[], WORD user_task_count) {
  // 参数校验
  ASSERT(user_task_count > 0 && user_task_infos != nullptr);
  if (user_task_count == 0 || user_task_infos == nullptr)
    return;

  tagUserTaskEntry *user_task_entry_head = nullptr;
  auto it = user_task_entry_map_.find(user_id);
  if (it != user_task_entry_map_.end()) {
    user_task_entry_head = it->second;
  }

  // 找到链表尾部
  while (user_task_entry_head && user_task_entry_head->pNextTaskEntry) {
    user_task_entry_head = user_task_entry_head->pNextTaskEntry;
  }

  for (WORD i = 0; i < user_task_count; i++) {
    tagUserTaskEntry *user_task_entry = CreateUserTaskEntry();
    if (user_task_entry == nullptr) break;

    user_task_entry->cbTaskStatus = user_task_infos[i].cbTaskStatus;
    user_task_entry->wTaskProgress = user_task_infos[i].wTaskProgress;
    user_task_entry->dwResidueTime = user_task_infos[i].dwResidueTime;
    user_task_entry->dwLastUpdateTime = user_task_infos[i].dwLastUpdateTime;

    // 查找参数
    auto it = task_parameter_map_.find(user_task_infos[i].wTaskID);
    user_task_entry->pTaskParameter = (it != task_parameter_map_.end()) ? it->second : nullptr;

    // 链接任务
    if (user_task_entry_head) {
      user_task_entry_head->pNextTaskEntry = user_task_entry;
      user_task_entry_head = user_task_entry_head->pNextTaskEntry;
    }

    // 设置链表头
    if (i == 0 && user_task_entry_head == nullptr) {
      user_task_entry_head = user_task_entry;
      user_task_entry_map_[user_id] = user_task_entry_head;
    }
  }
}

// 移除任务
VOID CUserTaskManager::RemoveUserTask(DWORD user_id) {
  // 查找对象
  auto it = user_task_entry_map_.find(user_id);
  if (it != user_task_entry_map_.end()) {
    // 添加到缓冲
    tagUserTaskEntry *user_task_entry = it->second;
    while (user_task_entry) {
      user_task_entry_buffer_.Add(user_task_entry);
      user_task_entry = user_task_entry->pNextTaskEntry;
    }
    // 移除对象
    user_task_entry_map_.erase(it);
  }
}

// 获取任务
tagUserTaskEntry* CUserTaskManager::GetUserTaskEntry(DWORD user_id) {
  auto it = user_task_entry_map_.find(user_id);
  return it != user_task_entry_map_.end() ? it->second : nullptr;
}

// 获取任务
tagUserTaskEntry* CUserTaskManager::GetUserTaskEntry(DWORD user_id, BYTE task_status) {
  auto it = user_task_entry_map_.find(user_id);
  if (it == user_task_entry_map_.end()) return nullptr;

  tagUserTaskEntry* user_task_entry_head = nullptr;
  tagUserTaskEntry* user_task_entry_link = nullptr;

  tagUserTaskEntry* user_task_entry = it->second;
  // 遍历链表
  while (user_task_entry) {
    if (user_task_entry->cbTaskStatus == task_status) {
      if (user_task_entry_head == nullptr) {
        user_task_entry_head = user_task_entry;
        user_task_entry_link = user_task_entry;
      } else {
        user_task_entry_link->pNextStatusEntry = user_task_entry;
        user_task_entry_link = user_task_entry;
      }
      user_task_entry_link->pNextStatusEntry = nullptr;
    }
    user_task_entry = user_task_entry->pNextTaskEntry;
  }
  return user_task_entry_head;
}

// 创建任务
tagUserTaskEntry* CUserTaskManager::CreateUserTaskEntry() {
  // 变量定义
  tagUserTaskEntry* user_task_entry = nullptr;

  // 查找缓冲
  if (user_task_entry_buffer_.GetCount() > 0) {
    user_task_entry = user_task_entry_buffer_[0];
    user_task_entry_buffer_.RemoveAt(0);
  }

  // 创建任务
  if (user_task_entry == nullptr) {
    try {
      user_task_entry = new tagUserTaskEntry;
      if (user_task_entry == nullptr)
        throw TEXT("内存不足，资源申请失败！");
    } catch (...) {
      ASSERT(FALSE);
      SafeDelete(user_task_entry);
    }
  }

  // 初始内存
  ZeroMemory(user_task_entry, sizeof(tagUserTaskEntry));

  return user_task_entry;
}

// 重置对象
VOID CUserTaskManager::ResetTaskManager() {
  // 移除参数
  RemoveTaskParameter();

  for (auto& pair : user_task_entry_map_) {
    tagUserTaskEntry* user_task_entry = pair.second;
    while (user_task_entry) {
      user_task_entry_buffer_.Add(user_task_entry);
      user_task_entry = user_task_entry->pNextTaskEntry;
    }
  }

  // 移除元素
  task_parameter_map_.clear();
  user_task_entry_map_.clear();
}

//////////////////////////////////////////////////////////////////////////////////
