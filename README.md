# ascend-910b-irev

昇腾 910B(dav-c220 / Ascend910_9382)AscendC kernel 实验与指令编码逆向工程。

## 逆向工程成果(irev/)

通过"受控变体 + 字节级 diff"方法(单指令最小 kernel,`-D` 宏一次改一个参数,bisheng 编译 → llvm-objcopy 提取 .text → diff),破译了 910B 主要指令类的编码格式。**详见 [irev/README.md](irev/README.md)。**

核心结论速览:

| 指令 | 侧 | 净长度 | 结构 |
|---|---|---|---|
| `mad`(矩阵乘) | AIC | 12B = 3×32bit | 自包含:shape/flag 全在指令内 |
| 向量算术(Add/Mul/Exp...) | AIV | 8B 本体 + 每操作数 4B 地址字 | 地址字外置 |
| `LoadData`(→L0) | AIC | 4B issue + 共享 config | config/issue 两段式 |
| `fixp`(L0C→GM) | AIC | 4B/copy + 配置字 | 三层,非默认字段才发射 |
| `DataCopy`(GM↔UB) | AIV | 4B/copy + 描述符字 | 描述符共享 |
| `set_flag/wait_flag` | AIV | 4B | eventID 走独立立即数字 |

分报告:[loaddata](irev/loaddata.md) / [fixp](irev/fixp.md) / [vector_arith](irev/vector_arith.md) / [datacopy_flag](irev/datacopy_flag.md) / [rare_classes](irev/rare_classes.md)

## 目录说明

| 目录 | 内容 |
|---|---|
| `irev/` | 逆向工程报告 + 反汇编工具(`msdisasm.py` 等)。`output/` 下的大体积反汇编产物未入库,可用工具重新生成 |
| `demo_mad_format/` | mad 指令编码格式专项实验 |
| `demo_add/` | 最小 AscendC Add kernel(CMake + bisheng 完整流程示例) |
| `demo_matmul/` | matmul kernel 实验 |
| `demo_mma/` | mma 指令实验 |
| `demo_scalar/` | 标量类 kernel 批量编译脚本与样例 |
| `demo_bench/` | kernel 性能 bench + 变体生成器(`gen_variants.py` 等)及实测结果 |
| `test_progs/` | 编译 + 反汇编联调脚本(`build_and_disasm.sh`)及参考反汇编输出 |

## 环境依赖

- 硬件:昇腾 910B(Ascend910_9382)
- 工具链:CANN toolkit(`/usr/local/Ascend/ascend-toolkit/latest`),bisheng 编译器,`llvm-objcopy` / `msobjdump`
- 各 demo 的 `CMakeLists.txt` 中通过 `ascendc_kernel_cmake/ascendc.cmake` 接入 CANN 工具链

## 免责声明

本项目为个人逆向工程研究,指令编码结论均来自实验观测与推断,非官方文档,不保证与华为未公开规范一致;不同 CANN 版本/芯片型号行为可能不同。
