#include "TCPSocketService.h"

#include "AsyncEventHub.h"
#include "TraceService.h"

//////////////////////////////////////////////////////////////////////////////////
// 连接请求
struct tagConnectRequest {
  WORD wPort; // 连接端口
  DWORD dwServerIP; // 连接地址
};

// 发送命令请求
struct tagSendCmdRequest {
  WORD wMainCmdID; // 主命令码
  WORD wSubCmdID; // 子命令码
};

// 发送数据请求
struct tagSendDataRequest {
  WORD wMainCmdID; // 主命令码
  WORD wSubCmdID; // 子命令码
  WORD wDataSize; // 数据大小
  BYTE cbSendBuffer[SOCKET_TCP_PACKET]; // 发送缓冲
};

//////////////////////////////////////////////////////////////////////////////////
// 执行连接
DWORD CTCPSocketService::PerformConnect(DWORD server_ip, WORD port) {
  // 效验参数
  ASSERT(!socket_->is_open());

  // 效验状态
  if (socket_->is_open())
    return CONNECT_EXCEPTION;

  // auto self = shared_from_this();
  auto handler = [this](const std::error_code ec) {
    // 事件通知
    OnSocketLink(ec.value());
    if (!ec) {
      PerformRecvData(); // 开始接收数据
    } else {
      std::error_code ignored;
      socket_->close(ignored);
    }
  };
  // 连接服务器
  const asio::ip::tcp::endpoint endpoint(asio::ip::address_v4(server_ip), port);
  socket_->async_connect(endpoint, asio::bind_executor(*strand_, std::move(handler)));

  return CONNECT_SUCCESS;
}

// 发送函数
DWORD CTCPSocketService::PerformSendData(WORD main_cmd_id, WORD sub_cmd_id) {
  // 效验状态
  if (!IsValidSocket())
    return 0;

  // 变量定义
  // tcp_head 不可以再指向其他地址但是可以修改它所指向的内容
  const auto tcp_head = std::make_shared<TCP_Head>();

  // 设置变量
  tcp_head->CommandInfo.wSubCmdID = sub_cmd_id;
  tcp_head->CommandInfo.wMainCmdID = main_cmd_id;

  // 加密数据(消息头不加密)
  const auto packet = reinterpret_cast<uint8_t*>(tcp_head.get());
  (void) EncryptBuffer(packet, sizeof(TCP_Head));
  // 异步发送数据
#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("@ SEND {} CMD: {}-{}"), ipv4_port_, main_cmd_id, sub_cmd_id);
#endif
  auto handler = [this, _ = std::move(tcp_head)](std::error_code ec, std::size_t) {
    if (ec) {
      // 关闭连接
      PerformCloseSocket(SHUT_REASON_EXCEPTION);
    }
  };
  asio::async_write(*socket_, asio::buffer(packet, sizeof(TCP_Head)), asio::bind_executor(*strand_, std::move(handler)));
  return sizeof(TCP_Head);
}

// 发送函数
///! 可以优化为先发送包头再发送包体
DWORD CTCPSocketService::PerformSendData(WORD main_cmd_id, WORD sub_cmd_id, const void* body, WORD body_size) {
  if (body == nullptr || body_size == 0)
    return PerformSendData(main_cmd_id, sub_cmd_id);
  // 效验状态
  if (!IsValidSocket())
    return 0;

  // 效验大小
  ASSERT(body_size <= SOCKET_TCP_PACKET);
  if (body_size > SOCKET_TCP_PACKET)
    return 0;

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

  // 加密数据
  (void) EncryptBuffer(packet->data(), packet->size());
  // 异步发送数据
#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("@ SEND {} CMD: {}-{} -> {}"), ipv4_port_, main_cmd_id, sub_cmd_id, body_size);
#endif
  auto handler = [&, packet](std::error_code ec, std::size_t) {
    if (ec) {
      // 关闭连接
      PerformCloseSocket(SHUT_REASON_EXCEPTION);
    }
  };
  asio::async_write(*socket_, asio::buffer(*packet), asio::bind_executor(*strand_, std::move(handler)));
  return packet->size();
}

DWORD CTCPSocketService::PerformSendData(WORD main_cmd_id, WORD sub_cmd_id, const std::vector<uint8_t>& body) {
  return PerformSendData(main_cmd_id, sub_cmd_id, body.data(), body.size());
}

// 网络读取
DWORD CTCPSocketService::PerformRecvData() {
  // 效验变量
  ASSERT(IsValidSocket());

  asio::async_read(*socket_, asio::buffer(recv_buffer_.data(), sizeof(TCP_Head)),
                   asio::bind_executor(*strand_, std::bind(&CTCPSocketService::PerformRecvHead, this, std::placeholders::_1, std::placeholders::_2)));
  return true;
}

void CTCPSocketService::PerformRecvHead(const asio::error_code& ec, size_t length) {
  if (PerformHandleError(ec))
    return;

  // 处理数据
  if (length != sizeof(TCP_Head)) {
    const auto msg = fmt::format("HEAD 长度期望 {} 实际 {}", sizeof(TCP_Head), length);
    TraceException(msg);
    return;
  }

  const auto tcp_head = reinterpret_cast<TCP_Head*>(recv_buffer_.data());
  if (tcp_head->TCPInfo.cbDataKind != DK_MAPPED && tcp_head->TCPInfo.cbDataKind != 0x05) {
    const auto msg = fmt::format(FMT_STRING("数据包版本不匹配: {:#x}"), tcp_head->TCPInfo.cbDataKind);
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
    asio::async_read(
        *socket_, asio::buffer(body_data, body_len),
        asio::bind_executor(*strand_, std::bind(&CTCPSocketService::PerformRecvBody, this, std::placeholders::_1, std::placeholders::_2)));
    return;
  }
  (void) CrevasseBuffer(recv_buffer_.data(), sizeof(TCP_Head));
  const TCP_Command command = tcp_head->CommandInfo;
#ifdef DEBUG_ENABLED
  CLogger::Debug(FMT_STRING("@ RECV {} CMD: {}-{}"), ipv4_port_, command.wMainCmdID, command.wSubCmdID);
#endif
  if (command.wMainCmdID == MDM_KN_COMMAND) { // 内核数据
    if (command.wSubCmdID == SUB_KN_DETECT_SOCKET) { // 网络检测
      PerformSendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
    } else if (command.wSubCmdID == SUB_KN_CLOSE_SOCKET) {
      // 关闭链接
      PerformCloseSocket(SHUT_REASON_NORMAL);
      return;
    }
  } else { // 处理数据
    OnSocketRead(command, body_data, body_len);
  }
  recv_packet_count_++; // 之前忘记加这个调试了半天🤦‍
  PerformRecvData();
}

void CTCPSocketService::PerformRecvBody(const asio::error_code& ec, size_t length) {
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
  CLogger::Debug(FMT_STRING("@ RECV {} CMD: {}-{} -> {}"), ipv4_port_, command.wMainCmdID, command.wSubCmdID, length);
#endif
  if (command.wMainCmdID == MDM_KN_COMMAND) { // 内核数据
    if (command.wSubCmdID == SUB_KN_DETECT_SOCKET) { // 网络检测
      PerformSendData(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
    } else if (command.wSubCmdID == SUB_KN_CLOSE_SOCKET) {
      // 关闭链接
      PerformCloseSocket(SHUT_REASON_NORMAL);
      return;
    }
  } else { // 处理数据
    OnSocketRead(command, body_data, body_len);
  }
  recv_packet_count_++;
  PerformRecvData();
}

bool CTCPSocketService::PerformHandleError(const asio::error_code& ec) {
  if (ec == asio::error::eof || ec == asio::error::operation_aborted) {
    // 远程主动关闭了连接
    CLogger::Info("IP {} 终止了服务", ipv4_port_);
  } else if (ec) {
    CLogger::Warn("IP {} 异常：{}", ipv4_port_, ToSimpleUtf8(ec.message()));
  } else {
    return false;
  }

  if (IsValidSocket()) {
    std::error_code ignored;
    socket_->close(ignored);
  }
  // 关闭通知
  OnSocketShut(ec.value());
  return true;
}

// 关闭连接
VOID CTCPSocketService::PerformCloseSocket(BYTE shut_reason) {
  // 关闭判断
  if (IsValidSocket()) {
    // 关闭连接
    std::error_code ec;
    socket_->shutdown(asio::ip::tcp::socket::shutdown_send, ec);
  }
}

// 随机映射
WORD CTCPSocketService::SeedRandMap(WORD wSeed) {
  DWORD dwHold = wSeed;
  return (WORD) ((dwHold = dwHold * 241103L + 2533101L) >> 16);
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

  for (int i = sizeof(TCP_Info); i < packet_size; i++) {
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
bool CTCPSocketService::InitiateService(std::shared_ptr<asio::io_context> io_context) {
  // 运行判断
  ASSERT((!is_running_) /*&& (socket_event_ != nullptr)*/);
  if ((is_running_) /*|| (socket_event_ == nullptr)*/)
    return false;

  // 设置变量
  is_running_ = true;

  socket_ = std::make_unique<asio::ip::tcp::socket>(*io_context);
  strand_ = std::make_unique<asio::strand<asio::any_io_executor>>(socket_->get_executor());

  return true;
}

// 停止服务
bool CTCPSocketService::ConcludeService() {
  // 设置变量
  is_running_ = false;

  // 关闭连接
  asio::dispatch(*strand_, [this]() {
    PerformCloseSocket(SHUT_REASON_INSIDE);
  });
  return true;
}

// 配置函数
bool CTCPSocketService::SetServiceID(WORD wServiceID) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // 设置变量
  service_id_ = wServiceID;

  return true;
}

// 设置接口
bool CTCPSocketService::SetTCPSocketEvent(IUnknownEx* sink_any) {
  // 状态效验
  ASSERT(!is_running_);
  if (is_running_)
    return false;

  // // 查询接口
  // socket_event_ = QUERY_OBJECT_PTR_INTERFACE(sink_any, ITCPSocketEvent);
  //
  // // 错误判断
  // if (socket_event_ == nullptr) {
  //   ASSERT(FALSE);
  //   return false;
  // }

  return true;
}

void CTCPSocketService::TraceException(const std::string& msg) {
  CLogger::Error("SocketService OnRecvCompleted 发生“{}”异常", msg);
  // 关闭链接
  PerformCloseSocket(SHUT_REASON_EXCEPTION);
}

// 关闭连接
bool CTCPSocketService::CloseSocket() {
  // 状态效验
  ASSERT(is_running_);
  if (!is_running_)
    return false;

  asio::dispatch(*strand_, [this]() {
    PerformCloseSocket(SHUT_REASON_NORMAL);
  });
  return true;
}

// 连接服务器
bool CTCPSocketService::Connect(DWORD server_ip, WORD port) {
  // 状态效验
  ASSERT(is_running_);
  if (!is_running_)
    return false;

  // 执行异步连接
  asio::dispatch(*strand_, [this, server_ip, port]() {
    PerformConnect(server_ip, port);
  });
  return true;
}

// 连接地址
bool CTCPSocketService::Connect(LPCTSTR server_ip, WORD port) {
  auto server_ip_v4 = TranslateAddress(server_ip);
  return Connect(server_ip_v4, port);
}

// 发送函数
bool CTCPSocketService::SendData(WORD main_cmd_id, WORD sub_cmd_id) {
  // 状态效验
  ASSERT(is_running_);
  if (!is_running_)
    return false;

  // 异步发送数据(仅包头)
  asio::dispatch(*strand_, [this, main_cmd_id, sub_cmd_id]() {
    PerformSendData(main_cmd_id, sub_cmd_id);
  });
  return true;
}

// 发送函数
bool CTCPSocketService::SendData(WORD main_cmd_id, WORD sub_cmd_id, VOID* body, WORD body_size) {
  // 状态效验
  ASSERT(is_running_);
  if (!is_running_)
    return false;

  // 数据处理
  std::vector<uint8_t> body_copy;
  body_copy.insert(body_copy.end(), static_cast<uint8_t*>(body), static_cast<uint8_t*>(body) + body_size);
  // 异步发送数据(数据包)
  asio::dispatch(*strand_, [this, main_cmd_id, sub_cmd_id, body = std::move(body_copy)]() {
    PerformSendData(main_cmd_id, sub_cmd_id, body);
  });
  return true;
}

// 连接消息
bool CTCPSocketService::OnSocketLink(INT error_code) {
  std::error_code ec;
  const auto endpoint = socket_->remote_endpoint(ec);
  if (!ec) {
    ipv4_port_ = fmt::format("{}:{}", endpoint.address().to_v4().to_string(), endpoint.port());
    CLogger::Info("已与 IP {} 建立连接", ipv4_port_);
  }
  // // 投递事件
  // ASSERT(socket_event_ != nullptr);
  // return socket_event_->OnEventTCPSocketLink(service_id_, error_code);
  GlobalEventBus::Get()->Publish<TCPSocketLinkEventTag>(service_id_, error_code);
  return true;
}

// 关闭消息
bool CTCPSocketService::OnSocketShut(BYTE shut_reason) {
  // // 投递事件
  // ASSERT(socket_event_ != nullptr);
  // return socket_event_->OnEventTCPSocketShut(service_id_, shut_reason);
  CLogger::Debug("已与 #{} 失去了连接", service_id_);
  GlobalEventBus::Get()->Publish<TCPSocketShutEventTag>(service_id_, shut_reason);
  return true;
}

bool CTCPSocketService::OnSocketRead(TCP_Command tcp_cmd, nf::BufferPtr body) {
  // // 投递事件
  // ASSERT(socket_event_ != nullptr);
  // return socket_event_->OnEventTCPSocketRead(service_id_, Command, body->data(), body->size());
  GlobalEventBus::Get()->Publish<TCPSocketReadEventTag>(service_id_, tcp_cmd, body);
  return true;
}

// 读取消息
bool CTCPSocketService::OnSocketRead(TCP_Command tcp_cmd, VOID* body_data, WORD data_size) {
  // // 投递事件
  // ASSERT(socket_event_ != nullptr);
  // return socket_event_->OnEventTCPSocketRead(service_id_, tcp_cmd, body_data, data_size);
  auto body = ConvertToBytes(body_data, data_size);
  GlobalEventBus::Get()->Publish<TCPSocketReadEventTag>(service_id_, tcp_cmd, body);
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

// 组件创建函数
DECLARE_CREATE_MODULE(TCPSocketService);

//////////////////////////////////////////////////////////////////////////////////
