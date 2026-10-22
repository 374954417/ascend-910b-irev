#!/bin/bash
# 标量指令变体批量编译 + 提取 .text
set -e
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
OBJCPY=$A/ccec_compiler/bin/llvm-objcopy
SRC=/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_scalar/sc_one.cpp
OUT=/tmp/irev_scalar
mkdir -p $OUT

INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl"
FLAGS="--cce-aicore-arch=dav-c220-vec --cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang \
-include $A/../include/ascendc/asc_devkit_version.h"

bv() {
    local name=$1; shift
    $BISHENG $FLAGS $INCS "$@" -c $SRC -o $OUT/$name.o 2>/dev/null
    $LD -m aicorelinux -Ttext=0 $OUT/$name.o -o $OUT/$name.elf
    $OBJCPY --dump-section .text=$OUT/$name.text $OUT/$name.elf 2>/dev/null
    echo "built $name ($(stat -c%s $OUT/$name.text)B)"
}

bv base              -DOP=12                    # 恒等基线
bv add1              -DOP=0 -DKONST=1
bv add255            -DOP=0 -DKONST=255
bv add256            -DOP=0 -DKONST=256
bv add4095           -DOP=0 -DKONST=4095
bv add4096           -DOP=0 -DKONST=4096
bv add65535          -DOP=0 -DKONST=65535
bv add65536          -DOP=0 -DKONST=65536
bv addbig            -DOP=0 -DKONST=305419896   # 0x12345678
bv sub1              -DOP=1 -DKONST=1
bv mul3              -DOP=2 -DKONST=3
bv mul33             -DOP=2 -DKONST=33
bv shl1              -DOP=3 -DKONST=1
bv shl4              -DOP=3 -DKONST=4
bv shr3              -DOP=4 -DKONST=3
bv and15             -DOP=5 -DKONST=15
bv or255             -DOP=6 -DKONST=255
bv xor5a             -DOP=7 -DKONST=90          # 0x5a
bv cmp100            -DOP=8 -DKONST=100
bv not               -DOP=9
bv loop4             -DOP=10 -DKONST=4
bv loop100           -DOP=10 -DKONST=100
bv blkid             -DOP=11 -DKONST=1
echo ALL_DONE
