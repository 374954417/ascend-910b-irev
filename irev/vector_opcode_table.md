# 昇腾 910B (dav-c220) AIV 向量算术 opcode 表(运算 × dtype)

日期: 2026-10-12。方法: 单运算最小 kernel 受控变体 + 字节 diff(工具链 bisheng + ld.lld + llvm-objcopy)。
实验材料: `/tmp/irev_vops/`(gen.py 生成 92+ 个变体、build.sh 编译、extract.py 抽取 opword、opwords_summary.txt 全量原始数据)。
前置文档: `irev/vector_arith.md`(指令长度/类别/count/标量立即数结论仍然有效)。

记号: opword = 4B 小端,写作 `b0 b1 b2 b3`(b3 = 顶字节 = 指令类)。基线布局:
一元 = 2 buffer kernel;二元 = 3 buffer kernel;count=64。寄存器位随布局变化,表中给出的是基线布局的原始值。

---

## 1. 一元运算(类 0x83)

### 1.1 opcode 位域结论(核心科学问题)

对同一运算在 3~4 种寄存器压力下编译(2buf 基线 / 6buf 高压力 / 3buf 旧布局 / dst=src 复用 / 双连排),
不变比特 = opcode,变动比特 = 寄存器/寻址:

```
b3        = 0x83 (类)
b0[7]     = opcode 高位(亚类)
b0[6:5]   = 恒 0 (保留)
b0[4:2]   = 寄存器/寻址(随布局变)
b0[1:0]   = 恒 0 (保留)
b1[7:6]   = 恒 0(纯一元 kernel;复合 kernel 中见过非 0,属寻址)
b1[5:4]   = 寄存器/寻址(随布局变)
b1[3:0]   = opcode 低位(亚类)
b2[7:6]   = dtype (s32=00, f16=01, s16=10, f32=11)
b2[5:4]   = 恒 00
b2[3:0]   = 寄存器/寻址(随布局变)
```

一元 sub-opcode = `b0[7] : b1[3:0]`(5 bit):

| sub | 运算 |
|---|---|
| 0x10 | Exp |
| 0x11 | Relu |
| 0x12 | Ln |
| 0x1c | Sqrt |
| 0x03 | Abs |
| 0x01 | Rsqrt |
| 0x02 | Reciprocal |

**修正旧结论**: 旧报告称 "Rsqrt/Relu 编码相同(88,01)、Ln/Reciprocal 相同(88,02)"——**错误**,是旧布局下
寄存器位恰好掩盖了 b0[7]。新数据在全部布局下稳定区分: Relu=(b0[7]=1,lo=1) vs Rsqrt=(0,1);
Ln=(1,2) vs Reciprocal=(0,2)。

位域证据(Exp f32 四布局):
`84 30 c4 83`(2buf) / `94 10 c8 83`(6buf) / `88 00 c6 83`(旧3buf) / `84 30 c6 83`(dst=src)。
变动: b0 bits{2,3,4},b1 bits{4,5},b2 bits{1,2,3};不变: b3=83, b0[7]=1, b1[3:0]=0, b2[7:6]=11。
Abs f32 同法: `04 33 c4 83` / `14 13 c8 83` / `08 03 c6 83`(旧) → b0[7]=0、b1lo=3 不变。

### 1.2 一元 opword 基线值(N=1, 2buf 布局)

| 运算 | f16 | f32 | s32/s16 |
|---|---|---|---|
| Exp | `84 30 44 83` | `84 30 c4 83` | 不支持(编译失败) |
| Sqrt | `84 3c 44 83` | `84 3c c4 83` | 不支持 |
| Ln | `84 32 44 83` | `84 32 c4 83` | 不支持 |
| Relu | `84 31 44 83` | `84 31 c4 83` | 不支持 |
| Abs | `04 33 44 83` | `04 33 c4 83` | 不支持(s16/s32 均编译失败) |
| Rsqrt | `04 31 44 83` | `04 31 c4 83` | 不支持 |
| Reciprocal | `04 32 44 83` | `04 32 c4 83` | 不支持 |
| Not(逻辑非) | 不支持 | 不支持 | 仅 s16: `04 38 44 82`(类 0x82!见 §6) |
| Neg | — | — | dav-c220(2201)无此 API(impl 仅 3101/5102 开放) |

dtype 验证: 同一运算 f16↔f32 仅 b2[7:6] 变(0x44↔0xc4),其余字节逐位相同 ✓。

## 2. 二元运算(类 0x85/0x87/0x89/0x8b)

### 2.1 opcode 位域结论

```
b3      = 指令类 = 主 opcode: 0x85=Add/Sub, 0x87=Max/Min, 0x89=Mul, 0x8b=Div
b0[0]   = 极性位(仅 0x85/0x87 族): Add/Max=0, Sub/Min=1;Mul/Div 恒 0
b0[7:2] = 寄存器/寻址(观测到 bits{2,3,4,7} 随布局变;bits{1,5,6} 恒 0)
b1[7:0] = 寄存器/寻址(观测到 bits{0,1,4,6} 随布局变,其余恒 0)
b2[7:6] = dtype;b2[5:4] 恒 00;b2[3:0] 寄存器/寻址
```

**极性假设在 Min/Max 上成立** ✓(全部 4 dtype × 多布局验证: Max `08 xx xx 87` ↔ Min `09 xx xx 87`)。

证据(Add f16 五布局): `08 12 4a 85`(基线) / `88 12 4c 85`(6buf) / `14 01 46 85`(旧3buf) /
`88 40 4a 85`(操作数互换 Add(z,y,x)) / `08 12 42 85`(dst=src Add(x,x,y))。
变动: b0{2,3,4,7}, b1{0,1,4,6}, b2{1,2,3};不变: b3=85, b0[0]=0, b2[7:6]。

### 2.2 二元 opword 基线值(N=1, 3buf 布局,b0/b1=08/12 恒定)

| 运算 | s32 | f16 | s16 | f32 |
|---|---|---|---|---|
| Add | `08 12 0a 85` | `08 12 4a 85` | `08 12 8a 85` | `08 12 ca 85` |
| Sub | `09 12 0a 85` | `09 12 4a 85` | `09 12 8a 85` | `09 12 ca 85` |
| Max | `08 12 0a 87` | `08 12 4a 87` | `08 12 8a 87` | `08 12 ca 87` |
| Min | `09 12 0a 87` | `09 12 4a 87` | `09 12 8a 87` | `09 12 ca 87` |
| Mul | `08 12 0a 89` | `08 12 4a 89` | `08 12 8a 89` | `08 12 ca 89` |
| Div | 不支持 | `08 12 4a 8b` | 不支持 | `08 12 ca 8b` |

- **Div = 单指令,类 0x8b**(不是 recip+mul 复合),仅 f16/f32。6buf 变体 `88 12 cc 8b`,同样只动寄存器位。
- **Mod: toolkit 无此 API(basic/adv 均无 Mod/FMod),跳过。**
- **dtype 矩阵(task 5)成立** ✓: Add/Sub/Max/Min/Mul 在 s32/f16/s16/f32 下除 b2[7:6] 外逐位相同。

## 3. 比较运算(结果进 mask,类 0x98/0x99;CompareScalar 类 0x9a)

Compare(dst:u8, src0, src1, mode, count) → 单条向量指令,**同一族**:
```
b3      = 0x98 + mode 高位(bit0)
b2[7:6] = mode 低位 2bit
b2[5:0] = 00 1010(基线布局;应为寄存器位)
b0/b1   = 寄存器/寻址;dtype 在 b0[1:0](f16=01, s32=10, f32=11 —— 与 b2 的 dtype 映射不同!)
```

| CMPMODE | f32 opword | f16 opword | mode 编码 (b2[7:6], b3) |
|---|---|---|---|
| LT | `87 41 8a 98` | `85 41 8a 98` | (10, 0x98) |
| GT | `87 41 ca 98` | `85 41 ca 98` | (11, 0x98) |
| EQ | `87 41 0a 98` | `85 41 0a 98` | (00, 0x98) |
| NE | `87 41 4a 98` | — | (01, 0x98) |
| LE | `87 41 4a 99` | — | (01, 0x99) |
| GE | `87 41 0a 99` | — | (00, 0x99) |

(mode 三比特 = {b3[0], b2[7], b2[6]}: EQ=000, NE=001, LT=010, GT=011, GE=100, LE=101。)

- CompareScalar f32 LT = `07 41 8a 9a`(类 0x9a,标量经 movi 装载,与 vcmpv 同族)。
- **s32 Compare 异常**: LT/EQ/GT 三个 mode 编译出**完全相同的 .text**(opword `86 41 0a 98`,
  .o 仅符号名不同)。疑似 bisheng 对 int32 vcmpv 不降 mode 的 bug/限制,记录存疑。
- s16 Compare 编译失败(dtype 不支持,2201 仅 half/float/int32)。

## 4. 取整/转换族(类 0x8c,= vcvt 家族)

Floor/Round f32 为**单指令**;Ceil/Trunc f32 各 2 条;f16 全部复合(5~6 条,经 f32 中转):

| 运算 | dtype | 编码(基线布局) |
|---|---|---|
| Floor | f32 | `82 30 04 8c` |
| Round | f32 | `80 30 04 8c`(与 Floor 仅 b0 bit1 差) |
| Ceil | f32 | `03 94 14 8c` + `83 11 04 8c` |
| Trunc | f32 | `04 94 14 8c` + `84 11 04 8c` |
| Floor | f16 | 6 条 0x8c:`00 c4 5a 8c` `82 d4 1a 8c` `08 d5 16 8c` `00 32 44 8c` `82 21 04 8c` `88 21 02 8c` |
| Round | f16 | 6 条 0x8c:`80 b3 58 8c` `00 c4 18 8c` `88 c4 14 8c` `00 12 44 8c` `80 20 04 8c` `88 21 02 8c` |

注意 0x8c 的 b2 **不遵循** b2[7:6]=dtype 规律(f32 Floor b2=0x04)——该族 b2 编码 src/dst 类型组合与舍入模式,未完全解出。

## 5. 复合实现的"一元"运算(adv_api/math,非单指令)

| 运算 | f16 | f32 | 说明 |
|---|---|---|---|
| Sin | 50 条向量字 | 50 条 | 多项式(0x97 muls/adds + 0x85/0x89),无专用 opcode |
| Cos | 64 条 | 64 条 | 同上 |
| Tanh | 12~18 条 | 12~18 条 | 用到 0x83 单字(sub 0x03=Abs 与 sub 0x13,疑为 Exp 变体)+ 0x8c/0x96/0x97/0x8b |
| Sign | 12 条 | 12 条 | 0x98 比较 + 0x82 Not + 0x85 算术组合,无专用 opcode |

## 6. 新指令类汇总(本次新增)

| 顶字节 | 类别 | 证据 |
|---|---|---|
| 0x82 | 向量逻辑非 vnot(仅 s16 观察样本) | u_not_s16 |
| 0x8b | 向量除法 vdiv(f16/f32) | b_div_* |
| 0x8c | 转换/取整族 vcvt(Floor/Round/Ceil/Trunc) | u_floor/round/ceil/trunc_* |
| 0x98/0x99 | 向量比较 vcmpv(结果→mask) | c_*_* |
| 0x9a | 向量-标量比较 vcmpvs | c_lts_f32 |

## 7. 统一格式(更新版)

```
向量算术指令 = [0..3 条 0x07 类地址设置字] + 8B = [寻址模式字 4B][opword 4B]
opword:
  b3        = 指令类/主 opcode: 82=not 83=一元 85=add/sub 87=max/min 89=mul 8b=div
              8c=cvt/取整 96=maxs 97=adds/muls 98/99=cmpv 9a=cmpvs
  b2[7:6]   = dtype (s32=00,f16=01,s16=10,f32=11) —— 算术族(82/83/85/87/89/8b/96/97);
              cvt 族(8c)与比较族(98/99, dtype 在 b0[1:0])例外
  b2[5:0]   = 寄存器/寻址(算术族)
  b1, b0    = 低位 opcode ⊕ 寄存器位,按族:
    一元 0x83:  opcode = b0[7]:b1[3:0];寄存器 = b0[4:2], b1[5:4]
    二元 85/87: opcode = b0[0] 极性;寄存器 = b0[7:2] 大部, b1 全部
    mul/div 89/8b: opcode 全在 b3;b0/b1 全为寄存器
    比较 98/99: opcode = {b3[0], b2[7:6]} mode + b0[1:0] dtype
count/mask/repeat 不进 opword(由 0x07 类 setup 字携带)。
```

## 8. 跳过/失败清单

- Mod: toolkit 无 API。
- Neg: dav-c220 不开放(impl 仅 __NPU_ARCH__ 3101/5102)。
- 整数 dtype 的 Exp/Sqrt/Ln/Relu/Abs/Rsqrt/Reciprocal: 编译失败(仅 f16/f32)。
- Div s32/s16、Compare s16、Not f16/f32/s32: 编译失败(dtype 不支持)。
- s32 Compare mode 编码存疑(见 §3)。

## 9. 归约族补全(0x83 类,2026-10 追加,源码对照法)

宿主算子 norm_group_norm_swish 的未映射 sub=0x06/0x13 经查证**不是 Swish 的算术语义**
(该算子 Swish = Muls(0x97) + Exp(0x10) + Adds(0x97) + Div(0x8b);Cast f16↔f32 属 0x8c 类,
均不产生未知 0x83 字),而是其均值/方差归约路径 `ReduceSumCustom` 的归约指令。
方法:读源码确定候选 API → 单运算最小 kernel 编译(/tmp/irev_unary/,工具链同前文)→
对比 opword sub 值 → 回语料仓(/tmp/op_corpus 166 文件 + /tmp/sample_corpus 59 文件)全量交叉验证。

### 9.1 新增 sub 对照(全部 f32/f16 实测,编译产物在 /tmp/irev_unary/)

| sub | 运算 | 编译证据(最小 kernel → opword) | 源码证据 | 语料宿主(出现次数) |
|---|---|---|---|---|
| 0x06 | **BlockReduceSum** | vbrsum_f32.text `83c43604`(b0=04,b1=36→sub06,f32) | group_norm_swish_base.h:426,428,437,439;cross_entropy_loss_base.h:298-314 | norm_group_norm_swish(1144),cross_entropy 等 8 文件 |
| 0x13 | **WholeReduceSum** | vwrsum_f32.text `83c43384`(b0=84,b1=33→sub13,f32) | group_norm_swish_base.h:407-416,430,441;cross_entropy_loss.h:353-382 | norm_group_norm_swish(2019),共 29 文件 f32 + 5 文件 f16 |
| 0x04 | **WholeReduceMax** | vwrmax_f32.text `83c43404`;ORDER_ONLY_VALUE 变体 `83c43406`(同 sub,order 编码在 b2[3:0]);adv ReduceMax 内部亦展开为此字(vreducemax_adv_f32.text 3 个 sub04 字) | cross_entropy_loss_base.h:354,366,386;gather_v3_tmpl_2.h:175(ReduceMax);non_finite_check_n_d.h:202 | gather_v3(99),cross_entropy(53/52),grid_sample(42),non_finite_check(30+30) 等 11 文件 f32 + 2 文件 f16 |
| 0x05 | **BlockReduceMax** | vbrmax_f32.text `83c43504` | cross_entropy_loss_base.h:345,348,351,360,363 | cross_entropy_loss(48),cross_entropy_loss_with_zloss(48) |
| 0x14 | **WholeReduceMin** | vwrmin_f32.text `83c43484`;adv ReduceMin 同(vreducemin_adv_f32.text) | gather_v3_tmpl_2.h:182(ReduceMin);non_finite_check_n_d.h:213 | gather_v3(103),grid_sample(42),non_finite_check(30+30) 等 6 文件 f32 + reduce_min 样例 f16 |
| 0x15 | **BlockReduceMin** | vbrmin_f32.text `83c43584`(编译确认,语料无宿主——无生产算子调用) | —(toolkit 有 API,生产算子未用) | 0 |
| 0x17 | **PairReduceSum** | vprsum_f32.text `83c43784` | isamax_aiv.h:359,365,371(复数路径成对求和,每 kernel 恰 3 处) | isamax/isamin/icamax/icamin 各 3 次,仅此 4 文件 |

### 9.2 编码规律补充(归约族)

- Whole 族:Sum=0x13, Max=0x04, Min=0x14;Block 族:Sum=0x06, Max=0x05, Min=0x15。
  Min 相对 Max 仅 b0[7] 翻转(0x04↔0x14, 0x05↔0x15),符合 sub = b0[7]:b1[3:0] 布局。
- `WholeReduceMax` 的 `ReduceOrder` 参数编码在 b2[3:0](寄存器低位域):
  ORDER_VALUE_INDEX(默认)→b2[3:0]=4 区域,ORDER_ONLY_VALUE→6(见 9.1 两个字)。
- PairReduceSum(0x17)与 WholeReduceSum(0x13)同 b0[7]=1 族,b1[3:0]=7 vs 3。
- dtype 位 b2[7:6] 在归约族依然成立(f16 WholeReduceMax/Min/Sum 语料实测符合)。

### 9.3 排除项与修正

- **Cast(f16↔f32)不在 0x83 类**:vcast_f16f32/vcast_f32f16 编译产物的 Cast 指令字为
  `8c443080`(f16→f32)/`8c043088`(f32→f16),顶字节 0x8c(vcvt 族),两方向仅差 b0 bit6/b2 bit3。
- **Muls/Adds 不在 0x83 类**:编译产物顶字节 0x97(标量立即数由相邻 setup 字携带),与 §7 一致。
- 修正 §5 Tanh 行"sub 0x13 疑为 Exp 变体":adv_api Tanh 在当前工具链编译(vtanh_f32.text)
  仅含 sub=0x10(Exp)两个 0x83 字,无 0x13;生产 tanh 语料(math_tanh/foreach_tanh)也只含 0x10。
  0x13 确证为 WholeReduceSum。
