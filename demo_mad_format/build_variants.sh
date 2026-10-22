#!/bin/bash
# 批量编译 mad 变体并提取 .text 字节
set -e
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
SRC=/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_mad_format/mad_one.cpp
OUT=/tmp/mad_variants
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
    echo "built $name"
}

build_variant v00_nomad   -DNUM_MAD=0
build_variant v01_base
build_variant v02_m32     -DM_VAL=32
build_variant v03_n32     -DN_VAL=32
build_variant v04_k32     -DK_VAL=32
build_variant v05_k64     -DK_VAL=64
build_variant v06_init0   -DINIT_VAL=0
build_variant v07_uflag0  -DUNIT_FLAG=0
build_variant v08_int8    -DINT8_MODE=1
build_variant v09_m64n64k64 -DM_VAL=64 -DN_VAL=64 -DK_VAL=64
build_variant v10_two_mad -DNUM_MAD=2
build_variant v11_coff512 -DC_OFFSET_ELEMS=512
echo ALL_DONE
