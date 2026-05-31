#include "WHDataQueue.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CWHDataQueue::~CWHDataQueue() {
  SafeDeleteArray(data_queue_buffer_);
}

// 负荷信息
VOID CWHDataQueue::GetBurdenInfo(tagBurthenInfo& burden_info) const {
  // 设置变量
  burden_info.dwDataSize = data_size_;
  burden_info.dwBufferSize = buffer_size_;
  burden_info.dwDataPacketCount = data_packet_count_;
}

// 插入数据
bool CWHDataQueue::InsertData(WORD identifier, VOID* buffer, WORD data_size) {
  // 变量定义
  tagDataHead data_head;
  ZeroMemory(&data_head, sizeof(data_head));

  // 设置变量
  data_head.wDataSize = data_size;
  data_head.wIdentifier = identifier;

  // 调整存储
  if (!RectifyBuffer(sizeof(data_head) + data_head.wDataSize)) {
    ASSERT(FALSE);
    return false;
  }

  // 插入数据
  try {
    // 拷贝数据
    CopyMemory(data_queue_buffer_ + insert_pos_, &data_head, sizeof(data_head));

    // 附加数据
    if (data_size > 0) {
      ASSERT(buffer != nullptr);
      CopyMemory(data_queue_buffer_ + insert_pos_ + sizeof(data_head), buffer, data_size);
    }

    // 调整数据
    data_packet_count_++;
    data_size_ += sizeof(data_head) + data_size;
    insert_pos_ += sizeof(data_head) + data_size;
    terminal_pos_ = std::max(terminal_pos_, insert_pos_);

    return true;
  } catch (...) {
    ASSERT(FALSE);
    return false;
  }
}

// 插入数据
bool CWHDataQueue::InsertData(WORD identifier, tagDataBuffer data_buffer[], WORD data_count) {
  // 变量定义
  tagDataHead data_head;
  ZeroMemory(&data_head, sizeof(data_head));

  // 设置变量
  data_head.wDataSize = 0;
  data_head.wIdentifier = identifier;

  // 累计大小
  for (WORD i = 0; i < data_count; i++) {
    if (data_buffer[i].wDataSize > 0) {
      data_head.wDataSize += data_buffer[i].wDataSize;
    }
  }

  // 调整存储
  if (!RectifyBuffer(sizeof(data_head) + data_head.wDataSize)) {
    return false;
  }

  try {
    // 拷贝数据
    CopyMemory(data_queue_buffer_ + insert_pos_, &data_head, sizeof(data_head));

    // 附加数据
    if (data_head.wDataSize > 0) {
      // 变量定义
      WORD excursion = sizeof(data_head);

      // 插入数据
      for (WORD i = 0; i < data_count; i++) {
        if (data_buffer[i].wDataSize > 0) {
          // 效验状态
          ASSERT(data_buffer[i].pDataBuffer != nullptr);

          // 拷贝数据
          CopyMemory(data_queue_buffer_ + insert_pos_ + excursion, data_buffer[i].pDataBuffer, data_buffer[i].wDataSize);

          // 设置变量
          excursion += data_buffer[i].wDataSize;
        }
      }
    }

    // 调整数据
    data_packet_count_++;
    data_size_ += sizeof(data_head) + data_head.wDataSize;
    insert_pos_ += sizeof(data_head) + data_head.wDataSize;
    terminal_pos_ = std::max(terminal_pos_, insert_pos_);

    return true;
  } catch (...) {
    ASSERT(FALSE);
    return false;
  }
}

// 获取数据
bool CWHDataQueue::DistillData(tagDataHead& data_head, VOID* buffer, WORD buffer_size) {
  // 效验变量
  ASSERT(data_size_ > 0);
  ASSERT(data_packet_count_ > 0);
  ASSERT(data_queue_buffer_ != nullptr);

  // 效验变量
  if (data_size_ == 0) {
    return false;
  }
  if (data_packet_count_ == 0) {
    return false;
  }

  // 调整参数
  if (data_query_pos_ == terminal_pos_) {
    data_query_pos_ = 0;
    terminal_pos_ = insert_pos_;
  }

  // 获取指针
  ASSERT(buffer_size_ >= (data_query_pos_ + sizeof(tagDataHead)));
  tagDataHead* query_head = (tagDataHead*) (data_queue_buffer_ + data_query_pos_);
  ASSERT(buffer_size >= query_head->wDataSize);

  // 获取大小
  WORD packet_size = sizeof(data_head) + query_head->wDataSize;
  ASSERT(buffer_size_ >= (data_query_pos_ + packet_size));

  // 判断缓冲
  WORD copy_size = 0;
  ASSERT(buffer_size >= query_head->wDataSize);
  if (buffer_size >= query_head->wDataSize) {
    copy_size = query_head->wDataSize;
  }

  // 拷贝数据
  data_head = *query_head;
  if (data_head.wDataSize > 0) {
    if (buffer_size < query_head->wDataSize) {
      data_head.wDataSize = 0;
    } else {
      CopyMemory(buffer, query_head + 1, data_head.wDataSize);
    }
  }

  // 效验参数
  ASSERT(packet_size <= data_size_);
  ASSERT(buffer_size_ >= (data_query_pos_ + packet_size));

  // 设置变量
  data_packet_count_--;
  data_size_ -= packet_size;
  data_query_pos_ += packet_size;

  return true;
}

// 删除数据
VOID CWHDataQueue::RemoveData(bool free_memory) {
  // 设置变量
  data_size_ = 0;
  insert_pos_ = 0;
  terminal_pos_ = 0;
  data_query_pos_ = 0;
  data_packet_count_ = 0;

  // 删除内存
  if (free_memory) {
    buffer_size_ = 0;
    SafeDeleteArray(data_queue_buffer_);
  }
}

// 调整存储
bool CWHDataQueue::RectifyBuffer(DWORD need_size) {
  try {
    // 缓冲判断
    if ((data_size_ + need_size) > buffer_size_) {
      throw 0;
    }

    // 重新开始
    if ((insert_pos_ == terminal_pos_) && ((insert_pos_ + need_size) > buffer_size_)) {
      if (data_query_pos_ >= need_size) {
        insert_pos_ = 0;
      } else {
        throw 0;
      }
    }

    // 缓冲判断
    if ((insert_pos_ < terminal_pos_) && ((insert_pos_ + need_size) > data_query_pos_)) {
      throw 0;
    }
  } catch (...) {
    try {
      // 申请内存
      DWORD rise_size = std::max(buffer_size_ / 2, need_size * 10);
      uint8_t* new_queue_service_buffer = new BYTE[buffer_size_ + rise_size];

      // 错误判断
      ASSERT(new_queue_service_buffer != nullptr);
      if (new_queue_service_buffer == nullptr) {
        return false;
      }

      // 拷贝数据
      if (data_queue_buffer_ != nullptr) {
        // 效验状态
        ASSERT(terminal_pos_ >= data_size_);
        ASSERT(terminal_pos_ >= data_query_pos_);

        // 拷贝数据
        DWORD part_one_size = terminal_pos_ - data_query_pos_;
        if (part_one_size > 0) {
          CopyMemory(new_queue_service_buffer, data_queue_buffer_ + data_query_pos_, part_one_size);
        }
        if (data_size_ > part_one_size) {
          // ASSERT((m_dwInsertPos+dwPartOneSize)==m_dwDataSize);
          CopyMemory(new_queue_service_buffer + part_one_size, data_queue_buffer_, insert_pos_);
        }
      }

      // 设置变量
      data_query_pos_ = 0;
      insert_pos_ = data_size_;
      terminal_pos_ = data_size_;
      buffer_size_ = buffer_size_ + rise_size;

      // 设置缓冲
      SafeDeleteArray(data_queue_buffer_);
      data_queue_buffer_ = new_queue_service_buffer;
    } catch (...) {
      return false;
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
