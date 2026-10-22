#!/bin/bash
# 一键构建 + 运行 add_custom demo 算子
#
# 注意:
# 1. 本目录在 NFS(bucket) 上,cmake try_compile 在 NFS 上会失败,所以 build 目录放在 /tmp
# 2. 必须显式指定 -DCMAKE_BUILD_TYPE=Release,否则工具链的 merge_mix_obj.sh 会因
#    --build-type 参数为空而失败
# 3. 工具链的 merge_obj_text.sh 会"原地"改写 .o(REL->EXEC),导致增量构建必然失败,
#    因此每次构建都做全量清理
set -e

SRC_DIR=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR=/tmp/demo_add_build

rm -rf "$BUILD_DIR"
cmake -S "$SRC_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j 8

source /usr/local/Ascend/ascend-toolkit/latest/set_env.sh
LD_LIBRARY_PATH="$BUILD_DIR/lib:$LD_LIBRARY_PATH" "$BUILD_DIR/main"
