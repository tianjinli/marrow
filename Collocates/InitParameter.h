#pragma once

#include "GlobalDefine/Platform.h"
//////////////////////////////////////////////////////////////////////////////////

// 配置参数
class CInitParameter {
  // 函数定义
public:
  // 构造函数
  CInitParameter();
  // 析构函数
  virtual ~CInitParameter() = default;

  // 功能函数
public:
  // 生成配置文件模板
  static void GenerateParameterFile(const std::string& ini_file = std::string());

  // 加密重写配置文件
  static void EncryptParameterFile(const std::string& ini_file = std::string());
  // 解密重写配置文件
  static void DecryptParameterFile(const std::string& ini_file = std::string());

  // 局部更新配置文件
  static void UpdateParameterFile(const std::string& key_value, const std::string& ini_file = std::string());

  // 注解查看配置文件
  static void AnnotateParameterComment(const std::string& ini_file = std::string());
  // 明文查看配置文件
  static void InspectParameterFile(const std::string& ini_file = std::string());
};

//////////////////////////////////////////////////////////////////////////////////
