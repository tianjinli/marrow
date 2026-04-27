#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 系统服务
class SERVICE_CORE_CLASS CWHService {
  // 函数定义
private:
  // 构造函数
  CWHService() = default;
  virtual ~CWHService() = default;

  // 机器标识
public:
  // 机器标识
  static bool GetMachineID(TCHAR szMachineID[LEN_MACHINE_ID]);
  // 网卡地址
  static bool GetMACAddress(TCHAR szMACAddress[LEN_NETWORK_ID]);

  // 系统文件
public:
  // 进程目录
  static bool GetModuleAbsPath(TCHAR szModuleAbsPath[], WORD wBufferCount);
  // 短名称
  static bool GetNameWithoutExt(TCHAR szNameWithoutExt[], WORD wBufferCount);
  // 工作目录
  static bool GetWorkDirectory(TCHAR szWorkDirectory[], WORD wBufferCount);
  // 文件版本
  static bool GetModuleVersion(LPCTSTR pszModuleName, DWORD& dwVersionInfo);

  // 压缩函数
public:
  // 压缩数据
  static ULONG CompressData(uint8_t* pcbSourceData, ULONG lSourceSize, BYTE cbResultData[], ULONG lResultSize);
  // 解压数据
  static ULONG UncompressData(uint8_t* pcbSourceData, ULONG lSourceSize, uint8_t* cbResultData, ULONG lResultSize);
};

//////////////////////////////////////////////////////////////////////////////////
