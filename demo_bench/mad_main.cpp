/**
 * mad 变体 II 测量 host 驱动
 * 每个变体 kernel: NUM_C=8 个 L0C 轮转, repeat 条 mad
 * II = 双点斜率 / repeat
 */
#include <cstdio>
#include <cstdint>
#include <chrono>
#include "acl/acl.h"

using LaunchFn = uint32_t (*)(uint32_t, void *, uint8_t *, uint8_t *, uint8_t *, uint32_t);

#define DECL(name) extern "C" uint32_t aclrtlaunch_mad_##name(uint32_t, void *, uint8_t *, uint8_t *, uint8_t *, uint32_t);

// 4 dtype × 6 shape
DECL(f16_m16n16k16) DECL(f16_m32n32k32) DECL(f16_m64n64k64) DECL(f16_m16n16k64) DECL(f16_m64n16k16) DECL(f16_m16n64k16)
DECL(bf16_m16n16k16) DECL(bf16_m32n32k32) DECL(bf16_m64n64k64) DECL(bf16_m16n16k64) DECL(bf16_m64n16k16) DECL(bf16_m16n64k16)
DECL(f32_m16n16k16) DECL(f32_m32n32k32) DECL(f32_m64n64k64) DECL(f32_m16n16k64) DECL(f32_m64n16k16) DECL(f32_m16n64k16)
DECL(s8_m16n16k16) DECL(s8_m32n32k32) DECL(s8_m64n64k64) DECL(s8_m16n16k64) DECL(s8_m64n16k16) DECL(s8_m16n64k16)

struct Entry { const char *name; LaunchFn fn; int m, n, k; const char *dt; };
static const Entry entries[] = {
    {"f16_m16n16k16", aclrtlaunch_mad_f16_m16n16k16, 16, 16, 16, "fp16"},
    {"f16_m32n32k32", aclrtlaunch_mad_f16_m32n32k32, 32, 32, 32, "fp16"},
    {"f16_m64n64k64", aclrtlaunch_mad_f16_m64n64k64, 64, 64, 64, "fp16"},
    {"f16_m16n16k64", aclrtlaunch_mad_f16_m16n16k64, 16, 16, 64, "fp16"},
    {"f16_m64n16k16", aclrtlaunch_mad_f16_m64n16k16, 64, 16, 16, "fp16"},
    {"f16_m16n64k16", aclrtlaunch_mad_f16_m16n64k16, 16, 64, 16, "fp16"},
    {"bf16_m16n16k16", aclrtlaunch_mad_bf16_m16n16k16, 16, 16, 16, "bf16"},
    {"bf16_m32n32k32", aclrtlaunch_mad_bf16_m32n32k32, 32, 32, 32, "bf16"},
    {"bf16_m64n64k64", aclrtlaunch_mad_bf16_m64n64k64, 64, 64, 64, "bf16"},
    {"bf16_m16n16k64", aclrtlaunch_mad_bf16_m16n16k64, 16, 16, 64, "bf16"},
    {"bf16_m64n16k16", aclrtlaunch_mad_bf16_m64n16k16, 64, 16, 16, "bf16"},
    {"bf16_m16n64k16", aclrtlaunch_mad_bf16_m16n64k16, 16, 64, 16, "bf16"},
    {"f32_m16n16k16", aclrtlaunch_mad_f32_m16n16k16, 16, 16, 16, "fp32"},
    {"f32_m32n32k32", aclrtlaunch_mad_f32_m32n32k32, 32, 32, 32, "fp32"},
    {"f32_m64n64k64", aclrtlaunch_mad_f32_m64n64k64, 64, 64, 64, "fp32"},
    {"f32_m16n16k64", aclrtlaunch_mad_f32_m16n16k64, 16, 16, 64, "fp32"},
    {"f32_m64n16k16", aclrtlaunch_mad_f32_m64n16k16, 64, 16, 16, "fp32"},
    {"f32_m16n64k16", aclrtlaunch_mad_f32_m16n64k16, 16, 64, 16, "fp32"},
    {"s8_m16n16k16", aclrtlaunch_mad_s8_m16n16k16, 16, 16, 16, "int8"},
    {"s8_m32n32k32", aclrtlaunch_mad_s8_m32n32k32, 32, 32, 32, "int8"},
    {"s8_m64n64k64", aclrtlaunch_mad_s8_m64n64k64, 64, 64, 64, "int8"},
    {"s8_m16n16k64", aclrtlaunch_mad_s8_m16n16k64, 16, 16, 64, "int8"},
    {"s8_m64n16k16", aclrtlaunch_mad_s8_m64n16k16, 64, 16, 16, "int8"},
    {"s8_m16n64k16", aclrtlaunch_mad_s8_m16n64k16, 16, 64, 16, "int8"},
};

int main()
{
    if (aclInit(nullptr) != ACL_SUCCESS) return 1;
    if (aclrtSetDevice(0) != ACL_SUCCESS) return 1;
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    uint8_t *aD, *bD, *cD;
    aclrtMalloc((void **)&aD, 64 * 64 * 4, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&bD, 64 * 64 * 4, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&cD, 8 * 64 * 64 * 4, ACL_MEM_MALLOC_HUGE_FIRST);

    const uint32_t R1 = 2000, R2 = 40000;
    printf("%-8s %-16s %10s %12s\n", "dtype", "shape(m×n×k)", "II(ns)", "GFLOPS/核");
    printf("-----------------------------------------------\n");
    for (auto &e : entries) {
        auto run = [&](uint32_t r) {
            auto t0 = std::chrono::steady_clock::now();
            e.fn(1, stream, aD, bD, cD, r);
            aclrtSynchronizeStream(stream);
            auto t1 = std::chrono::steady_clock::now();
            return std::chrono::duration<double>(t1 - t0).count();
        };
        run(100);   // 预热
        double t1 = run(R1), t2 = run(R2);
        double ii = (t2 - t1) / (double)(R2 - R1) * 1e9;
        double flops = 2.0 * e.m * e.n * e.k;
        printf("%-8s %-16s %10.3f %12.1f\n", e.dt, e.name + 4, ii, flops / ii);
    }
    aclrtFree(aD); aclrtFree(bD); aclrtFree(cD);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();
    return 0;
}
