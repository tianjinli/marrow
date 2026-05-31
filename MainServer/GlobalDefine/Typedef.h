#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <cassert>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <mutex>
#include <queue>
#include <random>
#include <ranges>
#include <shared_mutex>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>

class POSITION {
public:
  POSITION() = default;
  POSITION(std::nullptr_t) {}
  POSITION(const POSITION& other) : self(other.self) {}
  template<typename T>
    requires(std::is_same_v<T, int> || std::is_same_v<T, long>)
  POSITION(T value) : self(nullptr) {
    if (value != 0) {
      throw std::logic_error("POSITION only accepts 0 or NULL");
    }
  };

  template<typename Iterator>
  POSITION(Iterator itr) : self(std::make_shared<Model<Iterator>>(itr)) {}

  operator bool() const { return !empty(); }

  template<typename Iterator>
  Iterator& get() {
    if (empty()) {
      throw std::logic_error("POSITION::get() called on empty POSITION");
    }
    return static_cast<Model<Iterator>&>(*self).itr;
  }

  bool empty() const { return !self; }

  // -----------------------------
  // operator=
  // -----------------------------
  POSITION& operator=(std::nullptr_t) {
    self.reset();
    return *this;
  }

  POSITION& operator=(const POSITION& other) {
    self = other.self;
    return *this;
  }

  template<typename Iterator>
  POSITION& operator=(Iterator itr) {
    self = std::make_shared<Model<Iterator>>(itr);
    return *this;
  }

  template<typename T>
    requires(std::is_same_v<T, int> || std::is_same_v<T, long>)
  POSITION& operator=(T value) {
    if (value != 0) {
      throw std::logic_error("POSITION only accepts 0 or NULL");
    }

    self.reset();
    return *this;
  };

  // -----------------------------
  // == nullptr
  // -----------------------------
  bool operator==(std::nullptr_t) const { return empty(); }
  bool operator!=(std::nullptr_t) const { return !empty(); }

  // -----------------------------
  // == POSITION
  // -----------------------------
  bool operator==(const POSITION& other) const { return self == other.self; }

  bool operator!=(const POSITION& other) const { return !(*this == other); }

  // -----------------------------
  // == iterator
  // -----------------------------
  template<typename Iterator>
    requires(!std::is_integral_v<Iterator>)
  bool operator==(const Iterator& other) const {
    if (empty()) {
      return false;
    }
    auto* model = dynamic_cast<Model<Iterator>*>(self.get());
    if (!model) {
      return false;
    }
    return model->itr == other;
  }

  template<typename Iterator>
    requires(!std::is_integral_v<Iterator>)
  bool operator!=(const Iterator& other) const {
    return !(*this == other);
  }

  // -----------------------------
  // == NULL (0/0L)
  // -----------------------------

  template<typename T>
    requires(std::is_same_v<T, int> || std::is_same_v<T, long>)
  bool operator==(T value) const {
    if (value != 0) {
      throw std::logic_error("POSITION only accepts 0 or NULL");
    }

    return *this == nullptr;
  }

  template<typename T>
    requires(std::is_same_v<T, int> || std::is_same_v<T, long>)
  bool operator!=(T value) const {
    if (value != 0) {
      throw std::logic_error("POSITION only accepts 0 or NULL");
    }

    return *this != nullptr;
  }

private:
  struct Concept {
    virtual ~Concept() = default;
  };

  template<typename Iterator>
  struct Model : Concept {
    explicit Model(Iterator itr) : itr(itr) {}
    Iterator itr;
  };

  std::shared_ptr<Concept> self;
};

#if defined(_WIN32)
#include <crtdbg.h>
#include <objbase.h>
#include <ws2def.h>
#define ASSERT _ASSERT
// 屏蔽 StartService 宏定义
#undef StartService
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sqltypes.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
} RECT;

typedef struct {
  int32_t x;
  int32_t y;
} POINT;

typedef struct {
  int32_t cx;
  int32_t cy;
} SIZE;

typedef struct {
  int16_t wYear;
  int16_t wMonth;
  int16_t wDayOfWeek;
  int16_t wDay;
  int16_t wHour;
  int16_t wMinute;
  int16_t wSecond;
  int16_t wMilliseconds;
} SYSTEMTIME;

typedef struct {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID;

using VOID = void;
using FLOAT = float;
using DOUBLE = double;
using INT_PTR = intptr_t;
using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using LONGLONG = int64_t;
using ULONGLONG = uint64_t;
using INT = int32_t;
using UINT = uint32_t;
using LPCTSTR = const TCHAR*;
using LPBYTE = uint8_t*;
using COLORREF = uint32_t;
using LONG = long;
using ULONG = unsigned long;
using HRESULT = int;
using LPARAM = uint64_t;
using WPARAM = uint64_t;
using LPVOID = void*;
using REFGUID = const GUID&;

constexpr bool TRUE = true;
constexpr bool FALSE = false;
constexpr DWORD MAX_PATH = 260;
constexpr DWORD INFINITE = 0xffffffff;

#ifdef DEBUG
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr) ((void) 0)
#endif

#define interface struct
#define TEXT(quote) quote
#define CONTAINING_RECORD(address, type, field) ((type*) ((char*) (address) - offsetof(type, field)))

#define MoveMemory(Destination, Source, Length) memmove((Destination), (Source), (Length))
#define CopyMemory(Destination, Source, Length) memcpy((Destination), (Source), (Length))
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#define RGB(r, g, b) ((COLORREF) (((BYTE) (r) | ((WORD) ((BYTE) (g)) << 8)) | (((DWORD) (BYTE) (b)) << 16)))

#define MAKEWORD(a, b) ((WORD) (((BYTE) (((DWORD) (a)) & 0xff)) | ((WORD) ((BYTE) (((DWORD) (b)) & 0xff))) << 8))
#define MAKELONG(a, b) ((LONG) (((WORD) (((DWORD) (a)) & 0xffff)) | ((DWORD) ((WORD) (((DWORD) (b)) & 0xffff))) << 16))
#define LOWORD(l) ((WORD) (((DWORD) (l)) & 0xffff))
#define HIWORD(l) ((WORD) ((((DWORD) (l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE) (((DWORD) (w)) & 0xff))
#define HIBYTE(w) ((BYTE) ((((DWORD) (w)) >> 8) & 0xff))

#endif
