#!/bin/bash
# 第二轮变体:地址选择子 / dtype / 边界值
set -e
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
OBJCPY=$A/ccec_compiler/bin/llvm-objcopy
SRC=/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_mad_format/mad_one.cpp
OUT=/tmp/mad_variants2
mkdir -p $OUT

INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl"
FLAGS="--cce-aicore-arch=dav-c220-cube --cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang \
-include $A/../include/ascendc/asc_devkit_version.h"

build_variant() {
    local name=$1; shift
    $BISHENG $FLAGS $INCS "$@" -c $SRC -o $OUT/$name.rel.o 2>/dev/null
    $LD -m aicorelinux -Ttext=0 $OUT/$name.rel.o -o $OUT/$name.elf
    $OBJCPY --dump-section .text=$OUT/$name.text $OUT/$name.elf 2>/dev/null
    echo "built $name ($(stat -c%s $OUT/$name.text)B)"
}

# 基线
build_variant w00_base
# C 偏移:fp32 每分形 256 元素
build_variant w01_coff256  -DC_OFF=256
build_variant w02_coff512  -DC_OFF=512
build_variant w03_coff768  -DC_OFF=768
# A 偏移:fp16 每分形 256 元素
build_variant w04_aoff256  -DA_OFF=256
build_variant w05_aoff512  -DA_OFF=512
# B 偏移
build_variant w06_boff256  -DB_OFF=256
build_variant w07_boff512  -DB_OFF=512
# dtype 系列
build_variant w08_int8     -DDTYPE=1
build_variant w09_bf16     -DDTYPE=2
build_variant w10_fp32     -DDTYPE=3
# 边界值
build_variant w11_m8       -DM_VAL=8
build_variant w12_k8       -DK_VAL=8
build_variant w13_m128     -DM_VAL=128
build_variant w14_k128     -DK_VAL=128
echo ALL_DONE
