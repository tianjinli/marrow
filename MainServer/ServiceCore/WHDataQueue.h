#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////
// 结构定义

// 数据列头
struct tagDataHead {
  WORD wDataSize; // 数据大小
  WORD wIdentifier; // 类型标识
};

// 负荷信息
struct tagBurthenInfo {
  DWORD dwDataSize; // 数据大小
  DWORD dwBufferSize; // 缓冲长度
  DWORD dwDataPacketCount; // 数据包数
};

// 数据信息
struct tagDataBuffer {
  WORD wDataSize; // 数据大小
  void* pDataBuffer; // 数据指针
};

//////////////////////////////////////////////////////////////////////////////////

// 数据队列
class [[deprecated]] SERVICE_CORE_CLASS CWHDataQueue {
public:
  // 构造函数
  CWHDataQueue() = default;
  // 析构函数
  virtual ~CWHDataQueue();

  // 功能函数
  // 负荷信息
  VOID GetBurdenInfo(tagBurthenInfo& burden_info) const;
  // 数据信息
  DWORD GetDataPacketCount() { return data_packet_count_; }

  // 插入数据
  // 插入数据
  bool InsertData(WORD identifier, VOID* buffer, WORD data_size);
  // 插入数据
  bool InsertData(WORD identifier, tagDataBuffer data_buffer[], WORD data_count);

  // 数据管理
  // 删除数据
  VOID RemoveData(bool free_memory);
  // 提取数据
  bool DistillData(tagDataHead& data_head, VOID* buffer, WORD buffer_size);

private:
  // 调整存储
  bool RectifyBuffer(DWORD need_size);

  // 查询变量
  DWORD insert_pos_ = 0; // 插入位置
  DWORD terminal_pos_ = 0; // 结束位置
  DWORD data_query_pos_ = 0; // 查询位置

  // 数据变量
  DWORD data_size_ = 0; // 数据大小
  DWORD data_packet_count_ = 0; // 数据包数

  // 缓冲变量
  DWORD buffer_size_ = 0; // 缓冲长度
  uint8_t* data_queue_buffer_ = nullptr; // 缓冲指针
};

//////////////////////////////////////////////////////////////////////////////////
