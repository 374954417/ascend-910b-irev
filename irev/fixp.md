# fixp (L0C→GM, `DataCopy(GlobalTensor, LocalTensor<CO1>, DataCopyCO12DstParams)`) 指令编码反推

目标: dav-c220 (910B) cube 侧 fixpipe 回写路径。底层 builtin `copy_matrix_cc_to_gm`。
方法: 单 kernel 受控变体 + .text 字节 diff。源码/脚本: `/tmp/irev_fixp/`(`fixp_one.cpp` + `build.sh` + `diff.py`)。

## 总体结构(与 mad 的"单条 12B 指令"模型不同)

DataCopy CO1→GM **不是一条自包含指令**,而是:
1. 一组 4 字节"写 cube 配置寄存器"字(顶字节 0x07,与 mad 同类标记),每个字携带一个字段,编译器只在值非默认时才发射;
2. 一个每 kernel 恰好出现一次的 0xf0 顶字节字(基线 `8c 10 ca f0`);
3. 每条 DataCopy 一个 4 字节指令字,顶字节 **0xc2**(基线 `84 51 00 c2`)。

证据: NUM_COPY=1/2/3/4 时,0xc2 顶字节字的个数恰为 1/2/3/4;0xf0 字恒为 1 个;配置字不随 copy 数增加(同参数多 copy 共享)。

## 字段映射表(字均为 4B 小端,byte3=类标记)

| 字段 | 字格式 (b0 b1 b2 b3) | 编码 | 证据 |
|---|---|---|---|
| nSize | `sid, nSize/16, 0x06, 0x07` | **nSize/16,单字节**(8 bit,4096→256 回绕为 0) | 16→`00 01`,32→`00 02`,48→`00 03`,272→`00 11`,512→`00 20`,4096→`00 00` |
| sid | 同上字 byte0 | 原值 | sid=1 → `01 01 06 07` |
| mSize | `mSize&0xff, mSize>>8, 0x47, 0x07` | **原值 uint16** | 16→`10 00`,17→`11 00`,256→`00 01` |
| dstStride | `dstStride, 00, 0x87, 0x07` | 原值(仅 ≠0 时发射;=0 走硬件默认 dense) | ds1→`01 00 87 07`,ds2→`02 00 87 07`,长度 +4 |
| srcStride | `srcStride, 00, 0x04, 0x07` | 原值 | ss1→`01 00 04 07`,ss2→`02 00 04 07` |
| quantPre | 配置字 `b0, b1, 0x85, 0x07` 的 b0[6:2] | **quantPre << 2** | F322F16(1)→0x04, VREQ8(8)→0x20, REQ8(9)→0x24, DEQF16(11)→0x2c, F322BF16(16)→0x40, QF322B8_PRE(24)→0x60 |
| reluPre | 同上 b0 bit7 | reluPre << 7 | relu=1 → `80 00 85 07` |
| unitFlag | 同上 b0 bit0 | 原值 | unitFlag=1 → `01 00 85 07` |
| channelSplit | 同上 b1 bit2 | << 2 | csplit → `00 04 85 07` |
| nz2ndEn | 同上 b1 bit3 | << 3 | nz2nd → `00 08 85 07` |
| (组合验证) | relu+REQ8+csplit | `a4 04 85 07` = 0x80\|9<<2, 0x04 | combo 变体 ✓ |
| ndNum | `ndNum, 00, 0x02, 0x07` | 原值 | SetFixpipeNz2ndFlag(2,32,32) → `02 00 02 07` |
| srcNdStride | `val, 00, 0x43, 0x07` | 原值 | 同上 → `20 00 43 07` |
| dstNdStride | `val, 00, 0x83, 0x07` | 原值 | 同上 → `20 00 83 07` |

`85 07` 配置字只在 quant/relu/csplit/nz2nd/unitFlag 任一非默认时发射;此时 `04 07`(srcStride)字也必伴随发射(即使值为 0)。ndNum 三个字仅由显式 `SetFixpipeNz2ndFlag` 产生。

## 指令本体(0xc2 顶字节字)

- 每条 DataCopy 4 字节,如 `84 51 00 c2`。变体中观察到低 3 字节随**寄存器分配**变化:
  base `84 51 00 c2`;加配置字后 `88 51 00 c2`;nc2 `04 51 00 c2`/`04 41 06 c2`;nc3 `84 51 00 c2`/`84 61 04 c2`/`84 41 00 c2`;src256 `04 32 00 c2`。
- **dst/src 地址不在指令内**: nc2(GM 偏移 4096B/copy)与 nc2b(8192B/copy)的 c2 字完全相同,只有前面的标量地址计算不同。地址经地址寄存器传入,c2 字低字节是寄存器选择子(无反汇编器,无法进一步定界)。
- 单条净长度 = 4B(每增加一条 copy,.text 增量 = 4B 指令 + ~12-16B 地址计算标量码)。

## 0xf0 顶字节字

- 每 kernel 恰好 1 个,基线 `8c 10 ca f0`(所有变体中位于同一偏移 0x19c)。
- **dtype 不在其中、也不在任何地方**: GM 输出 float / half / bf16 / int8(NoQuant)生成的 .text **逐字节相同**(base == half == bf16 == s8noq;ds1 == hnoq1)。加 dstStride 后仍相同。
  → 该回写路径是 dtype 无关的原始搬运,位宽转换完全由 quantPre 控制。先前"dtype 编码在 0xf0 配置字"的认知对本 DataCopy 重载不成立(可能来自 fixpipe 其他路径)。
- 0xf0 字低 16 bit 在"发射额外配置字"的变体中从 `8c 10` 变 `0c 21`,与地址寄存器重编号同步 → 疑似含寄存器操作数(可能是 fixp 地址对/上下文设置),而非常量参数字段。

## 与 mad 的对比

| | mad | fixp (DataCopy CO1→GM) |
|---|---|---|
| 形式 | 单条 12B = 3×32bit,顶字节 0x07 | 配置字组(4B×N,顶字节 0x07)+ 每 copy 一条 4B(顶字节 0xc2)+ 每 kernel 一个 0xf0 字 |
| 尺寸字段 | m/n 原值,k 为 k/16 | m 原值(uint16),n 为 n/16(单字节) |
| 地址 | L0 偏移在指令内(此前结论) | 地址走寄存器,指令内只有寄存器选择子 |
| 共享配置 | 无 | nSize/mSize/strides/quant 等写入配置寄存器,同参数多 copy 不重复 |

## 未解出/存疑

- 0xc2 指令字低 3 字节的位级语义(寄存器编号 vs 标志位无法区分,需反汇编或更多对齐实验)。
- 0xf0 字的字段划分(只知顶字节类标记 + 低 16bit 随寄存器分配变化)。
- 0x07 配置字 byte2(0x06/0x47/0x87/0x04/0x85/0x02/0x43/0x83)疑似"寄存器号"而非纯 opcode:mSize 字 byte2 见过 0x45/0x47/0x49 随分配变化;n 字 byte2 恒 0x06。未完全解释。
- quantPre ≥ 31 的 *_PRE 模式(VQF322F16_PRE=32、QF322BF16_PRE=34)走完全不同的 lowering(.text 更短、结构全变),未分析。
- clipReluPre、eltWiseOp 在本路径被完全忽略(改了不产生任何字节差)。
- 旧记录"int8 输出时 0x84→0x85"在本受控环境未复现(int8 NoQuant 与 float 逐字节相同);该差异可能来自当年变体的寄存器重编号。

## 复现

```
/tmp/irev_fixp/fixp_one.cpp   # 宏: NSIZE MSIZE DSTRIDE SSTRIDE QUANT RELU CSPLIT NZ2ND OUTDT NUM_COPY SRC_OFF DST_OFF SID UNITF CLIPR ELTOP
/tmp/irev_fixp/build.sh <name> [-D...]   # bisheng 编译 + ld.lld + objcopy .text
python3 /tmp/irev_fixp/diff.py <a> <b> [ctx]
```
变体产物: base n32 n48 n272 n512 n4096 m32 m17 m256 ds1 ds2 ss1 ss2 relu1 csplit nz2nd qf16 qbf16 q24 q32 q34 s8req s8vreq s8noq hdeq half bf16 sid1 unitf combo clipr eltop nc2 nc2b nc3 nc4 src256 (.o/.elf/.text 均在 /tmp/irev_fixp/)。
