#include "ChatServer.h"
#include "ChatServerHead.h"

int main(int argc, char* argv[]) {
#ifdef _WIN32
  // setlocale(LC_ALL, ".utf8");
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleA("聊天服务器");
#endif // _WIN32
  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);

  // 初始异步日志
  CLogger::Initialize();

  auto server = std::make_shared<CChatServer>();
  server->StartService();

  signals.async_wait([&](auto, auto) {
    server->ConcludeService();
    io_context.stop();
  });

  (void) std::async(std::launch::async, [server, &io_context]() -> void {
    std::string line;
    while (std::getline(std::cin, line)) {
      if (line == "cls") {
        // 清屏（平台相关）
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
      } else if (line == "start") {
        server->StartService();
      } else if (line == "stop") {
        server->ConcludeService();
      } else if (line == "exit") {
        io_context.stop(); // 触发退出
        break;
      }
    }
  });

  io_context.run();
  server = nullptr;
  return 0;
}
