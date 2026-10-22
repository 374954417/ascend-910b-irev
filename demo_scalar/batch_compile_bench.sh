#!/bin/bash
# 编译 MKB/CANN-Bench 提取的 kernel
set -u
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
OBJCPY=$A/ccec_compiler/bin/llvm-objcopy
OUT=/tmp/bench_corpus
mkdir -p $OUT

INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl -I$A/asc/include/tiling"
BASE="--cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang \
-include $A/../include/ascendc/asc_devkit_version.h -D__NPU_ARCH__=2201 -ferror-limit=1 \
-DDTYPE_X=half -DDTYPE_Y=half -DDTYPE_Z=half -DDTYPE_T=half -DDTYPE_OUT=half \
-DDTYPE_INPUT=half -DDTYPE_OUTPUT=half -DTILE_LENGTH=256 -DBLOCK_LENGTH=256"

ok_vec=0; ok_cube=0; fail=0
> $OUT/manifest.txt
for f in $(find /tmp/mkb_kernels /tmp/cann_bench -name "*.cpp" | xargs grep -l "__global__ __aicore__" 2>/dev/null | sort -u); do
    name=$(basename $f .cpp)_$(basename $(dirname $f))
    arch=""
    if $BISHENG --cce-aicore-arch=dav-c220-vec $BASE $INCS -I$(dirname $f) -I$(dirname $f)/../op_host -include /tmp/mkb_stubs/universal_stub.h -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=vec; ok_vec=$((ok_vec+1))
    elif $BISHENG --cce-aicore-arch=dav-c220-cube $BASE $INCS -I$(dirname $f) -I$(dirname $f)/../op_host -include /tmp/mkb_stubs/universal_stub.h -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=cube; ok_cube=$((ok_cube+1))
    else
        fail=$((fail+1)); echo "FAIL $name" >> $OUT/manifest.txt; continue
    fi
    $LD -m aicorelinux -Ttext=0 $OUT/$name.o -o $OUT/$name.elf 2>/dev/null
    $OBJCPY --dump-section .text=$OUT/$name.text $OUT/$name.elf 2>/dev/null
    echo "OK $arch $name $(stat -c%s $OUT/$name.text 2>/dev/null)B" >> $OUT/manifest.txt
done
echo "=== bench 语料: vec=$ok_vec cube=$ok_cube 失败=$fail"
