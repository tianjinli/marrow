# LogonServer 模块

LogonServer 是系统中的 **登录认证服务模块**，负责处理客户端的登录请求、账号校验与会话初始化，是用户进入系统的入口。

## ✨ 功能简介

- **数据库引擎接收器（数据库类）**
    - 以同步的方式处理用户发来的数据库请求
    - 新增 EmitDataBaseEngineResult 用于触发数据库引擎结果事件
    - __*DataBase 重命名为 DataBase，包含类名和成员函数名*__

- **调度引擎接收器（调度钩子）**
    - 根据以前的调度钩子改造，接口汇总来自于已有的接口
    - __*OnEventDataBase 更名为 OnEventDataBase*__
    - __*OnEventTCPNetworkBind 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__
    - __*OnEventTCPNetworkShut 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__
    - __*OnEventTCPNetworkRead 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__

- **服务单元**
    - 新增`SubscribeEvents`用于订阅事件回调
    - __*StartService 更名为 DoStartService*__
    - __*ConcludeService 更名为 DoStopService*__

- **主函数入口**
  - 支持启动、停止、退出、清屏等命令
