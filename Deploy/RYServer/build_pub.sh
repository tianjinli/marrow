#!/bin/bash
set -euo pipefail

# 尽量保证中文文件名可用
export LANG=C.UTF-8
export LC_ALL=C.UTF-8

# 工作目录：脚本所在目录
#WORK_DIR="$(cd "$(dirname "$0")" && pwd)"

SCRIPT_PATH="${BASH_SOURCE[0]}"
while [ -h "$SCRIPT_PATH" ]; do
    WORK_DIR="$(cd -P "$(dirname "$SCRIPT_PATH")" && pwd)"
    SCRIPT_PATH="$(readlink "$SCRIPT_PATH")"
    [[ $SCRIPT_PATH != /* ]] && SCRIPT_PATH="$WORK_DIR/$SCRIPT_PATH"
done
WORK_DIR="$(cd -P "$(dirname "$SCRIPT_PATH")" && pwd)"

BUILD_TYPE=Debug
SRC_DIR="${WORK_DIR}/../.." # CMake 源码目录
BUILD_DIR="${SRC_DIR}/cmake-build-wsl-${BUILD_TYPE,,}" # 构建目录 (全小写)
BIN_DIR="${SRC_DIR}/_Out/bin/${BUILD_TYPE}/RYServer"
PUB_DIR="${SRC_DIR}/_Out/pub/RYServer"

rm -rf $BIN_DIR
rm -rf $PUB_DIR

echo "==> 配置 CMake: $BUILD_DIR"
cmake -S $SRC_DIR -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo "==> 编译所有项目"
cmake --build $BUILD_DIR -j 8

echo "==> 创建发布目录: $PUB_DIR"
mkdir -p $PUB_DIR

echo "==> 拷贝生成的文件到 $PUB_DIR"
for f in $BIN_DIR/*; do
    name=$(basename "$f")
    target="$PUB_DIR/$name"

    if [ ! -e "$target" ]; then
        cp "$f" "$target"
        echo "Copied: $name"
    else
        echo "Skip: $name (already exists)"
    fi
done

echo "==> √完成 $BUILD_TYPE 构建"
