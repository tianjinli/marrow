#pragma once

#include "EventDelegate.h"
#include "KernelEngineHead.h"

//////////////////////////////////////////////////////////////////////////////////

// 调度引擎
class CAttemperEngine : public IAttemperEngine {
public:
  // 构造函数
  explicit CAttemperEngine() = default;
  // 析构函数
  virtual ~CAttemperEngine();

  // 基础接口
  // 释放对象
  VOID Release() override { delete this; }
  // 接口查询
  VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer) override;

  // 服务接口
  // 启动服务
  bool StartService(std::shared_ptr<asio::io_context> io_context) override;
  // 停止服务
  bool ConcludeService() override;

  // 设置接口
  bool SetAttemperEngineEvent(void* object_ptr) override;

  // 设置接口
  bool SetControlEngineEvent(void* object_ptr) override;

  // 控制事件
  // 控制事件
  bool OnEventControl(WORD wControlID, VOID* pData, WORD wDataSize) override;

public:
  mutable EventDelegate<IUnknownEx*> OnAttemperEngineStart;
  mutable EventDelegate<IUnknownEx*> OnAttemperEngineConclude;
  mutable EventDelegate<WORD, nf::BufferPtr> OnAttemperEngineControl;

private:
  std::atomic<bool> is_running_{false};

  // 组件变量
  std::shared_ptr<asio::io_context> io_context_;
  asio::strand<asio::any_io_executor> strand_{asio::make_strand(asio::system_executor())};
};

//////////////////////////////////////////////////////////////////////////////////
