#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////
// 结构定义

// 数据列头
struct tagDataHead {
  WORD wDataSize;   // 数据大小
  WORD wIdentifier; // 类型标识
};

// 负荷信息
struct tagBurdenInfo {
  DWORD dwDataSize;        // 数据大小
  DWORD dwBufferSize;      // 缓冲长度
  DWORD dwDataPacketCount; // 数据包数
};

// 数据信息
struct tagDataBuffer {
  WORD wDataSize;    // 数据大小
  void* pDataBuffer; // 数据指针
};

//////////////////////////////////////////////////////////////////////////////////

// 数据队列
class [[deprecated]] SERVICE_CORE_CLASS CWHDataQueue {
  // 查询变量
protected:
  DWORD m_dwInsertPos = 0;    // 插入位置
  DWORD m_dwTerminalPos = 0;  // 结束位置
  DWORD m_dwDataQueryPos = 0; // 查询位置

  // 数据变量
protected:
  DWORD m_dwDataSize = 0;        // 数据大小
  DWORD m_dwDataPacketCount = 0; // 数据包数

  // 缓冲变量
protected:
  DWORD m_dwBufferSize = 0;              // 缓冲长度
  uint8_t* m_pDataQueueBuffer = nullptr; // 缓冲指针

  // 函数定义
public:
  // 构造函数
  CWHDataQueue() = default;
  // 析构函数
  virtual ~CWHDataQueue();

  // 功能函数
public:
  // 负荷信息
  VOID GetBurdenInfo(tagBurdenInfo& BurdenInfo) const;
  // 数据信息
  DWORD GetDataPacketCount() { return m_dwDataPacketCount; }

  // 插入数据
public:
  // 插入数据
  bool InsertData(WORD wIdentifier, VOID* pBuffer, WORD wDataSize);
  // 插入数据
  bool InsertData(WORD wIdentifier, tagDataBuffer DataBuffer[], WORD wDataCount);

  // 数据管理
public:
  // 删除数据
  VOID RemoveData(bool bFreeMemory);
  // 提取数据
  bool DistillData(tagDataHead& DataHead, VOID* pBuffer, WORD wBufferSize);

  // 内部函数
private:
  // 调整存储
  bool RectifyBuffer(DWORD dwNeedSize);
};

//////////////////////////////////////////////////////////////////////////////////
