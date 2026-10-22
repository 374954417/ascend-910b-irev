#include "kernel_operator.h"
using namespace AscendC;
extern "C" __global__ __aicore__ void probe_a(GM_ADDR x, GM_ADDR z)
{
    TPipe pipe; TBuf<TPosition::VECCALC> xBuf, zBuf;
    pipe.InitBuffer(xBuf, 256 * sizeof(half)); pipe.InitBuffer(zBuf, 256 * sizeof(half));
    LocalTensor<half> xL = xBuf.Get<half>(), zL = zBuf.Get<half>();
    GlobalTensor<half> xGm, zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, 256); zGm.SetGlobalBuffer((__gm__ half *)z, 256);
    DataCopy(xL, xGm, 256);
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e); WaitFlag<HardEvent::MTE2_V>(e);
    Add(zL, xL, xL, 256);
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2); WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, 256);
}
extern "C" __global__ __aicore__ void probe_b(GM_ADDR x, GM_ADDR z)
{
    TPipe pipe; TBuf<TPosition::VECCALC> xBuf, zBuf;
    pipe.InitBuffer(xBuf, 256 * sizeof(half)); pipe.InitBuffer(zBuf, 256 * sizeof(half));
    LocalTensor<half> xL = xBuf.Get<half>(), zL = zBuf.Get<half>();
    GlobalTensor<half> xGm, zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, 256); zGm.SetGlobalBuffer((__gm__ half *)z, 256);
    DataCopy(xL, xGm, 256);
    TEventID e = pipe.FetchEventID(HardEvent::MTE2_V);
    SetFlag<HardEvent::MTE2_V>(e); WaitFlag<HardEvent::MTE2_V>(e);
    Mul(zL, xL, xL, 256);
    TEventID e2 = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e2); WaitFlag<HardEvent::V_MTE3>(e2);
    DataCopy(zGm, zL, 256);
}
