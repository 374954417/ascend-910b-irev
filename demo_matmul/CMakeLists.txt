cmake_minimum_required(VERSION 3.16)
project(MatmulBasicDemo)

set(ASCEND_CANN_PACKAGE_PATH /usr/local/Ascend/ascend-toolkit/latest)
set(SOC_VERSION ascend910_9382)

include(${ASCEND_CANN_PACKAGE_PATH}/compiler/tikcpp/ascendc_kernel_cmake/ascendc.cmake)

ascendc_library(matmul_basic SHARED matmul_basic.cpp)

add_executable(main main.cpp)
target_include_directories(main PRIVATE ${ASCEND_CANN_PACKAGE_PATH}/include)
target_link_libraries(main PRIVATE
    matmul_basic
    ${ASCEND_CANN_PACKAGE_PATH}/lib64/libascendcl.so
)
