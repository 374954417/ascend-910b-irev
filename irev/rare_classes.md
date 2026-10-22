# 罕见指令类破译报告(2026-10-12)

方法:语料统计 → 源码对照 → 受控变体验证(bisheng 编译最小 kernel,diff .text)。
所有变体源码/产物在 /tmp/irev_rare/(rv.cpp / lp*.cpp / tr*.cpp / ga.cpp / cp*.cpp / ms_*.cpp 等)。
辅助手段: bisheng 二进制内嵌的 HiIPU 指令助记符表(strings)交叉印证。

## 1. (0x4a, *) = JUMPCMP:标量比较+条件跳转融合指令 —— 置信度:高(变体验证)

- **触发**:标量循环回边。变体 `while(r<s2) r+=s2` → `a2ff494a`;`for(i=s;i<s2;i+=7)` → `a47f694a`;
  `while(r>s2)` → `a2ff4d4a`。每次循环体末尾恰好一条,位置=回边。
- **尾声固定形态**:每个 kernel 尾声恰好一条 `4580294a`,在 `XX 00 00 08`(addr_setup)+`00004041` 之后、
  `88 00 00 40`(branch.f +136)之前——是 auto-sync 尾声"事件等待分派表"的**条件跳过**判定
  (有待处理事件才进入轮询表)。语料中 `4580294a` 出现 545 次,全部为该尾声形态。
- **工具链佐证**:bisheng 字符串含 `JUMPCMP`、`fixup_HiIPU_jumpcmp`、`Enable JUMPCMP Opt for HiIPU`、
  `F32 JUMPCMP Operator`。
- **字段(部分确认)**:
  - byte2 bit0 = 1 恒定(语料 6046 条全部为奇数 byte2)
  - byte2 bit2:0 = 小于族(`<`),1 = 大于/小于等于族(`>`、`<=` 均为 1)——方向极性位
  - byte2 bit6/bit5 随操作数形式变化(while 型 0x4x,for 型 0x6x,尾声 0x29)
  - b0/b1 = 两个比较操作数的寄存器选择子(bit7 为标志);不同回边距离下编码不变 →
    **跳转目标不在指令内**,应为寄存器间接(目标由前面的 08/02 类 addr_setup 预装)或硬件循环描述符
  - 大循环体(>~1KB)编译器退化为普通 40/00+40/20 分支对,不用 0x4a(支持有限范围偏移)
- byte2 低 nibble 分布(语料):9(4303)/d(993)/5(400)/1(350) = bit2 与 bit3 的组合。
- 旁注:发现相邻新类 0x49(mul_sigmoid 中 `46004949`/`c602c149`),疑为 JUMPCMP 姊妹形态,未解。

## 2. (0x82, 0x80) 及全族 = 向量数据移动/置换族(VNCHWCONV/VTRN/VNOT/VCOPY/VGATHER2)—— 置信度:高(变体验证)

变体逐一触发(bisheng 助记符字符串印证):

| API | 发射字 | 解读 |
|---|---|---|
| TransDataTo5HD(f16) | `84264082` (byte2=0x40) | VNCHWCONV.b16 |
| TransDataTo5HD(f32) | `84268082` (byte2=0x80) | VNCHWCONV.b32 |
| Transpose(f16) | `003c4482` (byte2=0x44) | VTRN |
| 向量按位非(Duplicate+Xor 优化而来) | byte2=0x48 ×2 | VNOT.b16 |
| Copy(UB→UB) | `08108882` + `10174682`(f16)/`10178682`(f32) | VCOPY 双字 |
| Gather | `08108882` (byte2=0x88) | VGATHER2(dtype 无关字) |

- **字段**:byte2[5:0] = 子操作(0x00=VNCHWCONV,0x04=VTRN,0x06=VCOPY 参数字,0x08=VNOT/VGATHER2,
  0x0a=VCOPY 相关);**byte2[6]=16 位元素 / byte2[7]=32 位元素**(Copy f16→0x46/f32→0x86、
  5HD f16→0x40/f32→0x80 双重验证)。
- **语料中 (0x82,0x80) ×980 的真面目 = f32 数据的 TransDataTo5HD 发射字**:
  reflection_pad3d_grad(f32 计算,420 次)、grid_sample(120)、ge_glu_grad(72)全部是 f32 算子;
  且在 reflection_pad3d_grad 中 147/446 的 0x82 字落在 0x80 地址列表 ±20 字内(其余为 VCOPY 等)。
- 语料 0x82 byte2 长尾(0x82/0x84/0x8a/0x8c...)= 同族其它子操作/位宽组合(如 VNCHWCONV 的
  VA 变体,strings 中有 `VNCHWCONV.b16 [VA...`)。

## 3. (0x80, 奇数 byte2) = VNCHWCONV(TransDataTo5HD)地址列表字 —— 置信度:高(变体验证)

- TransDataTo5HD 需要 16 个 dst + 16 个 src UB 地址。变体 tr1(f16)产生:
  4×`XX YY 00 80` + 4×`XX YY 03 80` + 3×`04 80` + 3×`07 80` + 1×`06 80` 共 15 字;
  tr4(f32)同样结构。组内 b0 呈 +0x10 等差(00,10,20,30...),b1 为第二个地址——
  **每字打包 2 个 UB 地址(b0/b1 各一),byte2[2:0] = 列表内槽位/组号**。
- 语料 0x80 奇数 byte2 分布 1/3/5/7 ≈ 965/839/642/571,与地址列表槽位吻合;
  高含量文件(reflection_pad3d_grad ×2021、ge_glu_grad ×318、pad_v4_grad ×178、max_pool3d ×177)
  全部是 TransDataTo5HD/Transpose 用户(源码确认)。
- **修正旧标注**:0x80 类偶数 byte2(00/04/06)此前标为 "vaddr_mode3",实际多为同族地址列表字;
  byte2=0x40 的 "vaddr_mode2" 在普通 DataCopy 路径也独立存在,保持不变。

## 4. (0x40, 0x2c) = 41/40 配对的槽位索引控制字 —— 置信度:中低(统计+排除法)

- **强约束(语料 1553 条)**:
  - 100% 紧跟 `00 00 40 41`(类 0x41,byte2=0x40)之后;
  - b0 恒为 0x02;b1 恒为 16 的倍数(0x00/0x10/.../0xf0)→ **b1[7:4] = 槽位号(0-15)**;
  - 后继多为 03/0c 类地址字 + `00/cc` 标量字 + 48 类字 + branch.f +5(跳过一个配置块)。
- 出现的文件均为中大型生产算子(max_pool3d ×578、lstm ×514、cross_entropy ×44...);
  mul_sigmoid(单函数 828 字)中有 2 处,位置在 init() 尾部。
- **排除法**(全部变体实测不产生):函数调用/返回(noinline 无效)、ASSERT/KFC(消融 ms_noassert 不变)、
  auto-sync(去掉 --cce-auto-sync 不变)、EnQue/DeQue、运行时 count/stride 的 DataCopy、
  运行时大小 InitBuffer、SetMaskCount/SetVectorMask、尾块循环、Select/Sigmoid。
- 当前最佳假设:**"槽位 s 的条件执行/描述符操作"对**——`00004041` 为判定/发射,`02 s0 2c 40` 选择槽位,
  后随 branch.f 跳过备用配置块,疑与多路复用的搬运/同步描述符槽管理有关。待定。

## 5. 0x00 类标量 ALU 残余(byte2=0x1c/0xdc/0x3b 等)—— 置信度:中低

- **总图**:byte2 是一条偶数阶梯(0x00-0x3e 全部高频)+ 0xc0 位移组合(0xc2/0xc4/0xc6/0xdc...),
  即 byte2 = 子操作组 | 模式位(bits[7:6]);b0[5:0]/b1 主要为寄存器 id。
- 已定位:
  - `81 d7 3b 00`(00/3b)与 `84 f8 3a 00`(00/3a):**每个 kernel 序言固定各一条**(word 3 / word 10),
    为通用序言的参数/64 位地址拼装操作(所有 80+ 个变体 kernel 恒定);
  - 00/14/16/18:TransDataTo5HD 地址列表的标量地址计算(tr1 变体),byte2 随目标寄存器 +2 递增
    → byte2 内含寄存器位;
  - 00/c6、00/c8:clz(__builtin_clz)产生;00/46+00/c6:运行时标量除法(软件除法例程);
  - 00/04、00/08:常量除法/取模(乘法逆元序列)与 popc(SWAR)出现;
  - 00/dc(b0lo=9/10/11):地址密集代码高频(reflection_pad3d_grad、max_pool3d),疑为 64 位
    地址加法/拼装族的成员。
- 标量除法/取模(s/s2、s%s2)不降单指令,而是链接软件除法子程序(+55 字)。
- 具体子操作全表未逐一映射;b0[7:6] 疑为进位/高低半字标志(语料中 b0=0x01/0x81 成对出现)。

## 语料验证数据

| 类 | 语料总数 | 主要宿主 | 变体 |
|---|---|---|---|
| 0x4a | 6046 | max_pool3d ×1726, pad_v3 ×836, lstm ×590 | lp0-5/c0-5/br3-4/lp2x/lp9 |
| 0x82 | ~2500(全族) | reflection_pad3d ×420(82/80) | tr1/tr2/tr4/ga1/ga2/cp2/cp3/vnot |
| 0x80 奇 | 3063 | reflection_pad3d ×2021 | tr1/tr3/tr4 |
| 40/2c | 1553 | max_pool3d ×578, lstm ×514 | (20+ 阴性变体 + ms_* 消融) |
| 00 残余 | 0x1c ×2425, 0xdc ×982, 0x3b ×798 | 各地址密集算子 | s2_*/popc/sdiv/mv1/tr1 |
