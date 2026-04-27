#include "InitParameter.h"

#include <ServiceCore/WHEncrypt.h>
#include <ServiceCore/WHService.h>

// 可能存在头文件引用污染需要重新包含
#include <filesystem>
#include <fstream>
#include <iostream>

static StringT sections[]{TEXT("ServerInfo"), TEXT("PlatformDB"), TEXT("TreasureDB"), TEXT("AccountsDB")};

static StringT all_sections[]{TEXT("Correspond"), TEXT("LogonServer"), TEXT("ServerInfo"),
                              TEXT("PlatformDB"), TEXT("TreasureDB"),  TEXT("AccountsDB")};
static std::map<StringT, std::map<StringT, StringT>> all_comments{
    {TEXT("Correspond"),
     {
         {TEXT("ServicePort"), TEXT("协调服务器端口号")},
         {TEXT("ConnectMax"), TEXT("协调服务器最大连接数")},
     }},
    {TEXT("LogonServer"),
     {
         {TEXT("ServicePort"), TEXT("登录服务器端口号")},
         {TEXT("ConnectMax"), TEXT("登录服务器最大连接数")},
         {TEXT("DelayList"), TEXT("是否开启延时列表（0 关闭 1 开启）")},
     }},
    {TEXT("ServerInfo"),
     {
         {TEXT("ServiceName"), TEXT("当前服务器名称（允许中文）")},
         {TEXT("ServiceAddr"), TEXT("当前服务器 IP 地址")},
         {TEXT("CorrespondAddr"), TEXT("协调服务器 IP 地址")},
     }},
    {TEXT("PlatformDB"),
     {
         {TEXT("DBAddr"), TEXT("平台数据库 IP 地址（默认 localhost）")},
         {TEXT("DBPort"), TEXT("平台数据库端口号（默认 1433）")},
         {TEXT("DBUser"), TEXT("平台数据库用户名（默认 sa）")},
         {TEXT("DBPass"), TEXT("平台数据库密码")},
     }},
    {TEXT("TreasureDB"),
     {
         {TEXT("DBAddr"), TEXT("金币数据库 IP 地址（默认 localhost）")},
         {TEXT("DBPort"), TEXT("金币数据库端口号（默认 1433）")},
         {TEXT("DBUser"), TEXT("金币数据库用户名（默认 sa）")},
         {TEXT("DBPass"), TEXT("金币数据库密码")},
     }},
    {TEXT("AccountsDB"),
     {
         {TEXT("DBAddr"), TEXT("账号数据库 IP 地址（默认 localhost）")},
         {TEXT("DBPort"), TEXT("账号数据库端口号（默认 1433）")},
         {TEXT("DBUser"), TEXT("账号数据库用户名（默认 sa）")},
         {TEXT("DBPass"), TEXT("账号数据库密码")},
     }},
};

// 构造函数
CInitParameter::CInitParameter() {}

static std::filesystem::path GetFullPath(const std::string& ini_file_path) {
  // 如果传入有效路径，直接返回
  std::filesystem::path full_path = ToSimpleStringT(ini_file_path);
  if (!ini_file_path.empty()) {
    if (std::filesystem::is_directory(full_path)) {
      full_path /= TEXT("ServerParameter.ini");
    }
  } else {
    // 获取路径
    TCHAR work_dir[MAX_PATH] = TEXT("");
    CWHService::GetWorkDirectory(work_dir, std::size(work_dir));

    // 用 filesystem 拼接路径，比手动 snprintf 更可靠
    full_path = std::filesystem::path(work_dir) / TEXT("ServerParameter.ini");
  }
  std::cout << "配置文件路径：" << ToSimpleUtf8(full_path) << std::endl;
  return full_path;
}

void CInitParameter::GenerateParameterFile(const std::string& ini_file) {
  const auto full_path = GetFullPath(ini_file);
  if (std::filesystem::exists(full_path)) {
    do {
      std::cout << "配置文件已存在，是否替换？（y/n）";
      std::string input;
      std::cin >> input;
      if (input == "y" || input == "Y") {
        break;
      } else if (input == "n" || input == "N") {
        return;
      }
    } while (true);
  }

  std::ofstream ini_stream(full_path);
  if (ini_stream.is_open()) {
    uint8_t bom[] = {0xEF, 0xBB, 0xBF};
    ini_stream.write(reinterpret_cast<const char*>(bom), sizeof(bom));
    for (auto& section : all_sections) {
      ini_stream << fmt::format(FMT_STRING("[{}]"), ToSimpleUtf8(section)) << std::endl;
      for (auto& comment : all_comments[section]) {
        ini_stream << fmt::format(FMT_STRING("{}="), ToSimpleUtf8(comment.first)) << std::endl;
      }
      ini_stream << std::endl;
    }
    ini_stream.close();
  }

  std::cout << "默认配置文件已生成" << std::endl;
}

void CInitParameter::EncryptParameterFile(const std::string& ini_file) {
  const std::filesystem::path full_path = GetFullPath(ini_file);
  if (!std::filesystem::exists(full_path)) {
    std::cout << "配置文件不存在" << std::endl;
    return;
  }

  // 读取配置
  CSimpleIni ini_handler;
  ini_handler.LoadFile(full_path.c_str());
  ini_handler.SetSpaces(false);
  // 读取所有 section
  for (auto& section : sections) {
    std::list<CSimpleIni::Entry> names;
    ini_handler.GetAllKeys(section.c_str(), names);
    std::cout << ToSimpleUtf8(fmt::format(TEXT("[{}]"), section)) << std::endl;
    for (auto& name : names) {
      if (std::filesystem::path(name.pItem) == TEXT("DBPort")) {
        continue;  // 跳过不需要加密的数据
      }

      StringT value = ini_handler.GetValue(section.c_str(), name.pItem);
      TCHAR encrypt_data[MAX_PATH];
      if (CWHEncrypt::XorCrevasse(value.c_str(), encrypt_data, std::size(encrypt_data))) {
        continue;  // 跳过已加密的数据
      }
      if (CWHEncrypt::XorEncrypt(value.c_str(), encrypt_data, std::size(encrypt_data))) {
        ini_handler.SetValue(section.c_str(), name.pItem, encrypt_data);
        std::cout << ToSimpleUtf8(fmt::format(TEXT("{}={}"), name.pItem, encrypt_data)) << std::endl;
      }
    }
    std::cout << std::endl;
  }
  (void)ini_handler.SaveFile(full_path.c_str());
}

void CInitParameter::DecryptParameterFile(const std::string& ini_file) {
  const auto full_path = GetFullPath(ini_file);
  if (!std::filesystem::exists(full_path)) {
    std::cout << "配置文件不存在" << std::endl;
    return;
  }

  CSimpleIni ini_handler;
  ini_handler.LoadFile(full_path.c_str());
  ini_handler.SetSpaces(false);
  // 读取所有 section
  for (auto& section : sections) {
    std::list<CSimpleIni::Entry> names;
    ini_handler.GetAllKeys(section.c_str(), names);
    std::cout << ToSimpleUtf8(fmt::format(TEXT("[{}]"), section)) << std::endl;
    for (auto& name : names) {
      StringT value = ini_handler.GetValue(section.c_str(), name.pItem);
      TCHAR source_data[MAX_PATH];
      if (CWHEncrypt::XorCrevasse(value.c_str(), source_data, std::size(source_data))) {
        ini_handler.SetValue(section.c_str(), name.pItem, source_data);
        std::cout << ToSimpleUtf8(fmt::format(TEXT("{}={}"), name.pItem, source_data)) << std::endl;
      }
    }
    std::cout << std::endl;
  }
  (void)ini_handler.SaveFile(full_path.c_str());
}

void CInitParameter::UpdateParameterFile(const std::string& key_value, const std::string& ini_file) {
  const auto full_path = GetFullPath(ini_file);
  if (!std::filesystem::exists(full_path)) {
    std::cout << "配置文件不存在" << std::endl;
    return;
  }

  const StringT key_value_t = ToSimpleStringT(key_value);
  const auto key_values = StringUtils::Split(key_value_t, TEXT('='));
  if (key_values.size() != 2) {
    std::cout << "键值对格式错误" << std::endl;
    return;
  }

  StringT value = key_values[1];

  CSimpleIni ini_handler;
  ini_handler.LoadFile(full_path.c_str());
  ini_handler.SetSpaces(false);

  auto update = [&](const StringT& section, const StringT& key) {
    if (ini_handler.KeyExists(section.c_str(), key.c_str())) {
      StringT new_value = StringT(value);
      StringT old_value = ini_handler.GetValue(section.c_str(), key.c_str());
      TCHAR source_data[MAX_PATH];
      if (CWHEncrypt::XorCrevasse(old_value.c_str(), source_data, std::size(source_data))) {
        // 如果之前的数据是加密状态新的数据继续加密写入
        if (CWHEncrypt::XorEncrypt(new_value.c_str(), source_data, std::size(source_data))) {
          new_value = source_data;
        }
      }

      if (ini_handler.SetValue(section.c_str(), key.c_str(), new_value.c_str()) >= 0) {
        std::cout << ToSimpleUtf8(fmt::format(TEXT("[{}]"), section)) << std::endl;
        std::cout << ToSimpleUtf8(fmt::format(TEXT("{}={}"), key, new_value)) << std::endl;
        std::cout << std::endl;
      }
    }
  };

  const auto section_keys = StringUtils::Split(key_values[0], TEXT('.'));
  if (section_keys.size() == 2) {
    // 读取所有 section
    const StringT section = section_keys[0];
    const StringT key = section_keys[1];
    update(section, key);
  } else {
    const StringT key = key_values[0];
    // 读取所有 section
    for (auto& section : all_sections) {
      update(section, key);
    }
  }

  (void)ini_handler.SaveFile(full_path.c_str());
}

void CInitParameter::AnnotateParameterComment(const std::string& ini_file) {
  const auto full_path = GetFullPath(ini_file);
  if (!std::filesystem::exists(full_path)) {
    std::cout << "配置文件不存在" << std::endl;
    return;
  }

  // 读取配置
  CSimpleIni ini_handler;
  ini_handler.LoadFile(full_path.c_str());
  // 读取所有 section
  for (auto& section : all_sections) {
    std::list<CSimpleIni::Entry> names;
    ini_handler.GetAllKeys(section.c_str(), names);
    std::cout << fmt::format("[{}]", ToSimpleUtf8(section)) << std::endl;
    for (auto& name : names) {
      auto comments = all_comments[section];
      if (!comments.empty() && comments.count(name.pItem) > 0) {
        std::cout << fmt::format("{}={}", ToSimpleUtf8(name.pItem), ToSimpleUtf8(comments[name.pItem])) << std::endl;
      } else {
        ASSERT(false);
      }
    }
    std::cout << std::endl;
  }
}

void CInitParameter::InspectParameterFile(const std::string& ini_file) {
  const auto full_path = GetFullPath(ini_file);
  if (!std::filesystem::exists(full_path)) {
    std::cout << "配置文件不存在" << std::endl;
    return;
  }

  CSimpleIni ini_handler;
  ini_handler.LoadFile(full_path.c_str());
  ini_handler.SetSpaces(false);
  std::list<CSimpleIni::Entry> sections;
  ini_handler.GetAllSections(sections);
  // 读取所有 section
  for (auto& section : sections) {
    std::list<CSimpleIni::Entry> names;
    ini_handler.GetAllKeys(section.pItem, names);
    std::cout << ToSimpleUtf8(fmt::format(TEXT("[{}]"), section.pItem)) << std::endl;
    for (auto& name : names) {
      const StringT value = ini_handler.GetValue(section.pItem, name.pItem);
      TCHAR source_data[MAX_PATH];
      if (CWHEncrypt::XorCrevasse(value.c_str(), source_data, std::size(source_data))) {
        std::cout << "\033[31m" << ToSimpleUtf8(fmt::format(TEXT("{}={}"), name.pItem, source_data)) << "\033[0m" << std::endl;
      } else {
        std::cout << ToSimpleUtf8(fmt::format(TEXT("{}={}"), name.pItem, value)) << std::endl;
      }
    }
    std::cout << std::endl;
  }
  (void)ini_handler.SaveFile(full_path.c_str());
}
