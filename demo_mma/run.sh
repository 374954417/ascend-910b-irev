#!/bin/bash
# 一键构建 + 运行 MMA(cube mad 指令)微基准
# 用法: ./run.sh [R1 R2]   默认 20000 400000(双点斜率法)
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR=/tmp/demo_mma_build

rm -rf "$BUILD_DIR"
cmake -S "$SRC_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j 8

source /usr/local/Ascend/ascend-toolkit/latest/set_env.sh
LD_LIBRARY_PATH="$BUILD_DIR/lib:$LD_LIBRARY_PATH" "$BUILD_DIR/main" "${1:-20000}" "${2:-400000}"
