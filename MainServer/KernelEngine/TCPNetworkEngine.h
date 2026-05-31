#pragma once

#include "EventDelegate.h"
#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////
// 枚举定义

// 操作类型
enum enOperationType {
  enOperationType_Send, // 发送类型
  enOperationType_Recv, // 接收类型
};

//////////////////////////////////////////////////////////////////////////////////

// 类说明
class CTCPNetworkItem;
class CTCPNetworkEngine;

//////////////////////////////////////////////////////////////////////////////////
// 接口定义

// 连接对象回调接口
interface ITCPNetworkItemSink {
  // 绑定事件
  virtual bool OnEventSocketBind(CTCPNetworkItem * session) = 0;
  // 关闭事件
  virtual bool OnEventSocketShut(CTCPNetworkItem * session) = 0;
  // 读取事件
  virtual bool OnEventSocketRead(CTCPNetworkItem * session, TCP_Command command, VOID * data, WORD data_size) = 0;
};

//////////////////////////////////////////////////////////////////////////////////

// 连接子项
class CTCPNetworkItem : public std::enable_shared_from_this<CTCPNetworkItem> {
  // 友元声明
  friend class CTCPNetworkEngine;

public:
  // 构造函数
  explicit CTCPNetworkItem(WORD index_id, WORD round_id, asio::ip::tcp::socket socket, ITCPNetworkItemSink* engine_sink);
  // 析够函数
  virtual ~CTCPNetworkItem() = default;

  // 标识函数
  // 获取索引
  inline WORD GetIndex() { return index_; }
  // 获取计数
  inline WORD GetRoundID() { return round_id_; }
  // 获取标识
  inline DWORD GetSocketID() { return MAKELONG(index_, round_id_); }

  // 属性函数
  // 获取地址
  inline DWORD GetClientIP() { return client_ip_; }
  // 激活时间
  inline time_t GetActiveTime() { return active_time_; }
  // 发送包数
  inline DWORD GetSendPacketCount() { return send_packet_count_; }
  // 接收包数
  inline DWORD GetRecvPacketCount() { return recv_packet_count_; }

  // 状态函数
  // 群发允许
  inline bool IsAllowBatch() { return is_allow_batch_; }
  // 发送允许
  inline bool IsAllowSendData() { return recv_packet_count_ > 0L; }
  // 判断连接
  inline bool IsValidSocket() { return socket_.is_open(); }
  // 群发标示
  inline BYTE GetBatchMask() { return batch_mask_; }

  // 管理函数
  void PerformSocketBind();
  // 检测连接
  void PerformSocketDetect();
  // 发送队列
  void PerformSendFromQueue();

  // 控制函数
  // 发送函数
  bool SendData(WORD main_cmd_id, WORD sub_cmd_id);
  // 发送函数
  bool SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size);
  // 发送包
  bool SendPacket(std::vector<uint8_t>&& packet);
  // 接收操作
  bool PerformRecvData();
  // 关闭连接
  bool CloseSocket();

  // 状态管理
  // 设置关闭
  bool ShutDownSocket();
  // 允许群发
  bool AllowBatchSend(bool allow_batch, BYTE batch_mask);

private:
  // 接收包头
  void PerformRecvHead(const asio::error_code& ec, size_t length);
  // 接收包体
  void PerformRecvBody(const asio::error_code& ec, size_t length);
  // 处理异常
  bool PerformHandleError(const asio::error_code& ec);

  // 加密数据
  WORD EncryptBuffer(BYTE packet_data[], WORD packet_size);
  // 解密数据
  WORD CrevasseBuffer(BYTE packet_data[], WORD packet_size);
  void TraceException(const std::string& msg);

  // 随机映射
  inline WORD SeedRandMap(WORD wSeed);
  // 映射数据
  inline BYTE MapSendByte(BYTE cbData);
  // 映射数据
  inline BYTE MapRecvByte(BYTE cbData);

  // 连接属性（Protected Data Members）
protected:
  DWORD client_ip_ = 0; // 连接地址
  time_t active_time_ = 0; // 激活时间

  // 内核变量
  WORD index_ = 0; // 连接索引
  WORD round_id_ = 1; // 循环索引
  std::atomic<WORD> survival_time_ = 0; // 生存时间
  std::string ipv4_port_; // IPv4 地址
  asio::ip::tcp::socket socket_; // 连接句柄
  // asio::io_context::strand strand_;
  asio::strand<asio::any_io_executor> strand_;
  asio::steady_timer timer_;

  // 状态变量（只在 strand 线程修改/读取）
  std::atomic<bool> is_shutdown_ = false; // 关闭标志
  std::atomic<bool> is_allow_batch_ = false; // 接受群发
  std::atomic<BYTE> batch_mask_ = 0xFF; // 群发标示

  // 接收变量
  std::array<uint8_t, SOCKET_TCP_BUFFER> recv_buffer_; // 接收缓冲区

  // 发送队列：保证按顺序、不丢包地发送
  std::deque<std::vector<uint8_t>> send_queue_;

  ITCPNetworkItemSink* engine_sink_ = nullptr;

  // 计数变量
  DWORD send_packet_count_ = 0; // 发送计数
  DWORD recv_packet_count_ = 0; // 接受计数
};

//////////////////////////////////////////////////////////////////////////////////

// 网络引擎
class CTCPNetworkEngine : public ITCPNetworkEngine /*, public IAsynchronismEngineSink*/, public ITCPNetworkItemSink {
  struct SessionSlot {
    std::shared_ptr<CTCPNetworkItem> session;
    std::atomic<uint16_t> round;

    SessionSlot() = default;

    // 容器扩容或 resize 时，编译器会走这个通道，从而完美绕过物理拷贝限制
    SessionSlot(SessionSlot&& other) noexcept {
      session = std::move(other.session);
      // 原子变量不能移动，但我们可以把它里面的标量值普通地 load 出来再 store 进去
      round.store(other.round.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    DWORD NewRoundID() {
      // 原子操作，确保线程安全
      while (true) {
        if (const auto round_id = round.fetch_add(1, std::memory_order_relaxed); round_id != 0) {
          return round_id;
        }
      }
    }
  };

public:
  // 构造函数
  CTCPNetworkEngine() = default;
  // 析构函数
  virtual ~CTCPNetworkEngine() = default;

  // 基础接口
  // 释放对象
  virtual VOID Release() { delete this; }
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer);

  // 服务接口
  // 启动服务
  virtual bool StartService(std::shared_ptr<asio::io_context> io_context);
  // 停止服务
  virtual bool ConcludeService();

  // 信息接口
  // 当前端口
  WORD GetCurrentPort() override { return service_port_; }

  // 配置接口
  // 设置接口
  virtual bool SetTCPNetworkEngineEvent(void* object_ptr);
  // 设置参数
  virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation);

  // 发送接口
  // 发送函数
  bool SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id) override;
  // 发送函数
  bool SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size) override;
  // 批量发送
  bool SendDataBatch(WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size, BYTE batch_mask) override;

  // 控制接口
  // 关闭连接
  virtual bool CloseSocket(DWORD socket_id);
  // 设置关闭
  virtual bool ShutDownSocket(DWORD socket_id);
  // 允许群发
  virtual bool AllowBatchSend(DWORD socket_id, bool bAllowBatch, BYTE batch_mask);

  // 内部通知
  // 绑定事件
  virtual bool OnEventSocketBind(CTCPNetworkItem* session);
  // 关闭事件
  virtual bool OnEventSocketShut(CTCPNetworkItem* session);
  // 读取事件
  virtual bool OnEventSocketRead(CTCPNetworkItem* session, TCP_Command command, VOID* data, WORD data_size);

protected:
  // 获取对象
  std::shared_ptr<CTCPNetworkItem> GetNetworkItem(WORD wIndex);

private:
  // 接受连接
  void DoAcceptClient();

public:
  // 应答事件 (连接 ID, 客户端地址)
  mutable EventDelegate<DWORD, DWORD> OnEventTCPNetworkBind;
  // 关闭事件 (连接 ID, 客户端地址, 在线时长)
  mutable EventDelegate<DWORD, DWORD, DWORD> OnEventTCPNetworkShut;
  // 读取事件 (连接 ID, 命令, 数据)
  mutable EventDelegate<DWORD, TCP_Command, nf::BufferPtr> OnEventTCPNetworkReadInternal;

private:
  // 辅助变量
  std::atomic_bool is_running_ = false; // 服务标志

  // 配置变量
  WORD max_connect_ = 0; // 最大连接数
  WORD service_port_ = 0; // 监听端口

  // 内核变量
  std::shared_ptr<asio::io_context> io_context_;
  std::optional<asio::ip::tcp::acceptor> acceptor_;
  asio::strand<asio::any_io_executor> strand_{asio::make_strand(asio::system_executor())};

  std::vector<SessionSlot> slots_;
  // 🚀 存放当前所有空闲插槽下标的“栈”
  std::vector<uint16_t> frees_;

  // 子项变量
  std::mutex mutex_; // 会话锁定
  std::unordered_map<WORD, std::shared_ptr<CTCPNetworkItem>> sessions_; // 所有会话
};

//////////////////////////////////////////////////////////////////////////////////
