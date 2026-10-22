/**
 * Host: 全 1 输入(fp16 0x3C00), 期望 C = K = 64
 * A 按分形块排布(host 侧直接写, 全 1 时布局无关), B 行主序(第 kb 块天然连续)
 */
#include <cstdio>
#include <cstdint>
#include <cmath>
#include "acl/acl.h"

extern "C" uint32_t aclrtlaunch_matmul_basic(uint32_t blockDim, void *stream,
                                             uint8_t *a, uint8_t *b, uint8_t *c);

#define ACL_CHECK(ret, msg) \
    do { if ((ret) != ACL_SUCCESS) { printf("[ERROR] %s ret=%d\n", msg, (int)(ret)); return 1; } } while (0)

int main()
{
    constexpr int32_t M = 16, N = 16, K = 64;
    const size_t abSize = M * K * sizeof(uint16_t);
    const size_t cSize = M * N * sizeof(float);

    ACL_CHECK(aclInit(nullptr), "aclInit");
    ACL_CHECK(aclrtSetDevice(0), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    ACL_CHECK(aclrtCreateStream(&stream), "aclrtCreateStream");

    uint16_t *abHost;
    float *cHost;
    ACL_CHECK(aclrtMallocHost((void **)&abHost, abSize), "mallocHost ab");
    ACL_CHECK(aclrtMallocHost((void **)&cHost, cSize), "mallocHost c");
    for (int32_t i = 0; i < M * K; i++) abHost[i] = 0x3C00;   // 全 1.0 (fp16)

    uint8_t *aDev, *bDev, *cDev;
    ACL_CHECK(aclrtMalloc((void **)&aDev, abSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc a");
    ACL_CHECK(aclrtMalloc((void **)&bDev, abSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc b");
    ACL_CHECK(aclrtMalloc((void **)&cDev, cSize, ACL_MEM_MALLOC_HUGE_FIRST), "malloc c");
    ACL_CHECK(aclrtMemcpy(aDev, abSize, abHost, abSize, ACL_MEMCPY_HOST_TO_DEVICE), "copy a");
    ACL_CHECK(aclrtMemcpy(bDev, abSize, abHost, abSize, ACL_MEMCPY_HOST_TO_DEVICE), "copy b");

    uint32_t ret = aclrtlaunch_matmul_basic(1, stream, aDev, bDev, cDev);
    if (ret != 0) { printf("[ERROR] launch ret=%u\n", ret); return 1; }
    ACL_CHECK(aclrtSynchronizeStream(stream), "sync");

    ACL_CHECK(aclrtMemcpy(cHost, cSize, cDev, cSize, ACL_MEMCPY_DEVICE_TO_HOST), "copy c");
    int err = 0;
    for (int32_t i = 0; i < M * N; i++) {
        if (std::fabs(cHost[i] - 64.0f) > 1e-3) {
            if (err < 5) printf("[ERROR] C[%d]=%f, expect 64\n", i, cHost[i]);
            err++;
        }
    }
    aclrtFree(aDev); aclrtFree(bDev); aclrtFree(cDev);
    aclrtFreeHost(abHost); aclrtFreeHost(cHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    if (err == 0) {
        printf("[SUCCESS] C = A(16x64) x B(64x16), 全部 %d 个元素 = 64.0 (K 累加正确)\n", M * N);
        return 0;
    }
    printf("[FAILED] %d 个元素错误\n", err);
    return 1;
}
