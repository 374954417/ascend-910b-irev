#!/usr/bin/env python3
"""生成全计算指令的 II/延迟 benchmark kernel 源码"""
import os

OUT = "/home/ma-user/work/bucket-910c-6055/z00893531/kerneltest/demo_bench/bench_kernels.cpp"

# (名字, dtype, 形式, 调用表达式模板)  kind: bin/unary/scalar
# {D}=dst {A}=src0 {B}=src1 {C}=标量 {N}=count
OPS = [
    # 二元 fp16
    ("vadd", "half", "bin", "Add({D}, {A}, {B}, {N})"),
    ("vsub", "half", "bin", "Sub({D}, {A}, {B}, {N})"),
    ("vmul", "half", "bin", "Mul({D}, {A}, {B}, {N})"),
    ("vmax", "half", "bin", "Max({D}, {A}, {B}, {N})"),
    ("vmin", "half", "bin", "Min({D}, {A}, {B}, {N})"),
    ("vdiv", "half", "bin", "Div({D}, {A}, {B}, {N})"),
    # 标量版 fp16
    ("vadds", "half", "scalar", "Adds({D}, {A}, (half)1.0, {N})"),
    ("vmuls", "half", "scalar", "Muls({D}, {A}, (half)2.0, {N})"),
    ("vmaxs", "half", "scalar", "Maxs({D}, {A}, (half)1.0, {N})"),
    ("vmins", "half", "scalar", "Mins({D}, {A}, (half)1.0, {N})"),
    # 一元 fp16
    ("vexp", "half", "unary", "Exp({D}, {A}, {N})"),
    ("vln", "half", "unary", "Ln({D}, {A}, {N})"),
    ("vabs", "half", "unary", "Abs({D}, {A}, {N})"),
    ("vsqrt", "half", "unary", "Sqrt({D}, {A}, {N})"),
    ("vrsqrt", "half", "unary", "Rsqrt({D}, {A}, {N})"),
    ("vrelu", "half", "unary", "Relu({D}, {A}, {N})"),
    # 逻辑 u16
    ("vand", "uint16_t", "bin", "And({D}, {A}, {B}, {N})"),
    ("vor", "uint16_t", "bin", "Or({D}, {A}, {B}, {N})"),
    # 移位 s16
    ("vshl", "int16_t", "scalar", "ShiftLeft({D}, {A}, (int16_t)1, {N})"),
    # Axpy(融合乘加) fp16
    ("vmadd", "half", "scalar_inplace", "Axpy({D}, {A}, (half)2.0, {N})"),
    # 类型转换 f16->f32
    ("vcvt", "half", "cast", "Cast(({D}), {A}, RoundMode::CAST_NONE, {N})"),
    # 归约 f32
    ("vreduce_sum", "float", "reduce", "WholeReduceSum<float>({D}, {A}, {MK}, 1, 1, 1, 8)"),
    ("vreduce_max", "float", "reduce", "WholeReduceMax<float>({D}, {A}, {MK}, 1, 1, 1, 8)"),
    ("vreduce_min", "float", "reduce", "WholeReduceMin<float>({D}, {A}, {MK}, 1, 1, 1, 8)"),
    ("vblockreduce_sum", "float", "blockreduce", "BlockReduceSum<float>({D}, {A}, 1, 64, 1, 1, 256)"),
    # 比较(只测独立链)
    ("vcmpv", "half", "cmp", "Compare({MK8}, {A}, {B}, CMPMODE::LT, {N})"),
    ("vcmpvs", "half", "cmp", "CompareScalar({MK8}, {A}, (half)1.0, CMPMODE::LT, {N})"),
    # 选择(只测独立链)
    ("vsel", "half", "sel", "Select({D}, {MK8}, {A}, {B}, SELMODE::VSEL_CMPMASK_SPR, (uint64_t)0, 2, BinaryRepeatParams(1,1,1,1,1,1))"),
    # 排序(只测独立链)
    ("vbitsort", "half", "sort", "Sort32<half>({D}, {A}, {I32}, 4)"),
    # gather(只测独立链)
    ("vgather", "half", "gather", "Gather({D}, {A}, {I32}, (uint32_t)0, {N})"),
]

HEADER = """#include "kernel_operator.h"
using namespace AscendC;
constexpr int32_t ELEMS = 256;      // 每次调用 256 元素
constexpr int32_t SLOTS = 8;        // 独立链轮换 8 个目标区

"""

def gen_kernel(name, dtype, kind, call, dep):
    """dep=True 生成依赖链版本, 否则独立链"""
    is_cmp = kind in ("cmp",)
    is_sel = kind == "sel"
    is_sort = kind == "sort"
    is_gather = kind == "gather"
    out_dt = "uint8_t" if is_cmp else ("float" if kind == "cast" else dtype)
    c_extra = ""
    if is_cmp or is_sel:
        c_extra = "LocalTensor<uint8_t> mL = mBuf.Get<uint8_t>();"
    if is_sort or is_gather:
        c_extra = "LocalTensor<uint32_t> iL = iBuf.Get<uint32_t>();"
    if kind == "cast":
        c_extra = "LocalTensor<float> zF = zFBuf.Get<float>();"
    call_line = (call.replace("{D}", "zF" if kind == "cast" else ("mL" if is_cmp else "zL"))
                     .replace("{A}", "xL").replace("{B}", "yL").replace("{N}", "ELEMS")
                     .replace("{MK8}", "mL").replace("{I32}", "iL").replace("{MK}", "mk"))
    body_indep = f"""
        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {{
            {call_line.replace('zL', f'zL[u * ELEMS]').replace('mL', f'mL[u * 32]').replace('zF', f'zF[u * ELEMS]')};
        }}"""
    if kind == "scalar_inplace":  # axpy: dep = z = a*x+z 累加
        body_dep = f"        {call_line};"
    elif kind == "unary":
        body_dep = f"        {call_line.replace('xL', 'zL')};"  # z = f(z)
    elif kind in ("bin", "scalar"):
        body_dep = f"        {call_line.replace('xL', 'zL')};"  # z = z op y
    else:
        body_dep = body_indep
    body = body_dep if dep else body_indep

    extra_bufs = ""
    if is_cmp or is_sel:
        extra_bufs = "TBuf<TPosition::VECCALC> mBuf; pipe.InitBuffer(mBuf, SLOTS * 32 * sizeof(uint8_t));"
    if is_sort or is_gather:
        extra_bufs = "TBuf<TPosition::VECCALC> iBuf; pipe.InitBuffer(iBuf, ELEMS * sizeof(uint32_t));"
    if kind == "cast":
        extra_bufs = "TBuf<TPosition::VECCALC> zFBuf; pipe.InitBuffer(zFBuf, SLOTS * ELEMS * sizeof(float));"

    gm_extra = ""
    if is_sort or is_gather:
        gm_extra = """    GlobalTensor<uint32_t> iGm;
    iGm.SetGlobalBuffer((__gm__ uint32_t *)i, ELEMS);
    DataCopy(iL, iGm, ELEMS);
"""
    if is_sel:
        gm_extra += """    GlobalTensor<uint8_t> mGm;
    mGm.SetGlobalBuffer((__gm__ uint8_t *)i, SLOTS * 32);
    DataCopy(mL, mGm, SLOTS * 32);
"""

    return f"""
extern "C" __global__ __aicore__ void bench_{name}_{'d' if dep else 'i'}(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof({dtype}));
    pipe.InitBuffer(yBuf, ELEMS * sizeof({dtype}));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof({out_dt}));
    {extra_bufs}
    LocalTensor<{dtype}> xL = xBuf.Get<{dtype}>();
    LocalTensor<{dtype}> yL = yBuf.Get<{dtype}>();
    LocalTensor<{out_dt}> zL = zBuf.Get<{out_dt}>();
    {c_extra}
    GlobalTensor<{dtype}> xGm, yGm;
    GlobalTensor<{out_dt}> zGm;
    xGm.SetGlobalBuffer((__gm__ {dtype} *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ {dtype} *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ {out_dt} *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
{gm_extra}
    uint64_t mk[4] = {{0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0}};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {{
{body}
    }}
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}}
"""

parts = [HEADER]
kernels = []
for name, dtype, kind, call in OPS:
    parts.append(gen_kernel(name, dtype, kind, call, dep=False))
    kernels.append((name, "i"))
    if kind in ("bin", "unary", "scalar", "scalar_inplace"):
        parts.append(gen_kernel(name, dtype, kind, call, dep=True))
        kernels.append((name, "d"))

open(OUT, "w").write("".join(parts))
print(f"生成 {OUT}: {len(OPS)} 种指令, {len(kernels)} 个 kernel")
print("kernel 列表:", [f"{n}_{d}" for n, d in kernels])
