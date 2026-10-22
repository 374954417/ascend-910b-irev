#!/bin/bash
# 一键:编译测试 kernel + 反汇编
set -e
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl"
BASE="--cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang -include $A/../include/ascendc/asc_devkit_version.h"
SRC_DIR=$(cd "$(dirname "$0")" && pwd)
OUT=/tmp/test_progs
mkdir -p $OUT $SRC_DIR/dis

$A/ccec_compiler/bin/bisheng --cce-aicore-arch=dav-c220-cube $BASE $INCS -c $SRC_DIR/test_cube.cpp -o $OUT/test_cube.o
$A/ccec_compiler/bin/ld.lld -m aicorelinux -Ttext=0 $OUT/test_cube.o -o $OUT/test_cube.elf
$A/ccec_compiler/bin/bisheng --cce-aicore-arch=dav-c220-vec $BASE $INCS -c $SRC_DIR/test_vec.cpp -o $OUT/test_vec.o
$A/ccec_compiler/bin/ld.lld -m aicorelinux -Ttext=0 $OUT/test_vec.o -o $OUT/test_vec.elf

python3 $SRC_DIR/../irev/msdisasm.py $OUT/test_cube.elf > $SRC_DIR/dis/test_cube.dis.txt
python3 $SRC_DIR/../irev/msdisasm.py $OUT/test_vec.elf  > $SRC_DIR/dis/test_vec.dis.txt
echo "done -> $SRC_DIR/dis/"
