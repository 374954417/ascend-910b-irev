#!/usr/bin/env python3
"""cube mad 变体生成器: dtype × (m,n,k) 形状
输出: demo_bench/mad_variants/*.cpp"""
import os

OUT_DIR = "/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_bench/mad_variants"
os.makedirs(OUT_DIR, exist_ok=True)

# (名字, src dtype, dst dtype)
DTYPES = [
    ("f16", "half", "float"),
    ("bf16", "bfloat16_t", "float"),
    ("f32", "float", "float"),
    ("s8", "int8_t", "int32_t"),
]
SHAPES = [(16, 16, 16), (32, 32, 32), (64, 64, 64), (16, 16, 64), (64, 16, 16), (16, 64, 16)]

TEMPLATE = """#include "kernel_operator.h"
using namespace AscendC;
constexpr int32_t M = {M}, N = {N}, K = {K};
constexpr int32_t NUM_C = 8;
extern "C" __global__ __aicore__ void mad_{NAME}(GM_ADDR a, GM_ADDR b, GM_ADDR c, uint32_t repeat)
{{
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf; pipe.InitBuffer(a2Buf, M * K * sizeof({ST}));
    TBuf<TPosition::B2> b2Buf; pipe.InitBuffer(b2Buf, K * N * sizeof({ST}));
    TBuf<TPosition::CO1> c0Buf; pipe.InitBuffer(c0Buf, NUM_C * M * N * sizeof({DT}));
    LocalTensor<{ST}> a2 = a2Buf.Get<{ST}>();
    LocalTensor<{ST}> b2 = b2Buf.Get<{ST}>();
    LocalTensor<{DT}> cAll = c0Buf.Get<{DT}>();
    GlobalTensor<{ST}> aGm, bGm;
    GlobalTensor<{DT}> cGm;
    aGm.SetGlobalBuffer((__gm__ {ST} *)a, M * K);
    bGm.SetGlobalBuffer((__gm__ {ST} *)b, K * N);
    cGm.SetGlobalBuffer((__gm__ {DT} *)c, NUM_C * M * N);
    LoadData2DParams ldA(0, M * K / 256, 1, 0, 0, false, 0);
    LoadData2DParams ldB(0, K * N / 256, 1, 0, 0, false, 0);
    LoadData(a2, aGm, ldA);
    LoadData(b2, bGm, ldB);
    TEventID e0 = pipe.FetchEventID(HardEvent::MTE2_M);
    SetFlag<HardEvent::MTE2_M>(e0); WaitFlag<HardEvent::MTE2_M>(e0);
    MmadParams mm; mm.m = M; mm.n = N; mm.k = K;
    mm.isBias = false; mm.unitFlag = 1; mm.cmatrixSource = 0;
    mm.cmatrixInitVal = true;
    #pragma unroll
    for (int32_t u = 0; u < NUM_C; u++) Mmad(cAll[u * M * N], a2, b2, mm);
    mm.cmatrixInitVal = false;
    uint32_t iters = (repeat > NUM_C) ? (repeat - NUM_C) / NUM_C : 0;
    for (uint32_t j = 0; j < iters; j++) {{
        #pragma unroll
        for (int32_t u = 0; u < NUM_C; u++) Mmad(cAll[u * M * N], a2, b2, mm);
    }}
    TEventID e2 = pipe.FetchEventID(HardEvent::M_FIX);
    SetFlag<HardEvent::M_FIX>(e2); WaitFlag<HardEvent::M_FIX>(e2);
    DataCopyCO12DstParams cp(N, M, 0, 0, QuantMode_t::NoQuant, 0, false, false);
    DataCopy(cGm, cAll, cp);
}}
"""

for name, st, dt in DTYPES:
    for m, n, k in SHAPES:
        vname = f"{name}_m{m}n{n}k{k}"
        src = TEMPLATE.replace("{NAME}", vname).replace("{ST}", st).replace("{DT}", dt) \
                      .replace("{M}", str(m)).replace("{N}", str(n)).replace("{K}", str(k))
        open(f"{OUT_DIR}/mad_{vname}.cpp", "w").write(src)
print(f"生成 {len(DTYPES) * len(SHAPES)} 个 mad 变体")
