#include "kernel_operator.h"
using namespace AscendC;
constexpr int32_t ELEMS = 256;      // 每次调用 256 元素
constexpr int32_t SLOTS = 8;        // 独立链轮换 8 个目标区


extern "C" __global__ __aicore__ void bench_vadd_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Add(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vadd_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Add(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vsub_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Sub(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vsub_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Sub(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmul_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Mul(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmul_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Mul(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmax_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Max(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmax_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Max(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmin_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Min(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmin_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Min(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vdiv_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Div(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vdiv_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Div(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vadds_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Adds(zL[u * ELEMS], xL, (half)1.0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vadds_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Adds(zL, zL, (half)1.0, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmuls_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Muls(zL[u * ELEMS], xL, (half)2.0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmuls_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Muls(zL, zL, (half)2.0, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmaxs_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Maxs(zL[u * ELEMS], xL, (half)1.0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmaxs_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Maxs(zL, zL, (half)1.0, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmins_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Mins(zL[u * ELEMS], xL, (half)1.0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmins_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Mins(zL, zL, (half)1.0, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vexp_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Exp(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vexp_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Exp(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vln_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Ln(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vln_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Ln(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vabs_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Abs(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vabs_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Abs(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vsqrt_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Sqrt(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vsqrt_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Sqrt(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vrsqrt_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Rsqrt(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vrsqrt_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Rsqrt(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vrelu_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Relu(zL[u * ELEMS], xL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vrelu_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Relu(zL, zL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vand_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint16_t));
    
    LocalTensor<uint16_t> xL = xBuf.Get<uint16_t>();
    LocalTensor<uint16_t> yL = yBuf.Get<uint16_t>();
    LocalTensor<uint16_t> zL = zBuf.Get<uint16_t>();
    
    GlobalTensor<uint16_t> xGm, yGm;
    GlobalTensor<uint16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ uint16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ uint16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            And(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vand_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint16_t));
    
    LocalTensor<uint16_t> xL = xBuf.Get<uint16_t>();
    LocalTensor<uint16_t> yL = yBuf.Get<uint16_t>();
    LocalTensor<uint16_t> zL = zBuf.Get<uint16_t>();
    
    GlobalTensor<uint16_t> xGm, yGm;
    GlobalTensor<uint16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ uint16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ uint16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        And(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vor_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint16_t));
    
    LocalTensor<uint16_t> xL = xBuf.Get<uint16_t>();
    LocalTensor<uint16_t> yL = yBuf.Get<uint16_t>();
    LocalTensor<uint16_t> zL = zBuf.Get<uint16_t>();
    
    GlobalTensor<uint16_t> xGm, yGm;
    GlobalTensor<uint16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ uint16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ uint16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Or(zL[u * ELEMS], xL, yL, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vor_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(uint16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint16_t));
    
    LocalTensor<uint16_t> xL = xBuf.Get<uint16_t>();
    LocalTensor<uint16_t> yL = yBuf.Get<uint16_t>();
    LocalTensor<uint16_t> zL = zBuf.Get<uint16_t>();
    
    GlobalTensor<uint16_t> xGm, yGm;
    GlobalTensor<uint16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ uint16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ uint16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Or(zL, zL, yL, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vshl_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(int16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(int16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(int16_t));
    
    LocalTensor<int16_t> xL = xBuf.Get<int16_t>();
    LocalTensor<int16_t> yL = yBuf.Get<int16_t>();
    LocalTensor<int16_t> zL = zBuf.Get<int16_t>();
    
    GlobalTensor<int16_t> xGm, yGm;
    GlobalTensor<int16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ int16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ int16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ int16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            ShiftLeft(zL[u * ELEMS], xL, (int16_t)1, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vshl_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(int16_t));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(int16_t));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(int16_t));
    
    LocalTensor<int16_t> xL = xBuf.Get<int16_t>();
    LocalTensor<int16_t> yL = yBuf.Get<int16_t>();
    LocalTensor<int16_t> zL = zBuf.Get<int16_t>();
    
    GlobalTensor<int16_t> xGm, yGm;
    GlobalTensor<int16_t> zGm;
    xGm.SetGlobalBuffer((__gm__ int16_t *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ int16_t *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ int16_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        ShiftLeft(zL, zL, (int16_t)1, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmadd_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Axpy(zL[u * ELEMS], xL, (half)2.0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vmadd_d(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {
        Axpy(zL, xL, (half)2.0, ELEMS);
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vcvt_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(float));
    TBuf<TPosition::VECCALC> zFBuf; pipe.InitBuffer(zFBuf, SLOTS * ELEMS * sizeof(float));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<float> zL = zBuf.Get<float>();
    LocalTensor<float> zF = zFBuf.Get<float>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<float> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ float *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Cast((zF[u * ELEMS]), xL, RoundMode::CAST_NONE, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vreduce_sum_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(float));
    
    LocalTensor<float> xL = xBuf.Get<float>();
    LocalTensor<float> yL = yBuf.Get<float>();
    LocalTensor<float> zL = zBuf.Get<float>();
    
    GlobalTensor<float> xGm, yGm;
    GlobalTensor<float> zGm;
    xGm.SetGlobalBuffer((__gm__ float *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ float *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ float *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            WholeReduceSum<float>(zL[u * ELEMS], xL, mk, 1, 1, 1, 8);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vreduce_max_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(float));
    
    LocalTensor<float> xL = xBuf.Get<float>();
    LocalTensor<float> yL = yBuf.Get<float>();
    LocalTensor<float> zL = zBuf.Get<float>();
    
    GlobalTensor<float> xGm, yGm;
    GlobalTensor<float> zGm;
    xGm.SetGlobalBuffer((__gm__ float *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ float *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ float *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            WholeReduceMax<float>(zL[u * ELEMS], xL, mk, 1, 1, 1, 8);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vreduce_min_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(float));
    
    LocalTensor<float> xL = xBuf.Get<float>();
    LocalTensor<float> yL = yBuf.Get<float>();
    LocalTensor<float> zL = zBuf.Get<float>();
    
    GlobalTensor<float> xGm, yGm;
    GlobalTensor<float> zGm;
    xGm.SetGlobalBuffer((__gm__ float *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ float *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ float *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            WholeReduceMin<float>(zL[u * ELEMS], xL, mk, 1, 1, 1, 8);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vblockreduce_sum_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(float));
    
    LocalTensor<float> xL = xBuf.Get<float>();
    LocalTensor<float> yL = yBuf.Get<float>();
    LocalTensor<float> zL = zBuf.Get<float>();
    
    GlobalTensor<float> xGm, yGm;
    GlobalTensor<float> zGm;
    xGm.SetGlobalBuffer((__gm__ float *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ float *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ float *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            BlockReduceSum<float>(zL[u * ELEMS], xL, 1, 64, 1, 1, 256);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vcmpv_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint8_t));
    TBuf<TPosition::VECCALC> mBuf; pipe.InitBuffer(mBuf, SLOTS * 32 * sizeof(uint8_t));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<uint8_t> zL = zBuf.Get<uint8_t>();
    LocalTensor<uint8_t> mL = mBuf.Get<uint8_t>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<uint8_t> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint8_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Compare(mL[u * 32], xL, yL, CMPMODE::LT, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vcmpvs_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(uint8_t));
    TBuf<TPosition::VECCALC> mBuf; pipe.InitBuffer(mBuf, SLOTS * 32 * sizeof(uint8_t));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<uint8_t> zL = zBuf.Get<uint8_t>();
    LocalTensor<uint8_t> mL = mBuf.Get<uint8_t>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<uint8_t> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ uint8_t *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            CompareScalar(mL[u * 32], xL, (half)1.0, CMPMODE::LT, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vsel_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    TBuf<TPosition::VECCALC> mBuf; pipe.InitBuffer(mBuf, SLOTS * 32 * sizeof(uint8_t));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    LocalTensor<uint8_t> mL = mBuf.Get<uint8_t>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
    GlobalTensor<uint8_t> mGm;
    mGm.SetGlobalBuffer((__gm__ uint8_t *)i, SLOTS * 32);
    DataCopy(mL, mGm, SLOTS * 32);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Select(zL[u * ELEMS], mL[u * 32], xL, yL, SELMODE::VSEL_CMPMASK_SPR, (uint64_t)0, 2, BinaryRepeatParams(1,1,1,1,1,1));
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vbitsort_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    TBuf<TPosition::VECCALC> iBuf; pipe.InitBuffer(iBuf, ELEMS * sizeof(uint32_t));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    LocalTensor<uint32_t> iL = iBuf.Get<uint32_t>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
    GlobalTensor<uint32_t> iGm;
    iGm.SetGlobalBuffer((__gm__ uint32_t *)i, ELEMS);
    DataCopy(iL, iGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Sort32<half>(zL[u * ELEMS], xL, iL, 4);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}

extern "C" __global__ __aicore__ void bench_vgather_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(half));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    TBuf<TPosition::VECCALC> iBuf; pipe.InitBuffer(iBuf, ELEMS * sizeof(uint32_t));
    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();
    LocalTensor<uint32_t> iL = iBuf.Get<uint32_t>();
    GlobalTensor<half> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ half *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
    GlobalTensor<uint32_t> iGm;
    iGm.SetGlobalBuffer((__gm__ uint32_t *)i, ELEMS);
    DataCopy(iL, iGm, ELEMS);

    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e);
    WaitFlag<HardEvent::MTE2_V>(e);
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Gather(zL[u * ELEMS], xL, iL, (uint32_t)0, ELEMS);
        }
    }
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2);
    WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, SLOTS * ELEMS);
}
