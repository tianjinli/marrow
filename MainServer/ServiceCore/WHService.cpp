#include "WHService.h"

#include "StringUtils.h"
#include "WHEncrypt.h"

//////////////////////////////////////////////////////////////////////////////////

// 压缩文件
#include "zlib.h"
#ifndef _WIN32
#include <dlfcn.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#else // _WIN32
#include <nb30.h>
#pragma comment(lib, "zlib")
#endif // _WIN32

//////////////////////////////////////////////////////////////////////////////////

// 机器标识
bool CWHService::GetMachineID(TCHAR machine_id[LEN_MACHINE_ID]) {
  // 变量定义
  TCHAR szMACAddress[LEN_NETWORK_ID] = TEXT("");

  // 网卡标识
  GetMACAddress(szMACAddress);

  // 转换信息
  ASSERT(LEN_MACHINE_ID >= LEN_MD5);
  CWHEncrypt::MD5Encrypt(szMACAddress, machine_id);

  return true;
}

// 网卡地址
bool CWHService::GetMACAddress(TCHAR mac_address[LEN_NETWORK_ID]) {
#ifdef _WIN32
  // 状态信息
  struct tagAstatInfo {
    ADAPTER_STATUS AdapterStatus; // 网卡状态
    NAME_BUFFER NameBuff[16]; // 名字缓冲
  };

  // 变量定义
  HINSTANCE hInstance = nullptr;

  // 执行逻辑
  __try {
    // 加载 DLL
    hInstance = LoadLibrary(TEXT("NetApi32.dll"));
    if (hInstance == nullptr) {
      __leave;
    }

    // 获取函数
    typedef BYTE __stdcall NetBiosProc(NCB * Ncb);
    NetBiosProc* pNetBiosProc = (NetBiosProc*) GetProcAddress(hInstance, "Netbios");
    if (pNetBiosProc == nullptr) {
      __leave;
    }

    // 变量定义
    NCB Ncb;
    LANA_ENUM LanaEnum;
    ZeroMemory(&Ncb, sizeof(Ncb));
    ZeroMemory(&LanaEnum, sizeof(LanaEnum));

    // 枚举网卡
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_length = sizeof(LanaEnum);
    Ncb.ncb_buffer = (BYTE*) &LanaEnum;
    if ((pNetBiosProc(&Ncb) != NRC_GOODRET) || (LanaEnum.length == 0)) {
      __leave;
    }

    // 获取地址
    if (LanaEnum.length > 0) {
      // 变量定义
      tagAstatInfo Adapter;
      ZeroMemory(&Adapter, sizeof(Adapter));

      // 重置网卡
      Ncb.ncb_command = NCBRESET;
      Ncb.ncb_lana_num = LanaEnum.lana[0];
      if (pNetBiosProc(&Ncb) != NRC_GOODRET) {
        __leave;
      }

      // 获取状态
      Ncb.ncb_command = NCBASTAT;
      Ncb.ncb_length = sizeof(Adapter);
      Ncb.ncb_buffer = (BYTE*) &Adapter;
      Ncb.ncb_lana_num = LanaEnum.lana[0];
      std::strcpy((char*) Ncb.ncb_callname, "*");
      if (pNetBiosProc(&Ncb) != NRC_GOODRET) {
        __leave;
      }

      // 获取地址
      for (INT i = 0; i < 6; i++) {
        ASSERT((i * 2) < LEN_NETWORK_ID);
        // 第二个参数可以写死 3 (包含终止符)
        SnprintfT(&mac_address[i * 2], LEN_NETWORK_ID - (i * 2), TEXT("%02X"), Adapter.AdapterStatus.adapter_address[i]);
      }
    }
  }

  // 结束清理
  __finally {
    // 释放资源
    if (hInstance != nullptr) {
      FreeLibrary(hInstance);
      hInstance = nullptr;
    }

    // 错误断言
    if (AbnormalTermination()) {
      ASSERT(FALSE);
    }
  }

  return true;
#else
  int sockfd;
  char buf[1024] = {};
  struct ifreq ifr = {};
  struct ifconf ifc = {};

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror("socket error");
    exit(1);
  }

  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sockfd, SIOCGIFCONF, &ifc) == -1) {
    perror("ioctl error");
    exit(2);
  }
  struct ifreq* it = ifc.ifc_req;
  const struct ifreq* const end = it + (ifc.ifc_len / sizeof(struct ifreq));

  for (; it != end; ++it) {
    std::strcpy(ifr.ifr_name, it->ifr_name);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0) {
      if (!(ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == 0) {
          auto data = (uint8_t*) ifr.ifr_hwaddr.sa_data;
          std::sprintf(mac_address, "%02X%02X%02X%02X%02X%02X", data[0], data[1], data[2], data[3], data[4], data[5]);
          return true;
        }
      }
    } else {
      perror("ioctl error");
      exit(3);
    }
  }

  return false;
#endif // _WIN32
}

// 进程路径
bool CWHService::GetModuleAbsPath(TCHAR module_abs_path[], WORD buffer_count) {
  // 模块路径
  TCHAR szModulePath[MAX_PATH] = TEXT("");
#ifdef _WIN32
  DWORD nCount = GetModuleFileName(GetModuleHandle(nullptr), szModulePath, std::size(szModulePath));
  if (nCount < 0 || nCount >= buffer_count) {
    return false;
  }
#elif defined(__APPLE__)
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) != 0) {
    return false;
  }
#else
  // 获取当前程序绝对路径
  int nCount = readlink("/proc/self/exe", szModulePath, std::size(szModulePath));
  if (nCount < 0 || nCount >= buffer_count) {
    perror("readlink error");
    return false;
  }
#endif // _WIN32
  // 设置结果
  ASSERT(szModulePath[0] != 0);
  lstrcpyn(module_abs_path, szModulePath, buffer_count);

  return true;
}

bool CWHService::GetNameWithoutExt(TCHAR name_without_ext[], WORD buffer_count) {
  // 模块路径
  TCHAR szModulePath[MAX_PATH] = TEXT("");
  if (GetModuleAbsPath(szModulePath, std::size(szModulePath))) {
    // 获取当前目录绝对路径，即去掉程序名
    if (auto pLastSeparate = StrrChrT(szModulePath, cSeparateChar)) {
      if (auto pDotSeparate = StrrChrT(++pLastSeparate, '.')) {
        *pDotSeparate = 0;
      }

      lstrcpyn(name_without_ext, pLastSeparate, buffer_count);
      return true;
    }
  }
  return false;
}

// 进程目录
bool CWHService::GetWorkDirectory(TCHAR work_directory[], WORD buffer_count) {
  if (GetModuleAbsPath(work_directory, buffer_count)) {
    if (auto last_slash = StrrChrT(work_directory, cSeparateChar)) {
      // 系统根目录则无需把分隔符替换成终止符
      if (last_slash != work_directory) {
        *last_slash = 0;
      }
      return true;
    }
  }
  return false;
}

// 文件版本
// extern "C" const uint32_t GetFileVersion() { return 0x01010000; }
bool CWHService::GetModuleVersion(LPCTSTR module_name, DWORD& version_info) {
  auto handle = DYNLIB_LOAD(module_name);
  if (handle) {
    typedef DWORD (*GetFileVersion)();
    if (auto get_file_version = (GetFileVersion) DYNLIB_GETSYM(handle, "GetFileVersion")) {
      version_info = get_file_version();
    } else {
      version_info = 0x01000000; // 01.00.00.00
    }
    DYNLIB_UNLOAD(handle);
    return true;
  }

  return false;
}

// 压缩数据
ULONG CWHService::CompressData(uint8_t* source_data, ULONG source_size, BYTE result_data[], ULONG result_size) {
  // 压缩数据
  if (compress(result_data, &result_size, source_data, source_size) == 0) {
    return result_size;
  }

  return 0;
}

// 解压数据
ULONG CWHService::UncompressData(uint8_t* source_data, ULONG source_size, uint8_t* result_data, ULONG result_size) {
  // 解压数据
  if (uncompress(result_data, &result_size, source_data, source_size) == 0) {
    return result_size;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////////
