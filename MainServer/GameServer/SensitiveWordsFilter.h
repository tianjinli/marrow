#pragma once

#include "GameServerHead.h"

//////////////////////////////////////////////////////////////////////////
#define MAX_CH 65535 // 最大的中文字符数
#define CH 2
#define EN 1

static int nNewCount = 0;

//////////////////////////////////////////////////////////////////////////
// 字符的值,val_byte代表m_pAnsiArray字符，val_word代表中文字符
typedef union {
  CHAR val_byte;
  struct {
    CHAR high;
    CHAR low;
  } val_word;
} character;


class CToken;

//////////////////////////////////////////////////////////////////////////
// 用于存放子节点
class tokenArray {
public:
  CToken* getToken(unsigned short code);
  CToken* insertToken(unsigned short code, character& _char);

  std::vector<CToken*> alltokens;
};


// 一个字符，可以是中文也可以是英文的
class CToken {
public:
  CToken(unsigned short code, character& _char) : code(code), _character(_char), isend(false) {}

  ~CToken() {
    for (size_t j = 0; j < byte_children.alltokens.size(); j++) {
      delete (byte_children.alltokens[j]);
      nNewCount--;
    }
    byte_children.alltokens.clear();
    for (size_t j = 0; j < word_children.alltokens.size(); j++) {
      delete (word_children.alltokens[j]);
      nNewCount--;
    }
    word_children.alltokens.clear();
  }
  character* getCharacter() { return &_character; }

  unsigned short getCode() { return code; }

  void setEnd() { isend = true; }

  bool isEnd() // 当前字符是否是一个词条的结尾
  {
    return isend;
  }

  static int isChinese(character* _char);
  static int isChinese(LPCTSTR buf, int pos, int size);
  tokenArray* getChild(character& _char);

  CToken* addChild(unsigned short code, character& _char);

public:
  unsigned short code; // 字符的编码
  character _character;
  tokenArray byte_children; // 后续子节点,(中文）
  tokenArray word_children; // 后续子节点,(英文)

  bool isend; // 是否是一个词的词尾
};

//////////////////////////////////////////////////////////////////////////
using CForbidWordArray = std::vector<StringT>;

//////////////////////////////////////////////////////////////////////////
// 禁词过滤树
class CFilterTree {
public:
  CForbidWordArray m_ForbidArray;

  // 函数定义
public:
  CFilterTree() {}

  ~CFilterTree();

  // 功能函数
public:
  // 创建过滤表
  void BuildTree();
  // 预处理一个词
  void BuildWord(StringViewT msg);
  // 下面用于过滤
  StringT Filtrate(StringViewT msg);
  // 重置
  void Reset();
};

//////////////////////////////////////////////////////////////////////////
// 过滤敏感词
class CSensitiveWordsFilter {
  // 变量定义
protected:
  CFilterTree m_FilterTree; // 关键词树
  // CToken				m_Token;

  // 函数定义
public:
  // 构造函数
  CSensitiveWordsFilter() {}
  // 析构函数
  virtual ~CSensitiveWordsFilter() {}

  // 功能函数
public:
  // 初始化
  void ResetSensitiveWordArray();
  // 添加禁词
  bool AddSensitiveWords(StringT& pSensitiveWords);
  // 添加完成
  void FinishAdd();
  // 过滤敏感词
  template<typename CharT>
  auto FiltrateImpl(std::basic_string_view<CharT> msg) {
    StringT tmp = StringUtils::ToString(msg);
    StringT chat = m_FilterTree.Filtrate(tmp);
    std::basic_string<CharT> out = StringUtils::ToString(chat);
    return out;
  }
  template<typename CharT>
  auto Filtrate(std::basic_string_view<CharT> msg) {
    return FiltrateImpl(msg);
  }
  template<typename CharT>
  auto Filtrate(const std::basic_string<CharT>& msg) {
    return FiltrateImpl(std::basic_string_view<CharT>(msg));
  }
  template<typename CharT>
  auto Filtrate(const CharT* msg) {
    return FiltrateImpl(std::basic_string_view<CharT>(msg));
  }
};

//////////////////////////////////////////////////////////////////////////
