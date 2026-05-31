#pragma once

#include "ServiceCoreHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 系统服务
class SERVICE_CORE_CLASS CWHService {
public:
  // 机器标识
  static bool GetMachineID(TCHAR machine_id[LEN_MACHINE_ID]);
  // 网卡地址
  static bool GetMACAddress(TCHAR mac_address[LEN_NETWORK_ID]);

  // 系统文件
  // 进程目录
  static bool GetModuleAbsPath(TCHAR module_abs_path[], WORD buffer_count);
  // 短名称
  static bool GetNameWithoutExt(TCHAR name_without_ext[], WORD buffer_count);
  // 工作目录
  static bool GetWorkDirectory(TCHAR work_directory[], WORD buffer_count);
  // 文件版本
  static bool GetModuleVersion(LPCTSTR module_name, DWORD& version_info);

  // 压缩函数
  // 压缩数据
  static ULONG CompressData(uint8_t* source_data, ULONG source_size, BYTE result_data[], ULONG result_size);
  // 解压数据
  static ULONG UncompressData(uint8_t* source_data, ULONG source_size, uint8_t* result_data, ULONG result_size);

private:
  // 构造函数
  CWHService() = default;
  virtual ~CWHService() = default;
};

//////////////////////////////////////////////////////////////////////////////////
