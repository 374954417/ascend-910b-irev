/**
 * 手写 Matmul 算子(cube 侧): C[16x16] = A[16x64] x B[64x16]
 * fp16 输入, fp32 累加;K 方向 4 个分形循环, 累加进同一块 L0C。
 *
 * 数据流: GM --(LoadData)--> L0A/L0B --(mad)--> L0C --(fixp)--> GM
 * 同步: MTE2_M(加载→计算) / M_MTE2(计算完→允许下一轮覆写) / M_FIX(计算→回写)
 */
#include "kernel_operator.h"

using namespace AscendC;

constexpr int32_t M_SIZE = 16;
constexpr int32_t N_SIZE = 16;
constexpr int32_t K_SIZE = 64;
constexpr int32_t K_FRACT = K_SIZE / 16;      // 4 个 k 分形
constexpr int32_t FRACTAL_ELEMS = 256;        // 16x16

extern "C" __global__ __aicore__ void matmul_basic(GM_ADDR a, GM_ADDR b, GM_ADDR c)
{
    TPipe pipe;
    TBuf<TPosition::A2> a2Buf;
    TBuf<TPosition::B2> b2Buf;
    TBuf<TPosition::CO1> c0Buf;
    pipe.InitBuffer(a2Buf, FRACTAL_ELEMS * sizeof(half));
    pipe.InitBuffer(b2Buf, FRACTAL_ELEMS * sizeof(half));
    pipe.InitBuffer(c0Buf, FRACTAL_ELEMS * sizeof(float));

    LocalTensor<half> a2 = a2Buf.Get<half>();
    LocalTensor<half> b2 = b2Buf.Get<half>();
    LocalTensor<float> cL0c = c0Buf.Get<float>();

    GlobalTensor<half> aGm, bGm;
    GlobalTensor<float> cGm;
    aGm.SetGlobalBuffer((__gm__ half *)a, M_SIZE * K_SIZE);
    bGm.SetGlobalBuffer((__gm__ half *)b, K_SIZE * N_SIZE);
    cGm.SetGlobalBuffer((__gm__ float *)c, M_SIZE * N_SIZE);

    LoadData2DParams ld(0, 1, 0, 0, 0, false, 0);  // 单分形直载
    MmadParams mm;
    mm.m = M_SIZE;
    mm.n = N_SIZE;
    mm.k = 16;
    mm.isBias = false;
    mm.unitFlag = 1;
    mm.cmatrixSource = 0;

    for (int32_t kb = 0; kb < K_FRACT; kb++) {
        // 加载第 kb 个分形: A 在 host 已按分形块预排布, B 行主序下第 kb 块天然连续
        LoadData(a2, aGm[kb * FRACTAL_ELEMS], ld);
        LoadData(b2, bGm[kb * FRACTAL_ELEMS], ld);
        TEventID e0 = pipe.FetchEventID(HardEvent::MTE2_M);
        SetFlag<HardEvent::MTE2_M>(e0);
        WaitFlag<HardEvent::MTE2_M>(e0);

        mm.cmatrixInitVal = (kb == 0);           // 首个分形清零 L0C, 其余累加
        Mmad(cL0c, a2, b2, mm);

        TEventID e1 = pipe.FetchEventID(HardEvent::M_MTE2);
        SetFlag<HardEvent::M_MTE2>(e1);          // mad 读完 L0 前禁止下一轮 LoadData 覆写
        WaitFlag<HardEvent::M_MTE2>(e1);
    }

    TEventID e2 = pipe.FetchEventID(HardEvent::M_FIX);
    SetFlag<HardEvent::M_FIX>(e2);
    WaitFlag<HardEvent::M_FIX>(e2);
    DataCopyCO12DstParams cp(N_SIZE, M_SIZE, 0, 0, QuantMode_t::NoQuant, 0, false, false);
    DataCopy(cGm, cL0c, cp);
}
