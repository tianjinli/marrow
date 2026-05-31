#include "WHSha1.h"

/////////////////////////////////////////////////////////////////////////////
// 常量定义
const UINT K[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};

/////////////////////////////////////////////////////////////////////////////

// 编码助手
class CWHShaAide {
  // 函数定义
public:
  // 构造函数
  CWHShaAide();
  // 析构函数
  virtual ~CWHShaAide() = default;

  // 哈希数据
  int HashData(const void* data, UINT data_size, UINT msg_digest[5]);

  // 辅助函数
protected:
  // 重置数据
  void ResetData();
  // 数据处理
  void PadData();
  // 处理数据
  void ProcessDataBlock();
  // 函数定义
  static UINT CircularLeftShift32(int bits, UINT data);

  // 变量定义
  UINT H[5] = {};
  BYTE data_block_[64] = {};
  int data_index_ = 0;
  uint64_t data_size_in_bits_ = 0;
};

/////////////////////////////////////////////////////////////////////////////

// 构造函数
CWHShaAide::CWHShaAide() {
  ResetData();
}

// 重置数据
void CWHShaAide::ResetData() {
  // 设置变量
  H[0] = 0x67452301;
  H[1] = 0xEFCDAB89;
  H[2] = 0x98BADCFE;
  H[3] = 0x10325476;
  H[4] = 0xC3D2E1F0;

  // 设置变量
  data_index_ = 0;
  data_size_in_bits_ = 0;
}

// 数据处理
void CWHShaAide::PadData() {
  data_block_[data_index_++] = 0x80;
  if (data_index_ > 56) {
    while (data_index_ < 64) {
      data_block_[data_index_++] = 0;
    }

    ProcessDataBlock();
  }

  while (data_index_ < 56) {
    data_block_[data_index_++] = 0;
  }

  for (int i = 56; data_index_ < 64; i -= 8) {
    data_block_[data_index_++] = (data_size_in_bits_ >> i) & 0xFF;
  }

  ProcessDataBlock();
}

// 处理数据
void CWHShaAide::ProcessDataBlock() {
  UINT W[80];

  int t;
  UINT A, B, C, D, E, temp;

  for (t = 0; t < 16; t++) {
    W[t] = (data_block_[t * 4] << 24) | (data_block_[t * 4 + 1] << 16) | (data_block_[t * 4 + 2] << 8) | (data_block_[t * 4 + 3]);
  }

  for (; t < 80; t++) {
    W[t] = CircularLeftShift32(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
  }

  A = H[0];
  B = H[1];
  C = H[2];
  D = H[3];
  E = H[4];

  for (t = 0; t < 20; t++) {
    temp = CircularLeftShift32(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
    E = D;
    D = C;
    C = CircularLeftShift32(30, B);
    B = A;
    A = temp;
  }

  for (; t < 40; t++) {
    temp = CircularLeftShift32(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
    E = D;
    D = C;
    C = CircularLeftShift32(30, B);
    B = A;
    A = temp;
  }

  for (; t < 60; t++) {
    temp = CircularLeftShift32(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
    E = D;
    D = C;
    C = CircularLeftShift32(30, B);
    B = A;
    A = temp;
  }

  for (; t < 80; t++) {
    temp = CircularLeftShift32(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
    E = D;
    D = C;
    C = CircularLeftShift32(30, B);
    B = A;
    A = temp;
  }

  H[0] += A;
  H[1] += B;
  H[2] += C;
  H[3] += D;
  H[4] += E;

  data_index_ = 0;
}

// 函数定义
UINT CWHShaAide::CircularLeftShift32(int bits, UINT data) {
  return (data << bits) | (data >> (32 - bits));
}

// 哈希数据
int CWHShaAide::HashData(const void* data, UINT data_size, UINT msg_digest[5]) {
  // 重置数据
  ResetData();

  if (uint64_t(-1) - data_size_in_bits_ < (uint64_t) data_size) {
    return SHA1_DATA_TOO_LONG;
  }

  BYTE* dataByte = (BYTE*) data;

  while (data_size--) {
    data_block_[data_index_++] = *dataByte++;
    data_size_in_bits_ += 8;
    if (data_index_ == 64) {
      ProcessDataBlock();
    }
  }

  PadData();

  for (int i = 0; i < std::size(H); i++) {
    msg_digest[i] = H[i];
  }

  return SHA1_DATA_PUSH_SUCCEED;
}

/////////////////////////////////////////////////////////////////////////////

// 哈希数据
int CWHSha1::HashData(const void* data, UINT data_size, UINT msg_digest[5]) {
  // 重置数据
  CWHShaAide sha_aide;
  return sha_aide.HashData(data, data_size, msg_digest);
}
/////////////////////////////////////////////////////////////////////////////
