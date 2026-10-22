#!/bin/bash
# 批量编译 cann-ops 的 AscendC kernel(带 tiling stub 生成)
# 用法: batch_compile.sh [数量上限]
set -u
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
OBJCPY=$A/ccec_compiler/bin/llvm-objcopy
SRC_ROOT=/tmp/cann_ops/src
OUT=/tmp/op_corpus
STUBS=$OUT/stubs
DEMO_DIR=$(cd "$(dirname "$0")" && pwd)
mkdir -p $OUT $STUBS

INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl \
-I$SRC_ROOT/common/inc"
BASE="--cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang \
-include $A/../include/ascendc/asc_devkit_version.h -D__NPU_ARCH__=2201 -ferror-limit=1 \
-DDTYPE_X=half -DDTYPE_X1=half -DDTYPE_X2=half -DDTYPE_Y=half -DDTYPE_Y1=half -DDTYPE_Y2=half \
-DDTYPE_Z=half -DDTYPE_OUT=half -DDTYPE_POINTS=half -DDTYPE_INDICES=int32_t -DDTYPE_WEIGHTS=half \
-DDTYPE_FEATURE=half -DDTYPE_LABELS=int32_t -DDTYPE_INPUT=half -DDTYPE_OUTPUT=half -DDTYPE_DELTA=half"

LIMIT=${1:-99999}
n=0; ok_vec=0; ok_cube=0; fail=0
> $OUT/manifest.txt

for f in $(find $SRC_ROOT -name "*.cpp" -path "*op_kernel*" | sort); do
    grep -q "__global__ __aicore__" "$f" || continue
    name=$(echo "${f#$SRC_ROOT/}" | tr '/' '_')
    n=$((n+1)); [ $n -gt $LIMIT ] && break
    op_dir=$(dirname $(dirname $f))
    stub=$STUBS/$(echo "${op_dir#$SRC_ROOT/}" | tr '/' '_')_tiling_stub.h
    [ -f "$stub" ] || python3 $DEMO_DIR/gen_tiling_stub.py "$op_dir" "$stub" > /dev/null 2>&1
    extra=""
    if [ -s "$stub" ]; then extra="-include $stub"; fi
    arch=""
    if $BISHENG --cce-aicore-arch=dav-c220-vec $BASE $INCS $extra -I$(dirname $f) -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=vec; ok_vec=$((ok_vec+1))
    elif $BISHENG --cce-aicore-arch=dav-c220-cube $BASE $INCS $extra -I$(dirname $f) -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=cube; ok_cube=$((ok_cube+1))
    else
        fail=$((fail+1)); echo "FAIL $name" >> $OUT/manifest.txt; continue
    fi
    $LD -m aicorelinux -Ttext=0 $OUT/$name.o -o $OUT/$name.elf 2>/dev/null
    $OBJCPY --dump-section .text=$OUT/$name.text $OUT/$name.elf 2>/dev/null
    echo "OK $arch $name $(stat -c%s $OUT/$name.text 2>/dev/null)B" >> $OUT/manifest.txt
done
echo "=== 共 $n 个候选: vec=$ok_vec cube=$ok_cube 失败=$fail"
