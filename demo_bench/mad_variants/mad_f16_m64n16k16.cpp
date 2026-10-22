#include "kernel_operator.h"
using namespace AscendC;
constexpr int32_t M = 64, N = 16, K = 16;
constexpr int32_t NUM_C = 8;
extern "C" __global__ __aicore__ void mad_f16_m64n16k16(GM_ADDR a, GM_ADDR b, GM_ADDR c, uint32_t repeat)
{{
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf; pipe.InitBuffer(a2Buf, M * K * sizeof(half));
    TBuf<TPosition::B2> b2Buf; pipe.InitBuffer(b2Buf, K * N * sizeof(half));
    TBuf<TPosition::CO1> c0Buf; pipe.InitBuffer(c0Buf, NUM_C * M * N * sizeof(float));
    LocalTensor<half> a2 = a2Buf.Get<half>();
    LocalTensor<half> b2 = b2Buf.Get<half>();
    LocalTensor<float> cAll = c0Buf.Get<float>();
    GlobalTensor<half> aGm, bGm;
    GlobalTensor<float> cGm;
    aGm.SetGlobalBuffer((__gm__ half *)a, M * K);
    bGm.SetGlobalBuffer((__gm__ half *)b, K * N);
    cGm.SetGlobalBuffer((__gm__ float *)c, NUM_C * M * N);
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
