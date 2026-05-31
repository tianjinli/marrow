#include "StringUtils.h"

#include <cctype>
#include <regex>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>

bool StringUtils::ToUtf8(std::string_view local_in, std::string& utf8_out, const uint32_t local_cp) {
  std::wstring utf16;
  if (ToUtf16(local_in, utf16, local_cp)) {
    int u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), int(utf16.size()), nullptr, 0, nullptr, nullptr);
    if (u8_size > 0) {
      const std::string utf8(u8_size, 0);
      u8_size = WideCharToMultiByte(CP_UTF8, 0, utf16.data(), int(utf16.size()), const_cast<char*>(utf8.data()), int(utf8.size()), nullptr, nullptr);
      if (local_cp == CP_UTF8) {
        return (u8_size == int(local_in.size())) ? (utf8_out = local_in), true : false;
      }
      return (u8_size == int(utf8.size())) ? ((utf8_out = utf8), true) : false;
    }
  }
  return false; // 在这里可以抛异常
}

std::string StringUtils::ToUtf8(std::string_view local_in, uint32_t local_cp) {
  std::string utf8_out;
  if (ToUtf8(local_in, utf8_out, local_cp)) {
    return utf8_out;
  }
  return {};
}

bool StringUtils::ToUtf8(std::wstring_view utf16_in, std::string& utf8_out) {
  return ToLocal(utf16_in, utf8_out, CP_UTF8);
}

std::string StringUtils::ToUtf8(std::wstring_view utf16_in) {
  return ToLocal(utf16_in, CP_UTF8);
}

bool StringUtils::ToUtf16(std::string_view local_in, std::wstring& utf16_out, uint32_t local_cp) {
  int u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), nullptr, 0);
  if (u16_size > 0) {
    const std::wstring utf16(u16_size, 0);
    u16_size = MultiByteToWideChar(local_cp, MB_ERR_INVALID_CHARS, local_in.data(), int(local_in.size()), const_cast<wchar_t*>(utf16.data()),
                                   int(utf16.size()));
    if (u16_size > 0) {
      utf16_out = utf16;
      return true;
    }
  }
  return false; // 在这里可以抛异常
}

std::wstring StringUtils::ToUtf16(std::string_view local_in, uint32_t local_cp) {
  std::wstring utf16_out;
  if (ToUtf16(local_in, utf16_out, local_cp)) {
    return utf16_out;
  }
  return {};
}

bool StringUtils::ToLocal(std::wstring_view utf16_in, std::string& local_out, uint32_t local_cp) {
  int mb_size = WideCharToMultiByte(local_cp, 0, utf16_in.data(), int(utf16_in.size()), nullptr, 0, nullptr, nullptr);
  if (mb_size > 0) {
    const std::string local(mb_size, 0);
    mb_size =
        WideCharToMultiByte(local_cp, 0, utf16_in.data(), int(utf16_in.size()), const_cast<char*>(local.data()), int(local.size()), nullptr, nullptr);

    return (mb_size == int(local.size())) ? ((local_out = local), true) : false;
  }
  return false;
}

std::string StringUtils::ToLocal(std::wstring_view utf16_in, uint32_t local_cp) {
  std::string utf8_out;
  if (ToLocal(utf16_in, utf8_out, local_cp)) {
    return utf8_out;
  }
  return {};
}
#else
#include <iconv.h>

bool StringUtils::ToUtf16(std::string_view utf8_in, std::u16string& utf16_out) {
  // 🚀 性能优化核心：使用 thread_local 保证多线程安全的同时，复用 iconv 句柄。
  // "UTF-16LE" 代表小端序的 UTF-16（Windows 和 SQL Server 的原生标准）
  thread_local iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
  if (cd == reinterpret_cast<iconv_t>(-1)) {
    return false;
  }

  // 重置句柄状态
  iconv(cd, nullptr, nullptr, nullptr, nullptr);

  // 准备输入缓冲区
  size_t in_bytes_left = utf8_in.size();

  // UTF‑16 最坏情况：每个 UTF‑8 字节可能变成 2 字节
  size_t max_out_bytes = utf8_in.size() * sizeof(char16_t);
  utf16_out.resize(utf8_in.size());

  // 调用 Linux 底层 C 语言优化版的 iconv 字符集映射（极快）
  char* in_buf = const_cast<char*>(utf8_in.data());
  char* out_buf = reinterpret_cast<char*>(utf16_out.data());
  size_t out_bytes_left = max_out_bytes;
  size_t res = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
  if (res == static_cast<size_t>(-1)) {
    // 转换失败（可能输入了非法的 UTF-8 序列）
    return false;
  }

  // 强转为符合 16位 宽度的 char16_t 塞入输出对象中
  size_t real_out_bytes = max_out_bytes - out_bytes_left;
  size_t real_out_elements = real_out_bytes / sizeof(char16_t);
  utf16_out.resize(real_out_elements);
  return true;
}

std::u16string StringUtils::ToUtf16(std::string_view utf8_in) {
  std::u16string utf16_out;
  if (ToUtf16(utf8_in, utf16_out)) {
    return utf16_out;
  }
  return {};
}

bool StringUtils::ToUtf8(std::u16string_view utf16_in, std::string& utf8_out) {
  // UTF-16LE → UTF-8
  thread_local iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd == reinterpret_cast<iconv_t>(-1)) {
    return false;
  }

  // 重置句柄状态
  iconv(cd, nullptr, nullptr, nullptr, nullptr);

  // 输入缓冲区
  size_t in_bytes_left = utf16_in.size() * sizeof(char16_t);

  // 🚀 直接分配空间给传出对象
  size_t max_out_bytes = utf16_in.size() * 4; // UTF-8 最多 4 字节/字符，结尾不需要额外+1
  utf8_out.resize(max_out_bytes);

  // 调用 Linux 底层 C 语言优化版的 iconv 字符集映射（极快）
  char* in_buf = reinterpret_cast<char*>(const_cast<char16_t*>(utf16_in.data()));
  char* out_buf = utf8_out.data();
  size_t out_bytes_left = max_out_bytes;
  size_t res = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
  if (res == static_cast<size_t>(-1)) {
    return false;
  }

  size_t real_out_bytes = max_out_bytes - out_bytes_left;
  utf8_out.resize(real_out_bytes);
  return true;
}

std::string StringUtils::ToUtf8(std::u16string_view utf16_in) {
  std::string utf8_out;
  if (ToUtf8(utf16_in, utf8_out)) {
    return utf8_out;
  }
  return {};
}

bool StringUtils::ToNChar(std::string_view utf8_in, std::vector<uint8_t>& nchar_out) {
  if (utf8_in.empty()) {
    return false;
  }

  // thread_local 保证多线程安全 + iconv 句柄复用（性能最高）
  static thread_local iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
  if (cd == reinterpret_cast<iconv_t>(-1)) {
    return false;
  }

  // 重置 iconv 状态
  iconv(cd, nullptr, nullptr, nullptr, nullptr);

  // 输入缓冲区
  size_t in_bytes_left = utf8_in.size();

  // UTF‑16 最多 2 字节/字符 + 2 字节终止符
  size_t out_bytes_left = utf8_in.size() * sizeof(char16_t);
  nchar_out.resize(out_bytes_left);

  // 调用 iconv
  char* in_buf = const_cast<char*>(utf8_in.data());
  char* out_buf = reinterpret_cast<char*>(nchar_out.data());
  size_t res = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
  if (res == static_cast<size_t>(-1)) {
    nchar_out.clear();
    return false;
  }

  // 调整 vector 大小为实际写入字节数
  size_t real_out_bytes = nchar_out.size() - out_bytes_left;
  nchar_out.resize(real_out_bytes);
  return true;
}

std::vector<uint8_t> StringUtils::ToNChar(std::string_view utf8_in) {
  std::vector<uint8_t> nchar_out;
  if (ToNChar(utf8_in, nchar_out)) {
    return nchar_out;
  }
  return {};
}

bool StringUtils::CopyNChar(WideChar* str1, const char* str2, int max_len, int* out_len) {
  if (!str1 || !str2 || max_len <= 0) {
    return false;
  }

  // Linux: UTF‑8 → UTF‑16LE (u16string)
  auto w = ToUtf16(str2);

  const size_t max_chars = static_cast<size_t>(max_len - 1);
  const size_t len = std::min(max_chars, w.size());

  std::memcpy(str1, w.data(), len * sizeof(char16_t));
  str1[len] = u'\0';

  if (out_len) {
    *out_len = static_cast<uint16_t>((len + 1) * sizeof(NCHAR));
  }

  return true;
}

int StringUtils::CopyNChar(WideChar* str1, const char* str2, int max_len) {
  int out_len = 0;
  if (CopyNChar(str1, str2, max_len, &out_len)) {
    return out_len;
  }
  return 0;
}

bool StringUtils::CopyNChar(char* str1, const WideChar* str2, int max_len, int* out_len) {
  if (!str1 || !str2 || max_len <= 0) {
    return false;
  }

  // Linux: UTF‑16 (u16string) → UTF‑8
  auto w = ToUtf8(str2);

  const size_t max_chars = static_cast<size_t>(max_len - 1);
  const size_t len = std::min(max_chars, w.size());

  std::memcpy(str1, w.data(), len * sizeof(char));
  str1[len] = '\0';

  if (out_len) {
    *out_len = static_cast<uint16_t>((len + 1) * sizeof(char));
  }

  return true;
}

int StringUtils::CopyNChar(char* str1, const WideChar* str2, int max_len) {
  int out_len = 0;
  if (CopyNChar(str1, str2, max_len, &out_len)) {
    return out_len;
  }
  return 0;
}

#endif

bool StringUtils::ToBool(std::string_view string_in, bool& bool_out) {
  if (string_in == "true" || string_in == "True" || string_in == "TRUE" || string_in == "yes" || string_in == "Yes" || string_in == "YES") {
    bool_out = true;
    return true;
  }
  if (string_in == "false" || string_in == "False" || string_in == "FALSE" || string_in == "no" || string_in == "No" || string_in == "NO") {
    bool_out = false;
    return false;
  }
  return false;
}

bool StringUtils::ToBool(std::wstring_view string_in, bool& bool_out) {
  std::string new_string;
  for (size_t i = 0; i < string_in.length(); i++) {
    new_string.push_back(static_cast<const char>(string_in.at(i)));
  }
  return ToBool(new_string, bool_out);
}

std::string& StringUtils::LTrim(std::string& string_in) {
  string_in.erase(string_in.begin(), std::ranges::find_if(string_in, [](int ch) {
                    return !std::isspace(ch);
                  }));
  return string_in;
}

std::wstring& StringUtils::LTrim(std::wstring& string_in) {
  string_in.erase(string_in.begin(), std::ranges::find_if(string_in, [](int ch) {
                    return !std::isspace(ch);
                  }));
  return string_in;
}

std::string& StringUtils::RTrim(std::string& string_in) {
  string_in.erase(std::ranges::find_if_not(string_in,
                                           [](const int ch) {
                                             return !std::isspace(ch);
                                           }),
                  string_in.end());
  return string_in;
}

std::wstring& StringUtils::RTrim(std::wstring& string_in) {
  string_in.erase(std::ranges::find_if_not(string_in,
                                           [](const int ch) {
                                             return !std::isspace(ch);
                                           }),
                  string_in.end());
  return string_in;
}

std::string& StringUtils::Trim(std::string& string_in) {
  return LTrim(string_in), RTrim(string_in);
}

std::wstring& StringUtils::Trim(std::wstring& string_in) {
  return LTrim(string_in), RTrim(string_in);
}

bool StringUtils::Equals(const std::string& l_string, const std::string& r_string, bool ignore_case) {
  if (ignore_case) {
    return std::ranges::equal(l_string,r_string, [](const char l, const char r) {
      return tolower(l) == tolower(r);
    });
  }
  return l_string == r_string;
}

bool StringUtils::Equals(const std::wstring& l_string, const std::wstring& r_string, bool ignore_case) {
  if (ignore_case) {
    return std::ranges::equal(l_string, r_string, [](const wchar_t l, const wchar_t r) {
      return tolower(l) == tolower(r);
    });
  }
  return l_string == r_string;
}

std::vector<std::string> StringUtils::Split(const std::string& input, std::string_view patten, bool skip_empty) {
  // passing -1 as the submatch index parameter performs splitting
  std::regex regex((patten.data()));
  std::sregex_token_iterator first{input.begin(), input.end(), regex, -1}, last;

  std::vector<std::string> tokens;
  for (auto itr = first; itr != last; ++itr) {
    std::string each = *itr;
    if (skip_empty) {
      if (auto& trim = Trim(each); trim.empty()) {
        continue;
      }
    }
    tokens.push_back(each);
  }
  return tokens;
}

std::vector<std::wstring> StringUtils::Split(const std::wstring& input, std::wstring_view patten, bool skip_empty) {
  // passing -1 as the submatch index parameter performs splitting
  std::wregex regex(patten.data());
  std::wsregex_token_iterator first{input.begin(), input.end(), regex, -1}, last;

  // std::vector<std::wstring> tokens = {first, last};
  // if (skip_empty) {
  //   for (auto itr = tokens.begin(); itr != tokens.end();) {
  //     std::wstring each = *itr;
  //     if (auto& trim = Trim(each); trim.empty()) {
  //       itr = tokens.erase(itr);
  //       continue;
  //     }
  //     ++itr;
  //   }
  // }

  // 下面代码运行效率为注释代码的两倍
  std::vector<std::wstring> tokens;
  for (auto itr = first; itr != last; ++itr) {
    std::wstring each = *itr;
    if (skip_empty) {
      if (auto& trim = Trim(each); trim.empty()) {
        continue;
      }
    }
    tokens.push_back(each);
  }
  return tokens;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const char delim, const bool skip_empty) {
  std::istringstream split(input);
  std::vector<std::string> tokens;
  if (skip_empty) {
    for (std::string each; std::getline(split, each, delim);) {
      if (auto& trim = Trim(each); trim.empty()) {
        continue;
      }
      tokens.push_back(each);
    }
  } else {
    for (std::string each; std::getline(split, each, delim); tokens.push_back(each))
      ;
  }
  return tokens;
}

std::vector<std::wstring> StringUtils::Split(const std::wstring& input, const wchar_t delim, const bool skip_empty) {
  std::wistringstream split(input);
  std::vector<std::wstring> tokens;
  if (skip_empty) {
    for (std::wstring each; std::getline(split, each, delim);) {
      if (auto& trim = Trim(each); trim.empty()) {
        continue;
      }
      tokens.push_back(each);
    }
  } else {
    for (std::wstring each; std::getline(split, each, delim); tokens.push_back(each))
      ;
  }
  return tokens;
}

int StringUtils::Replace(std::string& inout, const std::string& from, const std::string& to) {
  int count = 0;
  for (size_t pos = 0; (pos = inout.find(from, pos)) != std::string::npos; pos += to.length()) {
    inout.replace(pos, from.length(), to);
    count++;
  }
  return count;
}

int StringUtils::Replace(std::wstring& inout, const std::wstring& from, const std::wstring& to) {
  int count = 0;
  for (size_t pos = 0; (pos = inout.find(from, pos)) != std::wstring::npos; pos += to.length()) {
    inout.replace(pos, from.length(), to);
    count++;
  }
  return count;
}

int StringUtils::Compare(std::string_view l_string, std::string_view r_string, bool ignore_case) {
  if (ignore_case) {
    const size_t n = std::min(l_string.size(), r_string.size());
    for (size_t i = 0; i < n; ++i) {
      auto ca = std::tolower(l_string[i]);
      auto cb = std::tolower(r_string[i]);

      if (ca < cb) {
        return -1;
      }
      if (ca > cb) {
        return 1;
      }
    }

    if (l_string.size() < r_string.size()) {
      return -1;
    }
    if (l_string.size() > r_string.size()) {
      return 1;
    }

    return 0;
  }
  return l_string.compare(r_string);
}

int StringUtils::Compare(std::wstring_view l_string, std::wstring_view r_string, bool ignore_case) {
  if (ignore_case) {
    const size_t n = std::min(l_string.size(), r_string.size());
    for (size_t i = 0; i < n; ++i) {
      auto ca = ::towlower(l_string[i]);
      auto cb = ::towlower(r_string[i]);

      if (ca < cb) {
        return -1;
      }
      if (ca > cb) {
        return 1;
      }
    }

    if (l_string.size() < r_string.size()) {
      return -1;
    }
    if (l_string.size() > r_string.size()) {
      return 1;
    }

    return 0;
  }
  return l_string.compare(r_string);
}
