#include "CorrespondServer.h"

int main(int argc, const char* argv[]) {
#ifdef _WIN32
  // setlocale(LC_ALL, ".utf8");

  /********************************************************************************
  <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
  <assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
    <assemblyIdentity version="1.0.0.0" name="YourAppName" type="win32"/>
    <application>
      <windowsSettings>
        <activeCodePage xmlns="http://schemas.microsoft.com/SMI/2019/WindowsSettings">UTF-8</activeCodePage>
      </windowsSettings>
    </application>
  </assembly>
  ********************************************************************************/
  // 如果嵌入了上面的 manifest 则可以删除 SetConsoleCP 和 SetConsoleOutputCP
  // mt.exe -nologo -manifest utf8.manifest -outputresource:myapp.exe;#1
  // 它会声明进程的活动代码页为 UTF‑8，这样 ANSI 版 WinAPI 就会按 UTF‑8 来解码
  // 支持的系统版本：Windows 10 1903+ / Windows Server 2022 (20348+)
  // 截至目前 2025/09/11 为止此功能还有 BUG 请谨慎使用
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleA("协调服务器");
#endif // _WIN32

  asio::io_context io_context;
  asio::signal_set signals(io_context, SIGINT, SIGTERM);

  // 初始化异步日志
  CLogger::Initialize();

  auto server = std::make_shared<CCorrespondServer>();
  server->DoStartService();

  signals.async_wait([&](auto, auto) {
    server->DoStopService();
    io_context.stop();
  });

  (void) std::async(
      std::launch::async,
      [server](asio::io_context& io_context) -> void {
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
            server->DoStartService();
          } else if (line == "stop") {
            server->DoStopService();
          } else if (line == "exit") {
            io_context.stop(); // 触发退出
            break;
          }
        }
      },
      std::ref(io_context));

  io_context.run();
  server = nullptr; 
  return 0;
}
