/**
 * cube 侧测试 kernel:LoadData + 两条不同参数的 mad + fixp 回写
 * 覆盖已破译指令:ld2d issue/config、mad(m/n/k/initVal/unitFlag)、fixp(nSize/mSize/dstStride)
 */
#include "kernel_operator.h"

using namespace AscendC;

extern "C" __global__ __aicore__ void test_cube(GM_ADDR a, GM_ADDR b, GM_ADDR c)
{
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf; pipe.InitBuffer(a2Buf, 64 * 64 * sizeof(half));
    TBuf<TPosition::B2> b2Buf; pipe.InitBuffer(b2Buf, 64 * 64 * sizeof(half));
    TBuf<TPosition::CO1> c0Buf; pipe.InitBuffer(c0Buf, 64 * 64 * sizeof(float));

    LocalTensor<half> a2 = a2Buf.Get<half>();
    LocalTensor<half> b2 = b2Buf.Get<half>();
    LocalTensor<float> cAll = c0Buf.Get<float>();

    GlobalTensor<half> aGm, bGm;
    GlobalTensor<float> cGm;
    aGm.SetGlobalBuffer((__gm__ half *)a, 64 * 64);
    bGm.SetGlobalBuffer((__gm__ half *)b, 64 * 64);
    cGm.SetGlobalBuffer((__gm__ float *)c, 64 * 64);

    // LoadData: repeatTimes=2, srcStride=1 —— 触发 extended/compact config
    LoadData2DParams ld(0, 2, 1, 0, 0, false, 0);
    LoadData(a2, aGm, ld);
    LoadData(b2, bGm, ld);
    TEventID e0 = pipe.FetchEventID(HardEvent::MTE2_M);
    SetFlag<HardEvent::MTE2_M>(e0);
    WaitFlag<HardEvent::MTE2_M>(e0);

    // mad #1: m=16 n=32 k=64, initVal=1
    MmadParams mm;
    mm.m = 16; mm.n = 32; mm.k = 64;
    mm.unitFlag = 1; mm.cmatrixSource = 0; mm.cmatrixInitVal = true;
    Mmad(cAll, a2, b2, mm);
    // mad #2: 同 shape,initVal=0(累加),写第二块 C
    mm.cmatrixInitVal = false;
    Mmad(cAll[512], a2, b2, mm);

    TEventID e1 = pipe.FetchEventID(HardEvent::M_FIX);
    SetFlag<HardEvent::M_FIX>(e1);
    WaitFlag<HardEvent::M_FIX>(e1);

    // fixp 回写: nSize=32, mSize=16, dstStride=1
    DataCopyCO12DstParams cp(32, 16, 1, 0, QuantMode_t::NoQuant, 0, false, false);
    DataCopy(cGm, cAll, cp);
}
