#include "BaseSessionLogic.h"

#include "GlobalDefine/Packet.h"
#include "KernelEngine/TraceService.h"
#include "SessionCodec.h"

asio::awaitable<void> BaseSessionLogic::Run() {
  auto ctx = Context();
  if (ctx->name.empty()) {
    ctx->name = ctx->socket.remote_endpoint().address().to_string();
  }
  // 预发送数据
  co_await Prepare(ctx);
  bool ret = false;
  do {
    try {
      ret = co_await Execute(ctx);
    } catch (const asio::system_error& ec) {
      if (ec.code() == asio::error::eof || ec.code() == asio::error::connection_reset || ec.code() == asio::error::operation_aborted) {
        CLogger::Info("{} 会话已完成", ctx->name); // 客户端优雅的关闭了连接
      } else {
        CLogger::Warn("{} 异常退出: {}", ctx->name, ToSimpleUtf8(ec.what()));
      }
      ctx->socket.close();
      ret = false;
    } catch (...) {
      CLogger::Error("{} 未知异常退出", ctx->name);
      ctx->socket.close();
      ret = false;
    }
  } while (ret);
  ctx.reset();
  co_return;
}

asio::awaitable<bool> BaseSessionLogic::Execute(const std::shared_ptr<SessionContext>& ctx) {
  TCP_Head tcp_head{};
  // 读取包头信息
  std::size_t length = co_await asio::async_read(ctx->socket, asio::buffer(&tcp_head, sizeof(tcp_head)), asio::use_awaitable);
  if (length != sizeof(tcp_head)) {
    CLogger::Error("{}: 包头长度错误 {}", ctx->name, length);
    co_return false;
  }
  const std::size_t data_length = tcp_head.TCPInfo.wPacketSize - sizeof(tcp_head);
  if (data_length == 0) {
    if (!SessionCodec::Decode(&tcp_head)) {
      CLogger::Error("{}: 数据包解码失败", ctx->name);
      co_return false;
    }
    CLogger::Debug("{}: CMD: {}-{}", ctx->name, tcp_head.CommandInfo.wMainCmdID, tcp_head.CommandInfo.wSubCmdID);
    static std::vector<uint8_t> empty_data;
    co_await Publish(tcp_head.CommandInfo.wMainCmdID, tcp_head.CommandInfo.wSubCmdID, empty_data);
    co_return true;
  }
  std::vector<uint8_t> body_data(data_length);
  length = co_await asio::async_read(ctx->socket, asio::buffer(body_data.data(), data_length), asio::use_awaitable);
  if (length != data_length) {
    CLogger::Error("{}: 包体长度错误 {}", ctx->name, length);
    co_return false;
  }
  if (!SessionCodec::Decode(&tcp_head, body_data)) {
    CLogger::Error("{}: 数据包解码失败", ctx->name);
    co_return false;
  }
  CLogger::Debug("{}: CMD: {}-{} -> {}", ctx->name, tcp_head.CommandInfo.wMainCmdID, tcp_head.CommandInfo.wSubCmdID, data_length);
  co_await Publish(tcp_head.CommandInfo.wMainCmdID, tcp_head.CommandInfo.wSubCmdID, body_data);
  co_return true;
}

asio::awaitable<bool> BaseSessionLogic::Sending(const uint16_t main, const uint16_t sub, const uint8_t* const body, const size_t body_size) {
  std::vector<uint8_t> packet(sizeof(TCP_Head));
  if (body_size > 0)
    packet.insert(packet.end(), body, body + body_size);

  const auto tcp_head = reinterpret_cast<TCP_Head*>(packet.data());
  tcp_head->CommandInfo.wMainCmdID = main;
  tcp_head->CommandInfo.wSubCmdID = sub;
  SessionCodec::Encode(packet.data(), packet.size());
  co_return co_await asio::async_write(Context()->socket, asio::buffer(packet.data(), packet.size()), asio::use_awaitable) == packet.size();
}

auto BaseSessionLogic::Publish(const uint16_t main, const uint16_t sub, const std::vector<uint8_t>& body) -> asio::awaitable<void> {
  if (main == MDM_KN_COMMAND) {
    if (sub == SUB_KN_DETECT_SOCKET) {
      co_await Sending(MDM_KN_COMMAND, SUB_KN_DETECT_SOCKET);
    }
    if (sub == SUB_KN_CLOSE_SOCKET) {
      if (auto& socket = Context()->socket; socket.is_open()) {
        std::error_code ec;
        socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
      }
    }
  }
  const uint32_t event = MAKELONG(main, sub);
  co_await bus_.Publish(event, body);
}
