#pragma once

#include "GlobalDefine/Packet.h"

/**
 * @brief 消息编解码
 */
struct SessionCodec {
  /**
   * @brief 数据包编码
   *
   * @param packet 数据包
   * @param packet_size 数据包大小
   */
  static void Encode(uint8_t packet[], const uint16_t packet_size) {
    const auto tcp_head = reinterpret_cast<TCP_Head*>(packet);
    const auto body_data = packet + sizeof(TCP_Head);
    const auto body_data_size = packet_size - sizeof(TCP_Head);
    Encode(tcp_head, body_data, body_data_size);
  }

  /**
   * @brief 数据包编码
   *
   * @param tcp_head 仅包头的数据包
   */
  static void Encode(TCP_Head* const tcp_head) { Encode(tcp_head, nullptr, 0); }

  /**
   * @brief 数据包编码
   *
   * @param tcp_head 包头结构体
   * @param body_data 包体数据数组
   */
  static void Encode(TCP_Head* const tcp_head, std::vector<uint8_t>& body_data) { Encode(tcp_head, body_data.data(), body_data.size()); }

  /**
   * @brief 数据包编码
   *
   * @param tcp_head 包头结构体
   * @param body_data 包体数据
   * @param body_data_size 包体数据大小
   */
  static void Encode(TCP_Head* const tcp_head, uint8_t* const body_data, const uint16_t body_data_size) {
    // 效验参数
    ASSERT(body_data_size <= (SOCKET_TCP_BUFFER - sizeof(TCP_Head)));
    // 填写信息头
    tcp_head->TCPInfo.wPacketSize = sizeof(TCP_Head) + body_data_size;
    tcp_head->TCPInfo.cbDataKind = DK_MAPPED;

    const auto command_data = reinterpret_cast<uint8_t* const>(&tcp_head->CommandInfo);

    uint8_t check_code = 0;
    // 命令码字节映射
    for (uint16_t i = 0; i < sizeof(TCP_Command); i++) {
      check_code += command_data[i];
      command_data[i] = kSendByteMap[command_data[i]];
    }
    if (body_data_size > 0) {
      // 包体字节映射
      for (uint16_t i = 0; i < body_data_size; i++) {
        check_code += body_data[i];
        body_data[i] = kSendByteMap[body_data[i]];
      }
    }
    // 填充校验码
    tcp_head->TCPInfo.cbCheckCode = ~check_code + 1;
  }

  /**
   * @brief 数据包解码
   *
   * @param packet 数据包
   * @param packet_size 数据包大小
   * @return 解码结果
   */
  static bool Decode(uint8_t packet[], const uint16_t packet_size) {
    const auto tcp_head = reinterpret_cast<TCP_Head*>(packet);
    const auto body_data = packet + sizeof(TCP_Head);
    const auto body_data_size = packet_size - sizeof(TCP_Head);
    return Decode(tcp_head, body_data, body_data_size);
  }

  /**
   * @brief 数据解码
   *
   * @param tcp_head 包头结构体
   * @return 解码结果
   */
  static bool Decode(TCP_Head* const tcp_head) { return Decode(tcp_head, nullptr, 0); }

  /**
   * @brief 数据解码
   *
   * @param tcp_head 包头结构体
   * @param body_data 包体数据数组
   * @return 解码结果
   */
  static bool Decode(TCP_Head* const tcp_head, std::vector<uint8_t>& body_data) { return Decode(tcp_head, body_data.data(), body_data.size()); }

  /**
   * @brief 数据解码
   *
   * @param tcp_head 包头结构体
   * @param body_data 包体数据
   * @param body_data_size 包体数据大小
   * @return 解码结果
   */
  static bool Decode(TCP_Head* const tcp_head, uint8_t* const body_data, const uint16_t body_data_size) {
    // 效验参数
    ASSERT(tcp_head->TCPInfo.wPacketSize == sizeof(TCP_Head) + body_data_size);

    const auto command_data = reinterpret_cast<uint8_t* const>(&tcp_head->CommandInfo);

    uint8_t check_code = 0;
    // 命令码字节映射
    for (uint16_t i = 0; i < sizeof(TCP_Command); i++) {
      command_data[i] = kRecvByteMap[command_data[i]];
      check_code += command_data[i];
    }
    if (body_data_size > 0) {
      // 包体字节映射
      for (uint16_t i = 0; i < body_data_size; i++) {
        body_data[i] = kRecvByteMap[body_data[i]];
        check_code += body_data[i];
      }
    }
    // 比较校验码
    return tcp_head->TCPInfo.cbCheckCode == (uint8_t)(~check_code + 1);
  }
};
