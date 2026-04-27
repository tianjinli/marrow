#include "InitParameter.h"

#include <iostream>
using namespace std::chrono_literals;

int main(const int argc, const char* argv[]) {
#ifdef _WIN32
  // setlocale(LC_ALL, ".utf8");
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleTitleA("参数转换器");
#endif // _WIN32

  const std::string command = (argc > 1) ? argv[1] : "-h";
  std::string file_path = (argc > 2) ? argv[2] : "";
  if (command == "-h" || command == "--help") {
    std::cout << R"(用法: Collocates [OPTION] [FILE]
    -a, --annotate        [FILE] r 注解查看配置文件
    -d, --decrypt         [FILE] w 解密重写配置文件
    -e, --encrypt         [FILE] w 加密重写配置文件
    -g, --generate        [FILE] w 生成默认配置文件
    -i, --inspect         [FILE] r 高亮查看配置文件
    -u, --update [s.]k=v  [FILE] w 局部更新配置文件
        没有指定 section 时更新所有 key 的 value
)" << std::endl;
  } else if (command == "-g" || command == "--generate") {
    // 生成配置文件模板
    CInitParameter::GenerateParameterFile(file_path);
  } else if (command == "-e" || command == "--encrypt") {
    // 加密配置文件
    CInitParameter::EncryptParameterFile(file_path);
  } else if (command == "-d" || command == "--decrypt") {
    // 解密配置文件
    CInitParameter::DecryptParameterFile(file_path);
  } else if (command == "-u" || command == "--update") {
    // 修改配置文件
    const std::string key_value = std::move(file_path);
    file_path = (argc > 3) ? argv[3] : "";
    CInitParameter::UpdateParameterFile(key_value, file_path);
  } else if (command == "-a" || command == "--annotate") {
    // 生成配置文件注释
    CInitParameter::AnnotateParameterComment(file_path);
  } else if (command == "-i" || command == "--inspect") {
    // 明文查看配置文件
    CInitParameter::InspectParameterFile(file_path);
  } else {
    std::cout << "查看帮助: Collocates -h 或 Collocates --help" << std::endl;
  }

  return 0;
}
