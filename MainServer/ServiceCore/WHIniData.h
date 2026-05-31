#pragma once

#include "ServiceCoreHead.h"
#include "SimpleIni.h"

//////////////////////////////////////////////////////////////////////////////////

// 配置数据
class SERVICE_CORE_CLASS CWHIniData {
public:
  // 构造函数
  CWHIniData() = default;
  // 析构函数
  virtual ~CWHIniData() = default;

  // 路径函数
  // 设置路径
  bool SetIniFilePath(const std::filesystem::path& ini_file);

  // 数据读取
  // 读取矩形
  bool ReadRect(RECT& rect, LPCTSTR item, LPCTSTR sub_item);
  // 读取尺寸
  bool ReadSize(SIZE& size, LPCTSTR item, LPCTSTR sub_item);
  // 读取坐标
  bool ReadPoint(POINT& point, LPCTSTR item, LPCTSTR sub_item);
  // 读取颜色
  bool ReadColor(COLORREF& color, LPCTSTR item, LPCTSTR sub_item);

  // 常规读取
  // 读取数值
  UINT ReadInt(LPCTSTR item, LPCTSTR sub_item, INT def);
  // 读取字符
  LPCTSTR ReadString(LPCTSTR item, LPCTSTR sub_item, LPCTSTR def, LPTSTR text, WORD max_count);
  // 读取字符
  LPCTSTR ReadEncryptString(LPCTSTR item, LPCTSTR sub_item, LPCTSTR def, LPTSTR text, WORD max_count);

protected:
  // 转换数值
  LONG SwitchStringToValue(LPCTSTR& text);

  // 变量定义
  CSimpleIni ini_reader_;
};

//////////////////////////////////////////////////////////////////////////////////
