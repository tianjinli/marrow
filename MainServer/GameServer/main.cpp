#include "GameServer.h"
#include "GameServerHead.h"

int main(int argc, char* argv[]) {
#ifdef _WIN32
  // setlocale(LC_ALL, ".utf8");
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleA("游戏服务器");
#endif // _WIN32
  cmdline::parser parser;
  parser.add<int>("server-id", 's', "游戏组件ID", false, 0, cmdline::range(1, 65535));
  parser.parse_check(argc, argv);
  auto game_server_id = parser.get<int>("server-id");

  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);

  // 初始异步日志
  CLogger::Initialize();

  auto server = std::make_shared<CGameServer>();
  if (server->LoadConfigByID(game_server_id) == false) {
    return 1;
  }
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
