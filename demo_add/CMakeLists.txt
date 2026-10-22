cmake_minimum_required(VERSION 3.16)
project(AddCustomDemo)

# CANN 工具链路径与本机芯片型号(Ascend910_9382,910B 系列)
set(ASCEND_CANN_PACKAGE_PATH /usr/local/Ascend/ascend-toolkit/latest)
set(SOC_VERSION ascend910_9382)

include(${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/ascendc_kernel_cmake/ascendc.cmake)

# 编译 device kernel,并自动生成 host stub(libadd_custom.so)
ascendc_library(add_custom SHARED add_custom.cpp)

# host 可执行程序
add_executable(main main.cpp)
target_include_directories(main PRIVATE ${ASCEND_CANN_PACKAGE_PATH}/include)
target_link_libraries(main PRIVATE
    add_custom
    ${ASCEND_CANN_PACKAGE_PATH}/lib64/libascendcl.so
)
