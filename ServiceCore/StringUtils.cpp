#include "StringUtils.h"

#include <cctype>
#include <regex>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>

bool StringUtils::ToUtf8(const std::string& local_in, std::string& utf8_out, const uint32_t local_cp) {
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
  return false;  // 在这里可以抛异常
}

std::string StringUtils::ToUtf8(const std::string& local_in, uint32_t local_cp) {
  std::string utf8_out;
  if (ToUtf8(local_in, utf8_out, local_cp)) {
    return utf8_out;
  }
  return std::move(std::string());
}

bool StringUtils::ToUtf8(const std::wstring& utf16_in, std::string& utf8_out) { return ToLocal(utf16_in, utf8_out, CP_UTF8); }

std::string StringUtils::ToUtf8(const std::wstring& utf16_in) { return ToLocal(utf16_in, CP_UTF8); }

bool StringUtils::ToUtf16(const std::string& local_in, std::wstring& utf16_out, uint32_t local_cp) {
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
  return false;  // 在这里可以抛异常
}

std::wstring StringUtils::ToUtf16(const std::string& local_in, uint32_t local_cp) {
  std::wstring utf16_out;
  if (ToUtf16(local_in, utf16_out, local_cp)) {
    return utf16_out;
  }
  return std::move(std::wstring());
}

bool StringUtils::ToUtf16Ex(const std::string& utf8_in, std::wstring& utf16_out) { return ToUtf16(utf8_in, utf16_out, CP_UTF8); }

std::wstring StringUtils::ToUtf16Ex(const std::string& utf8_in) { return ToUtf16(utf8_in, CP_UTF8); }

bool StringUtils::ToStringT(const std::wstring& utf16_in, STRINGT& stringt_out, uint32_t local_cp) {
#ifdef _UNICODE
  return (stringt_out = utf16_in), true;
#else
  return ToLocal(utf16_in, stringt_out, local_cp);
#endif  // _UNICODE
}

STRINGT StringUtils::ToStringT(const std::wstring& utf16_in, uint32_t local_cp) {
  STRINGT stringt_out;
  if (ToStringT(utf16_in, stringt_out, local_cp)) {
    return stringt_out;
  }
  return std::move(STRINGT());
}

bool StringUtils::ToStringT(const std::string& local_in, STRINGT& stringt_out, uint32_t local_cp) {
#ifdef _UNICODE
  return ToUtf16(local_in, stringt_out, local_cp);
#else
  // 假设 local_in 与 local_cp 一致
  return (stringt_out = local_in), true;
#endif  // _UNICODE
}

STRINGT StringUtils::ToStringT(const std::string& local_in, uint32_t local_cp) {
  STRINGT stringt_out;
  if (ToStringT(local_in, stringt_out, local_cp)) {
    return std::move(stringt_out);
  }
  return std::move(STRINGT());
}

bool StringUtils::ToLocal(const std::wstring& utf16_in, std::string& local_out, uint32_t local_cp) {
  int mb_size = WideCharToMultiByte(local_cp, 0, utf16_in.data(), int(utf16_in.size()), nullptr, 0, nullptr, nullptr);
  if (mb_size > 0) {
    const std::string local(mb_size, 0);
    mb_size =
        WideCharToMultiByte(local_cp, 0, utf16_in.data(), int(utf16_in.size()), const_cast<char*>(local.data()), int(local.size()), nullptr, nullptr);

    return (mb_size == int(local.size())) ? ((local_out = local), true) : false;
  }
  return false;
}

std::string StringUtils::ToLocal(const std::wstring& utf16_in, uint32_t local_cp) {
  std::string utf8_out;
  if (ToLocal(utf16_in, utf8_out, local_cp)) {
    return utf8_out;
  }
  return std::move(std::string());
}
#endif

bool StringUtils::ToBool(const std::string& string_in, bool& bool_out) {
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

bool StringUtils::ToBool(const std::wstring& string_in, bool& bool_out) {
  std::string new_string;
  for (size_t i = 0; i < string_in.length(); i++) {
    new_string.push_back(static_cast<const char>(string_in.at(i)));
  }
  return ToBool(new_string, bool_out);
}

std::string& StringUtils::LTrim(std::string& string_in) {
  string_in.erase(string_in.begin(), std::find_if(string_in.begin(), string_in.end(), [](int ch) { return !std::isspace(ch); }));
  return string_in;
}

std::wstring& StringUtils::LTrim(std::wstring& string_in) {
  string_in.erase(string_in.begin(), std::find_if(string_in.begin(), string_in.end(), [](int ch) { return !std::isspace(ch); }));
  return string_in;
}

std::string& StringUtils::RTrim(std::string& string_in) {
  string_in.erase(std::find_if(string_in.rbegin(), string_in.rend(), [](const int ch) { return !std::isspace(ch); }).base(), string_in.end());
  return string_in;
}

std::wstring& StringUtils::RTrim(std::wstring& string_in) {
  string_in.erase(std::find_if(string_in.rbegin(), string_in.rend(), [](const int ch) { return !std::isspace(ch); }).base(), string_in.end());
  return string_in;
}

std::string& StringUtils::Trim(std::string& string_in) { return LTrim(string_in), RTrim(string_in); }

std::wstring& StringUtils::Trim(std::wstring& string_in) { return LTrim(string_in), RTrim(string_in); }

bool StringUtils::Equals(const std::string& l_string, const std::string& r_string, bool ignore_case) {
  if (ignore_case) {
    return std::equal(l_string.begin(), l_string.end(), r_string.begin(), r_string.end(),
                      [](const char l, const char r) { return tolower(l) == tolower(r); });
  }
  return l_string == r_string;
}

bool StringUtils::Equals(const std::wstring& l_string, const std::wstring& r_string, bool ignore_case) {
  if (ignore_case) {
    return std::equal(l_string.begin(), l_string.end(), r_string.begin(), r_string.end(),
                      [](const wchar_t l, const wchar_t r) { return tolower(l) == tolower(r); });
  }
  return l_string == r_string;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const char* patten, bool skip_empty) {
  // passing -1 as the submatch index parameter performs splitting
  std::regex regex(patten);
  std::sregex_token_iterator first{input.begin(), input.end(), regex, -1}, last;

  std::vector<std::string> tokens;
  for (auto it = first; it != last; ++it) {
    std::string each = *it;
    if (skip_empty) {
      if (auto& trim = Trim(each); trim.empty()) {
        continue;
      }
    }
    tokens.push_back(each);
  }
  return tokens;
}

std::vector<std::wstring> StringUtils::Split(const std::wstring& input, const wchar_t* patten, bool skip_empty) {
  // passing -1 as the submatch index parameter performs splitting
  std::wregex regex(patten);
  std::wsregex_token_iterator first{input.begin(), input.end(), regex, -1}, last;

  // std::vector<std::wstring> tokens = {first, last};
  // if (skip_empty) {
  //   for (auto it = tokens.begin(); it != tokens.end();) {
  //     std::wstring each = *it;
  //     if (auto& trim = Trim(each); trim.empty()) {
  //       it = tokens.erase(it);
  //       continue;
  //     }
  //     ++it;
  //   }
  // }

  // 下面代码运行效率为注释代码的两倍
  std::vector<std::wstring> tokens;
  for (auto it = first; it != last; ++it) {
    std::wstring each = *it;
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
    for (std::string each; std::getline(split, each, delim); tokens.push_back(each));
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
    for (std::wstring each; std::getline(split, each, delim); tokens.push_back(each));
  }
  return tokens;
}