#!/usr/bin/env bash
set -e  # 任一步出错直接退出

sysOS=$(uname -s)

if [ "$sysOS" = "Darwin" ]; then
  echo "[INFO] macOS 系统，安装 GCC 13..."
  brew update
  brew install gcc@13
  echo "[INFO] GCC 版本:"
  gcc-13 --version

elif [ "$sysOS" = "Linux" ]; then
  # 判断 apt-get 命令是否存在
  if command -v apt-get >/dev/null 2>&1; then
    echo "[INFO] Ubuntu/Debian 系统，安装 g++-13..."
    sudo apt-get -y update
    sudo apt-get -y install zip pkg-config g++-13 libtool # libstdc++-static
    echo "[INFO] GCC 版本:"
    g++-13 --version
  else
    echo "[INFO] CentOS/RHEL 系统，安装 devtoolset-13..."
    sudo yum -y install centos-release-scl
    sudo yum -y install devtoolset-13
    sudo scl enable devtoolset-13 bash
    sudo yum -y install zip pkg-config libtool # libstdc++-static
    echo "[INFO] GCC 版本:"
    gcc --version
  fi
else
  echo "[ERROR] 未知系统类型: $sysOS"
  exit 1
fi

echo "[INFO] 编译器安装完成，已支持 C++20"
