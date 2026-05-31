#include "WHMD5CheckSum.h"
#include <fstream>

/////////////////////////////////////////////////////////////////////////////////////////////////////
// 填充数据
static uint8_t kPadding[64] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0};

// 获取校验和
std::string CWHMD5Checksum::GetMD5(const std::filesystem::path& file_path) {
  // 打开文件
  std::ifstream input(file_path, std::ios::binary);
  if (!input.is_open()) {
    return ("");
  }

  // 变量定义
  CWHMD5Checksum md5_checksum;
  int len = 0;
  constexpr int buffer_size = 1024;
  BYTE Buffer[buffer_size];

  // 校验文件
  while ((len = input.readsome(reinterpret_cast<char*>(Buffer), buffer_size)) > 0) {
    md5_checksum.Update(Buffer, len);
  }

  // 完成校验
  return md5_checksum.Final();
}

// 获取校验和
std::string CWHMD5Checksum::GetMD5(BYTE* buf, UINT length) {
  // 变量定义
  CWHMD5Checksum md5_checksum;

  // 计算校验和
  md5_checksum.Update(buf, length);

  // 完成校验
  return md5_checksum.Final();
}

//
std::string CWHMD5Checksum::Final() {
  // 变量定义
  BYTE bits[8];
  DWordToByte(bits, count_, 8);

  // 变量定义
  UINT index = (UINT) ((count_[0] >> 3) & 0x3f);
  UINT pad_len = (index < 56) ? (56 - index) : (120 - index);
  Update(kPadding, pad_len);

  // 更新数据
  Update(bits, 8);

  // 变量定义
  const int md5_size = 16;
  BYTE output[md5_size];
  DWordToByte(output, md5_sum_, md5_size);

  // 转换类型
  std::string md5;
  for (int i = 0; i < md5_size; i++) {
    char szBuf[16] = {};
    std::sprintf(szBuf, ("%02x"), output[i]);

    ASSERT(strlen(szBuf) == 2);
    md5 += szBuf;
  }

  // 结果校验
  ASSERT(md5.length() == 32);
  return md5;
}

// 转换数据
VOID CWHMD5Checksum::Transform(BYTE block[64]) {
  // 变量定义
  DWORD a = md5_sum_[0];
  DWORD b = md5_sum_[1];
  DWORD c = md5_sum_[2];
  DWORD d = md5_sum_[3];

  // 变量定义
  DWORD output[16];

  // 数据转换
  ByteToDWord(output, block, 64);

  // 1轮转换
  FF(a, b, c, d, output[0], MD5_S11, MD5_T01);
  FF(d, a, b, c, output[1], MD5_S12, MD5_T02);
  FF(c, d, a, b, output[2], MD5_S13, MD5_T03);
  FF(b, c, d, a, output[3], MD5_S14, MD5_T04);
  FF(a, b, c, d, output[4], MD5_S11, MD5_T05);
  FF(d, a, b, c, output[5], MD5_S12, MD5_T06);
  FF(c, d, a, b, output[6], MD5_S13, MD5_T07);
  FF(b, c, d, a, output[7], MD5_S14, MD5_T08);
  FF(a, b, c, d, output[8], MD5_S11, MD5_T09);
  FF(d, a, b, c, output[9], MD5_S12, MD5_T10);
  FF(c, d, a, b, output[10], MD5_S13, MD5_T11);
  FF(b, c, d, a, output[11], MD5_S14, MD5_T12);
  FF(a, b, c, d, output[12], MD5_S11, MD5_T13);
  FF(d, a, b, c, output[13], MD5_S12, MD5_T14);
  FF(c, d, a, b, output[14], MD5_S13, MD5_T15);
  FF(b, c, d, a, output[15], MD5_S14, MD5_T16);

  // 2轮转换
  GG(a, b, c, d, output[1], MD5_S21, MD5_T17);
  GG(d, a, b, c, output[6], MD5_S22, MD5_T18);
  GG(c, d, a, b, output[11], MD5_S23, MD5_T19);
  GG(b, c, d, a, output[0], MD5_S24, MD5_T20);
  GG(a, b, c, d, output[5], MD5_S21, MD5_T21);
  GG(d, a, b, c, output[10], MD5_S22, MD5_T22);
  GG(c, d, a, b, output[15], MD5_S23, MD5_T23);
  GG(b, c, d, a, output[4], MD5_S24, MD5_T24);
  GG(a, b, c, d, output[9], MD5_S21, MD5_T25);
  GG(d, a, b, c, output[14], MD5_S22, MD5_T26);
  GG(c, d, a, b, output[3], MD5_S23, MD5_T27);
  GG(b, c, d, a, output[8], MD5_S24, MD5_T28);
  GG(a, b, c, d, output[13], MD5_S21, MD5_T29);
  GG(d, a, b, c, output[2], MD5_S22, MD5_T30);
  GG(c, d, a, b, output[7], MD5_S23, MD5_T31);
  GG(b, c, d, a, output[12], MD5_S24, MD5_T32);

  // 3轮转换
  HH(a, b, c, d, output[5], MD5_S31, MD5_T33);
  HH(d, a, b, c, output[8], MD5_S32, MD5_T34);
  HH(c, d, a, b, output[11], MD5_S33, MD5_T35);
  HH(b, c, d, a, output[14], MD5_S34, MD5_T36);
  HH(a, b, c, d, output[1], MD5_S31, MD5_T37);
  HH(d, a, b, c, output[4], MD5_S32, MD5_T38);
  HH(c, d, a, b, output[7], MD5_S33, MD5_T39);
  HH(b, c, d, a, output[10], MD5_S34, MD5_T40);
  HH(a, b, c, d, output[13], MD5_S31, MD5_T41);
  HH(d, a, b, c, output[0], MD5_S32, MD5_T42);
  HH(c, d, a, b, output[3], MD5_S33, MD5_T43);
  HH(b, c, d, a, output[6], MD5_S34, MD5_T44);
  HH(a, b, c, d, output[9], MD5_S31, MD5_T45);
  HH(d, a, b, c, output[12], MD5_S32, MD5_T46);
  HH(c, d, a, b, output[15], MD5_S33, MD5_T47);
  HH(b, c, d, a, output[2], MD5_S34, MD5_T48);

  // 4轮转换
  II(a, b, c, d, output[0], MD5_S41, MD5_T49);
  II(d, a, b, c, output[7], MD5_S42, MD5_T50);
  II(c, d, a, b, output[14], MD5_S43, MD5_T51);
  II(b, c, d, a, output[5], MD5_S44, MD5_T52);
  II(a, b, c, d, output[12], MD5_S41, MD5_T53);
  II(d, a, b, c, output[3], MD5_S42, MD5_T54);
  II(c, d, a, b, output[10], MD5_S43, MD5_T55);
  II(b, c, d, a, output[1], MD5_S44, MD5_T56);
  II(a, b, c, d, output[8], MD5_S41, MD5_T57);
  II(d, a, b, c, output[15], MD5_S42, MD5_T58);
  II(c, d, a, b, output[6], MD5_S43, MD5_T59);
  II(b, c, d, a, output[13], MD5_S44, MD5_T60);
  II(a, b, c, d, output[4], MD5_S41, MD5_T61);
  II(d, a, b, c, output[11], MD5_S42, MD5_T62);
  II(c, d, a, b, output[2], MD5_S43, MD5_T63);
  II(b, c, d, a, output[9], MD5_S44, MD5_T64);

  // 设置校验和
  md5_sum_[0] += a;
  md5_sum_[1] += b;
  md5_sum_[2] += c;
  md5_sum_[3] += d;
}

// 更新数据
VOID CWHMD5Checksum::Update(BYTE* input, ULONG input_len) {
  // 变量定义
  UINT index = count_[0] >> 3 & 0x3F;

  // 更新位数
  if ((count_[0] += input_len << 3) < (input_len << 3)) {
    count_[1]++;
  }

  // 设置变量
  count_[1] += (input_len >> 29);

  // 重复转换
  UINT i = 0;
  UINT part_len = 64 - index;
  if (input_len >= part_len) {
    memcpy(&buffer_[index], input, part_len);
    Transform(buffer_);
    for (i = part_len; i + 63 < input_len; i += 64) {
      Transform(&input[i]);
    }
    index = 0;
  }

  // 拷贝缓冲
  memcpy(&buffer_[index], &input[i], input_len - i);
}

// 类型转换
VOID CWHMD5Checksum::ByteToDWord(DWORD* output, BYTE* input, UINT length) {
  // 参数校验
  ASSERT(length % 4 == 0);

  // 变量定义
  UINT out_index = 0;
  UINT loop_index = 0;

  // 转换拷贝
  for (; loop_index < length; out_index++, loop_index += 4) {
    output[out_index] =
        (ULONG) input[loop_index] | (ULONG) input[loop_index + 1] << 8 | (ULONG) input[loop_index + 2] << 16 | (ULONG) input[loop_index + 3] << 24;
  }
}

// 类型转换
VOID CWHMD5Checksum::DWordToByte(BYTE* output, DWORD* input, UINT length) {
  // 校验变量
  ASSERT(length % 4 == 0);

  // 转换拷贝
  UINT in_index = 0;
  UINT loop_index = 0;
  for (; loop_index < length; in_index++, loop_index += 4) {
    output[loop_index] = (BYTE) (input[in_index] & 0xff);
    output[loop_index + 1] = (BYTE) ((input[in_index] >> 8) & 0xff);
    output[loop_index + 2] = (BYTE) ((input[in_index] >> 16) & 0xff);
    output[loop_index + 3] = (BYTE) ((input[in_index] >> 24) & 0xff);
  }
}

//
DWORD CWHMD5Checksum::RotateLeft(DWORD x, int n) {
  // 参数校验
  ASSERT(sizeof(x) == 4);

  // 返回结果
  return (x << n) | (x >> (32 - n));
}

VOID CWHMD5Checksum::FF(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T) {
  DWORD F = (B & C) | (~B & D);
  A += F + X + T;
  A = RotateLeft(A, S);
  A += B;
}

VOID CWHMD5Checksum::GG(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T) {
  DWORD G = (B & D) | (C & ~D);
  A += G + X + T;
  A = RotateLeft(A, S);
  A += B;
}

VOID CWHMD5Checksum::HH(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T) {
  DWORD H = (B ^ C ^ D);
  A += H + X + T;
  A = RotateLeft(A, S);
  A += B;
}

VOID CWHMD5Checksum::II(DWORD& A, DWORD B, DWORD C, DWORD D, DWORD X, DWORD S, DWORD T) {
  DWORD I = C ^ (B | ~D);
  A += I + X + T;
  A = RotateLeft(A, S);
  A += B;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
