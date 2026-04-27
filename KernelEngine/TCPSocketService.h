#pragma once

#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 网络服务
class CTCPSocketService : public ITCPSocketService {
  // 以下声明来自于线程内
#ifndef COPY_FROM_SOCKET_SERVICE_THREAD

  // 控制函数
private:
  // 关闭连接
  VOID PerformCloseSocket(BYTE shut_reason);
  // 连接服务器
  DWORD PerformConnect(DWORD dwServerIP, WORD wPort);
  // 发送函数
  DWORD PerformSendData(WORD main_cmd_id, WORD sub_cmd_id);
  // 发送函数
  DWORD PerformSendData(WORD main_cmd_id, WORD sub_cmd_id, const void* body, WORD body_size);
  // 发送函数
  DWORD PerformSendData(WORD main_cmd_id, WORD sub_cmd_id, const std::vector<uint8_t>& body);
  // 网络读取
  DWORD PerformRecvData();

private:
  // 接收包头
  void PerformRecvHead(const asio::error_code& ec, size_t length);
  // 接收包体
  void PerformRecvBody(const asio::error_code& ec, size_t length);
  // 处理异常
  bool PerformHandleError(const asio::error_code& ec);

  // 辅助函数
private:
  // 解密数据
  WORD CrevasseBuffer(BYTE packet_data[], WORD packet_size);
  // 加密数据
  WORD EncryptBuffer(BYTE packet_data[], WORD wDataSize);

  // 内联函数
private:
  // 随机映射
  inline WORD SeedRandMap(WORD wSeed);
  // 发送映射
  inline BYTE MapSendByte(BYTE cbData);
  // 接收映射
  inline BYTE MapRecvByte(BYTE cbData);
#endif

  // 内核变量
protected:
  bool is_running_ = false; // 服务标志
  WORD service_id_ = 0; // 服务标识

  /**
   * @brief 套接字对象
   * 用 strand_ 保护，避免多线程并发访问
   */
  std::shared_ptr<asio::ip::tcp::socket> socket_;
  ///! io_context 在单个线程 run 时无需 strand 加锁同步(故下行代码可以注释或删除)
  std::unique_ptr<asio::strand<executor_type>> strand_;

  // ITCPSocketEvent* socket_event_ = nullptr; // 事件接口

  // 组件变量
protected:
  int64_t send_packet_count_ = 0; // 发送计数
  int64_t recv_packet_count_ = 0; // 接受计数

  std::string ipv4_port_;
  std::array<uint8_t, SOCKET_TCP_BUFFER> recv_buffer_; // 接收缓冲区

  // 函数定义
public:
  // 构造函数
  CTCPSocketService() = default;
  // 析构函数
  virtual ~CTCPSocketService();

  // 基础接口
public:
  // 释放对象
  virtual VOID Release() {
    delete this;
  }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 服务接口
public:
  // 启动服务
  virtual bool InitiateService(std::shared_ptr<asio::io_context> io_context);
  // 停止服务
  virtual bool ConcludeService();

  // 配置接口
public:
  // 配置函数
  virtual bool SetServiceID(WORD wServiceID);
  // 设置接口
  virtual bool SetTCPSocketEvent(IUnknownEx* sink_any);

  // 功能接口
public:
  void TraceException(const std::string& msg);
  // 判断连接
  bool IsValidSocket() {
    return socket_ != nullptr && socket_->is_open();
  }
  // 关闭连接
  virtual bool CloseSocket();
  // 连接地址
  virtual bool Connect(DWORD server_ip, WORD port);
  // 连接地址
  virtual bool Connect(LPCTSTR server_ip, WORD port);
  // 发送函数
  virtual bool SendData(WORD main_cmd_id, WORD sub_cmd_id);
  // 发送函数
  virtual bool SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size);

  // 辅助函数
protected:
  // 连接消息
  bool OnSocketLink(INT error_code);
  // 关闭消息
  bool OnSocketShut(BYTE shut_reason);
  // 读取消息
  bool OnSocketRead(TCP_Command tcp_cmd, nf::BufferPtr body);
  // 读取消息
  bool OnSocketRead(TCP_Command tcp_cmd, VOID* body_data, WORD data_size);

  // 内部函数
private:
  // 地址解释
  DWORD TranslateAddress(LPCTSTR server_ip_t);
};

//////////////////////////////////////////////////////////////////////////////////
