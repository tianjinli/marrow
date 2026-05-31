#include "AttemperEngine.h"

//////////////////////////////////////////////////////////////////////////////////
// 析构函数
CAttemperEngine::~CAttemperEngine() {
  // 停止服务
  CAttemperEngine::ConcludeService();
}

// 接口查询
VOID* CAttemperEngine::QueryInterface(REFGUID Guid, DWORD dwQueryVer) {
  QUERYINTERFACE(IServiceModule, Guid, dwQueryVer);
  QUERYINTERFACE(IAttemperEngine, Guid, dwQueryVer);
  QUERYINTERFACE_IUNKNOWNEX(IAttemperEngine, Guid, dwQueryVer);
  return nullptr;
}

// 开始服务
bool CAttemperEngine::StartService(std::shared_ptr<asio::io_context> io_context) {
  // 状态效验
  ASSERT(io_context != nullptr && !is_running_.load(std::memory_order_relaxed));
  if (io_context == nullptr || is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  // 设置变量
  io_context_ = std::move(io_context);
  strand_ = asio::make_strand(io_context_->get_executor());

  // 初始化事件通知
  OnAttemperEngineStart.Setup(strand_);
  OnAttemperEngineConclude.Setup(strand_);
  OnAttemperEngineControl.Setup(strand_);

  OnAttemperEngineStart(this);
  return is_running_.exchange(true, std::memory_order_relaxed) == false;
}

// 停止服务
bool CAttemperEngine::ConcludeService() {
  // 设置变量
  if (!is_running_.exchange(false, std::memory_order_relaxed)) {
    return true;
  }

  OnAttemperEngineConclude(this);

  OnAttemperEngineStart.Clear();
  OnAttemperEngineConclude.Clear();
  OnAttemperEngineControl.Clear();

  // 重置内核资产生命周期
  io_context_.reset();
  return true;
}

bool CAttemperEngine::SetAttemperEngineEvent(void* object_ptr) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IAttemperEngineEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IAttemperEngineEvent);

  if (event_ptr == nullptr) {
    return false;
  }

  OnAttemperEngineStart += MEMBER_DELEGATE(&IAttemperEngineEvent::OnAttemperEngineStart, event_ptr);
  OnAttemperEngineConclude += MEMBER_DELEGATE(&IAttemperEngineEvent::OnAttemperEngineConclude, event_ptr);
  return true;
}

bool CAttemperEngine::SetControlEngineEvent(void* object_ptr) {
  // 状态效验
  ASSERT(!is_running_.load(std::memory_order_relaxed));
  if (is_running_.load(std::memory_order_relaxed)) {
    return false;
  }

  const auto unknown_ex = static_cast<IUnknownEx*>(object_ptr);
  ASSERT(QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IControlEngineEvent) != nullptr);
  const auto event_ptr = QUERY_OBJECT_PTR_INTERFACE(unknown_ex, IControlEngineEvent);

  if (event_ptr == nullptr) {
    return false;
  }

  OnAttemperEngineControl += MEMBER_DELEGATE(&IControlEngineEvent::OnEventControlInternal, event_ptr);
  return true;
}

bool CAttemperEngine::OnEventControl(WORD control_id, VOID* data, WORD data_size) {
  auto post_data = ConvertToBytes(data, data_size);
  OnAttemperEngineControl(control_id, std::move(post_data));
  return true;
}

//////////////////////////////////////////////////////////////////////////////////

// 组件创建函数
DECLARE_CREATE_MODULE(AttemperEngine);

//////////////////////////////////////////////////////////////////////////////////
