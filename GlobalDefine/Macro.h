#pragma once

#include "Typedef.h"
//////////////////////////////////////////////////////////////////////////////////
// 常用常量

// 无效数值
#define INVALID_BYTE ((BYTE) (0xFF)) // 无效数值
#define INVALID_WORD ((WORD) (0xFFFF)) // 无效数值
#define INVALID_DWORD ((DWORD) (0xFFFFFFFF)) // 无效数值

//////////////////////////////////////////////////////////////////////////////////

// 无效地址
#define INVALID_IP_ADDRESS(IPAddress) (((IPAddress == 0L) || (IPAddress == INADDR_NONE)))

//////////////////////////////////////////////////////////////////////////////////

// 存储长度
#define CountStringBufferA(s) ((UINT) ((strlen(s) + 1) * sizeof(char)))
#define CountStringBufferW(s) ((UINT) ((wcslen(s) + 1) * sizeof(wchar_t)))

#if defined(_DEBUG) || defined(DEBUG)
#define DEBUG_ENABLED
#endif

// 存储长度
#ifdef _UNICODE
#define CountStringBuffer CountStringBufferW
#define SnprintfT std::swprintf
#define SscanfT std::swscanf
#define StrnCpyT std::wcsncpy
#define StrChrT std::wcschr
#define StrrChrT std::wcsrchr
#define StrLenT std::wcslen
#define StrCmpT std::wcscmp
#define StringT std::wstring
#else
#define CountStringBuffer CountStringBufferA
#define SnprintfT std::snprintf
#define SscanfT std::sscanf
#define StrnCpyT std::strncpy
#define StrChrT std::strchr
#define StrrChrT std::strrchr
#define StrLenT std::strlen
#define StrCmpT std::strcmp
#define StringT std::string
#endif

//////////////////////////////////////////////////////////////////////////////////

// 接口释放
#define SafeRelease(pObject)  \
  do {                        \
    if (pObject != nullptr) { \
      pObject->Release();     \
      pObject = nullptr;      \
    }                         \
  } while (false)

// 删除指针
#define SafeDelete(pData) \
  do {                    \
    try {                 \
      delete pData;       \
    } catch (...) {       \
      ASSERT(FALSE);      \
    }                     \
    pData = nullptr;      \
  } while (false)

// 删除数组
#define SafeDeleteArray(pData) \
  do {                         \
    try {                      \
      delete[] pData;          \
    } catch (...) {            \
      ASSERT(FALSE);           \
    }                          \
    pData = nullptr;           \
  } while (false)

//////////////////////////////////////////////////////////////////////////////////

// 转换字符
//  T必须为 LONGLONG 或 double
template<typename T, typename = typename std::enable_if<std::is_same<T, LONGLONG>::value || std::is_same<T, double>::value>::type>
inline VOID SwitchScoreFormat(T lScore, UINT uSpace, LPCTSTR pszFormat, LPTSTR pszBuffer, WORD wBufferSize) {
  // 转换数值
  TCHAR szSwitchScore[32] = TEXT("");
  SnprintfT(szSwitchScore, std::size(szSwitchScore), pszFormat, lScore);

  if (uSpace > 0) {
    // 变量定义
    WORD wTargetIndex = 0;
    WORD wSourceIndex = 0;
    WORD wSourceStringLen = 0;

    // 计算长度
    while (szSwitchScore[wSourceStringLen] != 0 && szSwitchScore[wSourceStringLen] != '.') {
      ++wSourceStringLen;
    }

    // 转换字符
    for (INT i = 0; i < wSourceStringLen; i++) {
      // 拷贝字符
      pszBuffer[wTargetIndex++] = szSwitchScore[wSourceIndex++];

      // 插入逗号
      if ((wSourceStringLen - wSourceIndex > 0) && ((wSourceStringLen - wSourceIndex) % uSpace == 0)) {
        pszBuffer[wTargetIndex++] = TEXT(',');
      }
    }

    // 补充小数
    while (szSwitchScore[wSourceIndex] != 0) {
      pszBuffer[wTargetIndex++] = szSwitchScore[wSourceIndex++];
    }

    // 结束字符
    pszBuffer[wTargetIndex++] = 0;
  } else {
    CopyMemory(pszBuffer, szSwitchScore, wBufferSize);
  }
}

// 转换字符
inline VOID SwitchScoreFormat(LONGLONG lScore, UINT uSpace, LPCTSTR pszFormat, LPTSTR pszBuffer, WORD wBufferSize) {
  SwitchScoreFormat<LONGLONG>(lScore, uSpace, pszFormat, pszBuffer, wBufferSize);
}

// 转换字符
inline VOID SwitchScoreFormat(DOUBLE lScore, UINT uSpace, LPCTSTR pszFormat, LPTSTR pszBuffer, WORD wBufferSize) {
  SwitchScoreFormat<DOUBLE>(lScore, uSpace, pszFormat, pszBuffer, wBufferSize);
}

// simulation of Windows GetTickCount()
/// 需要使用内联函数否则会导致多重定义
inline int64_t GetTickCountImpl() {
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

#define LSTRCPYN(dst_str, src_str, max_len) \
  do {                                      \
    auto last_index = max_len - 1;          \
    StrnCpyT(dst_str, src_str, last_index); \
    dst_str[last_index] = 0;                \
  } while (false)

// 转换结构体到字节数组
template<typename T>
std::shared_ptr<std::vector<uint8_t>> ConvertToBytes(const T& obj) {
  static_assert(!std::is_pointer_v<T>, "不接受指针类型，请传值或引用");
  const auto begin = reinterpret_cast<const uint8_t*>(&obj);
  return std::make_shared<std::vector<uint8_t>>(begin, begin + sizeof(T));
}

inline std::shared_ptr<std::vector<uint8_t>>  ConvertToBytes(const void* data, const size_t size) {
  const auto begin = static_cast<const uint8_t*>(data);
  return std::make_shared<std::vector<uint8_t>>(begin, begin + size);
}

// 追加结构体到字节数组
template<typename T>
void AppendToBytes(std::shared_ptr<std::vector<uint8_t>>& dst, const T& obj) {
  static_assert(!std::is_pointer_v<T>, "不接受指针类型，请传值或引用");
  const auto begin = reinterpret_cast<const uint8_t*>(&obj);
  dst->insert(dst->end(), begin, begin + sizeof(T));
}

inline void AppendToBytes(const std::shared_ptr<std::vector<uint8_t>>& dst, const void* data, const size_t size) {
  const auto begin = static_cast<const uint8_t*>(data);
  dst->insert(dst->end(), begin, begin + size);
}

//////////////////////////////////////////////////////////////////////////////////
