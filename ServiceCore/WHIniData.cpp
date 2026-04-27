#include "WHIniData.h"

// 宏定义
#define XOR_TIMES 8                                  // 加密倍数
#define MAX_SOURCE_LEN 64                            // 最大长度
#define MAX_ENCRYPT_LEN (MAX_SOURCE_LEN * XOR_TIMES) // 最大长度
#include "WHEncrypt.h"

//////////////////////////////////////////////////////////////////////////////////
// 设置路径
bool CWHIniData::SetIniFilePath(const std::filesystem::path& ini_file) {
#ifdef _WIN32
  FILE* fp = NULL;
#if __STDC_WANT_SECURE_LIB__
  _wfopen_s(&fp, ini_file.c_str(), L"rb");
#else // !__STDC_WANT_SECURE_LIB__
  fp = _wfopen(ini_file.c_str(), L"rb");
#endif // __STDC_WANT_SECURE_LIB__
  if (!fp)
    return SI_FILE;
  SI_Error rc = ini_reader_.LoadFile(fp);
  fclose(fp);
  return (rc == 0);
#else
  FILE* fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
  fopen_s(&fp, ini_file.c_str(), "rb");
#else // !__STDC_WANT_SECURE_LIB__
  fp = fopen(ini_file.c_str(), "rb");
#endif // __STDC_WANT_SECURE_LIB__
  if (!fp) {
    return SI_FILE;
  }
  SI_Error rc = ini_reader_.LoadFile(fp);
  fclose(fp);;
  return (rc == 0);
#endif // _WIN32
}

// 读取数值
UINT CWHIniData::ReadInt(LPCTSTR pszItem, LPCTSTR pszSubItem, INT nDefault) { return ini_reader_.GetLongValue(pszItem, pszSubItem, nDefault); }

// 读取字符
LPCTSTR CWHIniData::ReadString(LPCTSTR pszItem, LPCTSTR pszSubItem, LPCTSTR pszDefault, LPTSTR pszString, WORD wMaxCount) {
  if (pszDefault == nullptr)
    pszDefault = TEXT("");
  auto str = ini_reader_.GetValue(pszItem, pszSubItem, pszDefault);

  if ((wMaxCount > 0))
    LSTRCPYN(pszString, str, wMaxCount);

  return nullptr;
}

// 读取字符
LPCTSTR CWHIniData::ReadEncryptString(LPCTSTR pszItem, LPCTSTR pszSubItem, LPCTSTR pszDefault, LPTSTR pszString, WORD wMaxCount) {
  if (pszDefault == nullptr)
    pszDefault = TEXT("");
  // 效验状态
  ASSERT(wMaxCount <= MAX_SOURCE_LEN);

  // 设置结果
  if (wMaxCount > 0)
    pszString[0] = 0;

  // 读取字符
  TCHAR szStringRead[MAX_ENCRYPT_LEN] = {};
  ReadString(pszItem, pszSubItem, TEXT(""), szStringRead, MAX_ENCRYPT_LEN);

  // 解密字符
  //  DWORD dwReadCount = strlen(szStringRead);
  //  if ((dwReadCount > 0) && (dwReadCount < std::size(szStringRead))) {
  if (szStringRead[0] > 0) {
    // 编译时候要注意
    CWHEncrypt WHEncrypt;
    WHEncrypt.XorCrevasse(szStringRead, pszString, wMaxCount);
  }

  // 默认参数
  if ((wMaxCount > 0) && (pszString[0] == 0))
    LSTRCPYN(pszString, pszDefault, wMaxCount);

  return pszString;
}

// 读取矩形
bool CWHIniData::ReadRect(RECT& ValueRect, LPCTSTR pszItem, LPCTSTR pszSubItem) {
  // 设置变量
  ZeroMemory(&ValueRect, sizeof(ValueRect));

  // 读取字符
  TCHAR szReadData[MAX_SOURCE_LEN] = {};
  ReadString(pszItem, pszSubItem, TEXT(""), szReadData, std::size(szReadData));

  // 数据处理
  if (szReadData[0] != 0) {
    // 读取变量
    LPCTSTR pszString = szReadData;
    ValueRect.left = SwitchStringToValue(pszString);
    ValueRect.top = SwitchStringToValue(pszString);
    ValueRect.right = SwitchStringToValue(pszString);
    ValueRect.bottom = SwitchStringToValue(pszString);

    return true;
  }

  return false;
}

// 读取尺寸
bool CWHIniData::ReadSize(SIZE& ValueSize, LPCTSTR pszItem, LPCTSTR pszSubItem) {
  // 设置变量
  ZeroMemory(&ValueSize, sizeof(ValueSize));

  // 读取字符
  TCHAR szReadData[MAX_SOURCE_LEN] = {};
  ReadString(pszItem, pszSubItem, TEXT(""), szReadData, std::size(szReadData));

  // 数据处理
  if (szReadData[0] != 0) {
    // 读取变量
    LPCTSTR pszString = szReadData;
    ValueSize.cx = SwitchStringToValue(pszString);
    ValueSize.cy = SwitchStringToValue(pszString);

    return true;
  }

  return false;
}

// 读取坐标
bool CWHIniData::ReadPoint(POINT& ValuePoint, LPCTSTR pszItem, LPCTSTR pszSubItem) {
  // 设置变量
  ZeroMemory(&ValuePoint, sizeof(ValuePoint));

  // 读取字符
  TCHAR szReadData[MAX_SOURCE_LEN] = {};
  ReadString(pszItem, pszSubItem, TEXT(""), szReadData, std::size(szReadData));

  // 数据处理
  if (szReadData[0] != 0) {
    // 读取变量
    LPCTSTR pszString = szReadData;
    ValuePoint.x = SwitchStringToValue(pszString);
    ValuePoint.y = SwitchStringToValue(pszString);

    return true;
  }

  return false;
}

// 读取颜色
bool CWHIniData::ReadColor(COLORREF& ValueColor, LPCTSTR pszItem, LPCTSTR pszSubItem) {
  // 设置变量
  ZeroMemory(&ValueColor, sizeof(ValueColor));

  // 读取字符
  TCHAR szReadData[MAX_SOURCE_LEN] = {};
  ReadString(pszItem, pszSubItem, TEXT(""), szReadData, std::size(szReadData));

  // 数据处理
  if (szReadData[0] != 0) {
    // 读取变量
    LPCTSTR pszString = szReadData;
    ValueColor = RGB(SwitchStringToValue(pszString), SwitchStringToValue(pszString), SwitchStringToValue(pszString));

    return true;
  }

  return false;
}

// 转换数值
LONG CWHIniData::SwitchStringToValue(LPCTSTR& pszString) {
  // 效验参数
  ASSERT((pszString != nullptr) && (pszString[0] != 0));
  if ((pszString == nullptr) || (pszString[0] == 0))
    return 0;

  // 寻找开始
  while (((pszString[0] > 0) && (pszString[0] < TEXT('0'))) || (pszString[0] > TEXT('9')))
    pszString++;

  // 读取数值
  LONG lValue = 0;
  while ((pszString[0] >= TEXT('0')) && (pszString[0] <= TEXT('9'))) {
    lValue = lValue * 10 + pszString[0] - TEXT('0');
    ++pszString;
  }

  return lValue;
}

//////////////////////////////////////////////////////////////////////////////////
