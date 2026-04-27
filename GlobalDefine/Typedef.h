#pragma once

#ifdef _WIN32
#include <fmt/xchar.h>
#endif
#include <fmt/format.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#if defined(_WIN32)
#include <crtdbg.h>
#include <objbase.h>
#define ASSERT _ASSERT
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sqltypes.h>
#include <sys/socket.h>
#include <unistd.h>

#define VOID void
#define WCHAR wchar_t
#define CONST const
#define FLOAT float
#define DOUBLE double
#define INT_PTR int
#define interface struct
#define BYTE uint8_t
#define WORD uint16_t
#define DWORD uint32_t
#define LONGLONG int64_t
#define ULONGLONG uint64_t
#define INT int32_t
#define UINT uint32_t
#define TCHAR char
#define TRUE true
#define FALSE false

#define TEXT(quote) quote
// typedef char *LPTSTR;
// typedef const char *LPCTSTR;
using LPTSTR = char*;
using LPCTSTR = const char*;
typedef DWORD COLORREF;
typedef long LONG;
typedef unsigned long ULONG;
typedef int HRESULT;
typedef uint64_t LPARAM;
typedef uint64_t WPARAM;
#define MAX_PATH 260
#define INFINITE ((DWORD)0xffffffff)
#define CONTAINING_RECORD(address, type, field) ((type*)((char*)(address) - offsetof(type, field)))

#ifdef DEBUG
#define ASSERT(f) assert(f)
#else
#define ASSERT(expr) ((void)0)
#endif

typedef struct {
  LONG left;
  LONG top;
  LONG right;
  LONG bottom;
} RECT;

typedef struct {
  LONG x;
  LONG y;
} POINT;

typedef struct {
  LONG cx;
  LONG cy;
} SIZE;

typedef struct {
  WORD wYear;
  WORD wMonth;
  WORD wDayOfWeek;
  WORD wDay;
  WORD wHour;
  WORD wMinute;
  WORD wSecond;
  WORD wMilliseconds;
} SYSTEMTIME;

typedef struct {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t Data4[8];
} GUID;
#define REFGUID const GUID&

#define MoveMemory(Destination, Source, Length) memmove((Destination), (Source), (Length))
#define CopyMemory(Destination, Source, Length) memcpy((Destination), (Source), (Length))
#define ZeroMemory(Destination, Length) memset((Destination), 0, (Length))
#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))

#define MAKEWORD(a, b) ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#define MAKELONG(a, b) ((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#define LOWORD(l) ((WORD)(((DWORD)(l)) & 0xffff))
#define HIWORD(l) ((WORD)((((DWORD)(l)) >> 16) & 0xffff))
#define LOBYTE(w) ((BYTE)(((DWORD)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD)(w)) >> 8) & 0xff))

#endif
