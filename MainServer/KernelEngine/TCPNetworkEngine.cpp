#include "TCPNetworkEngine.h"

#include "TraceService.h"

//////////////////////////////////////////////////////////////////////////////////
// 宏定义

// 系数定义
#define DEAD_QUOTIETY 0 // 死亡系数
#define DANGER_QUOTIETY 1 // 危险系数
#define SAFETY_QUOTIETY 2 // 安全系数

// // 动作定义
// #define ASYNCHRONISM_SEND_DATA 21 // 发送标识
// #define ASYNCHRONISM_SEND_BATCH 22 // 群体发送
// #define ASYNCHRONISM_SHUT_DOWN 23 // 安全关闭
// #define ASYNCHRONISM_ALLOW_BATCH 24 // 允许群发
// #define ASYNCHRONISM_CLOSE_SOCKET 25 // 关闭连接
// #define ASYNCHRONISM_DETECT_SOCKET 26 // 检测连接

// 索引辅助
#define SOCKET_INDEX(dwSocketID) LOWORD(dwSocketID) // 位置索引
#define SOCKET_ROUNDID(dwSocketID) HIWORD(dwSocketID) // 循环索引

//////////////////////////////////////////////////////////////////////////////////
// 结构定义

// 投递数据
struct tagSendDataRequest {
  WORD index; // 连接索引
  WORD round_id; // 循环索引
  WORD main_cmd_id; // 主命令码
  WORD sub_cmd_id; // 子命令码
  WORD data_size; // 数据大小
  // 下面存储真实数据
};

// 群发请求
struct tagBatchSendRequest {
  WORD main_cmd_id; // 主命令码
  WORD sub_cmd_id; // 子命令码
  BYTE batch_mask; // 数据掩码
  WORD data_size; // 数据大小
  // 下面存储真实数据
};

// 允许群发
struct tagAllowBatchSend {
  WORD index; // 连接索引
  WORD round_id; // 循环索引
  BYTE allow_batch; // 允许标志
  BYTE batch_mask; // 数据掩码
};

// 关闭连接
struct tagCloseSocket {
  WORD index; // 连接索引
  WORD round_id; // 循环索引
};

// 安全关闭
struct tagShutDownSocket {
  WORD index; // 连接索引
  WORD round_id; // 循环索引
};

//////////////////////////////////////////////////////////////////////////////////

static const std::unordered_set<int> kClosableErrorSet = {
    asio::error::eof, asio::error::connection_reset,
    // asio::error::connection_aborted,
    asio::error::operation_aborted, asio::error::network_down, asio::error::network_reset, asio::error::network_unreachable,
    asio::error::bad_descriptor, asio::error::not_connected,
    // asio::error::shut_down,
    // asio::error::broken_pipe,
    // asio::error::timed_out,
    // asio::error::address_in_use,
};


//////////////////////////////////////////////////////////////////////////////////
CTCPNetworkItem::CTCPNetworkItem(WORD index_id, WORD round_id, asio::ip::tcp::socket socket, ITCPNetworkItemSink* engine_sink) :
    index_(index_id),
    round_id_(round_id),
    socket_(std::move(socket)),
    engine_sink_(engine_sink),
    strand_(socket_.get_executor()),
    timer_(socket_.get_executor()) {
  std::error_code ec;
  const auto endpoint = socket_.remote_endpoint(ec);
  ipv4_port_ = std::format("{}:{}", endpoint.address().to_v4().to_string(), endpoint.port());
  const auto client_ip = endpoint.address().to_v4().to_uint();

  // 连接属性
  client_ip_ = htonl(client_ip);
  active_time_ = std::time(nullptr);
}

void CTCPNetworkItem::PerformSocketBind() {
  // 在 strand 上启动接收与通知，保证在 shared_ptr 已经存在时调用
  asio::post(strand_, [self = shared_from_this()]() {
    // 通知上层绑定事件（现在 safe）
    if (self->engine_sink_) {
      self->engine_sink_->OnEventSocketBind(self.get());
    }
    // 启动接收状态机（内部 lambda 会捕获 shared_ptr）
    self->PerformRecvData();
    self->PerformSocketDetect();
  });
}

void CTCPNetworkItem::PerformSocketDetect() {
  timer_.expires_after(std::chrono::seconds(10));
  timer_.async_wait([self = shared_from_this()](asio::error_code ec) {
    if (ec || self->is_shutdown_.load(std::memory_order_relaxed)) {
      return;
    }

    if (self->IsAllowSendData()) {
      auto survival_time = self->survival_time_.fetch_sub(1, std::memory_order_relaxed);
      switch (survival_time) {
        case DEAD_QUOTIETY: // 死亡连接
          self->CloseSocket();
          break;
        case DANGER_QUOTIETY: // 危险连接
          self->SendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
          break;
        default:
          break;
      }
    } else {
      self->CloseSocket();
      return;
    }
    self->PerformSocketDetect();
  });
}

// 执行发送队列
void CTCPNetworkItem::PerformSendFromQueue() {
  if (!IsValidSocket()) {
    return;
  }

  asio::async_write(socket_, asio::buffer(send_queue_.front()),
                    asio::bind_executor(strand_, [self = shared_from_this()](const asio::error_code ec, size_t) {
                      if (self->is_shutdown_.load(std::memory_order_relaxed)) {
                        return;
                      }

                      if (ec) {
                        self->PerformHandleError(ec);
                        self->send_queue_.clear();
                      } else {
                        self->send_packet_count_++;
                        self->send_queue_.pop_front();
                        if (!self->send_queue_.empty()) {
                          self->PerformSendFromQueue();
                        }
                      }
                    }));
}

// 发送函数
bool CTCPNetworkItem::SendData(WORD main_cmd_id, WORD sub_cmd_id) {
  // 发送判断
  if (!IsValidSocket() || is_shutdown_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 异步发送数据
#ifdef DEBUG_ENABLED
  CLogger::Debug("@ SEND {} -> CMD: {}-{}", ipv4_port_, main_cmd_id, sub_cmd_id);
#endif

  TCP_Head head{{0, 0, 0}, {main_cmd_id, sub_cmd_id}};
  auto packet = std::vector<uint8_t>();
  packet.reserve(sizeof(TCP_Head));

  const auto* head_ptr = reinterpret_cast<const std::uint8_t*>(&head);
  packet.insert(packet.end(), head_ptr, head_ptr + sizeof(TCP_Head));
  (void) EncryptBuffer(packet.data(), packet.size());
  return SendPacket(std::move(packet));
}

bool CTCPNetworkItem::SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size) {
  if (body == nullptr || body_size == 0) {
    return SendData(main_cmd_id, sub_cmd_id);
  }

  ASSERT(body_size <= SOCKET_TCP_PACKET);
  if (body_size > SOCKET_TCP_PACKET) {
    return false;
  }

  // 发送判断
  if (!IsValidSocket() || is_shutdown_.load(std::memory_order_relaxed)) {
    return false;
  }

#ifdef DEBUG_ENABLED
  CLogger::Debug("SEND {} -> CMD: {}-{} @{}", ipv4_port_, main_cmd_id, sub_cmd_id, body_size);
#endif

  TCP_Head head{{0, 0, 0}, {main_cmd_id, sub_cmd_id}};
  auto packet = std::vector<uint8_t>();
  packet.reserve(sizeof(TCP_Head) + body_size);

  const auto* head_ptr = reinterpret_cast<const std::uint8_t*>(&head);
  packet.insert(packet.end(), head_ptr, head_ptr + sizeof(TCP_Head));

  if (body_size > 0) {
    const auto* body_ptr = reinterpret_cast<const std::uint8_t*>(body);
    packet.insert(packet.end(), body_ptr, body_ptr + body_size);
  }
  (void) EncryptBuffer(packet.data(), packet.size());
  return SendPacket(std::move(packet));
}

bool CTCPNetworkItem::SendPacket(std::vector<uint8_t>&& packet) {
  // 异步发送数据(仅包头)
  asio::post(strand_, [self = shared_from_this(), packet = std::move(packet)]() mutable {
    self->send_queue_.emplace_back(packet);
    self->PerformSendFromQueue();
  });
  return true;
}

// 投递接收
bool CTCPNetworkItem::PerformRecvData() {
  asio::async_read(socket_, asio::buffer(recv_buffer_.data(), sizeof(TCP_Head)),
                   [self = shared_from_this()](const asio::error_code& ec, std::size_t bytes_transferred) {
                     self->PerformRecvHead(ec, bytes_transferred);
                   });
  return true;
}

void CTCPNetworkItem::PerformRecvHead(const asio::error_code& ec, const size_t length) {
  // 接收判断
  if (is_shutdown_.load(std::memory_order_relaxed)) {
    return;
  }

  survival_time_.store(SAFETY_QUOTIETY, std::memory_order_relaxed);
  // 处理数据
  if (PerformHandleError(ec)) {
    return;
  }

  if (length != sizeof(TCP_Head)) {
    const auto msg = std::format("HEAD 长度期望 {} 实际 {}", sizeof(TCP_Head), length);
    TraceException(msg);
    return;
  }

  const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
  if (tcp_head->TCPInfo.cbDataKind != DK_MAPPED && tcp_head->TCPInfo.cbDataKind != 0x05) {
    const auto msg = std::format("数据包版本不匹配: {0:#x}", tcp_head->TCPInfo.cbDataKind);
    TraceException(msg);
    return;
  }

  // 数据判断
  if (tcp_head->TCPInfo.wPacketSize > SOCKET_TCP_BUFFER) {
    const auto msg = std::format("单包长度最大 {} 实际 {}", SOCKET_TCP_BUFFER, tcp_head->TCPInfo.wPacketSize);
    TraceException(msg);
    return;
  }

  uint8_t* body_data = recv_buffer_.data() + sizeof(TCP_Head);
  const size_t body_len = tcp_head->TCPInfo.wPacketSize - sizeof(TCP_Head);
  if (body_len > 0) {
    asio::async_read(socket_, asio::buffer(body_data, body_len), [self = shared_from_this()](const asio::error_code& ec, const size_t length) {
      self->PerformRecvBody(ec, length);
    });
    return;
  }
  (void) CrevasseBuffer(recv_buffer_.data(), sizeof(TCP_Head));
  const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
  CLogger::Debug("RECV {} CMD: {}-{}", ipv4_port_, command.wMainCmdID, command.wSubCmdID);
#endif
  // 消息处理
  if (command.wMainCmdID != MDM_KN_COMMAND) {
    engine_sink_->OnEventSocketRead(this, command, body_data, body_len);
  }
  recv_packet_count_++; // 之前忘记加这个调试了半天🤦‍
  PerformRecvData();
}

void CTCPNetworkItem::PerformRecvBody(const asio::error_code& ec, const size_t length) {
  // 接收判断
  if (is_shutdown_.load(std::memory_order_relaxed)) {
    return;
  }

  survival_time_.store(SAFETY_QUOTIETY, std::memory_order_relaxed);
  // 处理数据
  if (PerformHandleError(ec)) {
    return;
  }

  const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
  (void) CrevasseBuffer(recv_buffer_.data(), tcp_head->TCPInfo.wPacketSize);
  uint8_t* body_data = recv_buffer_.data() + sizeof(TCP_Head);
  const size_t body_len = tcp_head->TCPInfo.wPacketSize - sizeof(TCP_Head);
  if (body_len != length) {
    const auto msg = std::format("BODY 长度期望 {} 实际 {}", body_len, length);
    TraceException(msg);
    return;
  }
  const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
  CLogger::Debug("RECV {} CMD: {}-{} -> {}", ipv4_port_, command.wMainCmdID, command.wSubCmdID, length);
#endif
  // 消息处理
  if (command.wMainCmdID != MDM_KN_COMMAND) {
    engine_sink_->OnEventSocketRead(this, command, body_data, body_len);
  }

  recv_packet_count_++;
  PerformRecvData();
}

bool CTCPNetworkItem::PerformHandleError(const asio::error_code& ec) {
  // 遇到异常时代表连接已经不可用了，直接 close() 就行。再调用 shutdown() 没意义。
  // 因为 TCP 状态机已经乱了，或者对端已经挂了。所以异常情况下，直接 close() 是正确的。
  if (ec == asio::error::eof || ec == asio::error::connection_reset || ec == asio::error::operation_aborted || ec == asio::error::broken_pipe) {
    // 正常关闭、异常关闭、socket.cancel()
    CLogger::Info("IP {} 断开了连接", ipv4_port_);
  } else if (ec) {
    CLogger::Info("IP {} 异常：{}", ipv4_port_, ToSimpleUtf8(ec.message()));
  } else {
    return false;
  }
  if (IsValidSocket()) {
    std::error_code ignored;
    socket_.close(ignored);
  }
  engine_sink_->OnEventSocketShut(this);
  return true;
}

// 统一的优雅关闭
bool CTCPNetworkItem::CloseSocket() {
  asio::post(strand_, [self = shared_from_this()]() {
    if (self->IsValidSocket()) {
      asio::error_code ec;
      self->socket_.close(ec);
      self->timer_.cancel();
    }
  });
  return true;
}

// 设置关闭
bool CTCPNetworkItem::ShutDownSocket() {
  if (is_shutdown_.load(std::memory_order_relaxed)) {
    return false;
  }
  is_shutdown_.store(true, std::memory_order_relaxed);
  SendData(MDM_KN_COMMAND, SUB_KN_SHUT_DOWN_SOCKET);
  return true;
}

// 允许群发
bool CTCPNetworkItem::AllowBatchSend(bool allow_batch, BYTE batch_mask) {
  is_allow_batch_.store(allow_batch, std::memory_order_relaxed);
  batch_mask_.store(batch_mask, std::memory_order_relaxed);
  return true;
}

// 加密数据
WORD CTCPNetworkItem::EncryptBuffer(BYTE packet_data[], WORD packet_size) {
  // 效验参数
  ASSERT(packet_size >= sizeof(TCP_Head) && packet_size <= SOCKET_TCP_BUFFER);

  // 填写信息头
  const auto tcp_head = reinterpret_cast<TCP_Head*>(packet_data);
  tcp_head->TCPInfo.wPacketSize = packet_size;
  tcp_head->TCPInfo.cbDataKind = DK_MAPPED;

  BYTE check_code = 0;
  for (WORD i = sizeof(TCP_Info); i < packet_size; i++) {
    check_code += packet_data[i];
    packet_data[i] = MapSendByte(packet_data[i]);
  }
  tcp_head->TCPInfo.cbCheckCode = ~check_code + 1;

  // 设置变量
  send_packet_count_++;
  return packet_size;
}

// 解密数据
WORD CTCPNetworkItem::CrevasseBuffer(BYTE packet_data[], WORD packet_size) {
  // 效验参数
  const auto tcp_head = reinterpret_cast<TCP_Head*>(packet_data);
  ASSERT(packet_size >= sizeof(TCP_Head) && tcp_head->TCPInfo.wPacketSize == packet_size);

  // 效验码与字节映射
  for (WORD i = sizeof(TCP_Info); i < packet_size; i++) {
    packet_data[i] = MapRecvByte(packet_data[i]);
  }

  return packet_size;
}

void CTCPNetworkItem::TraceException(const std::string& msg) {
  CLogger::Error("Index={} RoundID={} OnRecvCompleted 发生“{}”异常", index_, round_id_, msg);
  // 关闭链接
  CloseSocket();
}

// 随机映射
WORD CTCPNetworkItem::SeedRandMap(WORD wSeed) {
  DWORD dwHold = wSeed;
  return (WORD) ((dwHold = dwHold * 241103L + 2533101L) >> 16);
}

// 映射发送数据
BYTE CTCPNetworkItem::MapSendByte(BYTE const cbData) {
  BYTE cbMap = kSendByteMap[cbData];
  return cbMap;
}

// 映射接收数据
BYTE CTCPNetworkItem::MapRecvByte(BYTE const cbData) {
  BYTE cbMap = kRecvByteMap[cbData];
  return cbMap;
}

//////////////////////////////////////////////////////////////////////////////////
// 接口查询
VOID* CTCPNetworkEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE(ITCPNetworkEngine, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(ITCPNetworkEngine, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CTCPNetworkEngine::StartService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false; // 已经运行中
  }

  // 效验参数
  ASSERT(max_connect_ != 0);
  if (max_connect_ == 0) {
    return false;
  }

  ASSERT(slots_.empty());
  ASSERT(frees_.empty());
  slots_.resize(max_connect_);
  for (int i = static_cast<int>(max_connect_) - 1; i >= 0; --i) {
    frees_.push_back(static_cast<uint16_t>(i)); // 逆序推入，先连入的会从 index 0 开始使用
  }

  io_context_ = std::move(io_context);
  strand_ = asio::make_strand(*io_context_);
  acceptor_.emplace(*io_context_, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), service_port_));

  OnEventTCPNetworkBind.Setup(strand_);
  OnEventTCPNetworkShut.Setup(strand_);
  OnEventTCPNetworkReadInternal.Setup(strand_);

  // 自动随机端口
  if (service_port_ == 0) {
    service_port_ = acceptor_->local_endpoint().port();
    CLogger::Error(TEXT("未指定网络端口，随机分配 {} 端口"), service_port_);
  }

  CLogger::Info(TEXT("服务正在监听 {} 端口"), service_port_);

  // 应答线程
  DoAcceptClient();

  return is_running_.exchange(true, std::memory_order_relaxed) == false;
}

// 停止服务
bool CTCPNetworkEngine::ConcludeService() {
  // 设置变量
  if (!is_running_.exchange(false, std::memory_order_relaxed)) {
    return true;
  }

  // 清除事件
  OnEventTCPNetworkBind.Clear();
  OnEventTCPNetworkShut.Clear();
  OnEventTCPNetworkReadInternal.Clear();

  // 关闭连接
  {
    std::lock_guard lock(mutex_);
    for (auto& session: sessions_ | std::views::values) {
      session->CloseSocket();
    }
    slots_.clear();
    frees_.clear();
  }

  // 重置内核资产生命周期
  acceptor_.reset();
  io_context_.reset();
  return true;
}

// 设置参数
bool CTCPNetworkEngine::SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  ASSERT(wServicePort != 0);
  service_port_ = wServicePort;
  max_connect_ = wMaxConnect;
  return true;
}

// 设置接口
bool CTCPNetworkEngine::SetTCPNetworkEngineEvent(void* object_ptr) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITCPNetworkEngineEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITCPNetworkEngineEvent);

  if (event_ptr == nullptr) {
    return false;
  }

  OnEventTCPNetworkBind += MEMBER_DELEGATE(&ITCPNetworkEngineEvent::OnEventTCPNetworkBind, event_ptr);
  OnEventTCPNetworkShut += MEMBER_DELEGATE(&ITCPNetworkEngineEvent::OnEventTCPNetworkShut, event_ptr);
  OnEventTCPNetworkReadInternal += MEMBER_DELEGATE(&ITCPNetworkEngineEvent::OnEventTCPNetworkReadInternal, event_ptr);
  return true;
}

// 发送函数
bool CTCPNetworkEngine::SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id) {
  // 获取对象
  if (auto&& session = GetNetworkItem(socket_id)) {
    return session->SendData(main_cmd_id, sub_cmd_id);
  }
  return false;
}

// 发送函数
bool CTCPNetworkEngine::SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size) {
  // 校验数据
  ASSERT((data_size + sizeof(TCP_Head)) <= SOCKET_TCP_PACKET);
  if ((data_size + sizeof(TCP_Head)) > SOCKET_TCP_PACKET) {
    return false;
  }

  // 获取对象
  if (auto&& session = GetNetworkItem(socket_id)) {
    return session->SendData(main_cmd_id, sub_cmd_id, data, data_size);
  }
  return false;
}

// 批量发送
bool CTCPNetworkEngine::SendDataBatch(WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size, BYTE batch_mask) {
  // 效验数据
  ASSERT((data_size + sizeof(TCP_Head)) <= SOCKET_TCP_PACKET);
  if ((data_size + sizeof(TCP_Head)) > SOCKET_TCP_PACKET) {
    return false;
  }

  auto post_data = ConvertToBytes(data, data_size);

  std::lock_guard lock(mutex_);
  for (const auto& [session_id, session]: sessions_) {
    if (session->IsAllowBatch() /* && (session->GetBatchMask() & batch_mask) != 0*/) {
      session->SendData(main_cmd_id, sub_cmd_id, data, data_size);
    }
  }
  return true;
}

// 关闭连接
bool CTCPNetworkEngine::CloseSocket(DWORD socket_id) {
  // 获取对象
  auto session = GetNetworkItem(socket_id);
  if (session == nullptr) {
    return false;
  }

  // 关闭连接
  session->CloseSocket();
  return true;
}

// 设置关闭
bool CTCPNetworkEngine::ShutDownSocket(DWORD socket_id) {
  // 获取对象
  auto session = GetNetworkItem(socket_id);
  if (session == nullptr) {
    return false;
  }

  // 安全关闭
  session->ShutDownSocket();
  return true;
}

// 允许群发
bool CTCPNetworkEngine::AllowBatchSend(DWORD socket_id, bool allow_batch, BYTE batch_mask) {
  // 获取对象
  auto&& session = GetNetworkItem(socket_id);
  if (session == nullptr) {
    return false;
  }

  // 设置群发
  session->AllowBatchSend(allow_batch != 0, batch_mask);
  return true;
}

// 绑定事件
bool CTCPNetworkEngine::OnEventSocketBind(CTCPNetworkItem* session) {
  // 效验数据
  ASSERT(session != nullptr);

  // 投递消息
  DWORD dwClientIP = session->GetClientIP();
  DWORD dwSocketID = session->GetSocketID();
  OnEventTCPNetworkBind(dwSocketID, dwClientIP);
  return true;
}

// 关闭事件
bool CTCPNetworkEngine::OnEventSocketShut(CTCPNetworkItem* session) {
  // 效验参数
  ASSERT(session != nullptr);

  // 投递数据
  DWORD dwClientIP = session->GetClientIP();
  DWORD dwSocketID = session->GetSocketID();
  DWORD dwActiveTime = session->GetActiveTime();
  OnEventTCPNetworkShut(dwSocketID, dwClientIP, dwActiveTime);

  // 释放会话对象
  std::lock_guard lock(mutex_);
  return sessions_.erase(dwSocketID);
}

// 读取事件
bool CTCPNetworkEngine::OnEventSocketRead(CTCPNetworkItem* session, TCP_Command command, VOID* data, WORD data_size) {
  // 效验数据
  ASSERT(session != nullptr);

  // 投递消息
  DWORD dwSocketID = session->GetSocketID();
  auto buffer = ConvertToBytes(data, data_size);
  OnEventTCPNetworkReadInternal(dwSocketID, command, buffer);
  return true;
}

// 获取对象
std::shared_ptr<CTCPNetworkItem> CTCPNetworkEngine::GetNetworkItem(WORD wIndex) {
  // 锁定对象
  std::lock_guard lock(mutex_);
  if (const auto itr = sessions_.find(wIndex); itr != sessions_.end()) {
    return itr->second;
  }
  return nullptr;
}

void CTCPNetworkEngine::DoAcceptClient() {
  // 由于 lambda 表达式内的变量均是局部变量无线程安全问题
  acceptor_->async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
    if (!is_running_.load(std::memory_order_relaxed)) {
      return;
    }

    if (ec) {
      CLogger::Warn("连接客户端异常 {}", ToSimpleUtf8(ec.message()));
    } else {
      std::shared_ptr<CTCPNetworkItem> session;
      {
        std::lock_guard lock(mutex_);
        if (!frees_.empty()) {
          // 🚀 一步直达：直接从栈顶拿到空闲的 index
          uint16_t target_index = frees_.back();
          frees_.pop_back();

          // 轮次自增
          auto target_round = slots_[target_index].NewRoundID();

          session = std::make_shared<CTCPNetworkItem>(target_index, target_round, std::move(socket), this);
          slots_[target_index].session = session;
        }
      }

      if (session == nullptr) {
        asio::error_code ignored;
        socket.close(ignored); // 服务器满，安全拒绝
      } else {
        session->PerformSocketBind();
      }
    }
    DoAcceptClient(); // 进入下一个事件循环
  });
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(TCPNetworkEngine);

//////////////////////////////////////////////////////////////////////////////////
