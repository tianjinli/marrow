#!/usr/bin/env bash
set -euo pipefail

#::! 建议在 wsl2 下执行，省去了拷贝的麻烦

# 确保输出支持 UTF-8
export LANG=C.UTF-8
export LC_ALL=C.UTF-8

workDir="$(cd "$(dirname "$0")" && pwd -P)"
vcpkgDir="$workDir/vcpkg"
vcpkgExe="$vcpkgDir/vcpkg"
PATH="$PATH:$vcpkgDir"

linuxDir="$vcpkgDir/installed/x64-linux/"
debugSrc="$linuxDir/debug/lib"
releaseSrc="$linuxDir/lib"
includeSrc="$linuxDir/include"

debugDest="$workDir/lib/linux/Debug"
releaseDest="$workDir/lib/linux/Release"
includeDest="$workDir/include/linux"

# 检查 vcpkg 是否已存在
maxAgeDays=30
if [ -f "$vcpkgExe" ]; then
  echo -e "\033[32m[INFO] 检测到 vcpkg 已编译完成，跳过克隆与编译\033[0m"
  
  # 获取文件最后修改时间（单位：秒）
  lastWrite=$(stat -c %Y "$vcpkgExe")
  ageSec=$(($(date +%s) - lastWrite))
  ageDays=$((ageSec / 86400))
  
  if [ "$ageDays" -gt "$maxAgeDays" ]; then
    echo -e "\033[33m[INFO] vcpkg.exe 已存在但过期（${ageDays} 天），开始拉取并重新编译\033[0m"
    git -C "$vcpkgDir" pull
    "$vcpkgDir/bootstrap-vcpkg.sh"
  else
    echo -e "\033[32m[INFO] vcpkg.exe 已存在且较新（${ageDays} 天），跳过拉取与重新编译\033[0m"
  fi
else
  echo -e "\033[33m[INFO] vcpkg 不存在或未编译，开始克隆并编译...\033[0m"
  if ! git -C "$vcpkgDir" rev-parse --is-inside-work-tree > /dev/null 2>&1; then
    git clone https://github.com/Microsoft/vcpkg.git "$vcpkgDir"
  fi
  "$vcpkgDir/bootstrap-vcpkg.sh"
fi

# 安装依赖
vcpkg install asio:x64-linux fmt:x64-linux spdlog:x64-linux zlib:x64-linux --feature-flags=classic

# 清理库文件和头文件
echo -e "\033[34m[INFO] ⚠️ 清理库文件和头文件...\033[0m"
rm -rf "$debugDest" "$releaseDest" "$includeDest"
echo -e "\033[32m[INFO] ⚠️ 清理库文件和头文件完成\033[0m"

# 拷贝库文件和头文件
echo -e "\033[34m[INFO] 🔚 拷贝库文件和头文件...\033[0m"
mkdir -p "$debugDest" "$releaseDest" "$includeDest"

# 定义复制映射表：源文件 → 目标文件
declare -A copyMap=(
    ["$debugSrc/libz.a"]="$debugDest/libz.a"
    ["$releaseSrc/libz.a"]="$releaseDest/libz.a"
)
# 执行复制操作
for src in "${!copyMap[@]}"; do
    dest="${copyMap[$src]}"
    if [ -f "$src" ]; then
        mkdir -p "$(dirname "$dest")"
        cp -f "$src" "$dest"
        echo -e "\033[32m[INFO] 复制文件: $src → $dest\033[0m"
    else
        echo -e "\033[33m[WARN] 源文件不存在: $src\033[0m"
    fi
done
cp -r "$includeSrc/"* "$includeDest/"
echo -e "\033[32m[INFO] 🔚 拷贝库文件和头文件完成\033[0m"

if [ -d "$debugPkg" ]; then
    rm -rf "$debugPkg"
fi

if [ -d "$releasePkg" ]; then
    rm -rf "$releasePkg"
fi
