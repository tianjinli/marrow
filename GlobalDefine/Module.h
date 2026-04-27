#pragma once

#include "Typedef.h"

#ifdef _WIN32
#define DYNLIB_HANDLE HINSTANCE
#define DYNLIB_LOAD(a) LoadLibraryEx(a, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH)
#define DYNLIB_GETSYM(a, b) GetProcAddress(a, b)
#define DYNLIB_UNLOAD(a) FreeLibrary(a)
#define DYNLIB_ERROR() GetLastError()
#define DYNLIB_EXPORT __declspec(dllexport)
#else
#include <dlfcn.h>
#include <errno.h>
#define DYNLIB_HANDLE void*
#define DYNLIB_LOAD(a) dlopen(a, RTLD_LAZY | RTLD_GLOBAL)
#define DYNLIB_GETSYM(a, b) dlsym(a, b)
#define DYNLIB_UNLOAD(a) dlclose(a)
#define DYNLIB_ERROR() errno
#define DYNLIB_EXPORT
#endif // _WIN32

//////////////////////////////////////////////////////////////////////////////////
// 模块接口

#define VER_IUnknownEx INTERFACE_VERSION(1, 1)
static const GUID IID_IUnknownEx = {0x5feec21e, 0xdbf3, 0x46f0, {0x9f, 0x57, 0xd1, 0xcd, 0x71, 0x1c, 0x46, 0xde}};

// 基础接口
interface IUnknownEx {
  // 释放对象
  virtual VOID Release() = 0;
  // 接口查询
  virtual VOID* QueryInterface(REFGUID Guid, DWORD dwQueryVer) = 0;
};

//////////////////////////////////////////////////////////////////////////////////
// 版本比较

// 产品版本
#define BUILD_VER 0 // 授权版本
#define PRODUCT_VER 6 // 产品版本

// 接口版本
#define INTERFACE_VERSION(cbMainVer, cbSubVer) \
  (DWORD)((((BYTE) (PRODUCT_VER)) << 24) + (((BYTE) (cbMainVer)) << 16) + ((BYTE) (cbSubVer) << 8)) + ((BYTE) (BUILD_VER))

// 模块版本
#define PROCESS_VERSION(cbMainVer, cbSubVer, cbBuildVer) \
  (DWORD)((((BYTE) (PRODUCT_VER)) << 24) + (((BYTE) (cbMainVer)) << 16) + ((BYTE) (cbSubVer) << 8) + (BYTE) (cbBuildVer))

// 产品版本
inline BYTE GetProductVer(DWORD dwVersion) { return ((BYTE*) &dwVersion)[3]; }

// 主要版本
inline BYTE GetMainVer(DWORD dwVersion) { return ((BYTE*) &dwVersion)[2]; }

// 次要版本
inline BYTE GetSubVer(DWORD dwVersion) { return ((BYTE*) &dwVersion)[1]; }

// 编译版本
inline BYTE GetBuildVer(DWORD dwVersion) { return ((BYTE*) &dwVersion)[0]; }

// 版本比较
inline bool InterfaceVersionCompare(DWORD dwQueryVer, DWORD dwInterfaceVer) {
  if (GetSubVer(dwQueryVer) > GetSubVer(dwInterfaceVer))
    return false;
  if (GetMainVer(dwQueryVer) != GetMainVer(dwInterfaceVer))
    return false;
  if (GetBuildVer(dwQueryVer) != GetBuildVer(dwInterfaceVer))
    return false;
  return GetProductVer(dwQueryVer) == GetProductVer(dwInterfaceVer);
};

//////////////////////////////////////////////////////////////////////////////////
// 内部接口查询

// 查询接口
#define QUERYINTERFACE(Interface, Guid, dwQueryVer)                                                                          \
  do {                                                                                                                       \
    if ((std::memcmp(&Guid, &IID_##Interface, sizeof(GUID)) == 0) && (InterfaceVersionCompare(dwQueryVer, VER_##Interface))) \
      return static_cast<Interface*>(this);                                                                                  \
  } while (false)

// 查询接口
#define QUERYINTERFACE_IUNKNOWNEX(BaseInterface, Guid, dwQueryVer)                                                         \
  do {                                                                                                                     \
    if ((std::memcmp(&Guid, &IID_IUnknownEx, sizeof(GUID)) == 0) && (InterfaceVersionCompare(dwQueryVer, VER_IUnknownEx))) \
      return static_cast<IUnknownEx*>(static_cast<BaseInterface*>(this));                                                  \
  } while (false)

//////////////////////////////////////////////////////////////////////////////////
// 外部接口查询

// 查询接口
#define QUERY_ME_INTERFACE(Interface) ((Interface*) QueryInterface(IID_##Interface, VER_##Interface))

// 查询接口
#define QUERY_OBJECT_INTERFACE(Object, Interface) ((Interface*) Object.QueryInterface(IID_##Interface, VER_##Interface))

// 查询接口
#define QUERY_OBJECT_PTR_INTERFACE(Object, Interface) (Object ? (Interface*) Object->QueryInterface(IID_##Interface, VER_##Interface) : nullptr)

//////////////////////////////////////////////////////////////////////////////////
// 组件模板辅助模板

// 组件创建函数
typedef VOID*(ModuleCreateProc) (REFGUID Gudi, DWORD dwInterfaceVer);

// 组件辅助类模板
template<typename IModuleInterface>
class CTemplateHelper {
  // 接口属性
public:
  REFGUID guid_; // 接口标识
  const DWORD version_; // 接口版本

  // 组件属性
public:
  std::string create_proc_name_; // 创建函数
  StringT module_dll_name_; // 组件名字

  // 内核变量
public:
  DYNLIB_HANDLE dll_instance_ = nullptr; // DLL 句柄
  IModuleInterface* module_interface_ = nullptr; // 模块接口

  // 辅助变量
public:
  DWORD error_code_ = 0; // 错误代码
  StringT error_describe_; // 错误描述

  // 函数定义
public:
  // 构造函数
  CTemplateHelper(REFGUID guid, DWORD version);
  // 构造函数
  CTemplateHelper(REFGUID guid, DWORD version, LPCTSTR module_dll_name, const char* create_proc_name);
  // 析构函数
  virtual ~CTemplateHelper();

  // 管理函数
public:
  // 释放组件
  bool CloseInstance();
  // 创建函数
  bool CreateInstance();

  // 配置函数
public:
  // 创建信息
  VOID SetModuleCreateInfo(LPCTSTR module_dll_name, const char* create_proc);

  // 辅助函数
public:
  // 获取错误
  inline const StringT& GetErrorDescribe() const;
  // 指针重载
  inline IModuleInterface* operator->() const;
  // 获取接口
  inline IModuleInterface* GetInterface() const;
};

//////////////////////////////////////////////////////////////////////////////////
// CTemplateHelper<IModuleInterface> 外联函数

// 构造函数
template<typename IModuleInterface>
CTemplateHelper<IModuleInterface>::CTemplateHelper(REFGUID guid, DWORD version) : guid_(guid), version_(version) {}

// 构造函数
template<typename IModuleInterface>
CTemplateHelper<IModuleInterface>::CTemplateHelper(REFGUID guid, DWORD version, LPCTSTR module_dll_name, const char* create_proc_name) :
    guid_(guid), version_(version), create_proc_name_(create_proc_name), module_dll_name_(module_dll_name) {}

// 析构函数
template<typename IModuleInterface>
CTemplateHelper<IModuleInterface>::~CTemplateHelper() {
  CloseInstance();
}

#ifdef _WIN32
static StringT ConvertToStringT(const std::string& proc_name) {
  StringT proc_name_t;
  proc_name_t.reserve(proc_name.size());
  for (const auto name: proc_name) {
    proc_name_t += static_cast<TCHAR>(name);
  }
  return proc_name_t;
}
#define ToProcNameT(proc_name) ConvertToStringT(proc_name)
#else
#define ToProcNameT(proc_name) (proc_name)
#endif

// 创建组件
template<typename IModuleInterface>
bool CTemplateHelper<IModuleInterface>::CreateInstance() {
  // 释放组件
  CloseInstance();

  // 加载模块
  dll_instance_ = DYNLIB_LOAD(module_dll_name_.c_str());
  if (dll_instance_ == nullptr) {
    error_describe_ = fmt::format(TEXT("“{}”模块加载失败"), module_dll_name_);
    return false;
  }

  // 寻找函数
  const auto create_proc = (ModuleCreateProc*) DYNLIB_GETSYM(dll_instance_, create_proc_name_.c_str());
  if (create_proc == nullptr) {
    error_describe_ = fmt::format(TEXT("找不到组件创建函数“{}”: {}"), ToProcNameT(create_proc_name_), DYNLIB_ERROR());
    return false;
  }

  // 创建组件
  module_interface_ = (IModuleInterface*) create_proc(guid_, version_);
  if (module_interface_ == nullptr) {
    error_describe_ = fmt::format(TEXT("调用函数“{}”生成对象失败"), ToProcNameT(create_proc_name_));
    return false;
  }

  return true;
}

// 释放组件
template<typename IModuleInterface>
bool CTemplateHelper<IModuleInterface>::CloseInstance() {
  // 销毁对象
  if (module_interface_ != nullptr) {
    module_interface_->Release();
    module_interface_ = nullptr;
  }

  // 释放 DLL
  if (dll_instance_ != nullptr) {
    DYNLIB_UNLOAD(dll_instance_);
    dll_instance_ = nullptr;
  }

  return true;
}

// 创建信息
template<typename IModuleInterface>
inline VOID CTemplateHelper<IModuleInterface>::SetModuleCreateInfo(LPCTSTR module_dll_name, const char* create_proc) {
  // 设置信息
  create_proc_name_ = std::string(create_proc);
  module_dll_name_ = StringT(module_dll_name);
}

//////////////////////////////////////////////////////////////////////////////////
// CTemplateHelper<IModuleInterface> 内联函数

// 获取描述
template<typename IModuleInterface>
inline const StringT& CTemplateHelper<IModuleInterface>::GetErrorDescribe() const {
  return error_describe_;
}

// 指针重载
template<typename IModuleInterface>
inline IModuleInterface* CTemplateHelper<IModuleInterface>::operator->() const {
  return GetInterface();
}

// 获取接口
template<typename IModuleInterface>
inline IModuleInterface* CTemplateHelper<IModuleInterface>::GetInterface() const {
  return module_interface_;
}

//////////////////////////////////////////////////////////////////////////////////
// 组件辅助宏

// 组件创建函数
#define DECLARE_CREATE_MODULE(OBJECT_NAME)                                                 \
  extern "C" DYNLIB_EXPORT VOID* Create##OBJECT_NAME(REFGUID Guid, DWORD dwInterfaceVer) { \
    C##OBJECT_NAME* p##OBJECT_NAME = nullptr;                                              \
    try {                                                                                  \
      p##OBJECT_NAME = new C##OBJECT_NAME();                                               \
      if (p##OBJECT_NAME == nullptr)                                                       \
        throw TEXT("对象创建失败");                                                        \
      VOID* pObject = p##OBJECT_NAME->QueryInterface(Guid, dwInterfaceVer);                \
      if (pObject == nullptr)                                                              \
        throw TEXT("接口查询失败");                                                        \
      return pObject;                                                                      \
    } catch (...) {                                                                        \
    }                                                                                      \
    SafeDelete(p##OBJECT_NAME);                                                            \
    return nullptr;                                                                        \
  }

// 组件辅助类宏
#define DECLARE_MODULE_DYNAMIC(OBJECT_NAME)                                                                \
  class C##OBJECT_NAME##Helper : public CTemplateHelper<I##OBJECT_NAME> {                                 \
  public:                                                                                                  \
    C##OBJECT_NAME##Helper() : CTemplateHelper<I##OBJECT_NAME>(IID_I##OBJECT_NAME, VER_I##OBJECT_NAME) {} \
  };

// 组件辅助类宏
#define DECLARE_MODULE_HELPER(OBJECT_NAME, MODULE_DLL_NAME, CREATE_FUNCTION_NAME)                                                                \
  class C##OBJECT_NAME##Helper : public CTemplateHelper<I##OBJECT_NAME> {                                                                        \
  public:                                                                                                                                        \
    C##OBJECT_NAME##Helper() : CTemplateHelper<I##OBJECT_NAME>(IID_I##OBJECT_NAME, VER_I##OBJECT_NAME, MODULE_DLL_NAME, CREATE_FUNCTION_NAME) {} \
  };

//////////////////////////////////////////////////////////////////////////////////
