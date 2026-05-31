#pragma once

// 引入文件
#include "ServiceCoreHead.h"

#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#define WideChar wchar_t
#define WideString std::wstring
#define WideStringView std::wstring_view
#define ToSimpleUtf8(s) StringUtils::ToUtf8(s)
#define ToSimpleUtf16(s) (s)
#define ToSimpleString(s) (s)
#else
#define WideChar char16_t
#define WideString std::u16string
#define WideStringView std::u16string_view
#define ToSimpleUtf8(s) (s)
#define ToSimpleUtf16(s) StringUtils::ToUtf16(s)
#define ToSimpleString(s) StringUtils::ToString(s)
#define lstrcpyn(dst_str, src_str, max_len) StringUtils::CopyNChar(dst_str, src_str, max_len)
#endif // _WIN32

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
  static bool ToUtf8(std::string_view local_in, std::string& utf8_out, uint32_t local_cp = 0);
  static std::string ToUtf8(std::string_view local_in, uint32_t local_cp = 0);

  /**
   * \brief UTF8字符串转UTF16字符串
   * \param utf16_in UTF16字符串
   * \param utf8_out UTF8字符串(成功则修改)
   * \return 是否转换成功
   */
  static bool ToUtf8(std::wstring_view utf16_in, std::string& utf8_out);
  static std::string ToUtf8(std::wstring_view utf16_in);

  /**
   * \brief 本地字符串转UTF16字符串
   * \param local_in 本地字符串
   * \param utf16_out UTF16字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToUtf16(std::string_view local_in, std::wstring& utf16_out, uint32_t local_cp = 0);
  static std::wstring ToUtf16(std::string_view local_in, uint32_t local_cp = 0);

  /**
   * \brief UTF16字符串转本地字符串
   * \param utf16_in UTF16字符串
   * \param local_out 本地字符串(成功则修改)
   * \param local_cp 本地字符串编码
   * \return 是否转换成功
   */
  static bool ToLocal(std::wstring_view utf16_in, std::string& local_out, uint32_t local_cp = 0);
  static std::string ToLocal(std::wstring_view utf16_in, uint32_t local_cp = 0);
#else
  /**
   * \brief UTF16字符串转UTF8字符串
   * \param utf16_in UTF16字符串
   * \param utf8_out UTF8字符串(成功则修改)
   * \return 是否转换成功
   */
  static bool ToUtf8(std::u16string_view utf16_in, std::string& utf8_out);
  static std::string ToUtf8(std::u16string_view utf16_in);

  /**
   * \brief UTF8字符串转UTF16字符串
   * \param utf8_in 本地字符串
   * \param utf16_out UTF16字符串(成功则修改)
   * \return 是否转换成功
   */
  static bool ToUtf16(std::string_view utf8_in, std::u16string& utf16_out);
  static std::u16string ToUtf16(std::string_view utf8_in);

  /**
   * \brief UTF8字符串转UTF16字符串
   * \param utf8_in 本地字符串
   * \param utf16_out UTF16字符串(成功则修改)
   * \return 是否转换成功
   * \remark nanodbc 专用
   */
  static bool ToNChar(std::string_view utf8_in, std::vector<uint8_t>& nchar_out);
  static std::vector<uint8_t> ToNChar(std::string_view utf8_in);

  /**
   * \brief 🚀 延迟推导中转捕获器
   * 通过隔离内部存储类型，彻底消除重载二义性
   */
  template<typename IN>
  class StringConvertProxy {
  private:
    using DecayedIN = std::decay_t<IN>;

    // 智能甄别内部存储：如果是容器（string/u16string）则保持常引用以复用内存；
    // 如果是轻量级视图或指针（view/char*/char[]），则直接存值，完全零开销。
    static constexpr bool IsContainer = std::is_same_v<DecayedIN, std::string> || std::is_same_v<DecayedIN, std::u16string>;

    std::conditional_t<IsContainer, const DecayedIN&, std::decay_t<IN> > in_data_;

  public:
    // 🚀 唯一的构造函数：万能引用接驳，彻底杜绝多构造函数冲突
    explicit StringConvertProxy(IN&& in) : in_data_(std::forward<IN>(in)) {}

    // 🎯 魔法通道 1：当左边接收端需要 std::string 时触发
    operator std::string() const {
      if constexpr (std::is_convertible_v<DecayedIN, std::string_view>) {
        // 输入端本来就是窄串范畴 (string, string_view, char*) -> 直接安全构造返回（同类型零拷贝）
        return std::string(in_data_);
      } else {
        // 输入端是宽串范畴 -> 触发高性能 iconv 转换
        return ToUtf8(in_data_);
      }
    }

    // 🎯 魔法通道 2：当左边接收端需要 std::u16string 时触发
    operator std::u16string() const {
      if constexpr (std::is_convertible_v<DecayedIN, std::u16string_view>) {
        // 输入端本来就是宽串范畴 (u16string, u16string_view, char16_t*) -> 直接安全构造返回
        return std::u16string(in_data_);
      } else {
        // 输入端是窄串范畴 -> 触发高性能 iconv 转换
        return ToUtf16(in_data_);
      }
    }
  };

  /**
   * \brief 万能单参自适应网关入口
   * \code{.cpp}
   * std::string s1 = ToString("s1");
   * std::string s2 = ToString(s1);
   * std::string_view s3 = s2;
   * std::string s4 = ToString(s3);
   *
   * std::u16string u1 = ToString(u"u1");
   * std::u16string u2 = ToString(u1);
   * std::u16string_view u3 = u2;
   * std::u16string u4 = ToString(u3);
   * \endcode
   */
  template<typename IN>
  static auto ToString(IN&& in) {
    return StringConvertProxy<IN>(std::forward<IN>(in));
  }

  template<typename OUT, typename IN>
  static OUT ToString(IN&& in) {
    // 1. 获取剔除了 const、volatile、引用、数组、别名之后的纯净裸类型
    using DecayedOUT = std::decay_t<OUT>;
    using DecayedIN = std::decay_t<IN>;

    static_assert(std::is_same_v<DecayedOUT, std::string> || std::is_same_v<DecayedOUT, std::u16string>, "OUT must be std::string or std::u16string");

    // 2. 核心优化：完全同类型判定（智能识别 typedef / using 别名）
    if constexpr (std::is_same_v<DecayedOUT, DecayedIN>) {
      // 🚀 极致压榨性能：利用 std::forward。如果入参是临时右值，直接 Move 移走所有权；
      // 如果是左值 s1，直接触发正常的拷贝构造返回，绝不多产生任何内存分配。
      return std::forward<IN>(in);
    }

    // 3. 核心转换区 A：目标是 UTF-16 (std::u16string)
    else if constexpr (std::is_same_v<DecayedOUT, std::u16string>) {
      // 只要输入端能无损降维转换成标准的 std::string_view（包含 char*, char[], std::string 等）
      if constexpr (std::is_convertible_v<IN, std::string_view>) {
        return ToUtf16(std::string_view(in));
      } else {
        static_assert(!sizeof(IN), "The input type cannot be converted to UTF-16 (std::string_view equivalent expected).");
      }
    }

    // 4. 核心转换区 B：目标是 UTF-8 (std::string)
    else if constexpr (std::is_same_v<DecayedOUT, std::string>) {
      // 只要输入端能无损降维转换成标准的 std::u16string_view（包含 char16_t*, char16_t[], std::u16string 等）
      if constexpr (std::is_convertible_v<IN, std::u16string_view>) {
        return ToUtf8(std::u16string_view(in));
      } else {
        static_assert(!sizeof(IN), "The input type cannot be converted to UTF-8 (std::u16string_view equivalent expected).");
      }
    }

    return {};
  }

  /**
   * \brief 复制宽字符串到窄字符串
   * \param str1 目标字符串
   * \param str2 源字符串
   * \param max_len 最大复制长度
   * \param out_len 输出复制长度
   * \return 是否复制成功
   * \remark nanodbc 专用
   */
  static bool CopyNChar(WideChar* str1, const char* str2, int max_len, int* out_len);
  static int CopyNChar(WideChar* str1, const char* str2, int max_len);

  /**
   * \brief 复制窄字符串到宽字符串
   * \param str1 目标字符串
   * \param str2 源字符串
   * \param max_len 最大复制长度
   * \param out_len 输出复制长度
   * \return 是否复制成功
   * \remark nanodbc 专用
   */
  static bool CopyNChar(char* str1, const WideChar* str2, int max_len, int* out_len);
  static int CopyNChar(char* str1, const WideChar* str2, int max_len);

#endif

  template<typename CharT>
  static bool CopyNChar(CharT* dst, const CharT* src, int max_len, int* out_len = nullptr) {
    if (!dst || !src || max_len <= 0) {
      if (out_len) {
        *out_len = 0;
      }
      return false;
    }

    using Traits = std::char_traits<CharT>;

    // 源字符串长度（不含终止符）
    const std::size_t src_len = Traits::length(src);

    // 最多复制 max_len - 1 个字符
    const std::size_t max_chars = static_cast<std::size_t>(max_len - 1);
    const std::size_t copy_len = std::min(src_len, max_chars);

    // 拷贝字符（按字节）
    if (copy_len > 0) {
      std::memcpy(dst, src, copy_len * sizeof(CharT));
    }

    // 补终止符
    dst[copy_len] = CharT{};

    // 输出字符长度（不含终止符）
    if (out_len) {
      *out_len = static_cast<int>(copy_len);
    }

    return true;
  }

  /**
   * \brief 字符串转Bool类型
   * \param string_in 字符串
   * \param bool_out Bool类型(成功则修改)
   * \return 是否转换成功
   */
  static bool ToBool(std::string_view string_in, bool& bool_out);
  static bool ToBool(std::wstring_view string_in, bool& bool_out);

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
  static std::vector<std::string> Split(const std::string& input, std::string_view patten, bool skip_empty = true);
  static std::vector<std::wstring> Split(const std::wstring& input, std::wstring_view patten, bool skip_empty = true);

  /**
   * \brief 按照分隔符拆分字符串
   * \param input 要拆分的字符串
   * \param delim 分隔符
   * \param skip_empty 跳过空字符串
   * \return 拆分的结果集
   */
  static std::vector<std::string> Split(const std::string& input, char delim, bool skip_empty = true);
  static std::vector<std::wstring> Split(const std::wstring& input, wchar_t delim, bool skip_empty = true);

  /**
   * \brief 替换字符串中的子字符串
   * \param inout 要替换的字符串
   * \param from 要替换的子字符串
   * \param to 替换后的子字符串
   * \return 替换的次数
   */
  static int Replace(std::string& inout, const std::string& from, const std::string& to);
  static int Replace(std::wstring& inout, const std::wstring& from, const std::wstring& to);

  /**
   * \brief 比较字符串是否相等
   * \param l_string 字符串1
   * \param r_string 字符串2
   * \param ignore_case 忽略大小写
   * \return 是否相等
   */
  static int Compare(std::string_view l_string, std::string_view r_string, bool ignore_case = true);
  static int Compare(std::wstring_view l_string, std::wstring_view r_string, bool ignore_case = true);
};
