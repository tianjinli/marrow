#include "WHIniData.h"

#include "StringUtils.h"
#include "WHEncrypt.h"

// 宏定义
#define XOR_TIMES 8 // 加密倍数
#define MAX_SOURCE_LEN 64 // 最大长度
#define MAX_ENCRYPT_LEN (MAX_SOURCE_LEN * XOR_TIMES) // 最大长度

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
  if (!fp) {
    return SI_FILE;
  }
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
  fclose(fp);
  ;
  return (rc == 0);
#endif // _WIN32
}

// 读取数值
UINT CWHIniData::ReadInt(LPCTSTR item, LPCTSTR sub_item, INT def) {
  return ini_reader_.GetLongValue(item, sub_item, def);
}

// 读取字符
LPCTSTR CWHIniData::ReadString(LPCTSTR item, LPCTSTR sub_item, LPCTSTR def, LPTSTR text, WORD max_count) {
  if (def == nullptr) {
    def = TEXT("");
  }
  auto str = ini_reader_.GetValue(item, sub_item, def);

  if ((max_count > 0)) {
    lstrcpyn(text, str, max_count);
  }

  return nullptr;
}

// 读取字符
LPCTSTR CWHIniData::ReadEncryptString(LPCTSTR item, LPCTSTR sub_item, LPCTSTR def, LPTSTR text, WORD max_count) {
  if (def == nullptr) {
    def = TEXT("");
  }
  // 效验状态
  ASSERT(max_count <= MAX_SOURCE_LEN);

  // 设置结果
  if (max_count > 0) {
    text[0] = 0;
  }

  // 读取字符
  TCHAR encrypt_data[MAX_ENCRYPT_LEN] = {};
  ReadString(item, sub_item, TEXT(""), encrypt_data, MAX_ENCRYPT_LEN);

  // 解密字符
  //  DWORD encrypt_count = strlen(encrypt_data);
  //  if (encrypt_count > 0 && encrypt_count < std::size(encrypt_data)) {
  if (encrypt_data[0] > 0) {
    // 编译时候要注意
    CWHEncrypt encrypt;
    encrypt.XorCrevasse(encrypt_data, text, max_count);
  }

  // 默认参数
  if (max_count > 0 && (text[0] == 0)) {
    lstrcpyn(text, def, max_count);
  }

  return text;
}

// 读取矩形
bool CWHIniData::ReadRect(RECT& rect, LPCTSTR item, LPCTSTR sub_item) {
  // 设置变量
  ZeroMemory(&rect, sizeof(rect));

  // 读取字符
  TCHAR read_data[MAX_SOURCE_LEN] = {};
  ReadString(item, sub_item, TEXT(""), read_data, std::size(read_data));

  // 数据处理
  if (read_data[0] != 0) {
    // 读取变量
    LPCTSTR text = read_data;
    rect.left = SwitchStringToValue(text);
    rect.top = SwitchStringToValue(text);
    rect.right = SwitchStringToValue(text);
    rect.bottom = SwitchStringToValue(text);

    return true;
  }

  return false;
}

// 读取尺寸
bool CWHIniData::ReadSize(SIZE& size, LPCTSTR item, LPCTSTR sub_item) {
  // 设置变量
  ZeroMemory(&size, sizeof(size));

  // 读取字符
  TCHAR read_data[MAX_SOURCE_LEN] = {};
  ReadString(item, sub_item, TEXT(""), read_data, std::size(read_data));

  // 数据处理
  if (read_data[0] != 0) {
    // 读取变量
    LPCTSTR text = read_data;
    size.cx = SwitchStringToValue(text);
    size.cy = SwitchStringToValue(text);

    return true;
  }

  return false;
}

// 读取坐标
bool CWHIniData::ReadPoint(POINT& point, LPCTSTR item, LPCTSTR sub_item) {
  // 设置变量
  ZeroMemory(&point, sizeof(point));

  // 读取字符
  TCHAR read_data[MAX_SOURCE_LEN] = {};
  ReadString(item, sub_item, TEXT(""), read_data, std::size(read_data));

  // 数据处理
  if (read_data[0] != 0) {
    // 读取变量
    LPCTSTR text = read_data;
    point.x = SwitchStringToValue(text);
    point.y = SwitchStringToValue(text);

    return true;
  }

  return false;
}

// 读取颜色
bool CWHIniData::ReadColor(COLORREF& color, LPCTSTR item, LPCTSTR sub_item) {
  // 设置变量
  ZeroMemory(&color, sizeof(color));

  // 读取字符
  TCHAR read_data[MAX_SOURCE_LEN] = {};
  ReadString(item, sub_item, TEXT(""), read_data, std::size(read_data));

  // 数据处理
  if (read_data[0] != 0) {
    // 读取变量
    LPCTSTR text = read_data;
    color = RGB(SwitchStringToValue(text), SwitchStringToValue(text), SwitchStringToValue(text));

    return true;
  }

  return false;
}

// 转换数值
LONG CWHIniData::SwitchStringToValue(LPCTSTR& text) {
  // 效验参数
  ASSERT(text != nullptr && (text[0] != 0));
  if (text == nullptr || (text[0] == 0)) {
    return 0;
  }

  // 寻找开始
  while (((text[0] > 0) && (text[0] < TEXT('0'))) || (text[0] > TEXT('9'))) {
    text++;
  }

  // 读取数值
  LONG value = 0;
  while ((text[0] >= TEXT('0')) && (text[0] <= TEXT('9'))) {
    value = value * 10 + text[0] - TEXT('0');
    ++text;
  }

  return value;
}

//////////////////////////////////////////////////////////////////////////////////
