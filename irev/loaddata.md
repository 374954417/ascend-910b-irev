# LoadData (L1/GM → L0A/L0B) 指令编码逆向报告 — Ascend 910B (dav-c220-cube)

方法:受控变体 + 4 字节字级 diff(bisheng 编译最小 cube kernel,每次只改一个 `LoadData2DParams` 字段)。
源码与脚本:`/tmp/irev_loaddata/`(`ld_one.cpp` GM 源,`ld_l1.cpp` L1 源,`ld_v2.cpp` V2 参数,
`build_variants.sh` / `build_variants2.sh`,`wdiff.py` 字级 diff 工具,产物在 `out/`)。

## 0. 总体结构:config + issue 分离设计

GM→L0 的 `LoadData` **不是**一条自包含指令,而是"写配置寄存器 + 发射"两段式:

| 部分 | 长度 | 类标记(byte3) | 作用 |
|------|------|------|------|
| compact config | 4B | **0x10** | 参数全默认时的压缩配置字,多次同参 LoadData 共享(CSE) |
| extended config | 2–4 × 4B | **0x07** | 参数超压缩范围时的一组配置字(按 tag 分字段) |
| issue | 4B | **0x60** | 每次 LoadData 调用一条,引用 config slot |

证据:NUM_LD=1→2→3→4 阶梯,每多一条 LoadData .text 恰好 +4B(一条 issue 字);
两条参数相同的 LoadData 共享一个 config 字,参数不同则新增一个 config 字并使用不同 slot。

## 1. Issue 字(GM→L0,4B,类 0x60)

形式:`90 32 00 60`(小端字节序 byte0..byte3)

| 字段 | 位置 | 编码 | 证据 |
|------|------|------|------|
| 指令类 | byte3 | 固定 0x60 | 所有变体 |
| (保留) | byte2 | GM 路径固定 0x00 | — |
| dst 缓冲 | byte0 bit0 | 0=L0A(0x90), 1=L0B(0x91) | v14_dstb:`90320060`↔`91320060` |
| dst 地址相关位 | byte1 bit4 | 首个使用的 L0 buffer=1,其次=0(随分配顺序,非固定 L0A/L0B) | base: a2=`32` b2=`22`;v14 交换加载对象后 `32`/`22` 跟随交换;v17 四条 a2 全为 `32` |
| config slot | byte1 bit[3:0] | 原值(见到 1/2/3/4) | 与 config 字 byte2 的 slot 对应 |
| 固定位 | byte1 bit5 | 恒 1(0x20) | 所有 GM 变体 |

GM 源地址、L0 偏移不在 issue/config 字内,由周围地址建立指令(0x04/0x08/0x40 类字族)携带。

## 2. Compact config 字(4B,类 0x10)

形式(基线 rep=1,start=0,slot=2):`00 20 0a 10`

| 字段 | 位置 | 编码 | 证据 |
|------|------|------|------|
| 指令类 | byte3 | 固定 0x10 | — |
| config slot | byte2 | 0x02 \| (slot<<2);slot 见到 1→0x06, 2→0x0a, 3→0x0e, 4→0x10(slot4 时低 2bit 为 0,未完全解出) | 多 config 共存时 b2 用 `00200e10`;V2/l1 变体出现 0x06/0x10 |
| repeatTimes | byte1 bit[7:5] | **原值**,1–7;≥8 溢出转 extended 形式 | rep1/2/3/4/5/7 → byte1=0x20/40/60/80/a0/e0;rep8/16/255 转 ext |
| startIndex | byte0 | **startIndex/8**(仅支持 8 的倍数;否则转 ext) | start8→0x01, start16→0x02, start24→0x03(v21/v28/v29);start1/2/3 转 ext |
| 进入条件 | — | srcStride=0 且 dstGap=0 且 sid=0 且 startIndex%8==0 且 repeat≤7 | 任一不满足即转 extended |

## 3. Extended config 字族(各 4B,类 0x07)

byte2 = `tag_base | (slot<<2)`,slot2 时 tag 显现为 0x0a/0x4b/8b/cb;L1 路径 slot1 时为 0x06/0x47/0x87(同布局)。
按需发射,字段为 0 的字省略。

| 字 | tag(slot2) | 字段 | 位置/编码 | 证据 |
|----|-----------|------|-----------|------|
| W_a | 0x0a | startIndex | byte0 原值 | start1→`01000a07`, start2→`02000a07`, start3→`03000a07` |
| W_b | 0x4b | repeatTimes | byte0 原值(测到 255) | rep8→`08004b07`, rep16→`10004b07`, rep255→`ff004b07` |
| W_b | 0x4b | srcStride[7:0] | byte1 原值 | ss1/2/3/16→byte1=01/02/03/10;rep2+ss1→`02014b07` |
| W_c | 0x8b | srcStride[15:8] | byte0 | ss256→`01008b07`, ss512→`02008b07` |
| W_c | 0x8b | sid | byte1 bit[2:0] 原值 | sid1/2/3/7→byte1=01/02/03/07 |
| W_c | 0x8b | dstGap[3:0] | byte1 bit[7:4] | dg1/2/4/8/15→byte1=10/20/40/80/f0 |
| W_d | 0xcb | dstGap[7:4] | byte0(仅 dg≥16 时发射) | dg16→`0100cb07`, dg32→`0200cb07`, dg255→`0f00cb07`+`00f08b07` |

## 4. dtype(元素大小)

Load 的 config/issue 字**不含 dtype**。唯一随 dtype 变化的是程序更靠前的一个字 `00 SZ 0a 07`
(byte1 = 元素字节数 << 4):

| dtype | byte1 | 变体 |
|-------|-------|------|
| int8 | 0x10 | v13 |
| half / bf16 | 0x20 | base / v40(二者 load 编码完全相同) |
| fp32 | 0x40 | v41 |

语义假设:该字是 MTE2 加载通道的共享描述符(元素粒度),f16/bf16 不区分。
注意 fp32 变体 config slot 分配也从 2 变成 3(slot 号由编译器分配,非固定)。

## 5. L1→L0 路径(ld_l1.cpp)

- issue 为 **2 字 8B**,均类 0x60:基线 `00040060` + `11240c60`
- **ifTranspose 只在 L1 路径编码**:word0 byte0 bit2(`00040060`→`04040060`,u05)
- ext config 字段布局与 GM 完全相同,tag 随 slot 变化(0x06/0x47/0x87 @ slot1)
- L1 也有 compact 形式:rep2 → `00400610`(slot1, rep<<5 同布局)
- L1 issue 第二字 byte2=0x0c,其余字段未解出

## 6. 无编码效果的参数(重要坑)

| 参数 | 结果 |
|------|------|
| ifTranspose(GM→L0) | .text 与基线**逐字节相同**,被静默忽略 |
| addrMode=1(GM 与 L1) | 逐字节相同,静默忽略 |
| **LoadData2DParamsV2 全系列**(mStart/kStart/mStep/kStep/srcStride/dstStride/ifTranspose/sid,GM→L0) | **整个 LoadData 调用编译为空操作**:删掉该调用后 .text 逐字节相同(w00 vs w99_nov2)。dav-c220 上 V2 参数版 GM→L0 LoadData 不生成任何指令,使用时务必小心 |

## 7. 与 mad 指令对比

| | mad | LoadData(GM→L0) |
|---|---|---|
| 结构 | 单条 12B = 3×32bit | config(4B×1~4)+ issue(4B×N) |
| 类标记 | 0x07 | 0x10(compact cfg)/ 0x07(ext cfg)/ 0x60(issue) |
| 缩放字段 | k 编码为 k/16 | compact 中 startIndex 编码为 /8(分形组单位) |
| 其余字段 | m/n 原值、1bit 标志内联 | 全部走配置寄存器,issue 只带 slot 引用 |
| 风格 | 自包含 | 描述符/寄存器组 + 发射,同参多次加载摊薄 config |

## 8. 未解出 / 待验证

1. issue byte1 bit4 的确切语义(观测为"首个使用的 L0 buffer=1",疑为 L0 地址位,随分配器)
2. config slot 字段宽度与 byte2 低 2bit(slot4=0x10 时低 2bit=0 vs slot1-3 为 0b10)
3. GM 源地址 / L0 目的偏移绑定位置(周围 0x04/0x08/0xc2 类地址字族)
4. 程序开头 `f0ffXX10` 字与 slot 使用数相关(2 slots→0x08, 1 slot→0x06),未解
5. 多个同 tag(0x0a)类-0x07 字的关系:一个携带 elemsize<<4(byte1),ext W_a 携带 startIndex(byte0),疑为不同子寄存器或先后覆写
6. dstGap>8bit、sid>3bit、startIndex>255(ext byte1?)未测
7. L1 issue 字第 2 字(`11240c60`)各字段;L1 的 sid/dstGap 高字节未测
8. srcStride 在 L1 路径只测了 1

## 9. 变体清单(均在 /tmp/irev_loaddata/out/)

- GM 路径:v00_base, v01/02/03/18/24/25/26/27(rep 2/4/3/16/5/7/8/255), v04/05/19/36/37/43(ss 1/2/16/256/3/512),
  v06/07/20/31/32/33/34/35/44(dg 1/2/16/4/8/15/17/32/255), v08/09/21/28/29/30(start 1/2/16/8/24/3),
  v10(transpose), v11(addrmode), v12/22/38/39(sid 1/2/3/7), v13/40/41(int8/bf16/fp32),
  v14(dst L0B), v15/16/17(指令条数 2/3/4), v23(rep2+ss1), v45(rep2+start8), v42(无 a2 load)
- L1 路径:u00 基线, u01/02/03/04/05/06/07(start1/rep2/ss1/dg1/transpose/sid1/addrmode1)
- V2:w00 基线, w01–w10(全部与基线相同), w99_nov2(证明 V2 调用为空操作)
