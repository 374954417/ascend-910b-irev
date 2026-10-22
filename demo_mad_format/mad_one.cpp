/**
 * mad 指令编码反推模板 v2
 * 新增: A/B/C 的 L0 偏移、dtype 选择、偏置
 */
#include "kernel_operator.h"

using namespace AscendC;

#ifndef M_VAL
#define M_VAL 16
#endif
#ifndef N_VAL
#define N_VAL 16
#endif
#ifndef K_VAL
#define K_VAL 16
#endif
#ifndef INIT_VAL
#define INIT_VAL 1
#endif
#ifndef UNIT_FLAG
#define UNIT_FLAG 1
#endif
#ifndef NUM_MAD
#define NUM_MAD 1
#endif
// DTYPE: 0=half->float 1=int8->int32 2=bf16->float 3=float->float
#ifndef DTYPE
#define DTYPE 0
#endif
#ifndef A_OFF
#define A_OFF 0    // A 在 L0A 中的元素偏移
#endif
#ifndef B_OFF
#define B_OFF 0    // B 在 L0B 中的元素偏移
#endif
#ifndef C_OFF
#define C_OFF 0    // C 在 L0C 中的元素偏移
#endif

#if DTYPE == 1
using ABT = int8_t;  using CT = int32_t;
#elif DTYPE == 2
using ABT = bfloat16_t; using CT = float;
#elif DTYPE == 3
using ABT = float;   using CT = float;
#else
using ABT = half;    using CT = float;
#endif

constexpr int32_t AB_ELEMS = 64 * 64;
constexpr int32_t C_ELEMS = 64 * 64;

extern "C" __global__ __aicore__ void mad_one(GM_ADDR a, GM_ADDR b, GM_ADDR c)
{
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf; pipe.InitBuffer(a2Buf, AB_ELEMS * sizeof(ABT));
    TBuf<TPosition::B2> b2Buf; pipe.InitBuffer(b2Buf, AB_ELEMS * sizeof(ABT));
    TBuf<TPosition::CO1> c0Buf; pipe.InitBuffer(c0Buf, C_ELEMS * sizeof(CT));

    LocalTensor<ABT> a2 = a2Buf.Get<ABT>();
    LocalTensor<ABT> b2 = b2Buf.Get<ABT>();
    LocalTensor<CT> cAll = c0Buf.Get<CT>();

    GlobalTensor<ABT> aGm, bGm;
    GlobalTensor<CT> cGm;
    aGm.SetGlobalBuffer((__gm__ ABT *)a, AB_ELEMS);
    bGm.SetGlobalBuffer((__gm__ ABT *)b, AB_ELEMS);
    cGm.SetGlobalBuffer((__gm__ CT *)c, C_ELEMS);

    LoadData2DParams ld(0, 4, 1, 0, 0, false, 0);
    LoadData(a2, aGm, ld);
    LoadData(b2, bGm, ld);
    TEventID evtLoad = pipe.FetchEventID(HardEvent::MTE2_M);
    SetFlag<HardEvent::MTE2_M>(evtLoad);
    WaitFlag<HardEvent::MTE2_M>(evtLoad);

    MmadParams mm;
    mm.m = M_VAL;
    mm.n = N_VAL;
    mm.k = K_VAL;
    mm.isBias = false;
    mm.unitFlag = UNIT_FLAG;
    mm.cmatrixSource = 0;
    mm.cmatrixInitVal = (INIT_VAL != 0);

#pragma unroll
    for (int32_t i = 0; i < NUM_MAD; i++) {
        Mmad(cAll[C_OFF + i * 256], a2[A_OFF], b2[B_OFF], mm);
    }

    TEventID evtFix = pipe.FetchEventID(HardEvent::M_FIX);
    SetFlag<HardEvent::M_FIX>(evtFix);
    WaitFlag<HardEvent::M_FIX>(evtFix);
    DataCopyCO12DstParams cp(16, 16, 0, 0, QuantMode_t::NoQuant, 0, false, false);
    DataCopy(cGm, cAll, cp);
}
