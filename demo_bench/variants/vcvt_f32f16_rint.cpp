#include "kernel_operator.h"
using namespace AscendC;

extern "C" __global__ __aicore__ void bench_vcvt_f32f16_rint_i(GM_ADDR x, GM_ADDR y, GM_ADDR z, GM_ADDR i, uint32_t repeat)
{
    constexpr int32_t ELEMS = 256;
    constexpr int32_t SLOTS = 8;
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(yBuf, ELEMS * sizeof(float));
    pipe.InitBuffer(zBuf, SLOTS * ELEMS * sizeof(half));
    LocalTensor<float> xL = xBuf.Get<float>();
    LocalTensor<float> yL = yBuf.Get<float>();
    LocalTensor<half> zL = zBuf.Get<half>();
    
    GlobalTensor<float> xGm, yGm;
    GlobalTensor<half> zGm;
    xGm.SetGlobalBuffer((__gm__ float *)x, ELEMS);
    yGm.SetGlobalBuffer((__gm__ float *)y, ELEMS);
    zGm.SetGlobalBuffer((__gm__ half *)z, SLOTS * ELEMS);
    DataCopy(xL, xGm, ELEMS);
    DataCopy(yL, yGm, ELEMS);
    uint64_t mk[4] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0, 0};
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e); WaitFlag<HardEvent::MTE2_V>(e);
    int64_t cyc0 = GetSystemCycle();
    for (uint32_t r = 0; r < repeat; r++) {

        #pragma unroll
        for (int32_t u = 0; u < SLOTS; u++) {
            Cast(zL[u * ELEMS], xL, RoundMode::CAST_RINT, ELEMS);
        }
    }
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
}
