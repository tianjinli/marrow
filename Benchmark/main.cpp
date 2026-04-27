// 其他库头文件
#include "KernelEngine/TraceService.h"

// 本项目头文件
#include "LogonSessionLogic.h"
#include "ServiceCore/cmdline.h"
#include "SessionLogic.h"
#include "SessionPool.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef DEBUG_ENABLED
#define SESSION_COUNT 1
#else
#define SESSION_COUNT 100
#endif // DEBUG_ENABLED


int main(int argc, char* argv[]) {
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleA("网狐框架压测工具");
#endif
  cmdline::parser parser;
  parser.add<std::string>("host", 'h', "服务器主机 IP", false, "127.0.0.1");
  parser.add<int>("port", 'p', "服务器端口号", false, 6002, cmdline::range(1, 65535));
  parser.add<std::string>("logic", 'l', "逻辑处理类型", false, "logon", cmdline::oneof<std::string>("logon", "hall"));
  parser.add<int>("session", 's', "会话连接数量", false, SESSION_COUNT, cmdline::range(1, 100'0000));
  auto default_thread_count = std::max<int>(std::thread::hardware_concurrency(), 8);
  parser.add<int>("thread", 't', "并发线程数量", false, default_thread_count, cmdline::range(1, default_thread_count * 10));
  parser.parse_check(argc, argv);

  auto server_host = parser.get<std::string>("host");
  auto server_port = parser.get<int>("port");
  auto session_count = parser.get<int>("session");
  auto thread_count = parser.get<int>("thread");
  asio::error_code ec;
  asio::ip::address_v4 addr = asio::ip::make_address_v4(server_host, ec);
  if (ec) {
    std::cerr << "无效的 IPv4 地址: " << server_host << std::endl;
    return 1;
  }

  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);
  signals.async_wait([&](auto, auto) {
    io_context.stop();
  });

  // 初始化异步日志
  CLogger::Initialize();

  std::vector<std::thread> threads;
  for (auto i = 0; i < thread_count; ++i) {
    threads.emplace_back([&] {
      io_context.run();
    });
  }

  SessionPool session_pool(io_context, addr, server_port);
  session_pool.Spawn(
      [](auto ctx) {
        return std::make_shared<LogonSessionLogic>(ctx);
      },
      session_count);

  io_context.run();
  for (auto& thread: threads) {
    if (thread.joinable())
      thread.join();
  }
  return 0;
}
