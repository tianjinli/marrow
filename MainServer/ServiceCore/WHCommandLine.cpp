#include "WHCommandLine.h"

//////////////////////////////////////////////////////////////////////////////////

// 查询命令
bool CWHCommandLine::SearchCommandItem(LPCTSTR command_line, LPCTSTR command, TCHAR parameter[], WORD parameter_len) {
  // 效验参数
  ASSERT((command_line != nullptr) && (command != nullptr));
  if ((command_line == nullptr) || (command == nullptr)) {
    return false;
  }

  // 参数处理
  if (command_line[0] != 0) {
    // 变量定义
    UINT command_len = (UINT) lstrlen(command);
    LPCTSTR begin_string = command_line;

    // 提取参数
    while (true) {
      // 参数分析
      LPCTSTR end_string = StrChrT(begin_string, TEXT(' '));
      UINT string_length = (end_string == nullptr) ? (UINT) lstrlen(begin_string) : (UINT) (end_string - begin_string);

      // 命令分析
      if ((string_length >= command_len) && (memcmp(begin_string, command, command_len * sizeof(TCHAR)) == 0)) {
        // 长度效验
        ASSERT(parameter_len > (string_length - command_len));
        if ((parameter_len <= (string_length - command_len))) {
          return false;
        }

        // 提取参数
        parameter[string_length - command_len] = 0;
        CopyMemory(parameter, begin_string + command_len, (string_length - command_len) * sizeof(TCHAR));

        return true;
      }

      // 设置变量
      if (end_string == nullptr) {
        break;
      }
      begin_string = (end_string + 1);
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////////////
