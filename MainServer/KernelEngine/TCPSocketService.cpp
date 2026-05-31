#include "TCPSocketService.h"

#include "TraceService.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CTCPSocketService::~CTCPSocketService() {
  // 停止服务
  CTCPSocketService::ConcludeService();
}

// 接口查询
VOID* CTCPSocketService::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE(ITCPSocketService, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(ITCPSocketService, Guid, dwQueryVer);
  return nullptr;
}

// 启动服务
bool CTCPSocketService::StartService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(io_context != nullptr && !is_running_.load(std::memory_order_relaxed));
  if (io_context == nullptr || is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  io_context_ = std::move(io_context);
  socket_ = asio::ip::tcp::socket(*io_context_);
  strand_ = asio::make_strand(io_context_->get_executor());

  // 初始化事件通知总线的执行序列
  OnEventTCPSocketLink.Setup(strand_);
  OnEventTCPSocketShut.Setup(strand_);
  OnEventTCPSocketRead.Setup(strand_);
  return is_running_.exchange(true, std::memory_order_relaxed) == false;
}

// 停止服务
bool CTCPSocketService::ConcludeService() {
  // 设置变量
  if (!is_running_.exchange(false, std::memory_order_relaxed)) {
    return true;
  }

  // 释放外部绑定的多态事件
  OnEventTCPSocketLink.Clear();
  OnEventTCPSocketShut.Clear();
  OnEventTCPSocketRead.Clear();

  // 关闭连接
  asio::post(strand_, [this]() {
    PerformCloseSocket(SHUT_REASON_INSIDE);
  });

  // 重置内核资产生命周期
  io_context_.reset();
  return true;
}

// 配置函数
bool CTCPSocketService::SetServiceID(WORD service_id) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  service_id_ = service_id;
  return true;
}

// 设置接口
bool CTCPSocketService::SetTCPSocketEvent(void* object_ptr) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITCPSocketEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, ITCPSocketEvent);

  if (event_ptr == nullptr) {
    return false;
  }

  OnEventTCPSocketLink += MEMBER_DELEGATE(&ITCPSocketEvent::OnEventTCPSocketLink, event_ptr);
  OnEventTCPSocketShut += MEMBER_DELEGATE(&ITCPSocketEvent::OnEventTCPSocketShut, event_ptr);
  OnEventTCPSocketRead += MEMBER_DELEGATE(&ITCPSocketEvent::OnEventTCPSocketReadInternal, event_ptr);
  return true;
}

void CTCPSocketService::TraceException(const std::string& msg) {
  CLogger::Error("SocketService OnRecvCompleted 发生“{}”异常", msg);
  // 关闭物理链路
  asio::post(strand_, [this]() {
    PerformCloseSocket(SHUT_REASON_EXCEPTION);
  });
}

// 关闭连接
bool CTCPSocketService::CloseSocket() {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  asio::post(strand_, [this]() {
    PerformCloseSocket(SHUT_REASON_NORMAL);
  });
  return true;
}

// 连接服务器
bool CTCPSocketService::Connect(DWORD server_ip, WORD port) {
  // 状态效验
  ASSERT(is_running_.load(std::memory_order_relaxed));
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 执行异步连接
  asio::post(strand_, [this, server_ip, port]() {
    // if (auto lock = self.lock()) {
    PerformConnect(server_ip, port);
    // }
  });
  return true;
}

// 连接地址
bool CTCPSocketService::Connect(LPCTSTR server_ip, WORD port) {
  auto server_ip_v4 = TranslateAddress(server_ip);
  CLogger::Debug("正在连接 {}:{}", server_ip, port);
  return Connect(server_ip_v4, port);
}

// 发送函数
bool CTCPSocketService::SendData(WORD main_cmd_id, WORD sub_cmd_id) {
  // 状态效验
  ASSERT(is_running_.load(std::memory_order_relaxed));
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 异步发送数据
#ifdef DEBUG_ENABLED
  CLogger::Debug("@ SEND ${} -> CMD: {}-{}", service_id_, main_cmd_id, sub_cmd_id);
#endif

  TCP_Head head{{0, 0, 0}, {main_cmd_id, sub_cmd_id}};
  auto packet = std::vector<uint8_t>();
  packet.reserve(sizeof(TCP_Head));

  const auto* head_ptr = reinterpret_cast<const std::uint8_t*>(&head);
  packet.insert(packet.end(), head_ptr, head_ptr + sizeof(TCP_Head));
  (void) EncryptBuffer(packet.data(), packet.size());

  // 异步发送数据(仅包头)
  asio::post(strand_, [this, packet = std::move(packet)]() mutable {
    send_queue_.emplace_back(packet);
    PerformSendFromQueue();
  });
  return true;
}

// 发送函数
bool CTCPSocketService::SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size) {
  // 状态效验
  ASSERT(is_running_.load(std::memory_order_relaxed));
  if (!is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  if (body_size > SOCKET_TCP_PACKET) {
    return false;
  }

  // 异步发送数据
#ifdef DEBUG_ENABLED
  CLogger::Debug("@ SEND ${} -> CMD: {}-{} @{}", service_id_, main_cmd_id, sub_cmd_id, body_size);
#endif

  TCP_Head head{{0, 0, 0}, {main_cmd_id, sub_cmd_id}};
  auto packet = std::vector<uint8_t>();
  packet.reserve(sizeof(TCP_Head) + body_size);

  const auto* head_ptr = reinterpret_cast<const std::uint8_t*>(&head);
  packet.insert(packet.end(), head_ptr, head_ptr + sizeof(TCP_Head));

  if (body && body_size > 0) {
    const auto* body_ptr = reinterpret_cast<const std::uint8_t*>(body);
    packet.insert(packet.end(), body_ptr, body_ptr + body_size);
  }
  (void) EncryptBuffer(packet.data(), packet.size());

  // 将就地移动拷贝的数据完美打包投递给底层串行流
  asio::post(strand_, [this, packet = std::move(packet)]() mutable {
    send_queue_.emplace_back(packet);
    PerformSendFromQueue();
  });
  return true;
}

// 地址解释
DWORD CTCPSocketService::TranslateAddress(LPCTSTR server_ip_t) {
#ifdef _UNICODE
  char server_ip[MAX_PATH] = {0};
  std::wcstombs(server_ip, server_ip_t, std::size(server_ip));
#else
  const char* server_ip = server_ip_t;
#endif
  std::error_code ec;
  auto ipv4 = asio::ip::make_address_v4(server_ip, ec);
  if (ec) {
    CLogger::Error("地址 {} 解析失败: {}", std::string(server_ip), ToSimpleUtf8(ec.message()));
    return INADDR_NONE;
  }
  return ipv4.to_uint();
#if 0
  // 转化地址
  DWORD server_ip_v4 = 0; // inet_addr(ServerIP);
  struct sockaddr_in ipv4;
  inet_pton(AF_INET, server_ip, &ipv4.sin_addr);
  server_ip_v4 = ipv4.sin_addr.s_addr;
  // 域名解析
  if (server_ip_v4 == INADDR_NONE) {
    // hostent *lpHost = gethostbyname(server_ip);
    // if (lpHost == nullptr)
    //   return INADDR_NONE;
    // server_ip_v4 = ((in_addr *) lpHost->h_addr)->s_addr;

    addrinfo hints = {}, *addrs;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (const int err = getaddrinfo(server_ip, nullptr, &hints, &addrs); err == -1)
      return INADDR_NONE;

    auto sockaddr_in = reinterpret_cast<struct sockaddr_in*>(addrs[0].ai_addr);
    server_ip_v4 = sockaddr_in->sin_addr.s_addr;
  }

  return server_ip_v4;
#endif
}

//////////////////////////////////////////////////////////////////////////////////
// 执行连接
DWORD CTCPSocketService::PerformConnect(DWORD server_ip, WORD port) {
  // 把旧连接干净地关掉
  if (IsValidSocket()) {
    std::error_code ignored;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    socket_->close(ignored);
  }

  // 🔥 就地重新初始化套接字，并完美绑定当前 strand_
  // 物理内存毫无损耗，并且让 socket_ 出生即自带串行锁，后续 async 操作默认安全
  socket_.emplace(strand_);

  std::uint64_t generation = ++generation_;

  // 连接服务器
  const asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(server_ip), port);
  socket_->async_connect(endpoint, asio::bind_executor(strand_, [this, generation, endpoint](const asio::error_code& ec) {
                           // const auto lock = self.lock();
                           // 本模块对象已经被外部 Release 释放了，静默退出
                           // 这个回调属于被历史淘汰的“幽灵连接”，直接丢弃
                           if (generation != generation_) {
                             return;
                           }

                           // 处理物理建连失败
                           if (ec) {
                             // 触发多订阅事件总线通知上层：连接失败
                             OnEventTCPSocketLink(service_id_, ec.value());

                             // 安全清理物理套接字
                             if (IsValidSocket()) {
                               asio::error_code ignored;
                               socket_->close(ignored);
                             }
                             return;
                           }

                           // 触发建立连接成功通知
                           OnEventTCPSocketLink(service_id_, 0);
                           asio::error_code ignored;
                           std::string server_ip_port = std::format("{}:{}", endpoint.address().to_string(ignored), endpoint.port());
                           CLogger::Debug("已与 {} -> #{} 建立连接", server_ip_port, service_id_);

                           // 链式激活第一层固定大小包头抓取状态机，把“时间胶囊”继续无损传递下去
                           PerformRecvHead(generation);
                         }));

  return CONNECT_SUCCESS;
}

void CTCPSocketService::PerformSendFromQueue() {
  if (!IsValidSocket()) {
    return;
  }

  asio::async_write(*socket_, asio::buffer(send_queue_.front()),
                    asio::bind_executor(strand_, [this, gen = generation_](const asio::error_code ec, size_t) {
                      // auto lock = self.lock();
                      if (gen != generation_) {
                        return;
                      }

                      if (ec) {
                        PerformHandleError(ec);
                        send_queue_.clear();
                      } else {
                        send_packet_count_++;
                        send_queue_.pop_front();
                        if (!send_queue_.empty()) {
                          PerformSendFromQueue();
                        }
                      }
                    }));
}

// 接收包头
void CTCPSocketService::PerformRecvHead(std::uint64_t generation) {
  if (generation != generation_ || !IsValidSocket()) {
    return;
  }

  asio::async_read(*socket_, asio::buffer(recv_buffer_.data(), sizeof(TCP_Head)),
                   asio::bind_executor(strand_, [this, generation](const std::error_code ec, size_t length) {
                     //  auto lock = self.lock();
                     if (generation != generation_) {
                       return;
                     }
                     if (PerformHandleError(ec)) {
                       return;
                     }

                     // 处理数据
                     if (length != sizeof(TCP_Head)) {
                       const auto msg = std::format("HEAD 长度期望 {} 实际 {}", sizeof(TCP_Head), length);
                       TraceException(msg);
                       return;
                     }

                     const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
                     if (tcp_head->TCPInfo.cbDataKind != DK_MAPPED && tcp_head->TCPInfo.cbDataKind != 0x05) {
                       const auto msg = std::format("数据包版本不匹配: {:#x}", tcp_head->TCPInfo.cbDataKind);
                       TraceException(msg);
                       return;
                     }

                     // 数据判断
                     if (tcp_head->TCPInfo.wPacketSize > SOCKET_TCP_BUFFER) {
                       const auto msg = std::format("单包长度最大 {} 实际 {}", SOCKET_TCP_BUFFER, tcp_head->TCPInfo.wPacketSize);
                       TraceException(msg);
                       return;
                     }

                     const size_t body_len = tcp_head->TCPInfo.wPacketSize - sizeof(TCP_Head);
                     if (body_len > 0) {
                       // 进入第二阶段：链式读取变长包体
                       PerformRecvBody(generation, body_len);
                       return;
                     }
                     (void) CrevasseBuffer(recv_buffer_.data(), sizeof(TCP_Head));
                     const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
                     CLogger::Debug("@ RECV ${} -> CMD: {}-{}", service_id_, command.wMainCmdID, command.wSubCmdID);
#endif
                     if (command.wMainCmdID == MDM_KN_COMMAND) { // 内核数据
                       if (command.wSubCmdID == SUB_KN_DETECT_SOCKET) { // 心跳回应
                         SendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
                       } else if (command.wSubCmdID == SUB_KN_SHUT_DOWN_SOCKET) {
                         // 关闭链接
                         PerformCloseSocket(SHUT_REASON_NORMAL);
                         return;
                       }
                     } else { // 处理数据
                       OnEventTCPSocketRead(service_id_, command, nf::kEmptyBuffer);
                     }
                     recv_packet_count_++; // 之前忘记加这个调试了半天🤦‍
                     PerformRecvHead(generation);
                   }));
}

void CTCPSocketService::PerformRecvBody(std::uint64_t generation, size_t body_len) {
  if (generation != generation_ || !IsValidSocket()) {
    return;
  }

  uint8_t* body_ptr = recv_buffer_.data() + sizeof(TCP_Head);
  asio::async_read(*socket_, asio::buffer(body_ptr, body_len),
                   asio::bind_executor(strand_, [this, generation, body_len](const asio::error_code ec, size_t length) {
                     //  auto lock = self.lock();
                     if (generation != generation_) {
                       return;
                     }

                     if (PerformHandleError(ec)) {
                       return;
                     }

                     if (body_len != length) {
                       TraceException(std::format("BODY 长度不完整 期望 {} 实际 {}", body_len, length));
                       return;
                     }

                     const auto* tcp_head = reinterpret_cast<const TCP_Head*>(recv_buffer_.data());

                     // 工业级硬解密物理缓冲区
                     (void) CrevasseBuffer(recv_buffer_.data(), tcp_head->TCPInfo.wPacketSize);

                     const TCP_Command command = tcp_head->CommandInfo;
                     const uint8_t* payload_ptr = recv_buffer_.data() + sizeof(TCP_Head);

                     if (command.wMainCmdID == MDM_KN_COMMAND) {
                       if (command.wSubCmdID == SUB_KN_DETECT_SOCKET) {
                         SendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
                       } else if (command.wSubCmdID == SUB_KN_SHUT_DOWN_SOCKET) {
                         PerformCloseSocket(SHUT_REASON_NORMAL);
                         return;
                       }
                     } else {
                       // 完美的深度拷贝，保障多线程订阅总线下内存隔离发布的高安全性
                       auto body_payload = std::make_shared<std::vector<uint8_t>>(payload_ptr, payload_ptr + body_len);
                       OnEventTCPSocketRead(service_id_, command, std::move(body_payload));
                     }

                     recv_packet_count_++;
                     PerformRecvHead(generation);
                   }));
}

bool CTCPSocketService::PerformHandleError(const asio::error_code& ec) {
  if (ec == asio::error::eof || ec == asio::error::operation_aborted) {
    // 远程主动关闭了连接
    CLogger::Info("#{} 终止了服务", service_id_);
  } else if (ec) {
    CLogger::Warn("#{} 通信异常：{}", service_id_, ToSimpleUtf8(ec.message()));
  } else {
    return false;
  }

  PerformCloseSocket(SHUT_REASON_EXCEPTION);
  return true;
}

// 关闭连接
VOID CTCPSocketService::PerformCloseSocket(BYTE shut_reason) {
  // 关闭判断
  if (IsValidSocket()) {
    // 关闭发送
    std::error_code ignored;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    socket_->close(ignored);

    // 斩断过去的代际线
    ++generation_;

    // 激活解散通知委派总线
    OnEventTCPSocketShut(service_id_, shut_reason);
    CLogger::Debug("已与 #{} 失去了连接", service_id_);
  }
}

// 映射发送数据
BYTE CTCPSocketService::MapSendByte(BYTE const cbData) {
  const BYTE cbMap = kSendByteMap[cbData];
  return cbMap;
}

// 映射接收数据
BYTE CTCPSocketService::MapRecvByte(BYTE const cbData) {
  const BYTE cbMap = kRecvByteMap[cbData];
  return cbMap;
}

// 解密数据
WORD CTCPSocketService::CrevasseBuffer(BYTE packet_data[], WORD packet_size) {
  // 效验参数
  const auto tcp_head = reinterpret_cast<TCP_Head*>(packet_data);
  ASSERT(packet_size >= sizeof(TCP_Head));
  ASSERT(tcp_head->TCPInfo.wPacketSize == packet_size);

  for (WORD i = sizeof(TCP_Info); i < packet_size; i++) {
    packet_data[i] = MapRecvByte(packet_data[i]);
  }

  return packet_size;
}

// 加密数据
WORD CTCPSocketService::EncryptBuffer(BYTE packet_data[], WORD packet_size) {
  // 效验参数
  ASSERT(packet_size >= sizeof(TCP_Head) && packet_size <= SOCKET_TCP_BUFFER);

  // 填写信息头
  const auto tcp_head = reinterpret_cast<TCP_Head*>(packet_data);
  tcp_head->TCPInfo.wPacketSize = packet_size;
  tcp_head->TCPInfo.cbDataKind = DK_MAPPED;

  BYTE checkCode = 0;
  for (WORD i = sizeof(TCP_Info); i < packet_size; i++) {
    checkCode += packet_data[i];
    packet_data[i] = MapSendByte(packet_data[i]);
  }
  tcp_head->TCPInfo.cbCheckCode = ~checkCode + 1;

  // 设置变量
  send_packet_count_++;
  return packet_size;
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(TCPSocketService);

//////////////////////////////////////////////////////////////////////////////////
