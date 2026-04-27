#pragma once

// 引入文件
#include "ServiceCoreHead.h"

#include <string>
#include <vector>

#ifdef _UNICODE
#define STRINGT std::wstring
#else
#define STRINGT std::string
#endif // _UNICODE
#pragma once

#ifdef _WIN32
#define ToSimpleStringT(s) StringUtils::ToStringT(s)
#define ToSimpleUtf8(s) StringUtils::ToUtf8(s)
#else
#define ToSimpleStringT(s) (s) // s: UTF16 OR LOCAL
#define ToSimpleUtf8(s) (s)    // s: UTF16 OR LOCAL
#endif                         // _WIN32

/**
 * \brief 字符串实用类
 * \author sf
 * \date 2019年6月18日
 */
class SERVICE_CORE_CLASS StringUtils final {
public:
#ifdef _WIN32
  /**
   * \brief 本地字符串转UTF8字符串
   * \param local_in 本地字符串
   * \param utf8_out UTF8字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToUtf8(const std::string& local_in, std::string& utf8_out, uint32_t local_cp = 0);
  static std::string ToUtf8(const std::string& local_in, uint32_t local_cp = 0);

  /**
   * \brief UTF8字符串转UTF16字符串
   * \param utf16_in UTF16字符串
   * \param utf8_out UTF8字符串(成功则修改)
   * \return 是否转换成功
   */
  static bool ToUtf8(const std::wstring& utf16_in, std::string& utf8_out);
  static std::string ToUtf8(const std::wstring& utf16_in);

  /**
   * \brief 本地字符串转UTF16字符串
   * \param local_in 本地字符串
   * \param utf16_out UTF16字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToUtf16(const std::string& local_in, std::wstring& utf16_out, uint32_t local_cp = 0);
  static std::wstring ToUtf16(const std::string& local_in, uint32_t local_cp = 0);

  /**
   * \brief UTF8字符串转UTF16字符串
   * \param local_in 本地字符串
   * \param utf16_out UTF16字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToUtf16Ex(const std::string& utf8_in, std::wstring& utf16_out);
  static std::wstring ToUtf16Ex(const std::string& utf8_in);

  /**
   * \brief UTF16字符串转模板字符串
   * \param utf16_in UTF16字符串
   * \param stringt_out 模板字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToStringT(const std::wstring& utf16_in, STRINGT& stringt_out, uint32_t local_cp = 0);
  static STRINGT ToStringT(const std::wstring& utf16_in, uint32_t local_cp = 0);

  /**
   * \brief 本地字符串转模板字符串
   * \param local_in 本地字符串
   * \param stringt_out 模板字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToStringT(const std::string& local_in, STRINGT& stringt_out, uint32_t local_cp = 0);
  static STRINGT ToStringT(const std::string& local_in, uint32_t local_cp = 0);

  /**
   * \brief UTF16字符串转本地字符串
   * \param utf16_in UTF16字符串
   * \param local_out 本地字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToLocal(const std::wstring& utf16_in, std::string& local_out, uint32_t local_cp = 0);
  static std::string ToLocal(const std::wstring& utf16_in, uint32_t local_cp = 0);
 #endif

  /**
   * \brief 字符串转Bool类型
   * \param string_in 字符串
   * \param bool_out Bool类型(成功则修改)
   * \return 是否转换成功
   */
  static bool ToBool(const std::string& string_in, bool& bool_out);
  static bool ToBool(const std::wstring& string_in, bool& bool_out);

  /**
   * \brief 删除左边空白字符
   * \param string_in 输入/输出字符串
   * \return 修剪之后字符串
   */
  static std::string& LTrim(std::string& string_in);
  static std::wstring& LTrim(std::wstring& string_in);

  /**
   * \brief 删除右边空白字符
   * \param string_in 输入/输出字符串
   * \return 修剪之后字符串
   */
  static std::string& RTrim(std::string& string_in);
  static std::wstring& RTrim(std::wstring& string_in);

  /**
   * \brief 删除两边空白字符
   * \param string_in 输入/输出字符串
   * \return 修剪之后字符串
   */
  static std::string& Trim(std::string& string_in);
  static std::wstring& Trim(std::wstring& string_in);

  /**
   * \brief 比较字符串是否相等
   * \param l_string 字符串1
   * \param r_string 字符串2
   * \param ignore_case 忽略大小写
   * \return 是否相等
   */
  static bool Equals(const std::string& l_string, const std::string& r_string, bool ignore_case = true);
  static bool Equals(const std::wstring& l_string, const std::wstring& r_string, bool ignore_case = true);

  /**
   * \brief 按照正则表达式拆分字符串
   * \param input 要拆分的字符串
   * \param patten 正则表达式
   * \param skip_empty 跳过空字符串
   * \return 拆分的结果集
   */
  static std::vector<std::string> Split(const std::string& input, const char* patten, bool skip_empty = true);
  static std::vector<std::wstring> Split(const std::wstring& input, const wchar_t* patten, bool skip_empty = true);

  /**
   * \brief 按照分隔符拆分字符串
   * \param input 要拆分的字符串
   * \param delim 分隔符
   * \param skip_empty 跳过空字符串
   * \return 拆分的结果集
   */
  static std::vector<std::string> Split(const std::string& input, char delim, bool skip_empty = true);
  static std::vector<std::wstring> Split(const std::wstring& input, wchar_t delim, bool skip_empty = true);
};