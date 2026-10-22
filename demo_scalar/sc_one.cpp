/**
 * 标量指令(0x40 类)反推模板 — AIV 侧
 * 每次只改一个标量操作/常量,diff 定位编码
 */
#include "kernel_operator.h"

using namespace AscendC;

#ifndef OP
#define OP 0
#endif
#ifndef KONST
#define KONST 1
#endif

extern "C" __global__ __aicore__ void sc_one(GM_ADDR z, uint32_t s, uint32_t s2)
{
    TPipe pipe;
    TBuf<TPosition::VECCALC> zBuf;
    pipe.InitBuffer(zBuf, 256 * sizeof(half));
    LocalTensor<half> zL = zBuf.Get<half>();
    GlobalTensor<half> zGm;
    zGm.SetGlobalBuffer((__gm__ half *)z, 256);

    uint32_t r;
#if OP == 0
    r = s + KONST;                    // 加法/小立即数
#elif OP == 1
    r = s - KONST;                    // 减法
#elif OP == 2
    r = s * KONST;                    // 乘法
#elif OP == 3
    r = s << KONST;                   // 左移
#elif OP == 4
    r = s >> KONST;                   // 右移
#elif OP == 5
    r = s & KONST;                    // 与
#elif OP == 6
    r = s | KONST;                    // 或
#elif OP == 7
    r = s ^ KONST;                    // 异或
#elif OP == 8
    r = (s < KONST) ? s : 0;          // 比较+选择
#elif OP == 9
    r = ~s;                           // 取反(无立即数)
#elif OP == 10
    r = 0;                            // 循环: for i<KONST: r+=s
    for (uint32_t i = 0; i < KONST; i++) r += s;
#elif OP == 11
    r = GetBlockIdx() + KONST;        // block id 读取
#elif OP == 12
    r = s;                            // 恒等(基线:只读参数)
#elif OP == 13
    r = KONST;                        // 纯常量物化(movi)
#elif OP == 14
    if (s < KONST) {                  // 真实分支: if/else
        r = s + 1;
    } else {
        r = s + 2;
    }
#elif OP == 15
    r = 0;
    for (uint32_t i = s; i < KONST; i++) {  // 运行时下界的循环(防展开)
        r += i;
    }
#elif OP == 16
    r = s & s2;      // 寄存器-寄存器: 与
#elif OP == 17
    r = s | s2;      // 或
#elif OP == 18
    r = s ^ s2;      // 异或
#elif OP == 19
    r = s << (s2 & 7);  // 寄存器左移
#elif OP == 20
    r = s >> (s2 & 7);  // 寄存器右移
#elif OP == 21
    r = s * s2;      // 寄存器乘法
#endif

    DataCopy(zL, zGm, 256);
    Adds(zL, zL, (half)1.0, 256);
    TEventID e = pipe.FetchEventID(HardEvent::V_MTE3);
    SetFlag<HardEvent::V_MTE3>(e);
    WaitFlag<HardEvent::V_MTE3>(e);
    DataCopy(zGm, zL, r);             // r 直接作为 count 保持活性(不掩码)
}
