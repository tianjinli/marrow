#pragma once

#include "GameServiceHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 类型定义
typedef CWHArray<tagUserTaskEntry*> CUserTaskEntryArray;
typedef CWHArray<tagTaskParameter*> CTaskParameterArray;

using CUserTaskEntryMap = std::unordered_map<DWORD, tagUserTaskEntry*>;

//////////////////////////////////////////////////////////////////////////////////

// 道具管理
class GAME_SERVICE_CLASS CUserTaskManager : public IUserTaskManagerSink {
  // 变量定义
protected:
  CTaskParameterMap task_parameter_map_; // 任务参数
  CUserTaskEntryMap user_task_entry_map_; // 任务入口
  CTaskParameterArray task_parameter_buffer_; // 参数缓冲
  CUserTaskEntryArray user_task_entry_buffer_; // 任务缓冲

  // 函数定义
public:
  // 构造函数
  CUserTaskManager();
  // 析构函数
  virtual ~CUserTaskManager();

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() {
    return;
  }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID guid, DWORD query_ver);

  // 任务参数
public:
  // 移除参数
  virtual VOID RemoveTaskParameter();
  // 查找参数
  virtual tagTaskParameter* SearchTaskParameter(WORD task_id);
  // 枚举参数
  virtual tagTaskParameter* EnumTaskParameter(CTaskParameterMap::iterator& position);
  // 添加参数
  virtual bool AddTaskParameter(tagTaskParameter task_parameters[], WORD task_count);
  // 获取参数数目
  virtual WORD GetTaskParameterCount();

  // 用户任务
public:
  // 移除任务
  virtual VOID RemoveUserTask(DWORD user_id);
  // 获取任务
  virtual tagUserTaskEntry* GetUserTaskEntry(DWORD user_id);
  // 获取任务
  virtual tagUserTaskEntry* GetUserTaskEntry(DWORD user_id, BYTE task_status);
  // 设置任务
  virtual VOID SetUserTaskInfo(DWORD user_id, tagUserTaskInfo user_task_infos[], WORD user_task_count);

  // 辅助函数
protected:
  // 创建任务
  tagUserTaskEntry* CreateUserTaskEntry();

  // 辅助函数
public:
  // 重置对象
  VOID ResetTaskManager();
};

//////////////////////////////////////////////////////////////////////////////////
