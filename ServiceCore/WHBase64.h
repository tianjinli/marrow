#pragma once

// 引入文件
#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// Base64编码
class SERVICE_CORE_CLASS CWHBase64 {
  // 函数定义
public:
  // 构造函数
  CWHBase64() = default;
  // 析构函数
  virtual ~CWHBase64() = default;

  // 编码函数
public:
  // 编码函数
  static int Encode(const char* data, int data_len, TCHAR out_buffer[], int buffer_len);
  // 解码函数
  static int Decode(const TCHAR* code, int code_len, TCHAR out_buffer[], int buffer_len);
};

//////////////////////////////////////////////////////////////////////////////////
