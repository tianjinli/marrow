#include "WHBase64.h"

////////////////////////////////////////////////////////////////////////////////////////////////
//

// 常量定义
const TCHAR CHAR_63 = '+';
const TCHAR CHAR_64 = '/';
const TCHAR CHAR_PAD = '=';

// 字符表
const TCHAR kAlpha[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',     'T',    'U', 'V',
                        'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',     'p',    'q', 'r',
                        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', CHAR_63, CHAR_64};

// 缓冲定义
union unBuffer {
  unsigned char bytes[4];
  unsigned int block;
};

// 获取长度
inline int GetDataLength(int code_len) { return code_len - code_len / 4; }

// 获取长度
inline int GetCodeLength(int data_len) {
  int len = data_len + data_len / 3 + (int) (data_len % 3 != 0);
  if (len % 4)
    len += 4 - len % 4;
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// 编码函数
int CWHBase64::Encode(const char* data, int data_len, TCHAR out_buffer[], int buffer_len) {
  int len = GetCodeLength(data_len);
  // TCHAR* out = new TCHAR[len];
  std::pmr::vector<TCHAR> out(len);

  unBuffer buffer = {};
  const int mask = 0x3F;

  for (int i = 0, j = 0, left = data_len; i < data_len; i += 3, j += 4, left -= 3) {
    buffer.bytes[2] = data[i];

    if (left > 1) {
      buffer.bytes[1] = data[i + 1];
      if (left > 2)
        buffer.bytes[0] = data[i + 2];
      else
        buffer.bytes[0] = 0;
    } else {
      buffer.bytes[1] = 0;
      buffer.bytes[0] = 0;
    }

    out[j] = kAlpha[(buffer.block >> 18) & mask];
    out[j + 1] = kAlpha[(buffer.block >> 12) & mask];
    if (left > 1) {
      out[j + 2] = kAlpha[(buffer.block >> 6) & mask];
      if (left > 2)
        out[j + 3] = kAlpha[buffer.block & mask];
      else
        out[j + 3] = CHAR_PAD;
    } else {
      out[j + 2] = CHAR_PAD;
      out[j + 3] = CHAR_PAD;
    }
  }

  // 编码转换
  CopyMemory(out_buffer, out.data(), len * sizeof(TCHAR));

  // 设置结束符
  out_buffer[len] = 0;

  // 释放资源
  // SafeDeleteArray(out);
  return len;
}

// 解码函数
int CWHBase64::Decode(const TCHAR* code, int code_len, TCHAR out_buffer[], int buffer_len) {
  unBuffer buffer;
  buffer.block = 0;

  // 分配内存
  // TCHAR* out = new TCHAR[buffer_len];
  std::pmr::vector<TCHAR> out(buffer_len);

  int j = 0;
  for (int i = 0; i < code_len; i++) {
    int val = 0;
    int m = i % 4;
    TCHAR x = code[i];

    if (x >= 'A' && x <= 'Z')
      val = x - 'A';
    else if (x >= 'a' && x <= 'z')
      val = x - 'a' + 'Z' - 'A' + 1;
    else if (x >= '0' && x <= '9')
      val = x - '0' + ('Z' - 'A' + 1) * 2;
    else if (x == CHAR_63)
      val = 62;
    else if (x == CHAR_64)
      val = 63;

    if (x != CHAR_PAD)
      buffer.block |= val << (3 - m) * 6;
    else
      m--;

    if (m == 3 || x == CHAR_PAD) {
      out[j++] = buffer.bytes[2];
      if (x != CHAR_PAD || m > 1) {
        out[j++] = buffer.bytes[1];
        if (x != CHAR_PAD || m > 2)
          out[j++] = buffer.bytes[0];
      }

      buffer.block = 0;
    }

    if (x == CHAR_PAD)
      break;
    if (j == buffer_len)
      break;
  }

  // 设置结束符
  out[j] = 0;

  // 编码转换
  CopyMemory(out_buffer, out.data(), buffer_len * sizeof(TCHAR));

  // 释放资源
  // SafeDeleteArray(out);
  return j;
}

//////////////////////////////////////////////////////////////////////////////////////////////
