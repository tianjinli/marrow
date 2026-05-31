#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include <asio.hpp>

// --------------------------- MemberFuncTraits ---------------------------
template<typename T>
struct MemberFuncTraits;

template<typename ClassType, typename ReturnType, typename... MethodArgs>
struct MemberFuncTraits<ReturnType (ClassType::*)(MethodArgs...)> {
  using class_type = ClassType;
};

template<typename ClassType, typename ReturnType, typename... MethodArgs>
struct MemberFuncTraits<ReturnType (ClassType::*)(MethodArgs...) const> {
  using class_type = ClassType;
};

// --------------------------- EventDelegate ---------------------------
template<typename... Args>
class EventDelegate {
public:
  using HandlerType = std::function<void(Args...)>;

  struct DelegateItem {
    HandlerType handler;
    std::string func_name;
    void* class_object = nullptr;
    std::uintptr_t func_address = 0; // 稳定全局唯一 ID
  };

private:
  using InvokerList = std::vector<DelegateItem>;
  using InvokerListPtr = std::shared_ptr<InvokerList>;

  // 工业级重构：采用标准互斥锁，彻底杜绝多线程高并发下读写锁升级导致的潜在死锁
  struct MemberRegistry {
    std::mutex mtx;
    std::unordered_map<std::string, std::uintptr_t> map;
    std::atomic<std::uintptr_t> next_id{1};

    template<typename Method>
    static std::string MakeKey(const Method& mf) {
      std::string key;
      key += typeid(Method).name();
      const auto p = reinterpret_cast<const uint8_t*>(&mf);
      key.append(reinterpret_cast<const char*>(p), sizeof(mf));
      return key;
    }

    template<typename Method>
    std::uintptr_t operator[](const Method& mf) {
      auto key = MakeKey(mf);
      std::lock_guard lock(mtx); // 高并发下安全的串行化注册
      auto itr = map.find(key);
      if (itr != map.end()) {
        return itr->second;
      }

      std::uintptr_t id = next_id.fetch_add(1, std::memory_order_relaxed);
      map.emplace(std::move(key), id);
      return id;
    }
  };

  // C++ 17 确保全局物理唯一
  inline static MemberRegistry reg;

public:
  // 1. 🔥 恢复默认构造函数：允许在类成员变量中轻松声明（此时不分配 Strand）
  explicit EventDelegate() = default;

  // 允许通过另一个具有相同 strand 的委派进行拷贝/移动构造
  EventDelegate(const EventDelegate& other) = default;
  EventDelegate& operator=(const EventDelegate& other) = default;

  /**
   * @brief 装配事件委派的运行环境
   * @param strand 强制要求绑定的串行化调度器（非智能指针引用）
   * @param timeout 业务回调执行的超时监控阈值（毫秒），0 代表不监控
   */
  void Setup(asio::strand<asio::any_io_executor>& strand, const std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
    *this = strand;
    *this = timeout;
  }

  EventDelegate& operator=(asio::strand<asio::any_io_executor>& strand) {
    strand_.emplace(strand);
    return *this;
  }

  EventDelegate& operator=(const std::chrono::milliseconds timeout) {
    timeout_.store(timeout, std::memory_order_relaxed);
    return *this;
  }

  // 1. 绑定已有 DelegateItem (核心底座)
  EventDelegate& operator+=(const DelegateItem& item) {
    std::lock_guard lock(write_mutex_);
    auto cur = invokers_ptr_.load(std::memory_order_acquire);
    auto next = std::make_shared<InvokerList>(*cur);
    for (const auto& i: *next) {
      if (i.class_object == item.class_object && i.func_address == item.func_address) {
        throw std::runtime_error(std::format("EventDelegate: {} already subscribed", i.func_name));
      }
    }
    next->emplace_back(item);
    invokers_ptr_.store(next, std::memory_order_release);
    return *this;
  }

  EventDelegate& operator+=(std::pair<HandlerType, std::string_view> target) {
    DelegateItem item;
    item.handler = target.first;
    item.func_name = std::string(target.second);
    item.class_object = nullptr;
    std::uintptr_t id = reg[target.first];
    item.func_address = id;
    return *this += item;
  }

  // 2. 解绑 by pair (object, func_id) or (nullptr, func_id) for static
  EventDelegate& operator-=(std::pair<void*, std::uintptr_t> target) {
    std::lock_guard lock(write_mutex_);
    auto cur = invokers_ptr_.load(std::memory_order_acquire);
    auto next = std::make_shared<InvokerList>(*cur);

    auto itr = std::remove_if(next->begin(), next->end(), [target](const DelegateItem& item) {
      return item.class_object == target.first && item.func_address == target.second;
    });

    if (itr != next->end()) {
      next->erase(itr, next->end());
      invokers_ptr_.store(next, std::memory_order_release);
    }
    return *this;
  }

  // 3. 🚀 纯粹的异步排队触发
  void operator()(Args... args) const {
    // 🔥 从根源拦截：如果没有 InitStrand，直接在开发阶段崩溃，严防裸跑
    assert(strand_.has_value() && "EventDelegate: Strand must be initialized before invocation!");
    if (!strand_.has_value()) {
      return;
    }

    auto cur = invokers_ptr_.load(std::memory_order_acquire);
    if (!cur || cur->empty()) {
      return;
    }

    auto threshold = timeout_.load(std::memory_order_relaxed);
    auto perf_cb = OnPerfWarn; // 🔥 建立局部快照，杜绝动态修改导致的 Race Context Crash

    using ArgsTuple = std::tuple<std::decay_t<Args>...>;
    auto args_tuple = std::make_shared<ArgsTuple>(std::forward<Args>(args)...);

    auto invoke_all = [cur, args_tuple, threshold, perf_cb]() mutable {
      const size_t count = cur->size();
      for (size_t i = 0; i < count; ++i) {
        const auto& item = (*cur)[i];
        if (!item.handler) {
          continue;
        }

        // 通过 std::apply 配合内部 lambda 闭包提取出 tuple 元组参数
        auto execute_call = [&]<typename... T0>(T0&&... uncurried_args) {
          if (threshold == std::chrono::milliseconds::zero() || !perf_cb) {
            item.handler(std::forward<T0>(uncurried_args)...);
          } else {
            const auto start = std::chrono::steady_clock::now();
            item.handler(std::forward<T0>(uncurried_args)...);
            if (const auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start); dur > threshold) {
              try {
                perf_cb(item.func_name, dur, threshold);
              } catch (...) {
              }
            }
          }
        };

        std::apply(execute_call, (i == count - 1) ? std::move(*args_tuple) : *args_tuple);
      }
    };
    // 使用 post 保证异步排队（避免递归直接执行导致阻塞）
    asio::post(strand_->get(), std::move(invoke_all));
  }

  void Clear() {
    // 使用 post 保证异步排队（避免有事件未执行完成）
    asio::post(strand_->get(), [this]() {
      std::lock_guard lock(write_mutex_);
      invokers_ptr_.store(std::make_shared<InvokerList>(), std::memory_order_release);
    });
  }

  // 性能告警回调：func_name, duration_ms, threshold_ms
  std::function<void(const std::string&, std::chrono::milliseconds, std::chrono::milliseconds)> OnPerfWarn;

private:
  // 强类型内部成员绑定
  template<typename MemFuncPtr, typename ClassType = MemberFuncTraits<MemFuncPtr>::class_type>
  EventDelegate& BindMember(ClassType* instance, MemFuncPtr mf, std::string_view func_name) {
    if (!instance) {
      throw std::invalid_argument("BindMember: instance of class object is null");
    }

    std::uintptr_t id = reg[mf];
    HandlerType handler = [instance, mf](Args... a) {
      std::invoke(mf, instance, std::forward<Args>(a)...);
    };

    DelegateItem item;
    item.class_object = static_cast<void*>(instance);
    item.func_name = std::string(func_name);
    item.func_address = id;
    item.handler = std::move(handler);
    return *this += item;
  }

  template<typename MemFuncPtr, typename ClassType = MemberFuncTraits<MemFuncPtr>::class_type>
  EventDelegate& UnbindMember(ClassType* instance, MemFuncPtr mf) {
    if (!instance) {
      throw std::invalid_argument("UnbindMember: instance of class object is null");
    }

    std::uintptr_t id = reg[mf];
    return *this -= std::make_pair(static_cast<void*>(instance), id);
  }

  template<typename ClassType, typename MemFuncPtr>
  friend struct MemberBinderBridge;

  template<typename FuncPtr>
  friend struct StaticBinderBridge;

  std::atomic<std::shared_ptr<InvokerList>> invokers_ptr_ = std::make_shared<InvokerList>();
  // 🔥 改为 optional + reference_wrapper 既支持延迟初始化，又避开了指针的繁琐
  std::optional<std::reference_wrapper<asio::strand<asio::any_io_executor>>> strand_;
  std::atomic<std::chrono::milliseconds> timeout_;
  std::mutex write_mutex_;
};

// --------------------------- MemberBinderBridge ---------------------------
// 用法： MEMBER_DELEGATE(&Class::NonStaticMethod, instance)
// 用法： MEMBER_DELEGATE(&BaseClass::PureVirtualMethod, instance)
template<typename ClassType, typename MemFuncPtr>
struct MemberBinderBridge {
  ClassType* inst;
  MemFuncPtr method;
  std::string_view name;

  template<typename... Args>
  friend EventDelegate<Args...>& operator+=(EventDelegate<Args...>& delegate_obj, const MemberBinderBridge& bridge) {
    return delegate_obj.BindMember(bridge.inst, bridge.method, bridge.name);
  }

  template<typename... Args>
  friend EventDelegate<Args...>& operator-=(EventDelegate<Args...>& delegate_obj, const MemberBinderBridge& bridge) {
    return delegate_obj.UnbindMember(bridge.inst, bridge.method);
  }
};

#define MEMBER_DELEGATE(mem_func_ptr, instance_ptr)                                                           \
  MemberBinderBridge<typename MemberFuncTraits<decltype(mem_func_ptr)>::class_type, decltype(mem_func_ptr)> { \
    instance_ptr, mem_func_ptr, #mem_func_ptr                                                                 \
  }

// --------------------------- StaticBinderBridge ---------------------------
// 用法1： STATIC_DELEGATE(&Class::StaticMethod)
// 用法2： STATIC_DELEGATE(GlobalMethod)
template<typename FuncPtr>
struct StaticBinderBridge {
  std::decay_t<FuncPtr> method;
  std::string_view name;

  template<typename... Args>
  friend EventDelegate<Args...>& operator+=(EventDelegate<Args...>& delegate_obj, const StaticBinderBridge& bridge) {
    return delegate_obj += std::make_pair(bridge.method, bridge.name);
  }

  template<typename... Args>
  friend EventDelegate<Args...>& operator-=(EventDelegate<Args...>& delegate_obj, const StaticBinderBridge& bridge) {
    // 采用双重转换，彻底兼容静态成员函数指针与全局普通函数指针，消除全部编译警告
    auto id = reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*>(bridge.method));
    return delegate_obj -= std::make_pair(nullptr, id);
  }
};

#define STATIC_DELEGATE(func_ptr)          \
  StaticBinderBridge<decltype(func_ptr)> { \
    func_ptr, #func_ptr                    \
  }
