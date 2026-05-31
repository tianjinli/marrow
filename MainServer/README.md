# 网狐荣耀服务器

## 🧱 软件架构

```
├── Collocates # 配置管理工具
├── Correspond # 协调服务器
├── Dependencies # 第三方依赖库
├── GameServer # 游戏服务器
├── GameService # 游戏服务
├── GlobalDefine # 全局定义
├── KernelEngine # 游戏内核引擎
├── LogonServer # 登录服务器
├── MessageDefine # 消息定义
├── ModuleManager # 模块管理器
├── ServiceCore # 服务核心模块
```

## 特别说明

为了与多平台兼容，Linux 下消息字符串使用 char16_t 编码

* 使用 StringUtils::ToString 进行 char16_t 和 char 互转
```c++
#include "StringUtils.h"

std::string utf8 = StringUtils::ToString(u"你好，世界！");
std::u16string utf16 = StringUtils::ToString(utf8);
```

* 使用 clang-format 递归格式化 C/C++ 代码
```shell
Get-ChildItem -Recurse -Include *.h,*.hpp,*.cpp | % { clang-format -i $_.FullName }
```
