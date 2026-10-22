/**
 * Host 侧:ACL 初始化 -> 内存拷贝 -> 启动 add_custom kernel -> 校验结果
 *
 * add_custom 的 host stub 由 ascendc_library 自动生成,签名为:
 *   void add_custom(uint32_t blockDim, aclrtStream stream, ...kernel 参数)
 */
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include "acl/acl.h"

extern "C" uint32_t aclrtlaunch_add_custom(uint32_t blockDim, void *stream, uint8_t *x, uint8_t *y, uint8_t *z);

#define ACL_CHECK(ret, msg)                                                   \
    do {                                                                      \
        if ((ret) != ACL_SUCCESS) {                                           \
            printf("[ERROR] %s failed, ret=%d\n", msg, (int)(ret));           \
            return 1;                                                         \
        }                                                                     \
    } while (0)

int main()
{
    constexpr int32_t totalLength = 8 * 2048;
    constexpr uint32_t blockDim = 8;
    const size_t dataSize = totalLength * sizeof(uint16_t); // fp16 = 2 bytes

    // 1. 初始化 ACL,设置设备
    ACL_CHECK(aclInit(nullptr), "aclInit");
    ACL_CHECK(aclrtSetDevice(0), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    ACL_CHECK(aclrtCreateStream(&stream), "aclrtCreateStream");

    // 2. 申请 host/device 内存
    uint8_t *xHost, *yHost, *zHost;
    ACL_CHECK(aclrtMallocHost((void **)&xHost, dataSize), "aclrtMallocHost x");
    ACL_CHECK(aclrtMallocHost((void **)&yHost, dataSize), "aclrtMallocHost y");
    ACL_CHECK(aclrtMallocHost((void **)&zHost, dataSize), "aclrtMallocHost z");

    uint8_t *xDevice, *yDevice, *zDevice;
    ACL_CHECK(aclrtMalloc((void **)&xDevice, dataSize, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc x");
    ACL_CHECK(aclrtMalloc((void **)&yDevice, dataSize, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc y");
    ACL_CHECK(aclrtMalloc((void **)&zDevice, dataSize, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc z");

    // 3. 填充输入:x[i]=1.0, y[i]=2.0 (fp16 位模式 0x3C00 / 0x4000)
    auto *xh = reinterpret_cast<uint16_t *>(xHost);
    auto *yh = reinterpret_cast<uint16_t *>(yHost);
    for (int32_t i = 0; i < totalLength; i++) {
        xh[i] = 0x3C00; // 1.0
        yh[i] = 0x4000; // 2.0
    }
    ACL_CHECK(aclrtMemcpy(xDevice, dataSize, xHost, dataSize, ACL_MEMCPY_HOST_TO_DEVICE), "memcpy x H2D");
    ACL_CHECK(aclrtMemcpy(yDevice, dataSize, yHost, dataSize, ACL_MEMCPY_HOST_TO_DEVICE), "memcpy y H2D");

    // 4. 启动 kernel
    uint32_t launchRet = aclrtlaunch_add_custom(blockDim, stream, xDevice, yDevice, zDevice);
    if (launchRet != 0) {
        printf("[ERROR] aclrtlaunch_add_custom failed, ret=%u\n", launchRet);
        return 1;
    }
    ACL_CHECK(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream");

    // 5. 拷回结果并校验:期望 z[i] = 3.0 (fp16 位模式 0x4200)
    ACL_CHECK(aclrtMemcpy(zHost, dataSize, zDevice, dataSize, ACL_MEMCPY_DEVICE_TO_HOST), "memcpy z D2H");
    auto *zh = reinterpret_cast<uint16_t *>(zHost);
    int errorCount = 0;
    for (int32_t i = 0; i < totalLength; i++) {
        if (zh[i] != 0x4200) {
            if (errorCount < 5) {
                printf("[ERROR] z[%d] = 0x%04X, expect 0x4200\n", i, zh[i]);
            }
            errorCount++;
        }
    }

    // 6. 释放资源
    aclrtFree(xDevice); aclrtFree(yDevice); aclrtFree(zDevice);
    aclrtFreeHost(xHost); aclrtFreeHost(yHost); aclrtFreeHost(zHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    if (errorCount == 0) {
        printf("[SUCCESS] add_custom: %d 个元素全部正确, z = x + y = 3.0\n", totalLength);
        return 0;
    }
    printf("[FAILED] %d/%d 个元素错误\n", errorCount, totalLength);
    return 1;
}
