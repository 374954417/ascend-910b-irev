/**
 * Cube MMA 微基准:实测 mad 指令吞吐
 *
 * 方法:操作数一次性加载进 L0A/L0B(16x16 fp16 分形),然后在一个紧循环里
 * 反复发射 Mmad(底层即 bisheng builtin `mad` -> cube 矩阵乘累加指令),
 * 累加进同一块 L0C(fp32)。host 用两组不同 repeat 的运行时间做差,
 * 消掉 launch/加载开销,得到单次 mad(16x16x16) 的耗时。
 *
 * 单次 mad 的算力 = 2 * 16 * 16 * 16 = 8192 FLOPs
 */
#include "kernel_operator.h"

using namespace AscendC;

constexpr int32_t FRACTAL_SIZE = 16;                       // cube 原生分形 16x16
constexpr int32_t FRACTAL_ELEMS = FRACTAL_SIZE * FRACTAL_SIZE; // 256
constexpr int32_t NUM_C = 8;  // L0C 旋转缓冲数,覆盖 cube 累加延迟

extern "C" __global__ __aicore__ void mma_bench(GM_ADDR a, GM_ADDR b, GM_ADDR c, uint32_t repeat)
{
    // ---- buffer 分配:GM -> L0(A2/B2) 直载 -> L0C(CO1) ----
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf;
    TBuf<TPosition::B2> b2Buf;
    TBuf<TPosition::CO1> c0Buf;
    pipe.InitBuffer(a2Buf, FRACTAL_ELEMS * sizeof(half));
    pipe.InitBuffer(b2Buf, FRACTAL_ELEMS * sizeof(half));
    pipe.InitBuffer(c0Buf, NUM_C * FRACTAL_ELEMS * sizeof(float));

    LocalTensor<half> a2 = a2Buf.Get<half>();
    LocalTensor<half> b2 = b2Buf.Get<half>();
    LocalTensor<float> cAll = c0Buf.Get<float>();

    // 每个 AIC 核读写自己独立的 GM 区间,避免核间干扰
    int32_t blk = GetBlockIdx();
    GlobalTensor<half> aGm, bGm;
    GlobalTensor<float> cGm;
    aGm.SetGlobalBuffer((__gm__ half *)a + blk * FRACTAL_ELEMS, FRACTAL_ELEMS);
    bGm.SetGlobalBuffer((__gm__ half *)b + blk * FRACTAL_ELEMS, FRACTAL_ELEMS);
    cGm.SetGlobalBuffer((__gm__ float *)c + blk * FRACTAL_ELEMS, FRACTAL_ELEMS);

    // ---- 一次性加载:GM -> L0(单个 16x16 分形) ----
    LoadData2DParams ldParams(0, 1, 0, 0, 0, false, 0); // repeatTimes=1
    LoadData(a2, aGm, ldParams);
    LoadData(b2, bGm, ldParams);
    TEventID evtLoad = pipe.FetchEventID(HardEvent::MTE2_M);
    SetFlag<HardEvent::MTE2_M>(evtLoad);  // 等 LoadData 写完 L0A/L0B 再发 MAD
    WaitFlag<HardEvent::MTE2_M>(evtLoad);

    // ---- 紧循环发射 mad 指令,结果在 L0C 中累加 ----
    MmadParams mm;
    mm.m = FRACTAL_SIZE;
    mm.n = FRACTAL_SIZE;
    mm.k = FRACTAL_SIZE;
    mm.isBias = false;
    mm.unitFlag = 1;             // 开启 unit flag:cube 对同一 L0C 的连续累加做硬件依赖保护
    mm.cmatrixSource = 0;      // C 来自 L0C(累加模式)
    // 先对每块 L0C 做首次清零
    mm.cmatrixInitVal = true;
    #pragma unroll
    for (int32_t u = 0; u < NUM_C; u++) {
        Mmad(cAll[u * FRACTAL_ELEMS], a2, b2, mm);
    }
    // 主循环:手动 8 路展开 + 常量偏移,消除标量开销,纯测 mad 发射吞吐
    mm.cmatrixInitVal = false;
    uint32_t iters = (repeat > NUM_C) ? (repeat - NUM_C) / NUM_C : 0;
    for (uint32_t j = 0; j < iters; j++) {
        #pragma unroll
        for (int32_t u = 0; u < NUM_C; u++) {
            Mmad(cAll[u * FRACTAL_ELEMS], a2, b2, mm);
        }
    }

    // ---- 结果回拷 L0C -> GM(防止被优化掉 + host 校验;输出值均匀,布局无关) ----
    TEventID evtFix = pipe.FetchEventID(HardEvent::M_FIX);
    SetFlag<HardEvent::M_FIX>(evtFix);   // 等 MAD 写完 L0C 再回拷
    WaitFlag<HardEvent::M_FIX>(evtFix);
    // nSize=16, mSize=16, dstStride/srcStride=0, NoQuant, no relu, no channelSplit, no nz2nd
    DataCopyCO12DstParams copyParams(16, 16, 0, 0, QuantMode_t::NoQuant, 0, false, false);
    DataCopy(cGm, cAll, copyParams);
}
