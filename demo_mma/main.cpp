/**
 * Host:启动 mma_bench kernel,双点斜率法测单次 mad(16x16x16) 耗时
 *
 * t(R) = t_overhead + R * t_mad
 * 用 R1、R2 两次运行相减:t_mad = (t2 - t1) / (R2 - R1)
 */
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <initializer_list>
#include "acl/acl.h"

extern "C" uint32_t aclrtlaunch_mma_bench(uint32_t blockDim, void *stream,
                                          uint8_t *a, uint8_t *b, uint8_t *c, uint32_t repeat);

#define ACL_CHECK(ret, msg)                                            \
    do {                                                               \
        if ((ret) != ACL_SUCCESS) {                                    \
            printf("[ERROR] %s failed, ret=%d\n", msg, (int)(ret));    \
            return 1;                                                  \
        }                                                              \
    } while (0)

constexpr int32_t FRACTAL_ELEMS = 256;
constexpr double FLOPS_PER_MAD = 2.0 * 16 * 16 * 16; // 8192

static double RunOnce(uint32_t blockDim, uint32_t repeat, uint8_t *a, uint8_t *b, uint8_t *c,
                      aclrtStream stream)
{
    auto t0 = std::chrono::steady_clock::now();
    uint32_t ret = aclrtlaunch_mma_bench(blockDim, stream, a, b, c, repeat);
    if (ret != 0) {
        printf("[ERROR] aclrtlaunch_mma_bench ret=%u (blockDim=%u)\n", ret, blockDim);
        return -1;
    }
    aclError syncRet = aclrtSynchronizeStream(stream);
    if (syncRet != ACL_SUCCESS) {
        const char *errMsg = aclGetRecentErrMsg();
        printf("[ERROR] sync failed (blockDim=%u), ret=%d, msg=%s\n", blockDim, (int)syncRet,
               errMsg ? errMsg : "null");
        return -1;
    }
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

int main(int argc, char **argv)
{
    // 可选参数: R1 R2 [blockDim列表...]
    uint32_t R1 = 20000, R2 = 400000;
    if (argc >= 3) { R1 = atoi(argv[1]); R2 = atoi(argv[2]); }
    ACL_CHECK(aclInit(nullptr), "aclInit");
    ACL_CHECK(aclrtSetDevice(0), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    ACL_CHECK(aclrtCreateStream(&stream), "aclrtCreateStream");

    // 最多支持 64 个核的输入/输出区间
    const uint32_t maxCores = 64;
    const size_t abSize = maxCores * FRACTAL_ELEMS * sizeof(uint16_t);
    const size_t cSize = maxCores * FRACTAL_ELEMS * sizeof(float);

    uint16_t *abHost;
    float *cHost;
    ACL_CHECK(aclrtMallocHost((void **)&abHost, abSize), "mallocHost ab");
    ACL_CHECK(aclrtMallocHost((void **)&cHost, cSize), "mallocHost c");

    // 输入全填 1.0(fp16 0x3C00) => 每次 mad 每个 C 元素 += 16
    for (size_t i = 0; i < maxCores * FRACTAL_ELEMS; i++) abHost[i] = 0x3C00;

    uint8_t *aDev, *bDev, *cDev;
    ACL_CHECK(aclrtMalloc((void **)&aDev, abSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc a");
    ACL_CHECK(aclrtMalloc((void **)&bDev, abSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc b");
    ACL_CHECK(aclrtMalloc((void **)&cDev, cSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc c");
    ACL_CHECK(aclrtMemcpy(aDev, abSize, abHost, abSize, ACL_MEMCPY_HOST_TO_DEVICE), "copy a");
    ACL_CHECK(aclrtMemcpy(bDev, abSize, abHost, abSize, ACL_MEMCPY_HOST_TO_DEVICE), "copy b");

    const uint32_t R1unused = 0; (void)R1unused;

    printf("%-10s %10s %12s %12s %14s\n", "blockDim", "t_mad(ns)", "per-core(TFLOPS)", "total(TFLOPS)", "verify(C[0])");
    for (uint32_t blockDim : {1u, 2u, 4u, 8u, 16u, 20u, 24u}) {
        // 预热 + 正确性校验
        double tw = RunOnce(blockDim, R1, aDev, bDev, cDev, stream);
        if (tw < 0) continue;

        double t1 = RunOnce(blockDim, R1, aDev, bDev, cDev, stream);
        double t2 = RunOnce(blockDim, R2, aDev, bDev, cDev, stream);
        if (t1 < 0 || t2 < 0) continue;

        double tMad = (t2 - t1) / (R2 - R1);            // 单次 mad 秒数
        double tflopsCore = FLOPS_PER_MAD / tMad / 1e12;
        double tflopsTotal = tflopsCore * blockDim;

        // 校验: kernel 交替写两块 L0C, cL0c 累加了 ceil(R2/2) 次, C[0]=16*ceil(R2/2)
        ACL_CHECK(aclrtMemcpy(cHost, cSize, cDev, cSize, ACL_MEMCPY_DEVICE_TO_HOST), "copy c");
        double expect = 16.0 * ((R2 + 7) / 8); // 8 块 L0C 轮转, 0 号块命中 ceil(R2/8) 次
        const char *ok = std::fabs(cHost[0] - expect) / expect < 1e-3 ? "OK" : "MISMATCH";
        printf("%-10u %10.3f %16.3f %14.2f   C[0]=%.1f (expect %.1f) %s\n",
               blockDim, tMad * 1e9, tflopsCore, tflopsTotal, (double)cHost[0], expect, ok);
    }

    aclrtFree(aDev); aclrtFree(bDev); aclrtFree(cDev);
    aclrtFreeHost(abHost); aclrtFreeHost(cHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();
    return 0;
}
