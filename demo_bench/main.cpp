/**
 * 全计算指令 II/延迟测量 host 驱动
 * indep kernel: 每迭代 8 条独立指令 → II = dt/(R×8)
 * dep kernel:   每迭代 1 条串行依赖指令 → latency = dt/R
 * 双点斜率法消除 launch/加载开销
 */
#include <cstdio>
#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include "acl/acl.h"

using LaunchFn = uint32_t (*)(uint32_t, void *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint32_t);

#define DECL(name) extern "C" uint32_t aclrtlaunch_bench_##name(uint32_t, void *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint32_t);

DECL(vadd_i) DECL(vadd_d) DECL(vsub_i) DECL(vsub_d) DECL(vmul_i) DECL(vmul_d)
DECL(vmax_i) DECL(vmax_d) DECL(vmin_i) DECL(vmin_d) DECL(vdiv_i) DECL(vdiv_d)
DECL(vadds_i) DECL(vadds_d) DECL(vmuls_i) DECL(vmuls_d) DECL(vmaxs_i) DECL(vmaxs_d) DECL(vmins_i) DECL(vmins_d)
DECL(vexp_i) DECL(vexp_d) DECL(vln_i) DECL(vln_d) DECL(vabs_i) DECL(vabs_d)
DECL(vsqrt_i) DECL(vsqrt_d) DECL(vrsqrt_i) DECL(vrsqrt_d) DECL(vrelu_i) DECL(vrelu_d)
DECL(vand_i) DECL(vand_d) DECL(vor_i) DECL(vor_d) DECL(vshl_i) DECL(vshl_d)
DECL(vmadd_i) DECL(vmadd_d) DECL(vcvt_i)
DECL(vreduce_sum_i) DECL(vreduce_max_i) DECL(vreduce_min_i) DECL(vblockreduce_sum_i)
DECL(vcmpv_i) DECL(vcmpvs_i) DECL(vsel_i) DECL(vbitsort_i) DECL(vgather_i)

struct Entry { const char *name; LaunchFn indep, dep; };
static const Entry entries[] = {
    {"vadd", aclrtlaunch_bench_vadd_i, aclrtlaunch_bench_vadd_d},
    {"vsub", aclrtlaunch_bench_vsub_i, aclrtlaunch_bench_vsub_d},
    {"vmul", aclrtlaunch_bench_vmul_i, aclrtlaunch_bench_vmul_d},
    {"vmax", aclrtlaunch_bench_vmax_i, aclrtlaunch_bench_vmax_d},
    {"vmin", aclrtlaunch_bench_vmin_i, aclrtlaunch_bench_vmin_d},
    {"vdiv", aclrtlaunch_bench_vdiv_i, aclrtlaunch_bench_vdiv_d},
    {"vadds", aclrtlaunch_bench_vadds_i, aclrtlaunch_bench_vadds_d},
    {"vmuls", aclrtlaunch_bench_vmuls_i, aclrtlaunch_bench_vmuls_d},
    {"vmaxs", aclrtlaunch_bench_vmaxs_i, aclrtlaunch_bench_vmaxs_d},
    {"vmins", aclrtlaunch_bench_vmins_i, aclrtlaunch_bench_vmins_d},
    {"vexp", aclrtlaunch_bench_vexp_i, aclrtlaunch_bench_vexp_d},
    {"vln", aclrtlaunch_bench_vln_i, aclrtlaunch_bench_vln_d},
    {"vabs", aclrtlaunch_bench_vabs_i, aclrtlaunch_bench_vabs_d},
    {"vsqrt", aclrtlaunch_bench_vsqrt_i, aclrtlaunch_bench_vsqrt_d},
    {"vrsqrt", aclrtlaunch_bench_vrsqrt_i, aclrtlaunch_bench_vrsqrt_d},
    {"vrelu", aclrtlaunch_bench_vrelu_i, aclrtlaunch_bench_vrelu_d},
    {"vand", aclrtlaunch_bench_vand_i, aclrtlaunch_bench_vand_d},
    {"vor", aclrtlaunch_bench_vor_i, aclrtlaunch_bench_vor_d},
    {"vshl", aclrtlaunch_bench_vshl_i, aclrtlaunch_bench_vshl_d},
    {"vmadd(axpy)", aclrtlaunch_bench_vmadd_i, aclrtlaunch_bench_vmadd_d},
    {"vcvt(f16→f32)", aclrtlaunch_bench_vcvt_i, nullptr},
    {"vreduce_sum", aclrtlaunch_bench_vreduce_sum_i, nullptr},
    {"vreduce_max", aclrtlaunch_bench_vreduce_max_i, nullptr},
    {"vreduce_min", aclrtlaunch_bench_vreduce_min_i, nullptr},
    {"vblockreduce_sum", aclrtlaunch_bench_vblockreduce_sum_i, nullptr},
    {"vcmpv", aclrtlaunch_bench_vcmpv_i, nullptr},
    {"vcmpvs", aclrtlaunch_bench_vcmpvs_i, nullptr},
    {"vsel", aclrtlaunch_bench_vsel_i, nullptr},
    {"vbitsort", aclrtlaunch_bench_vbitsort_i, nullptr},
    {"vgather", aclrtlaunch_bench_vgather_i, nullptr},
};

static double RunOnce(LaunchFn fn, uint32_t repeat, uint8_t *x, uint8_t *y, uint8_t *z, uint8_t *i,
                      aclrtStream stream)
{
    auto t0 = std::chrono::steady_clock::now();
    if (fn(1, stream, x, y, z, i, repeat) != 0) return -1;
    if (aclrtSynchronizeStream(stream) != ACL_SUCCESS) return -1;
    auto t1 = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

int main()
{
    if (aclInit(nullptr) != ACL_SUCCESS) return 1;
    if (aclrtSetDevice(0) != ACL_SUCCESS) return 1;
    aclrtStream stream = nullptr;
    aclrtCreateStream(&stream);

    const size_t xySize = 256 * 2, zSize = 8 * 256 * 4, iSize = 8 * 256 * 4;
    uint16_t *xH;
    aclrtMallocHost((void **)&xH, xySize);
    for (size_t k = 0; k < 256; k++) xH[k] = 0x3C00;   // 1.0 fp16
    uint8_t *xD, *yD, *zD, *iD;
    aclrtMalloc((void **)&xD, xySize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&yD, xySize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&zD, zSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&iD, iSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMemcpy(xD, xySize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(yD, xySize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(iD, iSize, xH, xySize, ACL_MEMCPY_HOST_TO_DEVICE);  // 索引/掩码非零即可

    const uint32_t R1 = 2000, R2 = 40000;
    printf("%-18s %12s %12s\n", "指令", "II(ns/条)", "延迟(ns/条)");
    printf("------------------------------------------------\n");
    for (auto &e : entries) {
        double ii = -1, lat = -1;
        if (e.indep) {
            RunOnce(e.indep, 100, xD, yD, zD, iD, stream);   // 预热
            double t1 = RunOnce(e.indep, R1, xD, yD, zD, iD, stream);
            double t2 = RunOnce(e.indep, R2, xD, yD, zD, iD, stream);
            if (t1 > 0 && t2 > 0) ii = (t2 - t1) / (double)(R2 - R1) / 8.0 * 1e9;  // 每迭代 8 条
        }
        if (e.dep) {
            RunOnce(e.dep, 100, xD, yD, zD, iD, stream);
            double t1 = RunOnce(e.dep, R1, xD, yD, zD, iD, stream);
            double t2 = RunOnce(e.dep, R2, xD, yD, zD, iD, stream);
            if (t1 > 0 && t2 > 0) lat = (t2 - t1) / (double)(R2 - R1) * 1e9;
        }
        printf("%-18s %12.2f %12.2f\n", e.name, ii, lat);
    }
    aclrtFree(xD); aclrtFree(yD); aclrtFree(zD); aclrtFree(iD);
    aclrtFreeHost(xH);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();
    return 0;
}
