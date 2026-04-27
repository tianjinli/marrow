# KernelEngine 模块

KernelEngine 是系统中的 **核心内核引擎模块**，用于定义统一的接口规范和基础实现。

## ✨ 功能简介

- **接口与对象模型**
    - IUnknownEx 接口基类（GUID 与 VER_Xxx 关联，Xxx 为接口名）
    - QueryInterface/Release 统一接口查询与生命周期管理
    - 模块工厂宏（DECLARE_CREATE_MODULE 等）

- **异步事件总线**
    - 替换原来的异步引擎和事件接口，提供全局的对象
    - 以前的事件回调函数变成了 Subscribe，激活事件回调变成了 Publish
    - 以前的调度引擎成功的被上述函数给替换掉了，使系统更简洁了
    - GlobalEventBus::Init 进行初始化（在 CServiceUnits::DoStartService 中调用）
    - GlobalEventBus::Shutdown 进行反初始化（在 CServiceUnits::DoStopService 中调用）

- **数据库助手&引擎**
    - 使用 nanodbc 重新实现了对 MSSQL 的操作
    - 由于之前的结果集获取是需要返回值来判断而 nanodbc 却不支持，最终通过将数据临时存储来实现
    - 虽然 nanodbc 支持异步，但是之前的写法是同步的，所以目前数据库相关调用都是同步的

- **连接子项&网络引擎**
    - CTCPNetworkItem 已改为智能指针友好类型，不支持内存复用（容易出 BUG）
    - 每个 `void*, uint16_t` 数据组合都新增了数组类型智能指针
    - 支持优雅的关闭客户端和服务端

- **网络服务**
    - 支持优雅的关闭和服务端的连接

- **定时器引擎**
    - 使用 asio::steady_timer 重新实现

- **追踪服务**
    - 使用 spdlog 重新实现了对日志文件的操作
