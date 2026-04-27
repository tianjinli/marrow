#pragma once

// #include "AsynchronismEngine.h"
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

  // 连接属性
protected:
  DWORD client_ip_ = 0; // 连接地址
  time_t active_time_ = 0; // 激活时间

  // 内核变量
protected:
  WORD session_id_ = 0; // 连接索引
  WORD round_id_ = 1; // 循环索引
  WORD survival_time_ = 0; // 生存时间
  std::string ipv4_port_; // IPv4 地址
  std::shared_ptr<asio::ip::tcp::socket> socket_; // 连接句柄
  // asio::io_context::strand strand_;
  std::shared_ptr<asio::strand<executor_type>> strand_;
  // ITCPNetworkItemSink* event_handler_ = nullptr; // 为兼容旧工程保留裸指针（建议迁移为 weak_ptr）
  std::function<bool(uint16_t, uint16_t)> free_network_item_;
  std::atomic_bool free_network_item_called_ = false;

  // 状态变量
protected:
  // 状态变量（只在 strand 线程修改/读取）
  bool is_shutdown_ = false; // 关闭标志
  bool is_allow_batch_ = false; // 接受群发
  BYTE batch_mask_ = 0xFF; // 群发标示

  // 接收变量
protected:
  std::array<uint8_t, SOCKET_TCP_BUFFER> recv_buffer_; // 接收缓冲区

  // 发送队列：保证按顺序、不丢包地发送
  // std::deque<nf::BufferPtr> send_queue_;

  // 计数变量
protected:
  DWORD send_packet_count_ = 0; // 发送计数
  DWORD recv_packet_count_ = 0; // 接受计数

  // 函数定义
public:
  // 构造函数
  explicit CTCPNetworkItem(WORD session_id, WORD round_id);
  // 析够函数
  virtual ~CTCPNetworkItem() = default;

  // 标识函数
public:
  // 获取索引
  inline WORD GetIndex() {
    return session_id_;
  }
  // 获取计数
  inline WORD GetRoundID() {
    return round_id_;
  }
  // 获取标识
  inline DWORD GetIdentifierID() {
    return MAKELONG(session_id_, round_id_);
  }

  // 属性函数
public:
  // 获取地址
  inline DWORD GetClientIP() {
    return client_ip_;
  }
  // 激活时间
  inline time_t GetActiveTime() {
    return active_time_;
  }
  // 发送包数
  inline DWORD GetSendPacketCount() {
    return send_packet_count_;
  }
  // 接收包数
  inline DWORD GetRecvPacketCount() {
    return recv_packet_count_;
  }

  // 状态函数
public:
  // 群发允许
  inline bool IsAllowBatch() {
    return is_allow_batch_;
  }
  // 发送允许
  inline bool IsAllowSendData() {
    return recv_packet_count_ > 0L;
  }
  // 判断连接
  inline bool IsValidSocket() {
    return socket_ != nullptr && socket_->is_open();
  }
  // 群发标示
  inline BYTE GetBatchMask() {
    return batch_mask_;
  }

  // 管理函数
public:
  // 绑定对象
  DWORD Attach(std::shared_ptr<asio::ip::tcp::socket>&& socket);

  // 控制函数
public:
  // 发送函数
  bool SendData(WORD round_id, WORD main_cmd_id, WORD sub_cmd_id);
  // 发送函数
  bool SendData(WORD round_id, WORD main_cmd_id, WORD sub_cmd_id, nf::BufferPtr body);
  // 发送函数
  bool SendData(WORD round_id, WORD main_cmd_id, WORD sub_cmd_id, const VOID* body, WORD body_size);
  // 接收操作
  bool PerformRecvData();
  // 优雅关闭（替换之前的 CloseSocket）
  bool GracefulClose(uint16_t round_id);

private:
  // 接收包头
  void PerformRecvHead(const asio::error_code& ec, size_t length);
  // 接收包体
  void PerformRecvBody(const asio::error_code& ec, size_t length);
  // 处理异常
  bool PerformHandleError(const asio::error_code& ec);

  // 状态管理
public:
  // 设置关闭
  bool ShutDownSocket(WORD round_id);
  // 允许群发
  bool AllowBatchSend(WORD round_id, bool allow_batch, BYTE batch_mask);

  // 加密函数
private:
  // 加密数据
  WORD EncryptBuffer(BYTE packet_data[], WORD packet_size);
  // 解密数据
  WORD CrevasseBuffer(BYTE packet_data[], WORD packet_size);
  void TraceException(const std::string& msg);

  // 内联函数
private:
  // 随机映射
  inline WORD SeedRandMap(WORD wSeed);
  // 映射数据
  inline BYTE MapSendByte(BYTE cbData);
  // 映射数据
  inline BYTE MapRecvByte(BYTE cbData);

  // 辅助函数
private:
  // 发送判断
  inline bool SendVerdict(WORD round_id);
};

//////////////////////////////////////////////////////////////////////////////////

// 网络引擎
class CTCPNetworkEngine : public ITCPNetworkEngine /*, public IAsynchronismEngineSink*/, public ITCPNetworkItemSink {
private:
  // 接受连接
  void DoAcceptClient();
  // 检测连接
  void DoDetectClient();

  // 辅助变量
protected:
  std::atomic_bool is_running_ = false; // 服务标志
  BYTE send_buffer_[MAX_ASYNCHRONISM_DATA] = {}; // 临时对象

  // 配置变量
protected:
  WORD max_connect_ = 0; // 最大连接
  WORD service_port_ = 0; // 监听端口
  DWORD detect_seconds_ = 10; // 检测时间

  // 内核变量
protected:
  std::shared_ptr<asio::io_context> io_context_;
  std::unique_ptr<asio::ip::tcp::acceptor> acceptor_;
  // ITCPNetworkEngineEvent* event_handler_ = nullptr; // 事件接口

  // 定时检测（原检测线程）
  std::unique_ptr<asio::steady_timer> detect_timer_;

  // 子项变量
protected:
  std::mutex item_locked_; // 子项锁定

  // 改成
  std::unordered_map<WORD, std::shared_ptr<CTCPNetworkItem>> connections_; // 所有连接
  std::unordered_map<WORD, WORD> free_list_; // 空闲索引(连接 ID vs 循环 ID)
  std::unordered_set<WORD> active_list_; // 活动索引

  // 组件变量
protected:
  // CAsynchronismEngine asynchronism_engine_; // 异步对象

  // 函数定义
public:
  // 构造函数
  CTCPNetworkEngine() = default;
  // 析构函数
  virtual ~CTCPNetworkEngine() = default;

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

  // 信息接口
public:
  // 配置端口
  virtual WORD GetServicePort();
  // 当前端口
  virtual WORD GetCurrentPort();

  // 配置接口
public:
  // 设置接口
  virtual bool SetTCPNetworkEngineEvent(IUnknownEx* sink_any);
  // 设置参数
  virtual bool SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation);

  // 发送接口
public:
  // 发送函数
  bool SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id) override;
  // 发送函数
  virtual bool SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, nf::BufferPtr data);
  // 发送函数
  bool SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size) override;
  // 批量发送
  virtual bool SendDataBatch(WORD main_cmd_id, WORD sub_cmd_id, BYTE batch_mask, nf::BufferPtr data);
  // 批量发送
  bool SendDataBatch(WORD main_cmd_id, WORD sub_cmd_id, BYTE batch_mask, VOID* data, WORD data_size) override;

  // 控制接口
public:
  // 关闭连接
  virtual bool CloseSocket(DWORD socket_id);
  // 设置关闭
  virtual bool ShutDownSocket(DWORD socket_id);
  // 允许群发
  virtual bool AllowBatchSend(DWORD socket_id, bool bAllowBatch, BYTE batch_mask);

  // 异步接口
public:
  // 启动事件
  virtual bool OnAsynchronismEngineStart() {
    return true;
  }
  // 停止事件
  virtual bool OnAsynchronismEngineConclude() {
    return true;
  }
  // // 异步数据
  // bool OnAsynchronismEngineData(WORD identifier, nf::BufferPtr data);
  // // 异步数据
  // virtual bool OnAsynchronismEngineData(WORD identifier, VOID* data, WORD data_size);

  // 内部通知
public:
  // 绑定事件
  virtual bool OnEventSocketBind(CTCPNetworkItem* session);
  // 关闭事件
  virtual bool OnEventSocketShut(CTCPNetworkItem* session);
  // 读取事件
  virtual bool OnEventSocketRead(CTCPNetworkItem* session, TCP_Command command, VOID* data, WORD data_size);

  // 辅助函数
private:
  // 检测连接
  bool DetectSocket();
  // 网页验证
  bool WebAttestation();
  // 创建连接
  bool CreateSocket(WORD wAutoServicePort);

  // 对象管理
protected:
  // 激活空闲对象
  std::shared_ptr<CTCPNetworkItem> ActiveNetworkItem(std::shared_ptr<asio::ip::tcp::socket> socket);
  // 获取对象
  std::shared_ptr<CTCPNetworkItem> GetNetworkItem(WORD wIndex);
  // 释放连接对象
  bool FreeNetworkItem(WORD session_id, WORD round_id);
};

//////////////////////////////////////////////////////////////////////////////////
