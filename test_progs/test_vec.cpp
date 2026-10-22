/**
 * AIV 向量侧测试 kernel:DataCopy(GM<->UB, 含 strided)+ Add/Sub/Mul/Adds/Exp + 显式 flag
 * 覆盖已破译指令:dcopy 方向字、blockCount/blockLen/stride 参数字、
 *                向量二元/一元/标量算术、set_flag/wait_flag
 */
#include "kernel_operator.h"

using namespace AscendC;

extern "C" __global__ __aicore__ void test_vec(GM_ADDR x, GM_ADDR y, GM_ADDR z)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> xBuf, yBuf, zBuf;
    pipe.InitBuffer(xBuf, 512 * sizeof(half));
    pipe.InitBuffer(yBuf, 512 * sizeof(half));
    pipe.InitBuffer(zBuf, 512 * sizeof(half));

    LocalTensor<half> xL = xBuf.Get<half>();
    LocalTensor<half> yL = yBuf.Get<half>();
    LocalTensor<half> zL = zBuf.Get<half>();

    GlobalTensor<half> xGm, yGm, zGm;
    xGm.SetGlobalBuffer((__gm__ half *)x, 512);
    yGm.SetGlobalBuffer((__gm__ half *)y, 512);
    zGm.SetGlobalBuffer((__gm__ half *)z, 512);

    // 连续拷贝 GM->UB(count 版)
    DataCopy(xL, xGm, 512);
    // strided 拷贝:blockCount=2, blockLen=4(x32B), srcStride=1, dstStride=2
    DataCopyParams dcp(2, 4, 1, 2);
    DataCopy(yL, yGm, dcp);
    SetFlag<HardEvent::MTE2_V>(0);
    WaitFlag<HardEvent::MTE2_V>(0);

    // 向量算术:二元 Add/Sub/Mul + 标量 Adds + 一元 Exp
    Add(zL, xL, yL, 256);
    Sub(zL[256], xL, yL, 256);
    Mul(xL, xL, yL, 256);
    Adds(yL, yL, (half)2.0, 256);
    Exp(zL, zL, 128);

    SetFlag<HardEvent::V_MTE3>(1);
    WaitFlag<HardEvent::V_MTE3>(1);
    // 写回 UB->GM
    DataCopy(zGm, zL, 512);
}
