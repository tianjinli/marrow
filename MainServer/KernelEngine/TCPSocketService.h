#pragma once

#include "EventDelegate.h"
#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 网络服务
class CTCPSocketService : public ITCPSocketService {
public:
  // 构造函数
  CTCPSocketService() = default;
  // 析构函数
  virtual ~CTCPSocketService();

  // 基础接口
  // 释放对象
  VOID Release() override { delete this; }
  // 接口查询
  VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer) override;

  // 服务接口
  // 启动服务
  bool StartService(std::shared_ptr<asio::io_context> io_context) override;
  // 停止服务
  bool ConcludeService() override;

  // 配置接口
  // 配置函数
  bool SetServiceID(WORD service_id) override;
  // 设置接口
  bool SetTCPSocketEvent(void* object_ptr) override;

  // 功能接口
  void TraceException(const std::string& msg);
  // 判断连接
  bool IsValidSocket() const { return socket_.has_value() && socket_->is_open(); }
  // 关闭连接
  bool CloseSocket() override;
  // 连接地址
  bool Connect(DWORD server_ip, WORD port) override;
  // 连接地址
  bool Connect(LPCTSTR server_ip, WORD port) override;
  // 发送函数
  bool SendData(WORD main_cmd_id, WORD sub_cmd_id) override;
  // 发送函数
  bool SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size) override;

private:
  // 以下声明来自于线程内
#ifndef COPY_FROM_SOCKET_SERVICE_THREAD

  // 控制函数
  // 连接服务器
  DWORD PerformConnect(DWORD server_ip, WORD port);
  // 发送函数
  void PerformSendFromQueue();

  // 接收包头
  void PerformRecvHead(std::uint64_t generation);
  // 接收包体
  void PerformRecvBody(std::uint64_t generation, size_t length);
  // 处理异常
  bool PerformHandleError(const asio::error_code& ec);
  // 关闭连接
  void PerformCloseSocket(BYTE shut_reason);

  // 解密数据
  WORD CrevasseBuffer(BYTE packet_data[], WORD packet_size);
  // 加密数据
  WORD EncryptBuffer(BYTE packet_data[], WORD wDataSize);

  // 发送映射
  inline BYTE MapSendByte(BYTE cbData);
  // 接收映射
  inline BYTE MapRecvByte(BYTE cbData);
#endif

  // 地址解释
  DWORD TranslateAddress(LPCTSTR server_ip_t);

public:
  // 连接事件
  mutable EventDelegate<WORD, INT> OnEventTCPSocketLink;
  // 关闭事件
  mutable EventDelegate<WORD, BYTE> OnEventTCPSocketShut;
  // 读取事件
  mutable EventDelegate<WORD, TCP_Command, nf::BufferPtr> OnEventTCPSocketRead;

private:
  // 内核变量
  std::atomic<bool> is_running_{false}; // 服务标志
  WORD service_id_ = 0; // 服务 ID

  // io_context 对象
  std::shared_ptr<asio::io_context> io_context_;

  /**
   * @brief 套接字对象
   * 用 strand_ 保护，避免多线程并发访问
   */
  std::optional<asio::ip::tcp::socket> socket_;
  ///! io_context 在单个线程 run 时无需 strand 加锁同步(故下行代码可以注释或删除)
  /// 必须初始化否则报 bad executor 错误
  asio::strand<asio::any_io_executor> strand_{asio::make_strand(asio::system_executor())};

  // 🔥 新增：连接代际，用来区分第几次 Connect
  std::uint64_t generation_ = 0;

  // 发送队列
  std::deque<std::vector<uint8_t>> send_queue_;

  // 组件变量
  int64_t send_packet_count_ = 0; // 发送计数
  int64_t recv_packet_count_ = 0; // 接受计数

  std::array<uint8_t, SOCKET_TCP_BUFFER> recv_buffer_{}; // 接收缓冲区
};

//////////////////////////////////////////////////////////////////////////////////
