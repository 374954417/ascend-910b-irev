#!/usr/bin/env python3
"""全变体 benchmark 生成器: 每 op × dtype × mode
输出: demo_bench/variants/*.cpp(每变体一个文件, 含 indep+dep 两个 kernel)"""
import os
import subprocess
import sys

A = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux"
OUT_DIR = "/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_bench/variants"
os.makedirs(OUT_DIR, exist_ok=True)

BISHENG = f"{A}/ccec_compiler/bin/bisheng"
INCS = [f"-I{A}/asc/impl/adv_api", f"-I{A}/asc/impl/basic_api", f"-I{A}/asc/impl/utils",
        f"-I{A}/asc/include", f"-I{A}/asc/include/adv_api", f"-I{A}/asc/include/basic_api",
        f"-I{A}/asc/include/utils", f"-I{A}/tikcpp/tikcfw", f"-I{A}/tikcpp/tikcfw/interface",
        f"-I{A}/tikcpp/tikcfw/impl"]
FLAGS = ["--cce-aicore-arch=dav-c220-vec", "--cce-aicore-only", "--cce-auto-sync", "-O3",
         "-std=c++17", "--cce-aicore-lang", "-include",
         f"{A}/../include/ascendc/asc_devkit_version.h", "-D__NPU_ARCH__=2201"]

# ---------------- 变体矩阵 ----------------
# (name, kind, dtype, dtype_out, mode_label, call)
VARIANTS = []

# 二元算术 × dtype
for op, api in [("vadd", "Add"), ("vsub", "Sub"), ("vmul", "Mul"), ("vmax", "Max"), ("vmin", "Min"), ("vdiv", "Div")]:
    for dt in ["half", "float", "int32_t", "int16_t"]:
        VARIANTS.append((f"{op}_{dt.replace('_','').replace('int','s')}", "bin", dt, dt, "",
                         f"{api}({{D}}, {{A}}, {{B}}, {{N}})"))
# 标量版 × dtype
for op, api in [("vadds", "Adds"), ("vmuls", "Muls"), ("vmaxs", "Maxs"), ("vmins", "Mins")]:
    for dt in ["half", "float", "int16_t", "int32_t"]:
        c = "(half)1.0" if dt == "half" else ("2.0f" if dt == "float" else f"({dt})2")
        VARIANTS.append((f"{op}_{dt.replace('_','').replace('int','s')}", "scalar", dt, dt, "",
                         f"{api}({{D}}, {{A}}, {c}, {{N}})"))
# 一元 × dtype
for op, api in [("vexp", "Exp"), ("vln", "Ln"), ("vabs", "Abs"), ("vsqrt", "Sqrt"), ("vrsqrt", "Rsqrt"), ("vrelu", "Relu")]:
    for dt in ["half", "float"]:
        VARIANTS.append((f"{op}_{dt.replace('_','')}", "unary", dt, dt, "",
                         f"{api}({{D}}, {{A}}, {{N}})"))
# 逻辑 × dtype(整型)
for op, api in [("vand", "And"), ("vor", "Or"), ("vxor", "Xor")]:
    for dt in ["uint16_t", "int16_t", "int32_t", "uint32_t"]:
        VARIANTS.append((f"{op}_{dt.replace('_','').replace('int','')}", "bin", dt, dt, "",
                         f"{api}({{D}}, {{A}}, {{B}}, {{N}})"))
# 移位 × dtype
for op, api in [("vshl", "ShiftLeft"), ("vshr", "ShiftRight")]:
    for dt in ["int16_t", "uint16_t", "int32_t", "uint32_t"]:
        VARIANTS.append((f"{op}_{dt.replace('_','').replace('int','')}", "scalar", dt, dt, "",
                         f"{api}({{D}}, {{A}}, ({dt})1, {{N}})"))
# Axpy × dtype
for dt in ["half", "float"]:
    c = "(half)2.0" if dt == "half" else "2.0f"
    VARIANTS.append((f"vmadd_{dt}", "scalar_inplace", dt, dt, "",
                     f"Axpy({{D}}, {{A}}, {c}, {{N}})"))
# Cast: (src_dt, dst_dt) × round mode
CAST_MODES = [("none", "CAST_NONE"), ("round", "CAST_ROUND"), ("trunc", "CAST_TRUNC"),
              ("ceil", "CAST_CEIL"), ("floor", "CAST_FLOOR"), ("rint", "CAST_RINT")]
for src, dst, lbl in [("half", "float", "f16f32"), ("float", "half", "f32f16"),
                      ("float", "int32_t", "f32s32"), ("int32_t", "float", "s32f32"),
                      ("half", "int16_t", "f16s16"), ("int16_t", "half", "s16f16"),
                      ("float", "bfloat16_t", "f32bf16"), ("bfloat16_t", "float", "bf16f32")]:
    for mlbl, mval in CAST_MODES:
        VARIANTS.append((f"vcvt_{lbl}_{mlbl}", "cast", src, dst, mlbl,
                         f"Cast({{D}}, {{A}}, RoundMode::{mval}, {{N}})"))
# 归约 × dtype
for op, api in [("vreduce_sum", "WholeReduceSum"), ("vreduce_max", "WholeReduceMax"), ("vreduce_min", "WholeReduceMin")]:
    for dt in ["float", "half", "bfloat16_t"]:
        VARIANTS.append((f"{op}_{dt.replace('_','')}", "reduce", dt, dt, "",
                         f"{api}<{dt}>({{D}}, {{A}}, {{MK}}, 1, 1, 1, 8)"))
VARIANTS.append(("vblockreduce_sum_f32", "blockreduce", "float", "float", "",
                 "BlockReduceSum<float>({D}, {A}, 1, 64, 1, 1, 256)"))
VARIANTS.append(("vblockreduce_sum_f16", "blockreduce", "half", "half", "",
                 "BlockReduceSum<half>({D}, {A}, 1, 64, 1, 1, 256)"))
# 比较: dtype × mode
for dt in ["half", "float", "int32_t"]:
    for m in ["EQ", "NE", "LT", "GT", "GE", "LE"]:
        c = "(half)1.0" if dt == "half" else ("1.0f" if dt == "float" else "(int32_t)1")
        VARIANTS.append((f"vcmpv_{m.lower()}_{dt.replace('_','').replace('int','s')}", "cmp", dt, "uint8_t", m,
                         f"Compare({{MK8}}, {{A}}, {{B}}, CMPMODE::{m}, {{N}})"))
        VARIANTS.append((f"vcmpvs_{m.lower()}_{dt.replace('_','').replace('int','s')}", "cmp", dt, "uint8_t", m,
                         f"CompareScalar({{MK8}}, {{A}}, {c}, CMPMODE::{m}, {{N}})"))
# Select: dtype × selmode
for dt in ["half", "float"]:
    for sm, sl in [("SELMODE::VSEL_CMPMASK_SPR", "spr"), ("SELMODE::VSEL_TENSOR_TENSOR_MODE", "tt")]:
        VARIANTS.append((f"vsel_{sl}_{dt}", "sel", dt, dt, sl,
                         f"Select({{D}}, {{MK8}}, {{A}}, {{B}}, {sm}, (uint64_t)0, 2, BinaryRepeatParams(1,1,1,1,1,1))"))
# Sort32 × dtype
for dt in ["half", "float"]:
    VARIANTS.append((f"vbitsort_{dt}", "sort", dt, dt, "", f"Sort32<{dt}>({{D}}, {{A}}, {{I32}}, 4)"))
# Gather × dtype
for dt in ["half", "float"]:
    VARIANTS.append((f"vgather_{dt}", "gather", dt, dt, "",
                     f"Gather({{D}}, {{A}}, {{I32}}, (uint32_t)0, {{N}})"))

# ---------------- 代码模板 ----------------
HEADER = """#include "kernel_operator.h"
using namespace AscendC;
"""

def gen_variant(name, kind, dt, out_dt, call, dep):
    is_cmp = kind == "cmp"
    is_sel = kind == "sel"
    is_sort = kind == "sort"
    is_gather = kind == "gather"
    extra_bufs = ""
    c_extra = ""
    if is_cmp or is_sel:
        extra_bufs += "TBuf<TPosition::VECCALC> mBuf; pipe.InitBuffer(mBuf, SLOTS * 32 * sizeof(uint8_t));\n"
        c_extra = "LocalTensor<uint8_t> mL = mBuf.Get<uint8_t>();"
    if is_sort or is_gather:
        extra_bufs += "TBuf<TPosition::VECCALC> iBuf; pipe.InitBuffer(iBuf, ELEMS * sizeof(uint32_t));\n"
        c_extra += "LocalTensor<uint32_t> iL = iBuf.Get<uint32_t>();"
    call_line = (call.replace("{D}", "mL" if is_cmp else "zL").replace("{A}", "xL")
                     .replace("{B}", "yL").replace("{N}", "ELEMS").replace("{MK8}", "mL")
                     .replace("{I32}", "iL").replace("{MK}", "mk"))
    if dep:
        if kind == "unary":
            body = f"        {call_line.replace('xL', 'zL')};"
        elif kind in ("bin", "scalar", "scalar_inplace"):
            body = f"        {call_line.replace('xL', 'zL')};"
        else:
            return None
    else:
        body = f"""
        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {{
            {call_line.replace('zL', 'zL[u * ELEMS]').replace('mL', 'mL[u * 32]')};
        }}"""
    gm_i = ""
    if is_sort or is_gather:
        gm_i = f"""    GlobalTensor<uint32_t> iGm;
    iGm.SetGlobalBuffer((__gm__ uint32_t *)i, ELEMS);
    DataCopy(iL, iGm, ELEMS);
"""
    if is_sel:
        gm_i += f"""    GlobalTensor<uint8_t> mGm;
    mGm.SetGlobalBuffer((__gm__ uint8_t *)i, SLOTS * 32);
    DataCopy(mL, mGm, SLOTS * 32);
"""
    return f"""
extern "C" __global__ __aicore__ void bench_{name}_{'d' if dep else 'i'}(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{{
    constexpr int32_t ELEMS = 256;
    constexpr int32_t SLOTS = 8;
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof({dt}));
    pipe.InitBuffer(yBuf, ELEMS * sizeof({dt}));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof({out_dt}));
{extra_bufs}    LocalTensor<{dt}> xL = xBuf.Get<{dt}>();
    LocalTensor<{dt}> yL = yBuf.Get<{dt}>();
    LocalTensor<{out_dt}> zL = zBuf.Get<{out_dt}>();
    {c_extra}
    GlobalTensor<{dt}> xGm, yGm;
    GlobalTensor<{out_dt}> zGm;
    xGm.SetGlobalBuffer((__gm__ {dtype_cast(dt)} *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ {dtype_cast(dt)} *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ {out_dt} *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
{gm_i}    uint64_t mk[4] = {{0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0}};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e); WaitFlag<HardEvent::MTE2_V>(e);
    int64_t cyc0 = GetSystemCycle();
    for (uint32_t r = 0; r < repeat; r++) {{
{body}
    }}
    int64_t cyc1 = GetSystemCycle();
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2); WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
    // 把周期数写到 i 参数的位置(host 读回)
    GlobalTensor<int64_t> cycGm;
    cycGm.SetGlobalBuffer((__gm__ int64_t *)i, 2);
    TEventID e3 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e3); WaitFlag<HardEvent::V_MTE3>(e3);
    SetAtomicAdd<int64_t>();
    cycGm.SetValue(0, cyc1 - cyc0);
    SetAtomicNone();
}}
"""

def dtype_cast(dt):
    return dt

def main():
    quick = "--quick" in sys.argv
    n_written = 0
    n_dep = 0
    for name, kind, dt, out_dt, mlbl, call in VARIANTS:
        src = HEADER + gen_variant(name, kind, dt, out_dt, call, dep=False)
        if kind in ("bin", "unary", "scalar", "scalar_inplace"):
            src += gen_variant(name, kind, dt, out_dt, call, dep=True)
            n_dep += 1
        open(f"{OUT_DIR}/{name}.cpp", "w").write(src)
        n_written += 1
    print(f"生成 {n_written} 个变体文件(含 {n_dep} 个依赖链)")
    print(f"总计 kernel 数: {n_written + n_dep}")

if __name__ == "__main__":
    main()
