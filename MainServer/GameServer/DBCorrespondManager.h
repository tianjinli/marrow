#pragma once

#include "GameServerHead.h"

//////////////////////////////////////////////////////////////////////////
// 请求结构
struct tagDBRequestHead {
  BYTE cbCache;
  DWORD dwUserID;
  DWORD dwContextID;
  WORD wRequestID;
  WORD wDataSize;
};

using CDBRequestList = std::list<tagDBRequestHead*>;
using CDBRequestUserArray = std::vector<DWORD>;

//////////////////////////////////////////////////////////////////////////
class CDBCorrespondManager : public IDBCorrespondManager {
  // 变量定义
protected:
  bool m_bService; // 服务状态
  std::mutex m_AsyncCritical; // 锁定对象

  // 数据引擎
public:
  IDataBaseEngine* m_pIKernelDataBaseEngine; // 数据引擎

  // 数据记录
public:
  CDBRequestList m_DBRequestList; // 请求链表
  CDBRequestUserArray m_DBRequestUserArray; // 请求用户

public:
  CDBCorrespondManager(void);
  ~CDBCorrespondManager(void);

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() { delete this; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 基础接口
public:
  // 启动服务
  virtual bool StartService(std::shared_ptr<asio::io_context> io_context);
  // 停止服务
  virtual bool ConcludeService();

  // 配置接口
public:
  // 配置模块
  virtual bool InitDBCorrespondManager(IDataBaseEngine* pIDataBaseEngine);

  // 控制事件
public:
  // 请求事件
  virtual bool PostDataBaseRequest(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize, BYTE cbCache = FALSE);

  // 同步事件
public:
  // 请求完成
  virtual bool OnPostRequestComplete(DWORD dwUserID, bool bSucceed);

  // 定时事件
public:
  // 定时事件
  virtual bool OnTimerNotify();

  // 功能函数
public:
  // 已经提交请求
  bool IsPostDBRequest(DWORD dwUserID);
  // 获得索引
  INT_PTR GetUserArrayIndex(DWORD dwUserID);
  // 缓冲请求
  bool AmortizeSyncData(DWORD dwUserID, WORD wRequestID, DWORD dwContextID, VOID* pData, WORD wDataSize, BYTE cbCache = FALSE);
  // 执行缓冲
  VOID PerformAmortisation();
  // 清除缓存
  VOID ClearAmortizeData();
};

//////////////////////////////////////////////////////////////////////////
