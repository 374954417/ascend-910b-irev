# AIV (dav-c220-vec) DataCopy / SetFlag / WaitFlag 指令编码逆向报告

方法:受控变体 + 字节 diff(变体源码与构建脚本在 `/tmp/irev_datacopy/`,
`gen.py` 生成全部 kernel,`wdiff.py` 做 4 字节字对齐 diff)。

## 0. 总述:AIV 侧指令一律定长 4 字节(小端)

- 所有 kernel 的 .text 长度均为 4 的倍数;所有变体 diff 均按 4 字节字对齐命中;
  每多加一条 DataCopy 严格 +16B(4 字)。与 cube 侧 `mad`(12B/3字)不同。
- 顶字节(byte3)近似指令类标记:
  `0x07` = 标量立即数/描述符配置类,`0x40` = 标量运算类(含 set_flag/wait_flag),
  `0x70/0x71` = MTE 拷贝发射指令,另见 0x02/0x04/0x08/0x09/0x0f/0x0a/0x48 等。

## 1. DataCopy(GM↔UB、UB↔UB)

一次 `DataCopy(dst, src, DataCopyParams{bc,bl,ss,ds})` 编译为:
**描述符配置字(仅非常量默认值时各发一次,可被后续多条 copy 复用)
+ 描述符提交字(`e9` 族)+ 地址设置字 + 每条 copy 一个发射指令**。

### 1.1 发射指令(4B,每条 DataCopy 一个)

| 方向 | 观测码字(byte0..3) | 说明 |
|---|---|---|
| GM→UB (MTE2) | `08 11 00 71` | 另见 `88 21 00 71`/`08 31 0a 71`(寄存器位不同) |
| UB→GM (MTE3) | `10 01 82 70` | 另见 `90 01 82 70` |
| UB→UB | `08 01 82 70` | |

字段映射(payload;标注"噪声"的位随寄存器分配变化):

| 字段 | 位置 | 编码 | 证据 |
|---|---|---|---|
| 指令类/方向 | byte3 | `0x71` = src 在 GM(MTE2 load);`0x70` = 其他(MTE3 store、UB→UB) | gm2ub vs ub2gm/ub2ub diff,单字差异 |
| dst 空间 | byte0 bit3/bit4 | bit3=1: dst=UB(0x08);bit4=1: dst=GM(0x10);bit7 为噪声 | 三方向互 diff |
| src 空间 | byte2 bit7+bit1 | src=UB → 基础值 `0x82`;src=GM → `0x00`;bit0,2-6 噪声 | 同上 |
| 地址寄存器 | byte0[7]、byte1 其余位、byte2 低位 | GM/UB 地址经标量寄存器传入(运行时值,不在编码内) | a_n1→n4 阶梯:每 copy 带 2 个地址设置字 |
| 常量位 | byte1 bit0 = 1 | 所有方向均有 | — |

### 1.2 描述符参数字(顶字节 0x07,imm16 在低 2 字节,byte2 是字段选择子)

| 参数 | 码字模板(LE 4 字节) | 字段位置/宽度 | 证据 |
|---|---|---|---|
| blockCount | `[bc<<4 lo] [bc<<4 hi] [0x04+bank] 07` | 字内 bit[4:15],12bit,值=bc 左移 4;bc=4095 → `f0 ff` 封顶 | bc=1/2/3/4/16/256/4095 逐字节线性 |
| blockLen | `[bl lo] [bl hi&0x0F] [0x45+bank] 07` | 字内 bit[0:11],12bit,**原样值** | bl=64/127/128/256/4095;4095→`ff 0f` |
| srcStride | `[ss] 00 85 07` | bit[0:15],仅 ss≠0 时发射 | ss=1/2/3 → `01/02/03 00 85 07` |
| dstStride | `[ds] 00 c5 07` | bit[0:15],仅 ds≠0 时发射 | ds=1/2/3 → `01/02/03 00 c5 07` |

- **blockLen 单位 = 32B 块,dtype 不进指令**:half/float/int32/int8 同一参数
  产生逐字节相同的 .text(整个 928B 零差异)。与 impl 源码一致
  (`kernel_operator_data_copy_impl.h`: `burstLen = blockLen * ONE_BLK_SIZE(32)`)。
- byte2 选择子:bc=`0x04`、bl=`0x45`、ss=`0x85`、ds=`0xc5`;
  bit1 随 kernel 在 0/1 间变化(0x04↔0x06、0x45↔0x47)——疑似描述符 bank 选择,未定论。
- stride 与 strides=0 的差异还多出 2 个 `e9` 族字:无 stride 时 1 个 `08 e9 85 04`,
  有 stride(GM 拷贝)或 UB→UB 时 3 个(`14/08/18 e9 85 04`,UB→UB 为 `83`)。
  `XX e9 8Y 04` 族 = 描述符提交/写回类指令,字段语义未解出。
- 编译器对同参数多条 copy 只发一次 bc/bl 配置字;a_n1→a_n4 每条 copy
  增量恒为 16B = 2 地址设置字 + 1 发射字 + 1 个 `00 XX 06 07` 族字
  (imm 随 GM 字节偏移线性变化,如 copy3/4 的 `00 20/30 06 07` = 偏移 0x2000/0x3000)。

## 2. SetFlag / WaitFlag(各一条 4B 指令)

决定性证据:仅含 `SetFlag<V_MTE2>(0)` 与仅含 `WaitFlag` 的两个 kernel
**只有一个字不同**:`0406a240`(set)↔ `0406c240`(wait)。

| 字段 | 位置 | 编码 | 证据 |
|---|---|---|---|
| 指令类 | byte3 = `0x40` | 标量类 | — |
| set/wait | byte2 | `0xa2` = set_flag,`0xc2` = wait_flag | set-only vs wait-only 单字 diff |
| 管道对 | byte1 = `(src<<2) \| (dst>>1)`;byte0 bit7 = `dst&1` | 管道码:S=0, V=1, M=2, MTE1=3, MTE2=4, MTE3=5 | 22 种 HardEvent 全枚举(下表) |
| eventID 寄存器 | byte0 bit[2:3+](观测到 0x00/04/08/0c) | 引用标量寄存器 | 见下 |
| eventID 本身 | **不在 flag 指令内** | 由单独立即数字 `[eid] 00 [R] 07` 载入寄存器(byte0=eventID,id 0-15 验证);id=0 时不发射该字(用零寄存器) | id1 vs id3 全 kernel 仅 `01000007`→`03000007` 一字之差 |

管道对验证表(`B0 B1`,B0 低 2-3 位为寄存器噪声已剔除):

| HardEvent | B0&0x83 | B1 | | HardEvent | B0&0x83 | B1 |
|---|---|---|---|---|---|---|
| S_V | 0x80 | 0x00 | | V_MTE1 | 0x80 | 0x05 |
| V_S | 0x00 | 0x04 | | MTE1_V | 0x80 | 0x0c |
| S_MTE2 | 0x00 | 0x02 | | MTE2_M | 0x00 | 0x11 |
| MTE2_S | 0x00 | 0x10 | | M_MTE2 | 0x00 | 0x0a |
| S_MTE3 | 0x80 | 0x02 | | V_M | 0x00 | 0x05 |
| MTE3_S | 0x00 | 0x14 | | M_V | 0x80 | 0x08 |
| V_MTE2 | 0x00 | 0x06 | | MTE3_MTE2 | 0x00 | 0x16 |
| MTE2_V | 0x80 | 0x10 | | MTE2_MTE3 | 0x80 | 0x12 |
| V_MTE3 | 0x80 | 0x06 | | MTE3_MTE1 | 0x80 | 0x15 |
| MTE3_V | 0x80 | 0x14 | | MTE1_MTE3 | 0x80 | 0x0e |

校验:`S=0,V=1,M=2,MTE1=3,MTE2=4,MTE3=5` 代入 `B1=(src<<2)|(dst>>1)`、
`B0[7]=dst&1` 全部 20 项吻合(如 MTE2_V: src4<<2=0x10 | dst1>>1=0 → 0x10,B0[7]=1 ✓)。

### 特例 / 未解出
- **FIX 管道不符合上式**:MTE2_FIX → `04 51 a2 40`(B1=0x51=0x10|0x41,
  多 0x40 标志位),FIX_MTE2 → `04 2a a2 40`(B1=0x2a → src 码 0x0a)。FIX 编码未闭合。
- **不产生任何 set/wait 指令**的 HardEvent(编译器在 AIV 上直接省略):
  V_V、MTE2_MTE1、MTE1_MTE2、MTE1_M、M_MTE1、M_FIX、FIX_M。
- auto-sync 自动插入的 flag 与显式 SetFlag/WaitFlag 编码完全相同。

## 3. 指令长度结论

| 指令 | 长度 |
|---|---|
| DataCopy 发射指令 | 4B(每次调用 1 条;另加共享的描述符配置字 4B/个、提交字、地址字) |
| DataCopy 参数配置字 | 各 4B(bc/bl 恒发,ss/ds 仅非零时发) |
| set_flag / wait_flag | 各 4B(+ eventID≠0 时一条 4B 立即数字) |
| AIV 指令流整体 | 定长 4B/字 |

## 4. 未解出/存疑清单
1. `XX e9 8Y 04` 描述符提交族的字段语义(数量随 stride/UB↔UB 路径变化)。
2. 参数配置字 byte2 的 bit1(疑似描述符 bank 位)。
3. 发射指令中地址寄存器字段的精确位宽(byte0/1/2 的噪声位边界)。
4. flag 指令 B0 中 eventID 寄存器字段位宽(观测到 bit[2:3],可能更宽)。
5. FIX 管道编码;MTE2_MTE1 等被省略的管道对在 AIV 上的替代机制。
6. `2c ed 09 04`/`2c ed 09 03` 等每 kernel 各出现一次的 boilerplate 字语义
   (与 DataCopy 无关,所有 kernel 含空 kernel 都有)。

## 5. 复现
- 变体生成+编译:`/tmp/irev_datacopy/gen.py`(`python3 gen.py [名字...]`)
- 字节 diff:`/tmp/irev_datacopy/diff.py A B`;字对齐 diff:`/tmp/irev_datacopy/wdiff.py A B`
- 全部产物:`/tmp/irev_datacopy/k_*.cpp / .o / .elf / .text`
