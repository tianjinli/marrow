#include "TCPNetworkEngine.h"

#include "AsyncEventHub.h"
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
// CTCPNetworkItem::CTCPNetworkItem(WORD session_id, WORD round_id, ITCPNetworkItemSink* session_sink) :
//     session_id_(session_id), round_id_(std::max<uint16_t>(1, round_id + 1)), event_handler_(session_sink) {
//   // std::max 后面不加 uint16_t 的话 round_id + 1 的结果类型则为 int 类型
//   // 导致 (0xffff + 1) 等于 0x1'0000 而不是 0，所以最大值为 0x1'0000
//   // 把最大值转成 uint16_t 就变成了 0
// }

CTCPNetworkItem::CTCPNetworkItem(WORD session_id, WORD round_id) : session_id_(session_id), round_id_(std::max<uint16_t>(1, round_id + 1)) {
}

DWORD CTCPNetworkItem::Attach(std::shared_ptr<asio::ip::tcp::socket>&& socket) {
  socket_ = std::move(socket);
  strand_ = std::make_shared<asio::strand<executor_type>>(socket_->get_executor());

  std::error_code ec;
  const auto endpoint = socket_->remote_endpoint(ec);
  ipv4_port_ = fmt::format("{}:{}", endpoint.address().to_v4().to_string(), endpoint.port());
  const auto client_ip = endpoint.address().to_v4().to_uint();

  // 连接属性
  client_ip_ = htonl(client_ip);
  active_time_ = std::time(nullptr);

  // 核心变量
  survival_time_ = SAFETY_QUOTIETY;

  // 状态变量
  is_shutdown_ = false;
  is_allow_batch_ = false;
  batch_mask_ = 0xFF;

  // 计数变量
  send_packet_count_ = 0;
  recv_packet_count_ = 0;

  // // 发送通知
  // event_handler_->OnEventSocketBind(this);
  GlobalEventBus::Get()->Publish<TCPNetworkBindEventTag>(GetIdentifierID(), GetClientIP());

  // 准备接收数据
  PerformRecvData();
  return GetIdentifierID();
}

// 发送函数
bool CTCPNetworkItem::SendData(WORD round_id, WORD main_cmd_id, WORD sub_cmd_id) {
  auto self = shared_from_this();
  // asio::dispatch(*strand_, [self, round_id, main_cmd_id, sub_cmd_id]() {
  // 发送判断
  if (!self->SendVerdict(round_id))
    return false;

  // 使用 vector + asio::buffer，更现代
  auto packet = std::make_shared<std::vector<uint8_t>>(sizeof(TCP_Head));
  auto* tcp_head = reinterpret_cast<TCP_Head*>(packet->data());
  tcp_head->CommandInfo.wMainCmdID = main_cmd_id;
  tcp_head->CommandInfo.wSubCmdID = sub_cmd_id;
  (void) self->EncryptBuffer(packet->data(), packet->size());

#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("SEND {} CMD: {}-{}"), self->ipv4_port_, main_cmd_id, sub_cmd_id);
#endif
  // 捕获数据，生命周期与异步绑定
  auto handler = [self, round_id, packet](std::error_code ec, std::size_t) {
    if (self->round_id_ != round_id)
      return; // ABA 防护
    self->survival_time_ = SAFETY_QUOTIETY;
    if (ec) {
      self->GracefulClose(round_id);
    }
  };
  asio::async_write(*self->socket_, asio::buffer(*packet), asio::bind_executor(*self->strand_, std::move(handler)));
  // });

  return true;
}

/// 可以优化为先发送包头再发送包体
inline bool CTCPNetworkItem::SendData(uint16_t round_id, uint16_t main_cmd_id, uint16_t sub_cmd_id, nf::BufferPtr body) {
  return SendData(round_id, main_cmd_id, sub_cmd_id, body->data(), body->size());
}

bool CTCPNetworkItem::SendData(uint16_t round_id, uint16_t main_cmd_id, uint16_t sub_cmd_id, const void* body, uint16_t body_size) {
  if (body == nullptr || body_size == 0)
    return SendData(round_id, main_cmd_id, sub_cmd_id);

  ASSERT(body_size <= SOCKET_TCP_PACKET);
  if (body_size > SOCKET_TCP_PACKET)
    return false;

  // 构造数据
  const auto packet_size = sizeof(TCP_Head) + body_size;
  auto packet = std::make_shared<std::vector<uint8_t>>(packet_size);
  auto* tcp_head = reinterpret_cast<TCP_Head*>(packet->data());
  tcp_head->CommandInfo.wMainCmdID = main_cmd_id;
  tcp_head->CommandInfo.wSubCmdID = sub_cmd_id;
  // 附加数据
  if (body_size > 0) {
    ASSERT(body != nullptr);
    std::memcpy(tcp_head + 1, body, body_size);
  }

  auto self = shared_from_this();
  // asio::dispatch(*strand_, [self, round_id, main_cmd_id, sub_cmd_id, packet = std::move(packet), body_size]() {
  if (!self->SendVerdict(round_id))
    return false;

  (void) self->EncryptBuffer(packet->data(), packet->size());

#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("SEND {} CMD: {}-{} -> {}"), self->ipv4_port_, main_cmd_id, sub_cmd_id, body_size);
#endif
  const auto handler = [self, round_id, packet](std::error_code ec, std::size_t) {
    if (self->round_id_ != round_id)
      return; // ABA 防护
    self->survival_time_ = SAFETY_QUOTIETY;
    if (ec) {
      self->GracefulClose(round_id);
    }
  };
  asio::async_write(*self->socket_, asio::buffer(*packet), asio::bind_executor(*self->strand_, std::move(handler)));
  // });

  return true;
}

// 投递接收
bool CTCPNetworkItem::PerformRecvData() {
  // 效验变量
  auto self = shared_from_this();
  ASSERT(self->IsValidSocket());
  if (!self->IsValidSocket()) {
    return false; // 避免断言，在关闭后，读循环自然结束
  }
  asio::async_read(
      *self->socket_, asio::buffer(self->recv_buffer_.data(), sizeof(TCP_Head)),
      asio::bind_executor(*self->strand_, std::bind(&CTCPNetworkItem::PerformRecvHead, self, std::placeholders::_1, std::placeholders::_2)));
  return true;
}

void CTCPNetworkItem::PerformRecvHead(const asio::error_code& ec, const size_t length) {
  // // 中断判断
  // if (is_shutdown_)
  //   return;

  survival_time_ = SAFETY_QUOTIETY;
  // 处理数据
  if (PerformHandleError(ec))
    return;

  if (length != sizeof(TCP_Head)) {
    const auto msg = fmt::format("HEAD 长度期望 {} 实际 {}", sizeof(TCP_Head), length);
    TraceException(msg);
    return;
  }

  const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
  if (tcp_head->TCPInfo.cbDataKind != DK_MAPPED && tcp_head->TCPInfo.cbDataKind != 0x05) {
    const auto msg = fmt::format(FMT_STRING("数据包版本不匹配: {0:#x}"), tcp_head->TCPInfo.cbDataKind);
    TraceException(msg);
    return;
  }

  // 数据判断
  if (tcp_head->TCPInfo.wPacketSize > SOCKET_TCP_BUFFER) {
    const auto msg = fmt::format("单包长度最大 {} 实际 {}", SOCKET_TCP_BUFFER, tcp_head->TCPInfo.wPacketSize);
    TraceException(msg);
    return;
  }

  uint8_t* body_data = recv_buffer_.data() + sizeof(TCP_Head);
  const size_t body_len = tcp_head->TCPInfo.wPacketSize - sizeof(TCP_Head);
  if (body_len > 0) {
    auto self = shared_from_this();
    asio::async_read(*socket_, asio::buffer(body_data, body_len),
                     asio::bind_executor(*strand_, std::bind(&CTCPNetworkItem::PerformRecvBody, self, std::placeholders::_1, std::placeholders::_2)));
    return;
  }
  (void) CrevasseBuffer(recv_buffer_.data(), sizeof(TCP_Head));
  const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("RECV {} CMD: {}-{}"), ipv4_port_, command.wMainCmdID, command.wSubCmdID);
#endif
  // 消息处理
  if (command.wMainCmdID != MDM_KN_COMMAND) {
    // event_handler_->OnEventSocketRead(this, command, body_data, body_len);
    GlobalEventBus::Get()->Publish<TCPNetworkReadEventTag>(GetIdentifierID(), command, std::make_shared<std::vector<uint8_t>>());
  }
  recv_packet_count_++; // 之前忘记加这个调试了半天🤦‍

  PerformRecvData();
}

void CTCPNetworkItem::PerformRecvBody(const asio::error_code& ec, const size_t length) {
  // // 中断判断
  // if (is_shutdown_)
  //   return;

  survival_time_ = SAFETY_QUOTIETY;
  // 处理数据
  if (PerformHandleError(ec))
    return;

  const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
  (void) CrevasseBuffer(recv_buffer_.data(), tcp_head->TCPInfo.wPacketSize);
  uint8_t* body_data = recv_buffer_.data() + sizeof(TCP_Head);
  const size_t body_len = tcp_head->TCPInfo.wPacketSize - sizeof(TCP_Head);
  if (body_len != length) {
    const auto msg = fmt::format("BODY 长度期望 {} 实际 {}", body_len, length);
    TraceException(msg);
    return;
  }
  const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("RECV {} CMD: {}-{} -> {}"), ipv4_port_, command.wMainCmdID, command.wSubCmdID, length);
#endif
  // 消息处理
  if (command.wMainCmdID != MDM_KN_COMMAND) {
    // event_handler_->OnEventSocketRead(this, command, body_data, body_len);
    GlobalEventBus::Get()->Publish<TCPNetworkReadEventTag>(GetIdentifierID(), command, ConvertToBytes(body_data, body_len));
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
    socket_->close(ignored);
  }
  GlobalEventBus::Get()->Publish<TCPNetworkShutEventTag>(GetIdentifierID(), GetClientIP(), GetActiveTime());
  if (bool expected = false; free_network_item_called_.compare_exchange_strong(expected, true)) {
    (void) free_network_item_(session_id_, round_id_);
  }
  return true;
}

// 统一的优雅关闭
bool CTCPNetworkItem::GracefulClose(uint16_t round_id) {
  auto self = shared_from_this();
  if (self->round_id_ != round_id) {
    CLogger::Error(TEXT("BUG {}: {}<>{}"), TEXT(__FUNCTION__), self->round_id_, round_id);
    return false;
  }
  if (self->IsValidSocket()) {
    std::error_code ec;
    // 只关闭写方向，发 FIN
    self->socket_->shutdown(asio::ip::tcp::socket::shutdown_send, ec);
    // 不要 cancel 和 close。继续读，等待对端 FIN
  }

  return true;
}

// 设置关闭
bool CTCPNetworkItem::ShutDownSocket(WORD round_id) {
  // const auto strand = strand_;
  // if (strand == nullptr)
  //   return false;

  auto self = shared_from_this();
  // asio::dispatch(*strand, [self, round_id]() {
  if (self->socket_ == nullptr || self->round_id_ != round_id || self->is_shutdown_)
    return false;

  self->SendData(self->round_id_, MDM_KN_COMMAND, SUB_KN_CLOSE_SOCKET);
  self->is_shutdown_ = true;
  // });
  return true;
}

// 允许群发
bool CTCPNetworkItem::AllowBatchSend(WORD round_id, bool allow_batch, BYTE batch_mask) {
  const auto strand = strand_;
  if (strand == nullptr)
    return false;

  auto self = shared_from_this();
  asio::dispatch(*strand, [self, round_id, allow_batch, batch_mask]() {
    if (self->socket_ == nullptr || self->round_id_ != round_id)
      return;

    self->is_allow_batch_ = allow_batch;
    self->batch_mask_ = batch_mask;
  });
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
  CLogger::Error("SocketEngine Index={}，RoundID={}，OnRecvCompleted 发生“{}”异常", session_id_, round_id_, msg);
  // 关闭链接
  GracefulClose(round_id_);
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

// 发送判断
bool CTCPNetworkItem::SendVerdict(WORD round_id) {
  if ((round_id_ != round_id) || is_shutdown_)
    return false;
  if (!IsValidSocket())
    return false;

  return true;
}

//////////////////////////////////////////////////////////////////////////////////
void CTCPNetworkEngine::DoAcceptClient() {
  // 由于 lambda 表达式内的变量均是局部变量无线程安全问题
  auto socket = std::make_shared<asio::ip::tcp::socket>(*io_context_);
  acceptor_->async_accept(*socket, [&, socket](asio::error_code ec) {
    if (!is_running_)
      return;
    if (ec != asio::error::operation_aborted)
      DoAcceptClient(); // 进入下一个事件循环

    if (ec) {
      CLogger::Warn("连接客户端异常 {}", ToSimpleUtf8(ec.message()));
      return;
    }

    if (const auto session = ActiveNetworkItem(socket); session == nullptr) {
      // 如果服务端的 socket 还没有进入实际的收发阶段（即没有挂上 async_read / async_write），那么直接 close() 是完全安全的，不需要再走 shutdown()。
      socket->close(ec);
    }
  });
}

void CTCPNetworkEngine::DoDetectClient() {
  detect_timer_->expires_after(std::chrono::seconds(detect_seconds_));
  detect_timer_->async_wait([this](const std::error_code ec) {
    if (!ec) {
      DetectSocket();
      DoDetectClient(); // 重启定时器
    }
  });
}

//////////////////////////////////////////////////////////////////////////////////
// 接口查询
VOID* CTCPNetworkEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE(ITCPNetworkEngine, Guid, dwQueryVer);
  // QUERYINTERFACE(IAsynchronismEngineSink, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(ITCPNetworkEngine, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CTCPNetworkEngine::InitiateService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // 效验参数
  ASSERT(max_connect_ != 0);
  if (max_connect_ == 0)
    return false;

  io_context_ = std::move(io_context);
  if (acceptor_ == nullptr) {
    acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(*io_context_);
  }
  if (detect_timer_ == nullptr) {
    detect_timer_ = std::make_unique<asio::steady_timer>(*io_context_);
  }
  // 自动随机端口
  WORD wAutoServicePort = 3000;
  if (service_port_ == 0) {
    while (TRUE) {
      if (wAutoServicePort != PORT_LOGON && wAutoServicePort != PORT_CENTER && wAutoServicePort != PORT_MANAGER) {
        if (CreateSocket(wAutoServicePort)) {
          service_port_ = wAutoServicePort;
          // 输出端口
          CLogger::Error(TEXT("未指定网络端口，随机分配 {} 端口"), wAutoServicePort);
          break;
        }
      }
      if (wAutoServicePort >= 9000) {
        CLogger::Error(TEXT("尝试了众多的端口号，监听操作失败"));
        return false;
      }
      wAutoServicePort++;
    }
  } else {
    wAutoServicePort = service_port_;
    if (!CreateSocket(wAutoServicePort)) {
      return false;
    }
  }

  // 异步引擎
  // IUnknownEx* pIUnknownEx = QUERY_ME_INTERFACE(IUnknownEx);
  // if (!asynchronism_engine_.SetAsynchronismSink(pIUnknownEx)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 网页验证
  WebAttestation();

  // // 启动服务
  // if (!asynchronism_engine_.InitiateService(io_context_)) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  // 应答线程
  DoAcceptClient();

  // 检测线程
  DoDetectClient();

  // 设置变量
  is_running_ = true;

  return true;
}

// 停止服务
bool CTCPNetworkEngine::ConcludeService() {
  // 设置变量
  is_running_ = false;

  // 应答线程
  asio::error_code ec;
  if (acceptor_ && acceptor_->is_open()) {
    // 1. 停止接受新连接
    acceptor_->close(ec);
  }

  detect_timer_->cancel(ec);

  // // 异步引擎
  // asynchronism_engine_.ConcludeService();
  // 关闭连接
  std::lock_guard lock(item_locked_);
  for (auto idx: active_list_) {
    const auto& conn = connections_[idx];
    conn->GracefulClose(conn->GetRoundID());
  }

  return true;
}

// 设置参数
bool CTCPNetworkEngine::SetServiceParameter(WORD wServicePort, WORD wMaxConnect, LPCTSTR pszCompilation) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // 设置变量
  ASSERT(wServicePort != 0);
  service_port_ = wServicePort;
  max_connect_ = wMaxConnect;

  return true;
}

// 配置端口
WORD CTCPNetworkEngine::GetServicePort() {
  return service_port_;
}

// 当前端口
WORD CTCPNetworkEngine::GetCurrentPort() {
  return service_port_;
}

// 设置接口
bool CTCPNetworkEngine::SetTCPNetworkEngineEvent(IUnknownEx* sink_any) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // // 查询接口
  // event_handler_ = QUERY_OBJECT_PTR_INTERFACE(sink_any, ITCPNetworkEngineEvent);
  //
  // // 错误判断
  // if (event_handler_ == nullptr) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  return true;
}

// 发送函数
bool CTCPNetworkEngine::SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id) {
  // const tagSendDataRequest send_data_request{
  //     .index = SOCKET_INDEX(socket_id), .round_id = SOCKET_ROUNDID(socket_id), .main_cmd_id = main_cmd_id, .sub_cmd_id = sub_cmd_id};
  // auto post_data = ConvertToBytes(send_data_request);

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_SEND_DATA, std::move(post_data));

  // 获取对象
  if (auto&& session = GetNetworkItem(SOCKET_INDEX(socket_id))) {
    return session->SendData(SOCKET_ROUNDID(socket_id), main_cmd_id, sub_cmd_id);
  }
  return false;
}

bool CTCPNetworkEngine::SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, nf::BufferPtr data) {
  // return SendData(socket_id, main_cmd_id, sub_cmd_id, data->data(), data->size());
  // 获取对象
  if (auto session = GetNetworkItem(SOCKET_INDEX(socket_id))) {
    return session->SendData(SOCKET_ROUNDID(socket_id), main_cmd_id, sub_cmd_id, std::move(data));
  }
  return false;
}

// 发送函数
bool CTCPNetworkEngine::SendData(DWORD socket_id, WORD main_cmd_id, WORD sub_cmd_id, VOID* data, WORD data_size) {
  // 校验数据
  ASSERT((data_size + sizeof(TCP_Head)) <= SOCKET_TCP_PACKET);
  if ((data_size + sizeof(TCP_Head)) > SOCKET_TCP_PACKET)
    return false;

  // const tagSendDataRequest send_data_request{.index = SOCKET_INDEX(socket_id),
  //                                            .round_id = SOCKET_ROUNDID(socket_id),
  //                                            .main_cmd_id = main_cmd_id,
  //                                            .sub_cmd_id = sub_cmd_id,
  //                                            .data_size = data_size};

  // auto post_data = std::make_shared<std::vector<uint8_t>>();
  // // 分配内存（实际 size() 还是 0）
  // post_data->reserve(sizeof(tagSendDataRequest) + data_size);
  // // 构造数据
  // AppendToBytes(post_data, send_data_request);
  //
  // if (data_size > 0) {
  //   AppendToBytes(post_data, data, data_size);
  // }

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_SEND_DATA, std::move(post_data));
  // 获取对象
  if (auto&& session = GetNetworkItem(SOCKET_INDEX(socket_id))) {
    auto post_data = ConvertToBytes(data, data_size);
    return session->SendData(SOCKET_ROUNDID(socket_id), main_cmd_id, sub_cmd_id, std::move(post_data));
  }
  return false;
}

bool CTCPNetworkEngine::SendDataBatch(uint16_t main_cmd_id, uint16_t sub_cmd_id, uint8_t batch_mask, nf::BufferPtr data) {
  // 获取活动项
  std::vector<std::shared_ptr<CTCPNetworkItem>> snapshot;
  {
    std::lock_guard lock(item_locked_);
    snapshot.reserve(active_list_.size());
    for (auto idx: active_list_) {
      snapshot.emplace_back(connections_[idx]);
    }
  }

  // 群发数据
  for (const auto& session: snapshot) {
    // 发生数据
    if (session->IsAllowBatch() && session->GetBatchMask() == batch_mask) {
      session->SendData(session->GetRoundID(), main_cmd_id, sub_cmd_id, data);
    }
  }

  return true;
}

// 批量发送
bool CTCPNetworkEngine::SendDataBatch(WORD main_cmd_id, WORD sub_cmd_id, BYTE batch_mask, VOID* data, WORD data_size) {
  // 效验数据
  ASSERT((data_size + sizeof(TCP_Head)) <= SOCKET_TCP_PACKET);
  if ((data_size + sizeof(TCP_Head)) > SOCKET_TCP_PACKET)
    return false;

  // const tagBatchSendRequest batch_send_request{
  //     .main_cmd_id = main_cmd_id, .sub_cmd_id = sub_cmd_id, .batch_mask = batch_mask, .data_size = data_size};

  // auto post_data = std::make_shared<std::vector<uint8_t>>();
  // post_data->reserve(sizeof(tagBatchSendRequest) + data_size);
  // // 构造数据
  // AppendToBytes(post_data, batch_send_request);
  //
  // if (data_size > 0) {
  //   AppendToBytes(post_data, data, data_size);
  // }

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_SEND_BATCH, std::move(post_data));
  auto post_data = ConvertToBytes(data, data_size);
  return SendDataBatch(main_cmd_id, sub_cmd_id, batch_mask, std::move(post_data));
}

// 关闭连接
bool CTCPNetworkEngine::CloseSocket(DWORD socket_id) {
  // const tagCloseSocket close_socket{.index = SOCKET_INDEX(socket_id), .round_id = SOCKET_ROUNDID(socket_id)};
  // auto post_data = ConvertToBytes(close_socket);

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_CLOSE_SOCKET, std::move(post_data));

  // 获取对象
  auto session = GetNetworkItem(SOCKET_INDEX(socket_id));
  if (session == nullptr)
    return false;

  // 关闭连接
  session->GracefulClose(SOCKET_ROUNDID(socket_id));
  return true;
}

// 设置关闭
bool CTCPNetworkEngine::ShutDownSocket(DWORD socket_id) {
  // const tagShutDownSocket shut_down_socket{.index = SOCKET_INDEX(socket_id), .round_id = SOCKET_ROUNDID(socket_id)};
  // auto post_data = ConvertToBytes(shut_down_socket);

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_SHUT_DOWN, std::move(post_data));

  // 获取对象
  auto session = GetNetworkItem(SOCKET_INDEX(socket_id));
  if (session == nullptr)
    return false;

  // 安全关闭
  session->ShutDownSocket(SOCKET_ROUNDID(socket_id));
  return true;
}

// 允许群发
bool CTCPNetworkEngine::AllowBatchSend(DWORD socket_id, bool allow_batch, BYTE batch_mask) {
  // const tagAllowBatchSend allow_batch_send{
  //     .index = SOCKET_INDEX(socket_id), .round_id = SOCKET_ROUNDID(socket_id), .allow_batch = allow_batch, .batch_mask = batch_mask};
  // auto post_data = ConvertToBytes(allow_batch_send);

  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_ALLOW_BATCH, std::move(post_data));

  // 获取对象
  auto session = GetNetworkItem(SOCKET_INDEX(socket_id));
  if (session == nullptr)
    return false;

  // 设置群发
  session->AllowBatchSend(SOCKET_ROUNDID(socket_id), allow_batch != 0, batch_mask);
  return true;
}
//
// // 异步数据
// bool CTCPNetworkEngine::OnAsynchronismEngineData(WORD identifier, nf::BufferPtr data) {
//   switch (identifier) {
//     case ASYNCHRONISM_SEND_DATA: // 发送请求
//     {
//       // 效验数据
//       const auto request = reinterpret_cast<tagSendDataRequest*>(data->data());
//       ASSERT(data->size() == (sizeof(tagSendDataRequest) + request->data_size));
//
//       // 获取对象
//       auto&& session = GetNetworkItem(request->index);
//       if (session == nullptr)
//         return false;
//
//       // 发送数据
//       const auto actual_data = data->data() + sizeof(tagSendDataRequest);
//       session->SendData(request->round_id, request->main_cmd_id, request->sub_cmd_id, actual_data, request->data_size);
//       return true;
//     }
//     case ASYNCHRONISM_SEND_BATCH: // 群发请求
//     {
//       // 效验数据
//       const auto request = reinterpret_cast<tagBatchSendRequest*>(data->data());
//       ASSERT(data->size() == (sizeof(tagBatchSendRequest) + request->data_size));
//
//       // 获取活动项
//       std::vector<std::shared_ptr<CTCPNetworkItem>> snapshot;
//       {
//         std::lock_guard lock(item_locked_);
//         snapshot.reserve(active_list_.size());
//         for (auto idx: active_list_) {
//           snapshot.emplace_back(connections_[idx]);
//         }
//       }
//
//       // 群发数据
//       const auto actual_data = data->data() + sizeof(tagBatchSendRequest);
//       for (const auto& session: snapshot) {
//         // 发生数据
//         if (session->IsAllowBatch() && session->GetBatchMask() == request->batch_mask) {
//           session->SendData(session->GetRoundID(), request->main_cmd_id, request->sub_cmd_id, actual_data, request->data_size);
//         }
//       }
//
//       return true;
//     }
//     case ASYNCHRONISM_SHUT_DOWN: // 安全关闭
//     {
//       // 效验数据
//       ASSERT(data->size() == sizeof(tagShutDownSocket));
//       const auto shut_down_socket = reinterpret_cast<tagShutDownSocket*>(data->data());
//
//       // 获取对象
//       auto session = GetNetworkItem(shut_down_socket->index);
//       if (session == nullptr)
//         return false;
//
//       // 安全关闭
//       session->ShutDownSocket(shut_down_socket->round_id);
//       return true;
//     }
//     case ASYNCHRONISM_ALLOW_BATCH: // 允许群发
//     {
//       // 效验数据
//       ASSERT(data->size() == sizeof(tagAllowBatchSend));
//       const auto allow_batch_send = reinterpret_cast<tagAllowBatchSend*>(data->data());
//
//       // 获取对象
//       auto session = GetNetworkItem(allow_batch_send->index);
//       if (session == nullptr)
//         return false;
//
//       // 设置群发
//       session->AllowBatchSend(allow_batch_send->round_id, allow_batch_send->allow_batch != 0, allow_batch_send->batch_mask);
//       return true;
//     }
//     case ASYNCHRONISM_CLOSE_SOCKET: // 关闭连接
//     {
//       // 效验数据
//       ASSERT(data->size() == sizeof(tagCloseSocket));
//       const auto close_socket = reinterpret_cast<tagCloseSocket*>(data->data());
//
//       // 获取对象
//       auto session = GetNetworkItem(close_socket->index);
//       if (session == nullptr)
//         return false;
//
//       // 关闭连接
//       session->GracefulClose(close_socket->round_id);
//       return true;
//     }
//     case ASYNCHRONISM_DETECT_SOCKET: // 检测连接
//     {
//       // 获取活动项
//       std::vector<std::shared_ptr<CTCPNetworkItem>> snapshot;
//       {
//         std::lock_guard lock(item_locked_);
//         snapshot.reserve(active_list_.size());
//         for (auto idx: active_list_) {
//           snapshot.emplace_back(connections_[idx]);
//         }
//       }
//
//       // 检测连接
//       const time_t now_time = std::time(nullptr);
//       for (const auto& session: snapshot) {
//         // 有效判断
//         if (!session->IsValidSocket())
//           continue;
//
//         // 连接判断
//         if (session->IsAllowSendData()) {
//           switch (session->survival_time_) {
//             case DEAD_QUOTIETY: // 死亡连接
//             {
//               session->GracefulClose(session->GetRoundID());
//               break;
//             }
//             case DANGER_QUOTIETY: // 危险系数
//             {
//               session->survival_time_--;
//               session->SendData(session->GetRoundID(), MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
//               break;
//             }
//             default: // 默认处理
//             {
//               session->survival_time_--;
//               break;
//             }
//           }
//         } else // 特殊连接
//         {
//           if ((session->GetActiveTime() + 4) <= now_time) {
//             session->GracefulClose(session->GetRoundID());
//             continue;
//           }
//         }
//       }
//
//       return true;
//     }
//   }
//
//   // 效验数据
//   ASSERT(FALSE);
//
//   return false;
// }

// 绑定事件
bool CTCPNetworkEngine::OnEventSocketBind(CTCPNetworkItem* session) {
  // // 效验数据
  // ASSERT(session != nullptr);
  // ASSERT(event_handler_ != nullptr);
  //
  // // 投递消息
  // DWORD dwClientIP = session->GetClientIP();
  // DWORD dwSocketID = session->GetIdentifierID();
  // event_handler_->OnEventTCPNetworkBind(dwSocketID, dwClientIP);
  return true;
}

// 关闭事件
bool CTCPNetworkEngine::OnEventSocketShut(CTCPNetworkItem* session) {
  // // 效验参数
  // ASSERT(session != nullptr);
  // ASSERT(event_handler_ != nullptr);
  //
  // try {
  //   // 投递数据
  //   DWORD dwClientIP = session->GetClientIP();
  //   DWORD dwSocketID = session->GetIdentifierID();
  //   DWORD dwActiveTime = session->GetActiveTime();
  //   event_handler_->OnEventTCPNetworkShut(dwSocketID, dwClientIP, dwActiveTime);
  //
  //   // 释放连接
  //   FreeNetworkItem(session);
  // } catch (...) {
  // }

  return true;
}

// 读取事件
bool CTCPNetworkEngine::OnEventSocketRead(CTCPNetworkItem* session, TCP_Command command, VOID* data, WORD data_size) {
  // // 效验数据
  // ASSERT(session != nullptr);
  // ASSERT(event_handler_ != nullptr);
  //
  // // 投递消息
  // DWORD dwSocketID = session->GetIdentifierID();
  // event_handler_->OnEventTCPNetworkRead(dwSocketID, command, data, data_size);

  return true;
}

// 检测连接
bool CTCPNetworkEngine::DetectSocket() {
  // 投递数据
  // return asynchronism_engine_.PostAsynchronismData(ASYNCHRONISM_DETECT_SOCKET, std::make_shared<std::vector<uint8_t>>());
  // 获取活动项
  std::vector<std::shared_ptr<CTCPNetworkItem>> snapshot;
  {
    std::lock_guard lock(item_locked_);
    snapshot.reserve(active_list_.size());
    for (auto idx: active_list_) {
      snapshot.emplace_back(connections_[idx]);
    }
  }

  // 检测连接
  const time_t now_time = std::time(nullptr);
  for (const auto& session: snapshot) {
    // 有效判断
    if (!session->IsValidSocket())
      continue;

    // 连接判断
    if (session->IsAllowSendData()) {
      switch (session->survival_time_) {
        case DEAD_QUOTIETY: // 死亡连接
        {
          session->GracefulClose(session->GetRoundID());
          break;
        }
        case DANGER_QUOTIETY: // 危险系数
        {
          session->survival_time_--;
          session->SendData(session->GetRoundID(), MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
          break;
        }
        default: // 默认处理
        {
          session->survival_time_--;
          break;
        }
      }
    } else // 特殊连接
    {
      if ((session->GetActiveTime() + 4) <= now_time) {
        session->GracefulClose(session->GetRoundID());
        continue;
      }
    }
  }

  return true;
}

// 网页验证
bool CTCPNetworkEngine::WebAttestation() {
  return true;
}

// 创建连接
bool CTCPNetworkEngine::CreateSocket(WORD wAutoServicePort) {
  asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(), wAutoServicePort);

  try {
    CLogger::Info(TEXT("服务正在监听 {} 端口"), wAutoServicePort);

    acceptor_->open(endpoint.protocol());
    // acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_->bind(endpoint);
    acceptor_->listen();
    return true;
  } catch (std::error_code ec) {
    CLogger::Error("监听 {} 端口失败 → {}", wAutoServicePort, ToSimpleUtf8(ec.message()));
    return false;
  }
}

// 获取对象
std::shared_ptr<CTCPNetworkItem> CTCPNetworkEngine::ActiveNetworkItem(std::shared_ptr<asio::ip::tcp::socket> socket) {
  // 锁定队列
  std::lock_guard lock(item_locked_);

  WORD session_id = 0, round_id = 0;
  if (!free_list_.empty()) {
    std::tie(session_id, round_id) = *free_list_.begin();
    free_list_.erase(session_id);
  } else {
    if (connections_.size() >= max_connect_) {
      return nullptr;
    }
    session_id = static_cast<WORD>(connections_.size());
  }

  auto session = std::make_shared<CTCPNetworkItem>(session_id, round_id);
  session->free_network_item_ = std::bind(&CTCPNetworkEngine::FreeNetworkItem, this, std::placeholders::_1, std::placeholders::_2);
  active_list_.emplace(session_id);
  connections_.emplace(session_id, session);
  // 绑定对象（在这里 std::move 是安全的）
  session->Attach(std::move(socket));
  return session;
}

// 获取对象
std::shared_ptr<CTCPNetworkItem> CTCPNetworkEngine::GetNetworkItem(WORD wIndex) {
  // 锁定对象
  std::lock_guard lock(item_locked_);

  const auto it = connections_.find(wIndex);
  if (it == connections_.end())
    return nullptr;
  return it->second;
}

// 释放连接对象
bool CTCPNetworkEngine::FreeNetworkItem(WORD session_id, WORD round_id) {
  std::lock_guard lock(item_locked_);
  if (const auto it = active_list_.find(session_id); it != active_list_.end()) {
    active_list_.erase(it);
    connections_.erase(session_id); // 直接释放内存
    free_list_.emplace(session_id, round_id);
    return true;
  }
  // 释放失败
  ASSERT(FALSE);
  return false;
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(TCPNetworkEngine);

//////////////////////////////////////////////////////////////////////////////////
