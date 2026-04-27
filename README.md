# 网狐精华框架多平台版

🎮 一个基于 C++ 的经典网狐精华框架的多平台移植版本，支持 Linux、Windows 等主流操作系统。旨在完全不修改或极少量修改现有游戏模块代码，极大的减少了代码移植风险和成本。
本项目处于试验阶段，使用产生任何风险（含法律风险）与本作者无关。

---

## 📦 项目特点

- ✅ 移植自网狐精华框架，保留原始架构精髓
- 🧩 支持多平台编译（Linux / Windows）
- 🚀 高性能网络通信模块（基于 epoll / IOCP）
- 🔐 支持多线程任务调度与安全同步机制
- 🛠️ 模块化设计，便于二次开发与功能扩展

---

## 🧱 软件架构

```
├── Collocates # 配置管理工具
├── Correspond # 协调服务器
├── Dependencies # 第三方依赖库
├── GlobalDefine # 全局定义
├── KernelEngine # 游戏内核引擎
├── LogonServer # 登录服务器
├── MessageDefine # 消息定义
├── MssqlScripts # 数据库脚本
├── ServiceCore # 服务核心模块
```

---

## 🚀 快速开始

### 📥 准备工作

[Microsoft SQL Server 的 ODBC 驱动程序
](https://learn.microsoft.com/zh-cn/sql/connect/odbc/microsoft-odbc-driver-for-sql-server)

### Windows

[下载 Microsoft ODBC Driver 18 for SQL Server (x64)](https://go.microsoft.com/fwlink/?linkid=2307162)

```cmd
SET DB_HOST=localhost
SET DB_USER=sa
SET DB_PASS=poTP9YjMrLhVhm5B
cmd /c MssqlScripts\InitDbCore.bat
```

### Linux

[安装 Microsoft ODBC Driver 18 for SQL Server (Linux)](https://learn.microsoft.com/zh-cn/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server)

> ⚠ 请自行选择合适的脚本执行

```shell
export DB_HOST=localhost
export DB_USER=sa
export DB_PASS=poTP9YjMrLhVhm5B
bash MssqlScripts/InitDbCore.sh
```

## 🛠️ 构建方式（以 Ubuntu 为例）

### 安装依赖

```shell
sudo apt install cmake g++ unixodbc-dev
```

### 编译构建

```shell
# mkdir build && cd build
cmake .
make -j$(nproc)
```

## ⚙️ 配置说明

```shell
# 默认使用当前可执行文件所在目录下的 ServerParameter.ini
# 所有命令都可以在第二个参数指定配置文件/目录路径
# 例如：./Collocates /path/to/config/ -a
# 如果配置文件路径为目录则会自动添加 ServerParameter.ini

# 注解查看配置文件
./Collocates -a

# 解密重写配置文件
./Collocates -d

# 加密重写配置文件
./Collocates -e

# 生成默认配置文件
./Collocates -g

# 明文查看配置文件
./Collocates -i

# 局部更新配置文件
./Collocates -u DBUser=sa
```

## 🧪 示例运行

### 数据库

```shell
docker run -e "ACCEPT_EULA=Y" -e "SA_PASSWORD=poTP9YjMrLhVhm5B" -p 1433:1433 -v mssql:/var/opt/mssql --restart=always --name mssql -d mcr.microsoft.com/mssql/server:2019-latest
```

### 服务端

```shell
# 1. 首先启动协调服务器
./Correspond &
# 2. 再次启动登录服务器
./LogonServer &
# 3. 最后启动游戏服务器 (待实现)
./GameServer &
```

### 客户端

无论 Windows 还是 Android 或者 iOS 请使用 UTF8 来编码字符串以达到统一。

## 🤝 参与贡献

欢迎开发者参与改进本项目：

1. Fork 本仓库

2. 创建新分支：git checkout -b feat/your-feature

3. 提交更改：git commit -m 'Add your feature'

4. 推送分支：git push origin feat/your-feature

5. 创建 Pull Request

## 📄 开源许可

本项目采用 MIT 许可证，详见 [LICENSE](LICENSE)。

## 📬 服务支持

如有问题或建议，请提交 Issue 或联系项目维护者。欢迎加入交流群一起交流开发经验。
