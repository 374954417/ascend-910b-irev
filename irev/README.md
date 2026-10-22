# 昇腾 910B (dav-c220) 指令编码逆向工程总报告

方法:受控变体 + 字节级 diff(单指令最小 kernel,-D 宏一次改一个参数,bisheng 编译 → llvm-objcopy 提取 .text → diff)。
各分报告: [loaddata.md](loaddata.md) / [fixp.md](fixp.md) / [vector_arith.md](vector_arith.md) / [datacopy_flag.md](datacopy_flag.md);mad 部分见 demo_mad_format/。

## 统一架构图景

| 指令 | 侧 | 净长度 | 指令类顶字节 | 结构 |
|---|---|---|---|---|
| `mad`(矩阵乘) | AIC | **12B = 3×32bit** | 0x07 ×3 | 自包含:shape/flag 全在指令内 |
| 向量算术(Add/Mul/Exp...) | AIV | **8B 本体** + 每操作数 4B 地址字 | 0x83/85/87/89(二元)0x96/97(标量) | 地址字外置,指令只引用寄存器 |
| `LoadData`(→L0) | AIC | **4B issue** + 共享 config | issue 0x60,config 0x10/0x07 | config/issue 两段式,同参复用 |
| `fixp`(L0C→GM) | AIC | **4B/copy** + 配置字 | 0xc2(copy)+ 0x07(配置)+ 0xf0(全局) | 三层,非默认字段才发射 |
| `DataCopy`(GM↔UB) | AIV | **4B/copy** + 描述符字 | 0x70/0x71(方向) | 描述符共享,发射字定长 |
| `set_flag/wait_flag` | AIV | **4B** | 0x40 类,byte2: a2=set/c2=wait | eventID 走独立立即数字 |

## 跨指令共性(微架构推论)

1. **AIV 指令定长 4B;AIC 指令更长**(mad 12B)——cube 指令信息密度高
2. **地址几乎从不内嵌在指令里**:全部走"寄存器/描述符 + 紧凑选择子"模式,偏移由 0x07 类 setup 字物化(单位 256B)
3. **配置摊销**:相同参数的连续指令共享 config 字(LoadData 每加一条只 +4B;mad 每加一条 +12B 但共享 0xf0 配置字)
4. **dtype 大多不进搬运指令**(GM↔UB DataCopy、L0C→GM fixp 都是 dtype 无关,以 32B 块为单位);只有计算指令(mad 配置字、向量 opword byte2[7:6])关心 dtype
5. **shape 缩放编码**:cube 偏爱"除以分形"(k/16、nSize/16),AIV 偏爱 32B 块(blockLen 原值×32B)

## 已破译字段精选

- **mad**: m=W0.b0(原值,支持 8~128), k=W1.b0(=k/16), n=W1.b1, unitFlag=W2.b0.7, initVal=W2.b1.7, partial-K 标志=W0.b1.7
- **向量算术**: opcode=byte3(Add/Sub=0x85 靠 byte0.bit0 分极性;Mul=0x89...), dtype=byte2[7:6](s32/f16/s16/f32=00/01/10/11)
- **flag**: 管道对 B1=(src<<2)|(dst>>1), B0.7=dst&1,管道码 S/V/M/MTE1/MTE2/MTE3=0..5;set=0xa2/wait=0xc2
- **LoadData**: repeatTimes/startIndex(值/8)/srcStride/dstGap/sid 全部定位;compact(0x10)/extended(0x07 tag)双模式
- **fixp**: nSize=n/16, mSize=uint16 原值, quantPre<<2, relu=bit7, nz2nd=bit3

## 工具链坑(发现于逆向过程)

- `LoadData2DParamsV2`(GM→L0)在 dav-c220 上**编译为空操作**
- Ln≡Reciprocal、Rsqrt≡Relu(f32)编码完全相同——疑似降到同一硬件操作
- V_V、MTE2_MTE1 等 HardEvent 在 AIV 上不产生指令(编译器省略)

## 未解出(后续方向)

- mad 配置字(0xf0)除 dtype 外的字段;fixp 0xc2 字低 3 字节
- 0x40 类标量指令(AIV 数量最大的一类)
- bias/量化 mad 变体;cube 侧 set_flag 编码
- FIX 管道在 flag 编码中的异常(MTE2_FIX→B1=0x51 不符合通用公式)

## 附录:标量指令(0x40/0x00 类)破译结果(2026-10-10 补充,demo_scalar/)

| 指令 | 编码 | 说明 |
|---|---|---|
| 条件分支 | `[imm16] 00 40` | imm16 有符号,**单位=字**(经尾声轮询循环 13 字间距等差验证) |
| 标量加/减 reg | `81/82 11 02 00` | 0x00 类,byte0[1:0]: 1=add 2=sub |
| 比较/选择 | `ae 11 40 00` | 0x00 类 byte2=0x40;比较立即数在 `XX 00 06 07`(imm8 原值) |
| 定数低 4 位 | `00 [K&0xF <<4] 06 07` | 0x07 类 movi_lo |
| 定数高位 | `[lo][hi] 47 07` | 0x07 类 movi_h,完整定数=(imm16<<4)\|lo4(20bit 精度) |
| 向量寻址模式字 | `XX XX e0 40` | 跟在向量算术 opword 后 |
| 大常量不进 .rodata | — | dav-c220 kernel 的常量全部内联物化(验证了 .rodata 为空) |

未解: (0x40,0x20) 字(高频,疑似 nop/对齐)、0x02/0x0f/0x09 类(地址簿记)、乘常数(编译器强度削弱为移位组合,编码不稳定)。

## 附录 2:控制流与簿记指令(2026-10-10 第二轮补充)

| 编码模式 | 解码 | 置信度 | 证据 |
|---|---|---|---|
| `[imm16] 00 40` | branch.f(条件分支 A) | 高 | 尾声轮询分支偏移等差验证(imm 单位=字) |
| `[imm16] 20 40` | branch.t(条件分支 B,与 A 极性互补) | 中 | 负偏移(ff 前缀)与 A 型交错出现于轮询循环 |
| `XX YY ZZ 02` | poll_setup / 地址装载 | 中 | `80390602` 在每个 wait_flag 前出现,跨 kernel 恒定 |
| `XX YY ZZ 09` | ld_arg_gm(GM 参数地址加载) | 中 | 每 GM 参数恰好一条(cube/vec/add/mma 均 3 参数→3 条) |
| `XX YY ZZ 0f` | frame_setup(帧/TPipe 建立) | 中低 | 序言区,值呈帧槽式递增 |
| `XX YY ZZ 04/08/03` | addr_setup(GM 地址物化) | 中低 | 集中出现在 DataCopy 附近 |
| `89 41 c6 00` 等 | 标量 cmov/select(0x00 类 byte2=0xc6/04) | 中 | if/else 被编译成无分支 select(if100/if200 验证) |

覆盖率: 逐字 71-77%(4 个 kernel);计算/搬运/同步/控制流指令基本全识别。
注意:编译器会把标量循环变换为闭式(loop100r 中循环边界 100 未出现在编码里)。

## 附录 3:第三轮补充(覆盖率推至 94-97%)

- **向量 count/标量立即数 = 0x07 类 tag-0x04 字,imm16 原值**(count 128/256/512 阶梯严格验证;f16 标量 2.0 → imm16=0x4000 位型)。此前误判为 "blockCount<<4" 已修正
- **0x0a 类 = 事件轮询选择**(poll_event):byte0=事件槽位(尾声区递增),byte2: 0x00/0xc0 = 事件组;与 branch.t(0x40,0x20) 配对构成等待循环
- **0x80 类(byte2=0x40) = 向量寻址模式字第二形态**
- **`00 f0 2e 40` = 主程序/尾声分界标记**(疑似 ret),两侧 0x41 类为 exit_setup,0x48 类为 prologue_setup(低置信度,按固定位置推断)
- 0x07 类 imm16=0 的字 = cfg 清除/占位标记
- 残余未识别 <6%: 个别罕见路径单例字

## 附录 4:大规模语料仓验证(2026-10-12)

### 语料仓来源与构建
- **cann-ops**(gitee.com/ascend/cann-ops,生产级算子库): 262 个含 kernel 入口的 .cpp,166 个编译入库
- **ascend/samples**(gitee.com/ascend/samples): 122 个 kernel 文件,59 个入库
- 构建工具: demo_scalar/{batch_compile.sh, batch_compile_retry.sh, batch_compile_samples.sh, gen_tiling_stub.py, gen_global_stub.py}
  - gen_global_stub.py: 全树扫描 BEGIN_TILING_DATA_DEF/_ARR/_STRUCT 三种宏 + REGISTER_TILING_DATA_CLASS 注册表(560 条), 生成 267 个结构的 global_tiling_stub.h(拓扑序)
  - 生产算子直接编译要点: tiling stub(空 body, 仅过类型检查)、DTYPE_* 宏预定义、common/inc 递归 include

### 语料仓规模反汇编结果(msdisasm + corpus_disasm.py)
| 语料 | kernel 数 | 指令字 | 未识别 |
|---|---|---|---|
| cann-ops | 166 | 1,964,035 | 4.4% |
| samples | 59 | 79,986 | 4.1% |

### 语料仓暴露并修复的新模式
- **(0x40,0xa4)/(0x40,0xc4) = set_flag/wait_flag 的带值变体**(各 32190 次,成对;B0/B1 管道编码与 a2/c2 相同)
- 0x80 类偶数 byte2 = 向量寻址模式字第三形态
- 大规模数据证实此前所有字段编码在小样本外依然成立

## 附录 5:语料驱动收尾(2026-10-12)

- **标量逻辑族(寄存器-寄存器变体严格验证)**: 0x00 类 byte2=0xc4, byte0[3:0]: 0xa=and / 0xb=or / 0xc=xor
- **移位**: shl=`40 32 c2 02`, shr=`c0 32 42 02`;**乘法**: `03 XX 02 00`(byte2=0x02 族: 1=add 2=sub 3=mul)
- **修正**: 0x07 类 tag-0x04 是通用 imm16 装载(向量 count=元素数原值/标量位型), 之前误判为 blockCount
- 0x0a 类 = poll_event(事件轮询选择, byte0=槽位);0x0e 类 = addr_imm(低置信度)
- 全语料仓未识别率: cann-ops 2.9% / samples 2.6%;test_cube 等 4 个基准 kernel 95-98%
- 残余未知类: (0x4a,*)、(0x82,0x80)、(0x40,0x2c) 等罕见路径, 以及 fixp 语义门控在有/无 0xc2 字文件间的 tag 冲突

## 附录 6:向量运算 opcode 全表(vector_opcode_table.md, 子 agent 补全)

- **一元 0x83 类 sub-opcode = b0[7]:b1[3:0] (5bit)**: 0x10=Exp 0x11=Relu 0x12=Ln 0x01=Rsqrt 0x02=Reciprocal 0x03=Abs 0x1c=Sqrt;寄存器位=b0[4:2]/b1[5:4]/b2[3:0](多布局交叉验证剥离)
- **二元**: 0x85=Add/Sub, 0x87=Max/Min(极性 b0[0] 假设成立), 0x89=Mul, **0x8b=Div(新发现: 单指令)**
- **dtype 矩阵成立**: s32/f16/s16/f32 = b2[7:6] 00/01/10/11, 其余逐位相同
- **新指令类**: 0x82=vnot(s16), 0x8c=cvt/取整族(Floor/Round f32 单指令), 0x98/0x99=vcmpv, 0x9a=vcmpvs;比较 mode={b3[0],b2[7:6]}: EQ/NE/LT/GT/GE/LE=000/001/010/011/100/101, 比较族 dtype 在 b0[1:0]
- **修正旧结论**: "Rsqrt≡Relu、Ln≡Recip 同码" 是寄存器位掩盖 b0[7] 的假象,实为不同 opcode
- group_norm_swish 中观察到未列入表的 sub=0x6/0x13(Swish 相关一元运算,待后续)
- 修正后全仓未识别率: cann-ops 2.3% / samples 2.4%;group_norm_swish 98.9%;基准 kernel 98-99%

## 附录 7:s32 比较指令异常 + mode 位序修正(2026-10-12)

- **vcmpv mode 正确公式: mode = (b3[0]<<2) | b2[7:6]**(之前附录 6 的位序写反了;f16/f32 全 6 模式实测验证: eq/ne/lt/gt/ge/le 逐字节吻合)
- **s32 Compare 异常**: Compare(x, y, LT/EQ/GT) 在 s32 下编译出逐字节相同的 .text, 指令恒为 `vcmpv.eq.s32`(86410a98)——bisheng 对 s32 不降 mode 位。**疑为工具链 bug 或 s32 仅硬件支持 EQ**, 使用 s32 Compare LT/GT 的上层代码需警惕正确性

## 附录 8:罕见类破译(rare_classes.md, 语料+源码对照法)

- **0x4a 类 = JUMPCMP**(标量比较+条件跳转融合指令;循环变体逐一触发 + bisheng 内嵌字符串印证;0x49 为其姊妹形态)
- **0x82 类(byte2=0x80)= 向量数据移动/置换族发射字**: VNCHWCONV(5HD 转换)/VTRN/VCOPY/VNOT·VGATHER2, byte2[5:0]=子操作, byte2[6/7]=元素位宽
- **0x80 类奇数 byte2 = VNCHWCONV 地址列表字**(每字打包 2 个 UB 地址)
- **(0x40,0x2c) = 槽位控制字**(与 exit_setup 严格配对, 语义待定, 中低置信度)
- **0x00 类残余**: 标量 ALU 阶梯子操作成员;标量除法不降指令而是链接软件子程序
- 0x10 类非 0x0a 子 tag = 栈/地址调整(f0ff 负偏移样本,推断)
- **终态覆盖率**: 全语料仓 cann-ops 1.4% / samples 1.2%;reflection_pad3d_grad 等复杂算子 1.2-1.9%

## 附录 9:cann-ops-adv 融合算子语料 + FA 攻坚(2026-10-12)

### 语料扩展
- 新仓: [cann-ops-adv](https://gitee.com/ascend/cann-ops-adv)(融合算子: transformer/mc2),85 个 kernel 文件
- 入库: 13 个(含 MoE gating top-k softmax 637KB 巨型 kernel、grouped_matmul_add、IFA 驱动),255,701 指令字,**未识别率 1.1%**(最佳)
- 构建: batch_compile_adv.sh(通用 wrapper 剥离器:自动删除 __global__ 调 __global__ 的 dispatcher)+ gen_global_stub.py(并入工具链 adv_api/kernel_tiling.h 的 50+ 真实库 tiling 类型)

### FA 攻坚结论
- **IFA(增量 FlashAttention)双端编译成功**: vec 19KB / cube 6.7KB(ifa_driver_{vec,cube}.text),绕过 dispatch 直接实例化 IncreFlashAttentionAttenAllVecNew<half>
- **PFA x310 模板不可编**: 用了 dav-c220 不支持的 create_cbuf_matrix(真架构边界,非配置问题)
- 官方 fused_infer_attention_score.cpp 的 dispatcher 是 __global__ 调 __global__,bisheng 直编不允许
- jumpcmp 在复杂控制流 kernel 里高频出现(2638 次),破译正确性获大规模验证

## 附录 10:收尾攻坚——DWARF 行号归因(2026-10-12)

### 方法论突破: bisheng -g 有效!
设备 ELF 带完整 DWARF 行号表,`llvm-objdump --line-numbers --disassemble` 虽不解码指令但能给出**每条指令的源码行**——残余指令的语义归因从"猜"变成"查"。

### 本轮破译
- **0x01 类 = 64 位标量立即数/地址物化**(set_vector_mask(-1,-1)、uint64 路径;DWARF 归因实锤;~3300 字)
- **0x6b 类 = DataCopyPad 发射字**(pad_v3_grad_replication 的 DataCopyExtParams 路径;变体编译产出恰一条 0x6b 字实锤;byte2=方向/模式子操作)
- 0x85/0x87 stride=0 抑制残留修复

### 终态
| 语料仓 | 未识别率 |
|---|---|
| cann-ops (166 kernel, 196万字) | **0.4%** |
| samples (59 kernel) | **0.4%** |
| cann-ops-adv (13 kernel) | **0.3%** |

剩余为 flag/分支/比较族的罕见子变体长尾((0x40,0x2d)、(0x9d,*) 等, 各 <250 次)。

## 附录 11:清零(2026-10-12)

### 最后一批破译
- **0x9d/0x9e = vsel 向量选择**(CompareScalar+Select 受控变体:Select 恰好产生 0x9e+0x9d 二字)
- **0x9a 家族扩展**:vand/vor(b2=0x4a, b0 bit0 极性;and/or 变体一字之差)——vand/vor 与 vcmpvs 同族不同 b2
- **flag 族泛化**:byte2 bit7=1 且 ≠0xe0 即 flag;bit6=set(0)/wait(1);低位=子变体(罕见者标注 _xNN)
- **分支族泛化**:byte2 bit7=0 即分支(0x2e ret 标记除外);bit5=极性;低位=子模式
- **0x6b = DataCopyPad 发射字**(padcopy 变体实锤)
- **0x01 = 64 位标量立即数物化**(bisheng -g DWARF 行号归因实锤: set_vector_mask(-1,-1))
- **0x49/0x4b = JUMPCMP 姊妹形态**;0x10 非 0x0a 子 tag = 栈/地址调整(推断)

### 终态: 事实上的清零
| 语料 | 未识别 |
|---|---|
| cann-ops (196 万字) | 142 字 (0.007%) |
| samples (8 万字) | 26 字 (0.03%) |
| cann-ops-adv (25.6 万字) | 255 字 (0.1%, 集中在 moe_gating) |
| 4 个基准 kernel + group_norm_swish | **0** |

残余: moe_gating 里 168 字 + 零星单例,均为未见过的孤立位型,需要新的宿主源码才能归因。

## 附录 12:置信度提升轮(2026-10-12)

方法: bisheng -g 出 DWARF 行号表 + llvm-objdump --line-numbers 归因 + 受控变体。

| 标签 | 原置信度 | 现结论 | 证据 |
|---|---|---|---|
| prologue_setup/exit_setup/kernel_ret?/stack_adj?/addr_imm? | 中低 | **frame_enter/frame_exit/path_exit/stack_adj/scalar64_aux**(函数 ABI 帧管理,每执行路径一套) | DWARF 归因: 全部坍缩到 kernel 入口行=编译器生成帧代码 |
| vecop_8d? | 低 | **read_sysreg**(读系统寄存器) | DWARF 归因到 get_block_idx 区(common_impl.h:74) |
| vcmpsel_x?(0x9b) | 低 | **vcmpsel_x**(cmp/sel 族变体) | Power/abs/adam 的 compare+select 密集区共现 |
| cfg07 clear? | - | **cfg_init**(配置槽初始化标记) | 结构自证(imm16=0, 序言区) |
| vcmpvs dtype 位 b0[1:0] | ? | f16=01/s32=10/f32=11 不变;00 未识别(标 d0) | bf16 Compare 该重载不可编译,未能实测 |

终态: 全仓 230 万字中, 未识别+低置信合计 0.03%(423 字未识别, 284 字 dtype=d0)。

## 附录 13:真正清零(2026-12)

- **0x95 = vmadd**(Axpy 融合乘加, 变体实锤 `84324495`);标量-张量族补全: 0x95=vmadd / 0x96=vmin|maxs(b0 bit0 极性) / 0x97=vadds/vmuls
- **0x13/0x14 = 标量栈 load/store**(寄存器溢出;-O1 压力变体实锤 0x14=store, 0x13=load)
- **0x92 = matmul 路径地址描述字**(低置信);0x4c/0x4d = JUMPCMP 族;0x9c = vgathermask(masked_select 宿主)
- **终态: 三个语料仓(230 万指令字)未识别 = 0**

## 附录 14:最后两个中置信度标签的验证轮(2026-10-12)

- **0x9c(masked_select, 全仓 1 字)**: GatherMask 全模式(f16/f32/f64/reduceMode 两档/V1/V2)+ CompareScalar EQ + Cast i64→i32 均复现不出(均产 0x9d/0x9a/0x8c);DWARF 重编后该字消失(代码生成路径不同)。维持"vgathermask 族?"标注,疑似 8 字节路径特有限定,样本不足无法收编。
- **0x4c/0x4d(roi_align, 全仓 2 字)**: 系统性排除——LT/GT/NE/LE/&&/||/break/有符号/64 位标量循环全部只产 0x4a;二字相邻且共享 b1=0xfb(-5 字回跳偏移),与 branch.t 相邻。确认为 JUMPCMP 族控制流(负偏移回跳实锤),子模式位未解——升级为"族内确定、子模式待定"。
- 本轮终态: 230 万字中仅 0x9c(1 字)无法触发复现,其余全部有变体/DWARF/结构证据。

## 附录 15:中置信度标签收官(2026-10-12)

- **0x92 = KFC 消息字**(Kernel Function Call, AIC<->AIV 通信):DWARF 归因到 kfc_comm.h:290 的 KfcMsgMakeFlag(funID<<偏移+0x8000+instID);matmul/leakyrelu 样例的 Matmul API 内部通信。中→中高置信度。
- **0x4c/0x4d = JUMPCMP 内存操作数变体**:DWARF 归因到 dav_c220/kernel_operator_common_impl.h:74 的 `if (g_sysWorkspaceReserved == nullptr)`(全局变量条件检查)。所有标量循环形态(LT/GT/NE/LE/&&/||/break/有符号/64位)系统性排除只产 0x4a。中→中高置信度。
- **0x9c**: 唯一未收编字(全仓 1 字, masked_select)。GatherMask 全模式/CompareScalar/Cast 均复现不出, DWARF 重编后消失——单样本孤立, 保持标注。
- vcmpvs dtype=00(d0): 位域确认(b0[1:0]), 类型值未实测(bf16 重载不可编译)。

## 附录 16:0x9c 攻坚与重要修正(2026-10-12)

### 0x9c 攻坚全程(方法论样板)
1. 最小变体(GatherMask 全模式/CompareScalar/Cast i64)复现失败
2. **精确复现语料构建**:发现语料版带老版 stub(16832B)vs 新版(10344B)——0x9c 只在老版构建里
3. 老版 stub 重建 → 复现成功(同地址同字节 `d77bac9c`)
4. 分支级二分: 锁定 uint64(TILING_KEY_IS(8))路径
5. 函数级消融: 排除 GatherMask/CompareScalar/DataCopyPadDoubleWord
6. 读剩余源码 → 发现 GenerateMask 里的 **ShiftLeft**(掩码生成) → 变体实锤

### 最终结论(变体严格验证)
- **0x9c = vshl(ShiftLeft 向量左移)**:`8732849c`
- **0x9b = vshr(ShiftRight 向量右移)**:`8432449b` —— 两者 b0 仅差方向位(0x87/0x84)
- **修正旧标**: 0x9b 此前误标为 "vcmpsel_x"(cmp/sel 族)——abs_int32.h:81 的 ShiftRight(提取符号位)与 apply_adam 的内部移位才是真实语义

### 终态
三仓未识别 = 0;masked_select 的 `d77bac9c` 现解码为 vshl。全部 0x98-0x9e 家族收编: 0x98/0x99=vcmpv, 0x9a=vcmpvs/vand/vor, 0x9b=vshr, 0x9c=vshl, 0x9d/0x9e=vsel。

## 附录 17:评测基准语料扩充(2026-10-12)

### 新语料
- **CANN Bench**([gitcode.com/cann/cann-bench](https://gitcode.com/cann/cann-bench)):算子评测基准;tasks/ 只有 desc/proto/golden(无参考实现, agent 写 kernel),入库其 examples+infra 的 7 个 kernel
- **MultiKernelBench**([github.com/wzzll123/MultiKernelBench](https://github.com/wzzll123/MultiKernelBench)):多平台 kernel 生成基准;从 prompts 提取 8 个内嵌 AscendC kernel(add/layer_norm/leaky_relu/matmul_add/mse_loss/reduce_sum/relu)

### 扩充方法
- MKB kernel 内嵌在 .py/.json 字符串里, 写提取器(regex 抓 __global__ 代码块)
- 这批 kernel 没有独立 tiling 结构体 → 生成"万能 stub"(扫 tiling_data.* 字段名 + TCubeTiling 成员)
- 编译修复: float/uint32 字段类型区分(eps/negativeSlope 是 float)、tiling 头目录 include

### 外部验证结果
bench 语料 13 kernel / 17981 指令字, **未识别 = 0**——解码器对完全独立作者写的代码零改动通过。这是对逆向结论最强的外部印证。

## 附录 16:寄存器字段拆解(2026-10-12, 寄存器压力梯度变体)

实验: 结构相同的 kernel, 目标装载前压入 N=0..3 个 dummy GM 张量(各占一个地址寄存器)。

观测(以同一逻辑位置的字追踪):
| N | b1 | 相邻平移 |
|---|---|---|
| 0 | 0x11 | - |
| 1 | 0x31 | +0x20 |
| 2 | 0x41 | +0x10 |
| 3 | 0x52 | +0x10(近似) |

结论(0x09 类 ld_arg_gm 字段):
- **b1[7:4] = 寄存器号**(每加一个地址寄存器 +0x10 步进;kernel 内连续两条重载相差恒为 +0x10)
- **b1[3:0] = 子槽位**, **b0 bit7 = 标志位**, **b2 = 次级槽位**
- 与附录"ld_arg"(0x03 类 b0=参数偏移)互补: 参数索引在 0x03 类, 寄存器分配在 0x09 类——两层职责分离

四仓回归: 未识别仍 0/0/0/0。

## 附录 17b:细化轮(2026-10-12)

- **0x03 类参数装载**: b0/8 = GM 参数索引(4 参数变体等差 0/8/16/24 实锤)→ 新标签 `ld_arg #N`
- **0x09 类 ld_arg_gm 字段**: b1[7:4]=寄存器号(压力梯度 +0x10/寄存器实锤), b1[3:0]=子槽, b0 bit7=标志, b2=次级槽
- **0x02 类细分**: b2=0x06 且 b1=0x39 → poll_setup(wait_flag 配对,帧不变量);b2 偶数 ≥0xc0 → frame_regsave slot=(b2-0xc0)/2;其余 → addr_setup(帧不变量)
- 参数索引(0x03 类)与寄存器分配(0x09 类)职责分离确认
- 四仓回归: 0 未识别

## 附录 18:三个剩余细化项(2026-10-12, 寄存器压力梯度变体)

1. **标量 ALU(0x00 类)寄存器字段**: 否定性结论——操作数被 0x03 类 load 预置进固定工作寄存器, ALU 指令字节本身不含可变寄存器字段(s vs s2 变体仅 0x03 类字的 arg 偏移变, ALU 字不变;链式 add 第二级被并入 count)。没有什么可再解码的,这是结论。
2. **向量 opword 寄存器**: b2 低位 = 一个操作数的寄存器选择子(每加一个地址寄存器 b2 +2, N=0..3 单调: 0x46→0x48→0x4a→0x4c);b0/b1 = 其余操作数选择子(分配器重排, 非线性)。
3. **fixp 0xc2 字 regsel**: b1[7:4] = L0C 源寄存器(L0C 压力梯度:N=0/1 不变, N=2 时 +0x10——与 ld_arg_gm 的 b1 规则一致,+0x10/寄存器是全局统一规律)。

跨类统一发现: 寄存器选择子编码 = +0x10/寄存器(0x09 类和 0xc2 类一致)。

## 附录 19:计算指令微基准——II 与延迟(2026-10-12, demo_bench/)

### 方法
- 每条指令两个 kernel: **独立链**(8 个目标缓冲轮转,测发起间隔 II) + **依赖链**(串行,测延迟)
- 双点斜率法(R1=2000/R2=40000)消 launch 开销;每指令处理 256 元素
- 0x85 族 8 条轮转指令的 b2 选择子在反汇编中清晰可见(46/52/54/56/58/5a/4e/5c)

### 结果(ns/条, fp16 256 元素/条, AIV @ ~1.8GHz)
| 指令 | II | 延迟 | 备注 |
|---|---|---|---|
| vadd/vsub | 14.7 | 17.2 | 简单二元 |
| vmul | 15.3 | 17.2 | |
| vmax/vmin | 13.6 | 15.6 | |
| vdiv | 25.3 | 27.3 | 二元最慢 |
| vadds/vmuls | 13.6/14.2 | 15.6/16.2 | 标量版 |
| vmaxs/vmins | 12.5 | 14.6 | 最快 |
| vexp | 20.3 | 22.4 | 超越函数 |
| vln | 21.4 | 23.8 | |
| vsqrt | 20.8 | 22.8 | |
| vrsqrt | 13.1 | 15.1 | 比 vsqrt 快(牛顿迭代?) |
| vabs/vrelu | 12.5 | 14.6 | 最简单一元 |
| vand/vor | 12.0 | 13.9/14.1 | 位逻辑最快 |
| vshl | 13.1 | 15.3 | |
| vmadd(axpy) | 17.0 | 19.1 | 融合乘加 |
| vcvt(f16→f32) | 13.6 | - | 类型转换 |
| vreduce_sum/max/min | 23.9/17.2/17.3 | - | 归约 |
| vblockreduce_sum | 16.1 | - | 块归约 |
| vcmpv/vcmpvs | 13.3/12.2 | - | 比较 |
| vsel | 18.9 | - | 选择 |
| vbitsort | 46.7 | - | 双调排序(最慢) |
| vgather | 38.3 | - | 收集(慢) |
| **mad(参考,demo_mma)** | **1.11** | ~4.4(串行上界) | cube 16x16x16 = 8192 FLOPs/条 |

### 发现
- II 与延迟差恒定 ~2.5ns → 向量流水线很浅
- 单位换算: 256 f16 元素 ≈ 512B;II 12-15ns ≈ 22-27 cycles/指令(发射开销主导)
- mad 比任何向量指令快一个数量级(8192 FLOPs vs 512 FLOPs/条)
- 依赖链的 auto-sync 同步开销包含在"延迟"数字里(每条多一对 set/wait flag)

## 附录 20:全指令全变体微基准(2026-10-12, demo_bench/)

### 规模
- **170 个变体 kernel**(每 op × dtype × mode),预编译筛查剔除 7 个不支持组合(整型 vdiv、bf16 归约、32 位 vxor)
- 另加 **24 个 mad 变体**(cube 侧: fp16/bf16/fp32/int8 × 6 种形状)
- 构建: 6 组并行(组内 #include 合并,常量移入 kernel 作用域)

### 关键结论
1. **vcvt(类型转换)是最快指令**: ~3.5ns/条(256 元素),比 vadd 快 4 倍——转换走专用通路
2. **dtype 效应**: f16 全面最快;s32 略慢于 f16;f32 最慢(数据宽度决定);逻辑/移位指令 dtype 几乎无感
3. **vsel 两种模式差异巨大**: CMPMASK_SPR=18.9ns vs TENSOR_TENSOR=30.0ns(SPR 模式快 60%)
4. **比较 6 模式同速**(EQ/NE/LT/GT/GE/LE 无差异,差 <1%)
5. **mad dtype 阶梯(cube)**: int8(28.5 TFLOPS/核) ≈ 2× fp16(14.6) ≈ 4× fp32(3.7);bf16 ≈ fp16 略慢
6. **mad 形状**: 大方块吞吐最高(m64n64k64 fp16 14.6 TFLOPS/核);k 拉长(m16n16k64)效率掉到一半——cube 偏爱方形
7. **mad 实测 II 精化**: 无同步干扰的干净测量 = **0.705ns/条**(之前 demo_mma 的 1.11ns 含同步开销)

### 产物
- demo_bench/{gen_variants.py, gen_mad_variants.py}(生成器)
- demo_bench/variants/(177 个变体源)
- demo_bench/mad_variants/(24 个 mad 变体源)
- demo_bench/variant_results.txt(170 变体结果)
- demo_bench/mad_results.txt(24 mad 结果)

## 附录 21:周期级测量(2026-10-13)

### 方法突破
- **设备端 cycle 源**: `GetSystemCycle()` = 内联汇编 `MOV %0, SYS_CNT`(读 SYS_CNT 寄存器)——但 SYS_CNT 实测是 **~50MHz 固定参考计数器**(20ns/tick),不是核周期!
- **核频率**: npu-smi 显示 Aicore Freq=1800MHz(最大), 负载时 curFreq 实测升到 1800(空闲 800)
- **换算**: cycles = SYS_CNT ticks × 36 (= 1800/50)
- 交叉验证: vadd II = 14.75ns(host 计时) = 0.735 tick × 36 = 26.5 cycles ✓ 与 14.75ns×1.8GHz=26.5 一致

### 周期级结论(256 元素/条)
- 简单二元/逻辑: **22-30 cycles/条**(vand/vor 最快 21.5,vadd 26.5)
- 超越函数: vexp/vln/vsqrt ~73-77 cycles
- vdiv ~91 cycles;vbitsort ~168 cycles;vgather ~138 cycles
- vcvt(转换)~12.5 cycles(最快计算指令)
- **mad: 0.705ns = 1.27 cycles/条**(8192 FLOPs)——cube 基本每周期一条

## 附录 22:msdisasm 升级为完整分析工具(2026-10-14)

对标 GPU 工具(nvdisasm/cuobjdump/roc-obj)补齐五个能力:

| 功能 | 用法 | 对标 |
|---|---|---|
| **控制流图** | `--cfg` 输出 DOT 格式(分支偏移→绝对地址,基本块+边) | nvdisasm -cfg |
| **寄存器活性** | `--liveness`(地址/事件/L0C 寄存器的写读区间) | nvdisasm -plr |
| **资源占用** | `--resources`(指令数/最大寄存器号/分类直方图) | cuobjdump -res-usage |
| **JSON 输出** | `--json`(指令+CFG+活性+资源全量) | nvdisasm -json |
| **源码行关联** | `--source`(需 -g 编译,llvm-objdump 行号表) | nvdisasm -g/-gi |

实证(matmul_basic): CFG 73 块/138 边;活性追踪到 evt_r8 区间 [0x250..0x538];资源统计 383 字/最大地址寄存器 5。
