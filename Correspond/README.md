# Correspond 模块

Correspond 是系统中的 **消息通信与转发模块**，负责在服务端与服务端之间进行消息路由和数据交换。

## ✨ 功能简介

- **调度引擎接收器（调度钩子）**
    - 根据以前的调度钩子改造，接口汇总来自于已有的接口
    - __*OnEventTCPNetworkBind 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__
    - __*OnEventTCPNetworkShut 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__
    - __*OnEventTCPNetworkRead 参数签名已跟 ITCPNetworkEngineEvent 接口统一*__

- **服务单元**
    - 新增`SubscribeEvents`用于订阅事件回调
    - __*StartService 更名为 DoStartService*__
    - __*ConcludeService 更名为 DoStopService*__

- **主函数入口**
    - 支持启动、停止、退出、清屏等命令
