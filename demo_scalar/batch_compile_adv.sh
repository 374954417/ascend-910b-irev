#!/bin/bash
# 批量编译 cann-ops-adv 融合算子(全局 tiling stub + 真实库 tiling 类型)
set -u
A=/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux
BISHENG=$A/ccec_compiler/bin/bisheng
LD=$A/ccec_compiler/bin/ld.lld
OBJCPY=$A/ccec_compiler/bin/llvm-objcopy
SRC_ROOT=/tmp/cann_ops_adv/src
OUT=/tmp/adv_corpus
STUBS=$OUT/stubs
mkdir -p $OUT

INCS="-I$A/asc/impl/adv_api -I$A/asc/impl/basic_api -I$A/asc/impl/utils \
-I$A/asc/include -I$A/asc/include/adv_api -I$A/asc/include/basic_api \
-I$A/asc/include/utils -I$A/tikcpp/tikcfw -I$A/tikcpp/tikcfw/interface -I$A/tikcpp/tikcfw/impl \
-I$SRC_ROOT/utils -I$SRC_ROOT/transformer -I$SRC_ROOT/mc2"
BASE="--cce-aicore-only --cce-auto-sync -O3 -std=c++17 --cce-aicore-lang \
-include $A/../include/ascendc/asc_devkit_version.h -D__NPU_ARCH__=2201 -ferror-limit=1 \
-DORIG_DTYPE_QUERY=DT_FLOAT16 -DORIG_DTYPE_KEY=DT_FLOAT16 \
-DDTYPE_X=half -DDTYPE_Y=half -DDTYPE_OUT=half \
-DDTYPE_WEIGHT=half -DDTYPE_WEIGHTS=half -DDTYPE_SCALE=half -DDTYPE_BIAS=half -DDTYPE_PERTOKEN_SCALE=float \
-DDTYPE_X_QUANT=int8_t -DDTYPE_Y_QUANT=int8_t -DDTYPE_COMPUTE=float -DDTYPE_GROUP=int32_t -DDTYPE_A=half \
-DDTYPE_B=half -DDTYPE_C=half -DDTYPE_D=half -DDTYPE_E=half -DDTYPE_F=half \
-DTILING_DATA_TYPE=half -DORIG_DTYPE_ATTENTION_OUT=DT_FLOAT16 -DORIG_DTYPE_VALUE=DT_FLOAT16"

ok_vec=0; ok_cube=0; fail=0
> $OUT/manifest.txt

for f in $(grep -rl "__global__ __aicore__" $SRC_ROOT --include="*.cpp" 2>/dev/null | sort); do
    name=$(echo "${f#$SRC_ROOT/}" | tr '/' '_')
    src_dir=$(dirname $f)
    # 若文件内存在 __global__ 调用 __global__ 的 dispatcher wrapper, 生成剥离副本
    if [ $(grep -c "__global__ __aicore__" $f) -gt 1 ]; then
        pf=$OUT/${name}.patched.cpp
        python3 - "$f" "$pf" <<'PYEOF'
import re, sys
src = open(sys.argv[1]).read()
# 删除所有"只包含转发调用"的 __global__ wrapper(函数体里调用另一个本文件定义的 __global__)
globals_ = [(m.start(), m.group(1)) for m in re.finditer(r'extern\s+"C"\s+__global__\s+__aicore__\s+void\s+(\w+)\s*\(', src)]
names = [n for _, n in globals_]
out, last = [], 0
for pos, fname in globals_:
    # 找函数体
    brace = src.index('{', pos)
    depth, i = 0, brace
    while i < len(src):
        if src[i] == '{': depth += 1
        elif src[i] == '}':
            depth -= 1
            if depth == 0: break
        i += 1
    body = src[brace:i]
    calls_other = any(re.search(r'\b' + re.escape(o) + r'\s*\(', body) for o in names if o != fname)
    if calls_other:
        out.append(src[last:pos])   # 跳过 wrapper
        last = i + 1
out.append(src[last:])
open(sys.argv[2], 'w').write(''.join(out))
PYEOF
        [ -f "$pf" ] && f=$pf
    fi
    orig_dir=$(dirname $f)
    arch=""
    if $BISHENG --cce-aicore-arch=dav-c220-vec $BASE $INCS \
        -include $STUBS/global_tiling_stub.h -I$(dirname $f) -I$(dirname $f)/ophost \
        -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=vec; ok_vec=$((ok_vec+1))
    elif $BISHENG --cce-aicore-arch=dav-c220-cube $BASE $INCS \
        -include $STUBS/global_tiling_stub.h -I$(dirname $f) -I$(dirname $f)/ophost \
        -c $f -o $OUT/$name.o 2>/dev/null; then
        arch=cube; ok_cube=$((ok_cube+1))
    else
        fail=$((fail+1)); echo "FAIL $name" >> $OUT/manifest.txt; continue
    fi
    $LD -m aicorelinux -Ttext=0 $OUT/$name.o -o $OUT/$name.elf 2>/dev/null
    $OBJCPY --dump-section .text=$OUT/$name.text $OUT/$name.elf 2>/dev/null
    echo "OK $arch $name $(stat -c%s $OUT/$name.text 2>/dev/null)B" >> $OUT/manifest.txt
done
echo "=== cann-ops-adv: vec=$ok_vec cube=$ok_cube 失败=$fail"
