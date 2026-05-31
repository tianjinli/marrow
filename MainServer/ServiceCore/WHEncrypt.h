#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 宏定义
#define XOR_TIMES 8 // 加密倍数
#define MAX_SOURCE_LEN 64 // 最大长度
#define MAX_ENCRYPT_LEN (MAX_SOURCE_LEN * XOR_TIMES) // 最大长度

//////////////////////////////////////////////////////////////////////////////////

// 加密组件
class SERVICE_CORE_CLASS CWHEncrypt {
public:
  // 构造函数
  CWHEncrypt() = default;
  // 析构函数
  virtual ~CWHEncrypt() = default;

  // MD5 加密函数
  // 生成密文
  static bool MD5Encrypt(LPCTSTR source_data, TCHAR md5_result[LEN_MD5]);

  // XOR 加密函数
  // 生成密文
  static bool XorEncrypt(LPCTSTR source_data, LPTSTR encrypt_data, WORD max_count);
  // 解开密文
  static bool XorCrevasse(LPCTSTR encrypt_data, LPTSTR source_data, WORD max_count);

  // Map 加密函数
  // 生成密文
  static bool MapEncrypt(LPCTSTR source_data, LPTSTR encrypt_data, WORD max_count);
  // 解开密文
  static bool MapCrevasse(LPCTSTR encrypt_data, LPTSTR source_data, WORD max_count);
};

//////////////////////////////////////////////////////////////////////////////////
