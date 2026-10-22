#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
msdisasm — 昇腾 dav-c220 (Ascend 910B) 设备码初步反汇编器
基于受控变体字节 diff 逆向出的编码规则(见 irev/README.md)。

用法:
    msdisasm.py <file>          # file 可以是设备 ELF(.o/.elf)或裸 .text 二进制
    msdisasm.py <file> --raw    # 强制按裸 .text 解析

指令模型:定长 4B 小端字,顶字节 ≈ 指令大类;mad 为 3 字(12B),向量算术为 2 字(8B)。
"""
import sys
import subprocess
import tempfile
import os
import re
import json

OBJCOPY = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/ccec_compiler/bin/llvm-objcopy"

PIPES = ["S", "V", "M", "MTE1", "MTE2", "MTE3", "?", "?"]
DTYPES = {0: "s32", 1: "f16", 2: "s16", 3: "f32"}

# ---------------------------------------------------------------- 工具

def load_text(path, force_raw=False):
    """返回 .text 字节。ELF 则用 llvm-objcopy 抽取。"""
    with open(path, "rb") as f:
        head = f.read(4)
    if head == b"\x7fELF" and not force_raw:
        tmp = tempfile.mktemp(suffix=".text")
        subprocess.run([OBJCOPY, "--dump-section", f".text={tmp}", path],
                       check=True, capture_output=True)
        with open(path, "rb") as f:
            pass
        with open(tmp, "rb") as f:
            data = f.read()
        os.unlink(tmp)
        return data
    with open(path, "rb") as f:
        return f.read()

def words(data):
    return [data[i:i + 4] for i in range(0, len(data) - 3, 4)]

# ---------------------------------------------------------------- 各指令族解码

def dec_mad_w0(w):
    """mad 第 0 字:m 原值 + partialK + L0A 选择子(顶字节须为 0x07,m 为 8 的倍数)"""
    if w[3] != 0x07:
        return None
    m, b1, sel = w[0], w[1], w[2]
    if sel == 0x47 and b1 == 0:
        return None  # fixp mSize 字,优先让位
    if not (8 <= m <= 255 and m % 8 == 0 and b1 in (0x00, 0x80)):
        return None
    pk = " partialK" if b1 & 0x80 else ""
    return f"m={m}{pk} l0a_sel={sel:#04x}"

def dec_mad_w1(w):
    """mad 第 1 字:k/16 + n + L0B 选择子(顶字节 0x07,选择子为奇数)"""
    if w[3] != 0x07:
        return None
    kf, n, sel = w[0], w[1], w[2]
    if not (1 <= kf <= 32 and 8 <= n <= 255):
        return None
    k = kf * 16 if kf else 8  # kf=0 即 partial-K(k<16)
    return f"k={kf * 16 if kf else '<16'} n={n} l0b_sel={sel:#04x}"

def dec_mad_w2(w):
    """mad 第 2 字:unitFlag/initVal + L0C 选择子(顶字节 0x07)"""
    if w[3] != 0x07:
        return None
    b0, b1, sel = w[0], w[1], w[2]
    if not (b0 in (0x00, 0x80) and b1 in (0x00, 0x80) and sel >= 0x40):
        return None
    return f"unitFlag={b0 >> 7} initC={b1 >> 7} l0c_sel={sel:#04x}"

# 一元 0x83: sub-opcode = b0[7]:b1[3:0] (5bit, 寄存器位已剥离, 多布局交叉验证)
VUNARY_SUB = {0x10: "vexp", 0x11: "vrelu", 0x12: "vln", 0x01: "vrsqrt",
              0x02: "vrecp", 0x03: "vabs", 0x1c: "vsqrt",
              # 归约族(vector_opcode_table.md §9, 语料+源码对照确认)
              0x06: "vblockreduce_sum", 0x13: "vreduce_sum",
              0x04: "vreduce_max", 0x05: "vblockreduce_max",
              0x14: "vreduce_min", 0x15: "vblockreduce_min",
              0x17: "vpairreduce_sum"}
# 比较 mode = {b3[0], b2[7:6]}
VCMP_MODE = {0: "eq", 1: "ne", 2: "lt", 3: "gt", 4: "ge", 5: "le"}
VCMP_DT = {0: "d0(未识别类型)", 1: "f16", 2: "s32", 3: "f32"}

def dec_varith(w):
    """向量算术/比较操作码字(opword);opcode 位域经多布局交叉验证"""
    op = w[3]
    dt = DTYPES.get(w[2] >> 6, "?")
    if op == 0x83:                                   # 一元
        sub = ((w[0] >> 7) << 4) | (w[1] & 0x0f)
        name = VUNARY_SUB.get(sub, f"vunary_sub{sub:#x}")
        return f"{name}.{dt}"
    if op in (0x85, 0x87):                           # Add/Sub, Max/Min
        base = {0x85: "vadd", 0x87: "vmax"}[op]
        if w[0] & 1:
            base = {0x85: "vsub", 0x87: "vmin"}[op]
        return f"{base}.{dt}"
    if op == 0x89:
        return f"vmul.{dt}"
    if op == 0x8b:                                   # Div: 单指令(新发现)
        return f"vdiv.{dt}"
    if op == 0x82:                                   # 向量 permute/移动族(见 dec_vperm)
        return dec_vperm(w)
    if op == 0x8c:
        return f"vcvt/round.{dt} sub={w[0]:02x},{w[1]:02x}"
    if op == 0x96:
        return f"vmaxs.{dt}"
    if op == 0x97:
        return f"vadds/vmuls.{dt}"   # 二者位级区分未解出
    if op in (0x98, 0x99):                           # 向量-向量比较
        mode = VCMP_MODE.get(((op & 1) << 2) | ((w[2] >> 6) & 0x3), "?")
        return f"vcmpv.{mode}.{VCMP_DT.get(w[0] & 3, '?')}"
    if op == 0x9a:                                   # 向量-标量比较
        mode = VCMP_MODE.get(((op & 1) << 2) | ((w[2] >> 6) & 0x3), "?")
        return f"vcmpvs.{mode}.{VCMP_DT.get(w[0] & 3, '?')}"
    return None

# 0x82 族:向量数据移动/置换(2026-10-12 变体验证,见 rare_classes.md)
# byte2[5:0]=子操作; byte2[6]=16 位元素 / byte2[7]=32 位元素
VPERM_SUB = {0x00: "vnchwconv", 0x04: "vtrn", 0x06: "vcopy.cfg",
             0x08: "vnot/vgather2", 0x0a: "vcopy"}

def dec_vperm(w):
    """0x82 类:向量 permute/移动族(TransDataTo5HD/Transpose/Copy/Gather/Not)"""
    if w[3] != 0x82:
        return None
    sub = w[2] & 0x3f
    esz = "b16" if (w[2] & 0x40) else ("b32" if (w[2] & 0x80) else "b?")
    name = VPERM_SUB.get(sub, f"vperm_sub{sub:#x}")
    return f"{name}.{esz} (regs={w[0]:02x}/{w[1]:02x})"

def dec_jumpcmp(w):
    """0x4a 类:JUMPCMP 融合标量比较+条件跳转(循环回边/尾声分派跳过);变体验证"""
    if w[3] != 0x4a:
        return None
    dirn = "ge-side" if w[2] & 0x04 else "lt-side"
    return f"jumpcmp.{dirn} (regs={w[0]:02x}/{w[1]:02x} mode={w[2]:02x}, 目标寄存器间接)"

def dec_vnchwconv_addr(w):
    """0x80 类奇数 byte2:VNCHWCONV(TransDataTo5HD)地址列表字,每字打包 2 个 UB 地址"""
    if w[3] == 0x80 and (w[2] & 1):
        return f"vnchwconv.addr slot{w[2] & 0x7} addr0={w[0]:02x} addr1={w[1]:02x}"
    return None

def dec_ctrl_slot(w):
    """(0x40,0x2c):41/40 配对的槽位控制字;b0 恒 0x02,b1[7:4]=槽位 0-15(语义待定)"""
    if w[3] == 0x40 and w[2] == 0x2c:
        return f"ctrl_slot s={w[1] >> 4} (与 41/40 配对,语义待定)"
    return None

def dec_flag(w):
    """flag 族(0x40 类 byte2 bit7=1, 排除 0xe0=vaddr_mode):
    byte2 bit6: 0=set 1=wait;低位=变体(a2/c2 标准, a4/c4 带值);B1=(src<<2)|(dst>>1), B0.7=dst&1"""
    if w[3] != 0x40 or not (w[2] & 0x80) or w[2] == 0xe0:
        return None
    kind = "wait_flag" if (w[2] & 0x40) else "set_flag"
    if w[2] in (0xa4, 0xc4):
        kind += "_v"
    elif w[2] not in (0xa2, 0xc2):
        kind += "_x%02x" % w[2]   # 罕见子变体(如实标注)
    b0, b1 = w[0], w[1]
    src = (b1 >> 2) & 0x7
    dst = ((b1 & 0x1) << 1) | (b0 >> 7)
    return f"{kind} {PIPES[src]}->{PIPES[dst]} (reg{b0 & 0x7f})"

def dec_vcopy(w):
    """AIV DataCopy 发射字:顶字节 0x70/0x71,byte2 bit7=src 在 UB,byte0 bit3=dst UB / bit4=dst GM"""
    if w[3] == 0x71:
        return f"dcopy GM->UB (MTE2, regsel={w[0]:02x}{w[1]:02x}{w[2]:02x})"
    if w[3] == 0x70:
        src = "UB" if w[2] & 0x80 else "GM"
        if w[0] & 0x08:
            dst = "UB"
        elif w[0] & 0x10:
            dst = "GM"
        else:
            dst = "?"
        return f"dcopy {src}->{dst} (regsel={w[0]:02x}{w[1]:02x}{w[2]:02x})"
    return None

def dec_ld_issue(w):
    """LoadData issue 字(类 0x60)"""
    if w[3] != 0x60:
        return None
    dst = "L0B" if (w[0] & 1) else "L0A"
    return f"ld2d.issue -> {dst} cfgslot={w[1] & 0xf}"

def dec_ld_compact(w):
    """LoadData compact config(类 0x10,byte2=0x0a)"""
    if w[3] != 0x10 or w[2] != 0x0a:
        return None
    return f"ld2d.cfg start={w[0] * 8} repeat={(w[1] >> 5) & 0x7}"

def dec_fixp_copy(w):
    """fixp L0C->GM 拷贝字(类 0xc2);b1[7:4]=L0C 源寄存器(L0C 压力梯度 +0x10 实锤)"""
    if w[3] != 0xc2:
        return None
    return f"fixp.copy L0C->GM l0c_reg={w[1] >> 4} sel={w[0]:02x}/{w[2]:02x}"

def dec_opcfg(w):
    """mad/fixp 共享操作数配置字(类 0xf0)"""
    if w[3] != 0xf0:
        return None
    dt = []
    if w[0] & 1:
        dt.append("bf16")
    if not (w[2] & 0x80):
        dt.append("non-fp16-in")
    return f"opcfg{' ' + ','.join(dt) if dt else ' fp16'}"


def dec_scalar00(w):
    """0x00 类标量 ALU:byte2=族, byte0 低位=族内操作"""
    if w[3] != 0x00:
        return None
    if w[2] == 0x02:
        op = {1: "add", 2: "sub", 3: "mul"}.get(w[0] & 0x3)
        if op:
            return f"s_{op} (regs={w[0]:02x}/{w[1]:02x})"
    if w[2] == 0xc4:                       # 逻辑族(and/or/xor 经 rr 变体验证)
        op = {0xa: "and", 0xb: "or", 0xc: "xor"}.get(w[0] & 0xf)
        if op:
            return f"s_{op} (regs={w[0]:02x}/{w[1]:02x})"
    if w[2] == 0xc2 and w[0] == 0x40:
        return f"s_shl (regs={w[1]:02x})"
    if w[2] == 0x42 and w[0] == 0xc0:
        return f"s_shr (regs={w[1]:02x})"
    if w[2] == 0x40:
        return f"s_cmp/sel (regs={w[0]:02x}/{w[1]:02x})"
    if w[1] or w[0]:
        return f"scalar00 op={w[2]:02x} {w[0]:02x}{w[1]:02x}"
    return None

def dec_branch(w):
    """条件分支族(0x40 类, byte2 bit7=0):byte2 bit5=极性, 低位=子模式
    (0x2e=ret 标记等由更高优先级规则拦截)"""
    if w[3] != 0x40 or (w[2] & 0x80):
        return None
    if w[2] == 0x2e:
        return None  # kernel_ret 标记, 归 marker
    imm = w[0] | (w[1] << 8)
    if imm >= 0x8000:
        imm -= 0x10000
    pol = "t" if (w[2] & 0x20) else "f"
    sub = w[2] & 0x1f
    suffix = " sub=%#x" % sub if sub else ""
    return f"branch.{pol} {imm:+d} words{suffix}"

def dec_vaddrmode(w):
    """向量算术的寻址模式字(0x40 类 byte2=0xe0)"""
    if w[3] == 0x40 and w[2] == 0xe0:
        return f"vaddr_mode {w[0]:02x}{w[1]:02x}"
    return None

def dec_branch2(w):
    return None  # 已并入 dec_branch


def dec_poll_event(w):
    """0x0a 类:事件状态轮询选择;b0=事件槽位(递增), b2: 0xc0/0x00 = 事件组"""
    if w[3] != 0x0a:
        return None
    bank = {0xc0: "B", 0x00: "A"}.get(w[2], "g%02x" % w[2])
    return f"poll_event evt={w[0]} bank={bank}"

def dec_vaddrmode2(w):
    """0x80 类 byte2=0x40:向量寻址模式字(另一形态)"""
    if w[3] == 0x80 and w[2] == 0x40:
        return f"vaddr_mode2 {w[0]:02x}{w[1]:02x}"
    return None

def dec_marker(w):
    """尾声/序言标记(每 kernel 恒定位置出现,语义为推断)"""
    if w[3] == 0x40 and w[2] == 0x2e:
        return "path_exit (执行路径收尾, DWARF 归因: 函数尾声 ABI)"
    if w[3] == 0x41:
        return f"frame_exit {w[0]:02x}{w[1]:02x}{w[2]:02x} (DWARF 归因: 尾声)"
    if w[3] == 0x48:
        return f"frame_enter {w[0]:02x}{w[1]:02x}{w[2]:02x} (DWARF 归因: 序言)"
    return None

def dec_misc2(w):
    if w[3] == 0x80 and (w[2] & 1) == 0:
        return f"vaddr_mode3 {w[0]:02x}{w[1]:02x} sub={w[2]:02x}"
    if w[3] == 0x0e:
        return f"scalar64_aux {w[0]:02x}{w[1]:02x}{w[2]:02x} (64 位地址/立即数辅助)"
    if w[3] == 0x40 and w[2] in (0x2f, 0x02):
        return f"scalar40 sub={w[2]:02x} {w[0]:02x}{w[1]:02x}?"
    return None

def dec_vlogic(w):
    """0x9a 类扩展:向量位逻辑(And=b0 bit0=1 / Or=bit0=0, b2=0x4a 变体验证);
    0x9b = 同族掩码/比较变体(abs/adam 语境, 低置信);0x8d = 向量运算(roi 插值路径, 低置信)"""
    if w[3] == 0x9a and w[2] == 0x4a:
        op = "vand" if (w[0] & 1) else "vor"
        return f"{op} (regs={w[0]:02x}/{w[1]:02x})"
    if w[3] == 0x9b:
        return f"vshr (ShiftRight, 变体实锤 `8432449b`)"
    if w[3] == 0x8d:
        return f"read_sysreg {w[0]:02x}{w[1]:02x} sub={w[2]:02x} (DWARF 归因: get_block_idx 区)"
    return None

def dec_vsel(w):
    """0x9d/0x9e 类: vsel 向量选择(CompareScalar+Select 变体验证)"""
    if w[3] == 0x9e:
        return f"vsel.a {w[0]:02x}{w[1]:02x} sub={w[2]:02x}"
    if w[3] == 0x9d:
        return f"vsel.b {w[0]:02x}{w[1]:02x} sub={w[2]:02x}"
    return None

def dec_last(w):
    """最终长尾:
    0x95=vmadd(Axpy 融合乘加, 变体实锤 `84324495`);
    0x92=matmul 路径地址描述字(低置信, 宿主=matmul 样例);
    0x4c/0x4d=JUMPCMP 族(结构邻近, 中置信);
    0x9c=vgathermask(masked_select 宿主, 中置信)"""
    if w[3] == 0x95:
        return f"vmadd (Axpy 融合乘加)"
    if w[3] == 0x92:
        return f"kfc_msg {w[0]:02x}{w[1]:02x} sub={w[2]:02x} (KFC AIC<->AIV 消息, DWARF 归因 kfc_comm.h)"
    if w[3] == 0x4c:
        return f"jumpcmp.d {w[0]:02x}{w[1]:02x}{w[2]:02x} (JUMPCMP 内存操作数变体, DWARF: 全局变量 nullptr 检查)"
    if w[3] == 0x4d:
        return f"jumpcmp.e {w[0]:02x}{w[1]:02x}{w[2]:02x} (JUMPCMP 内存操作数变体, DWARF: 全局变量 nullptr 检查)"
    if w[3] == 0x9c:
        return f"vshl (ShiftLeft, 变体实锤 `8732849c`)"
    return None

def dec_stack_ls(w):
    """0x13/0x14 类:标量栈 load/store(寄存器溢出;-O1 压力变体实锤 store=0x14,load 对称)
    b0=槽位偏移(字), b1/b2=寄存器/标志"""
    if w[3] == 0x14:
        return f"st_scalar slot={w[0]} (寄存器溢出写栈)"
    if w[3] == 0x13:
        return f"ld_scalar slot={w[0]} (栈重载)"
    return None

def dec_final_misc(w):
    """最后一档: 0x49/0x4b=JUMPCMP 姊妹形态, 0x10 非 0x0a 子 tag=栈/地址调整(推断)"""
    if w[3] == 0x49:
        return f"jumpcmp.b {w[0]:02x}{w[1]:02x}{w[2]:02x} (JUMPCMP 姊妹形态)"
    if w[3] == 0x4b:
        return f"jumpcmp.c {w[0]:02x}{w[1]:02x}{w[2]:02x} (JUMPCMP 姊妹形态)"
    if w[3] == 0x10 and w[2] != 0x0a:
        imm = w[0] | (w[1] << 8)
        if imm >= 0x8000:
            imm -= 0x10000
        return f"stack_adj {imm:+d} (帧栈调整, DWARF 归因)"
    return None

def dec_scalar64(w):
    """0x01 类: 64 位标量立即数/地址物化(DWARF 归因 set_vector_mask(-1,-1));
    0x6b 类: DataCopyPad 发射字(padcopy 变体实锤)"""
    if w[3] == 0x01:
        return f"scalar64 imm_part {w[0]:02x}{w[1]:02x} sub={w[2]:02x}"
    if w[3] == 0x6b:
        sub = {0x86: "padcopy", 0xc6: "padcopy.b", 0xc4: "padcopy.c",
               0x88: "padcopy.d", 0xc2: "padcopy.e"}.get(w[2], "padcopy.%02x" % w[2])
        return f"{sub} (regsel={w[0]:02x}{w[1]:02x})"
    return None

def dec_argload(w):
    """0x03 类,b1==0 且 b2==0xc0 且 b0%8==0: 从参数区装载第 N 个 GM 参数的地址
    (变体实锤: 4 参数 kernel 分别只用 arg0-3, 该字 b0 严格等差 0/8/16/24)"""
    if w[3] == 0x03 and w[1] == 0 and w[2] == 0xc0 and w[0] % 8 == 0:
        return f"ld_arg #{w[0] // 8} (参数区偏移 {w[0]}B)"
    return None

def dec_housekeep(w):
    """地址簿记类:0x02=flag 轮询建立/地址装载, 0x09=GM 参数加载, 0x0f=帧/TPipe 建立, 0x04/0x08/0x03=地址setup
    0x09 类字段(寄存器压力梯度变体实锤): b1[7:4]=寄存器号, b1[3:0]=子槽, b0 bit7=标志位, b2=次级槽位"""
    if w[3] == 0x09:
        reg = w[1] >> 4
        sub = w[1] & 0xf
        flag = "f" if (w[0] & 0x80) else "-"
        return f"ld_arg_gm reg={reg} sub={sub} flag{flag} slot={w[2]:#04x}"
    if w[3] == 0x02:
        if w[1] == 0x39 and w[2] == 0x06:
            return "poll_setup (wait_flag 配对, 帧不变量)"
        if (w[2] & 1) == 0 and w[2] >= 0xc0:
            return f"frame_regsave slot={(w[2] - 0xc0) // 2} (帧寄存器存取)"
        return f"poll_setup/addr {w[0]:02x}{w[1]:02x}{w[2]:02x}"
    m = {0x0f: "frame_setup", 0x04: "addr_setup", 0x08: "addr_setup", 0x03: "addr_setup"}
    if w[3] in m:
        return f"{m[w[3]]} {w[0]:02x}{w[1]:02x}{w[2]:02x}"
    return None

def dec_cfg07(w, ctx=None):
    """0x07 类配置/setup 字,按 byte2 子 tag 解码;ctx 用于消除 tag 冲突(ld/fixp)"""
    if w[3] != 0x07:
        return None
    tag, b0, b1 = w[2], w[0], w[1]
    imm16 = b0 | (b1 << 8)
    if tag == 0x04:
        return f"imm16_setup val={imm16} (向量 count / 标量位型 / 拷贝参数,视消费者)"
    if tag == 0x06:
        if b0 == 0 and (b1 & 0x0f) == 0:
            return f"movi_lo lo4={(b1 >> 4) & 0xf} (定数低 4 位)"
        if b0 == 0:
            return f"fixp.nSize={b1 * 16}"
        return f"cmp_imm/movi8 imm8={b0}"
    if tag == 0x08:
        return f"setup imm={imm16} (count 或地址,地址单位为 256B → {imm16 * 256}B)"
    if tag == 0x0a:
        if b0 == 0 and b1 & 0x0f == 0 and b1 != 0:
            return f"ld2d.dtype elemsz={b1 >> 4}B"
        return f"movi imm16={imm16} (或 ld2d.startIndex ext)"
    if tag == 0x45:
        bl = imm16 & 0xfff
        if bl > 255:
            return f"cfg45 imm16={imm16} (超 blockLen 范围, 可能非拷贝参数)"
        return f"vec.blockLen={bl} (x32B = {bl * 32}B)"
    if tag == 0x47:
        if ctx == "fixp":
            return f"fixp.mSize={imm16}"
        return f"movi_h imm16={imm16} (定数高位, 完整值=(imm16<<4)|低4位)"
    if tag == 0x4b:
        return f"ld2d.ext repeat={b0} srcStrideLo={b1}"
    if tag == 0x85:
        if b1 == 0xff:
            return (f"fixp.quant pre={(b0 >> 2) & 0x1f} relu={b0 >> 7} "
                    f"unitFlag={b0 & 1} chSplit={(w[1] >> 2) & 1} nz2nd={(w[1] >> 3) & 1}")
        return f"vec.srcStride={b0}"
    if tag == 0x87:
        return f"fixp.dstStride={b0} (x32B)"
    if tag == 0x8b:
        return f"ld2d.ext srcStrideHi={b0} sid={b1 & 0x7} dstGapLo={(b1 >> 4) & 0xf}"
    if tag == 0xc5:
        return f"vec.dstStride={b0}"
    if tag == 0xcb:
        return f"ld2d.ext dstGapHi={b0}"
    if tag == 0x02:
        return f"fixp.ndNum={b0}"
    if tag == 0x43:
        return f"fixp.srcNdStride={imm16}"
    if tag == 0x83:
        return f"fixp.dstNdStride={imm16}"
    if imm16 == 0:
        return f"cfg_init tag={tag:#04x} (配置槽初始化/清除)"
    return f"cfg07 tag={tag:#04x} imm16={imm16}"

CLASS_NAMES = {0x40: "scalar/misc", 0x02: "mte/addr", 0x0a: "?", 0x0f: "?",
               0x08: "?", 0x04: "?", 0x09: "?", 0x03: "?", 0x41: "?", 0x48: "?"}

# ---------------------------------------------------------------- 主解码

def decode_word(w, ctx=None, allow_mad=True, has_fixp=True):
    """按优先级尝试各指令族,返回 (助记符, 类别)"""
    rules = []
    if allow_mad:
        rules += [(dec_mad_w2, "mad"), (dec_mad_w0, "mad"), (dec_mad_w1, "mad")]
    rules += [(dec_varith, "vec"), (dec_flag, "flag"), (dec_vcopy, "copy"),
              (dec_scalar00, "scalar"), (dec_branch, "scalar"), (dec_branch2, "scalar"),
              (dec_vaddrmode, "vec"), (dec_poll_event, "poll"), (dec_vaddrmode2, "vec"),
              (dec_vsel, "vec"), (dec_vlogic, "vec"), (dec_marker, "marker"), (dec_jumpcmp, "scalar"), (dec_vnchwconv_addr, "vec"),
              (dec_ctrl_slot, "ctrl"), (dec_misc2, "misc"), (dec_stack_ls, "scalar"),
              (dec_last, "misc"),
              (dec_final_misc, "misc"),
              (dec_scalar64, "scalar64"), (dec_argload, "ld"), (dec_housekeep, "hk"),
              (dec_ld_issue, "ld"), (dec_ld_compact, "ld"),
              (dec_fixp_copy, "fixp"), (dec_opcfg, "cfg")]
    for fn, kind in rules:
        r = fn(w)
        if r:
            return r, kind
    # fixp 专属 tag(02/06/43/47/83/87 及 85+ff)只在文件里真的存在 fixp.copy 时启用
    r = dec_cfg07(w, ctx)
    if r and not has_fixp:
        fixp_only = ("fixp.nSize", "fixp.mSize", "fixp.ndNum", "fixp.srcNdStride",
                     "fixp.dstNdStride", "fixp.dstStride", "fixp.quant")
        if r.startswith(fixp_only):
            # fixp 语义不适用于本文件, 退化为通用标签
            imm16 = w[0] | (w[1] << 8)
            return f"cfg07 tag={w[2]:#04x} imm16={imm16}", "cfg"
    if r:
        return r, "cfg"
    return None, CLASS_NAMES.get(w[3], f"class_{w[3]:02x}")

MAD_WINDOW = 8  # mad 三字允许的最大间隔(字)

def disasm(data):
    ws = words(data)
    # 第零遍:全局锚点——判断文件来自 AIC 还是 AIV
    # AIC 必有 ld2d issue(0x60 类)或 fixp.copy(0xc2 类);都没有则按 AIV 处理(禁掉 mad 规则)
    has_fixp = any(w[3] == 0xc2 for w in ws)
    is_cube = has_fixp or any(w[3] == 0x60 for w in ws)
    # 第一遍:逐字解码(带上下文)
    recs = []
    ctx = None
    for i, w in enumerate(ws):
        text, kind = decode_word(w, ctx, allow_mad=is_cube, has_fixp=has_fixp)
        if kind in ("ld", "fixp", "mad"):
            ctx = kind if kind != "mad" else ctx  # mad 不改变 ld/fixp 上下文锚
            if kind == "mad":
                ctx = "mad"
        recs.append([text, kind])
    # 第二遍:孤立 mad 候选(±MAD_WINDOW 内无其他 mad 字)降级重解码
    mad_idx = [i for i, r in enumerate(recs) if r[1] == "mad"]
    for i in mad_idx:
        if not any(j != i and abs(j - i) <= MAD_WINDOW for j in mad_idx):
            recs[i] = list(decode_word(ws[i], allow_mad=False, has_fixp=has_fixp))    # 第三遍:输出,mad 字按窗口聚合成块(允许块内夹带其他指令)
    lines = []
    last_mad = -MAD_WINDOW - 1
    in_block = False
    for i, w in enumerate(ws):
        addr = i * 4
        text, kind = recs[i]
        if kind == "mad":
            if not in_block or i - last_mad > MAD_WINDOW:
                lines.append("  ┌─ mad (cube mma, 12B) " + "─" * 26)
                in_block = True
            lines.append(f"  │ {addr:06x}  {w.hex():8s}    {text}")
            last_mad = i
            continue
        if in_block and i - last_mad > MAD_WINDOW:
            lines.append("  └" + "─" * 51)
            in_block = False
        if text:
            lines.append(f"  {addr:06x}  {w.hex():8s}  {text}")
        else:
            lines.append(f"  {addr:06x}  {w.hex():8s}  .word ({kind})")
    if in_block:
        lines.append("  └" + "─" * 51)
    return lines

# ---------------------------------------------------------------- 分析层(CFG/活性/资源/JSON/源码)

def analyze(data):
    """结构化解码:返回 [{addr, hex, text, kind, word}]"""
    ws = words(data)
    has_fixp = any(w[3] == 0xc2 for w in ws)
    is_cube = has_fixp or any(w[3] == 0x60 for w in ws)
    recs = []
    ctx = None
    for i, w in enumerate(ws):
        text, kind = decode_word(w, ctx, allow_mad=is_cube, has_fixp=has_fixp)
        if kind in ("ld", "fixp", "mad"):
            ctx = kind if kind != "mad" else ctx
            if kind == "mad":
                ctx = "mad"
        recs.append({"addr": i * 4, "hex": w.hex(), "text": text or f".word ({kind})",
                     "kind": kind, "word": w})
    # mad 孤立降级(与 disasm 同逻辑)
    mad_idx = [i for i, r in enumerate(recs) if r["kind"] == "mad"]
    for i in mad_idx:
        if not any(j != i and abs(j - i) <= MAD_WINDOW for j in mad_idx):
            t, k = decode_word(recs[i]["word"], allow_mad=False, has_fixp=has_fixp)
            recs[i]["text"] = t or f".word ({k})"
            recs[i]["kind"] = k
    return recs

def build_cfg(recs):
    """从 branch 字构建基本块和边。返回 (blocks, edges)。
    block = {start, end, instrs[]}; edge = (from_addr, to_addr, kind)"""
    branches = {}   # addr -> (target_addr, text)
    for r in recs:
        m = re.match(r"branch\.[tf] ([+-]\d+) words", r["text"])
        if m:
            off = int(m.group(1))
            branches[r["addr"]] = (r["addr"] + off * 4, r["text"])
    # 基本块边界: 分支目标 + 分支指令的下一条 + 起止
    cuts = {0, len(recs) * 4}
    for addr, (tgt, _) in branches.items():
        cuts.add(tgt)
        cuts.add(addr + 4)
    blocks = []
    sorted_cuts = sorted(c for c in cuts if 0 <= c <= len(recs) * 4)
    addr2idx = {r["addr"]: i for i, r in enumerate(recs)}
    for a, b in zip(sorted_cuts, sorted_cuts[1:]):
        blk = [r for r in recs if a <= r["addr"] < b]
        if blk:
            blocks.append({"start": a, "end": b, "instrs": blk})
    edges = []
    for addr, (tgt, text) in branches.items():
        edges.append((addr, tgt, text))
        edges.append((addr, addr + 4, "fallthrough"))
    return blocks, edges

def emit_dot(blocks, edges, name="kernel"):
    out = [f'digraph "{name}" {{', "  node [shape=box, fontname=monospace];"]
    for b in blocks:
        label = "\\l".join(f"{r['addr']:06x}: {r['text']}" for r in b["instrs"]) + "\\l"
        out.append(f'  b{b["start"]:x} [label="{label}"];')
    # 地址 → 所属块起始地址 的映射(分支指令在块尾, 源地址要归到块首)
    addr2blk = {}
    for b in blocks:
        for r in b["instrs"]:
            addr2blk[r["addr"]] = b["start"]
    def blk_of(addr):
        if addr in addr2blk:
            return addr2blk[addr]
        # 块边界外(如代码末尾之后): 归到最后一个块
        candidates = [b["start"] for b in blocks if b["start"] <= addr]
        return max(candidates) if candidates else addr
    for src, tgt, kind in edges:
        style = "solid" if "branch" in kind else "dashed"
        out.append(f"  b{blk_of(src):x} -> b{blk_of(tgt):x} [style={style}];")
    out.append("}")
    return "\n".join(out)

def liveness(recs):
    """寄存器活性:从我们解码的字段提取 写/读 区间。
    跟踪: 地址寄存器(ld_arg_gm reg=)、事件寄存器(flag reg=)、L0C(fixp l0c_reg=)"""
    reg_writes = {}   # reg -> [first_addr, last_addr]
    reg_reads = {}
    for r in recs:
        t = r["text"]
        m = re.search(r"ld_arg_gm reg=(\d+)", t)
        if m:
            reg_writes.setdefault(f"addr_r{m.group(1)}", []).append(r["addr"])
        m = re.search(r"(set_flag|wait_flag).*reg(\d+)", t)
        if m:
            reg_reads.setdefault(f"evt_r{m.group(2)}", []).append(r["addr"])
        m = re.search(r"fixp.copy.*l0c_reg=(\d+)", t)
        if m:
            reg_reads.setdefault(f"l0c_r{m.group(1)}", []).append(r["addr"])
        m = re.search(r"imm16_setup val=(\d+)", t)
        if m:
            pass  # imm 写不入寄存器追踪(值不是寄存器)
    result = {}
    for reg, addrs in sorted({**reg_writes, **reg_reads}.items()):
        w = reg_writes.get(reg, [])
        rd = reg_reads.get(reg, [])
        all_addrs = w + rd
        result[reg] = {"first": min(all_addrs), "last": max(all_addrs),
                       "writes": len(w), "reads": len(rd)}
    return result

def resources(recs):
    """资源占用统计"""
    from collections import Counter
    kinds = Counter(r["kind"] for r in recs)
    max_addr_reg = -1
    max_evt_reg = -1
    for r in recs:
        m = re.search(r"ld_arg_gm reg=(\d+)", r["text"])
        if m:
            max_addr_reg = max(max_addr_reg, int(m.group(1)))
        m = re.search(r"reg(\d+)", r["text"])
        if m and "flag" in r["text"]:
            max_evt_reg = max(max_evt_reg, int(m.group(1)))
    return {"instr_words": len(recs), "kind_histogram": dict(kinds),
            "max_addr_reg": max_addr_reg, "max_evt_reg": max_evt_reg}

def emit_json(recs, cfg, live, res, name):
    blocks, edges = cfg
    return json.dumps({
        "name": name,
        "instructions": [{"addr": r["addr"], "hex": r["hex"], "text": r["text"], "kind": r["kind"]}
                          for r in recs],
        "cfg": {"blocks": [{"start": b["start"], "end": b["end"]} for b in blocks],
                "edges": [{"from": s, "to": t, "kind": k} for s, t, k in edges]},
        "liveness": live,
        "resources": res,
    }, indent=2, ensure_ascii=False)

def source_lines(path, recs):
    """源码行关联: 若 ELF 带 debug info(-g 编译), 用 llvm-objdump 提取行号表"""
    addr2line = {}
    try:
        out = subprocess.run([OBJCOPY.replace("llvm-objcopy", "llvm-objdump"), "--line-numbers", "--disassemble", path],
                             capture_output=True, text=True, timeout=60).stdout
    except Exception:
        return addr2line
    cur = None
    for l in out.splitlines():
        m = re.match(r"^(/.+|[\w./-]+\.\w+):(\d+)", l)
        if m:
            cur = f"{os.path.basename(m.group(1))}:{m.group(2)}"
            continue
        m = re.match(r"^\s+([0-9a-f]+):", l)
        if m and cur:
            addr2line[int(m.group(1), 16)] = cur
    return addr2line

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    path = sys.argv[1]
    raw = "--raw" in sys.argv
    want_cfg = "--cfg" in sys.argv
    want_json = "--json" in sys.argv
    want_live = "--liveness" in sys.argv
    want_res = "--resources" in sys.argv
    want_src = "--source" in sys.argv
    data = load_text(path, raw)
    recs = analyze(data)

    if want_json:
        cfg = build_cfg(recs)
        live = liveness(recs)
        res = resources(recs)
        print(emit_json(recs, cfg, live, res, os.path.basename(path)))
        return

    print(f"; msdisasm: {path}  (.text {len(data)}B, {len(data) // 4} words)")

    srclines = source_lines(path, recs) if want_src else {}
    last_src = None
    for r in recs:
        if want_src and r["addr"] in srclines and srclines[r["addr"]] != last_src:
            last_src = srclines[r["addr"]]
            print(f"  ; ── {last_src}")
        print(f"  {r['addr']:06x}  {r['hex']:8s}  {r['text']}")

    if want_cfg:
        blocks, edges = build_cfg(recs)
        print("\n; ===== CFG (DOT) =====")
        print(emit_dot(blocks, edges, os.path.basename(path)))

    if want_live:
        print("\n; ===== 寄存器活性 =====")
        for reg, info in liveness(recs).items():
            print(f"; {reg}: [{info['first']:#x}..{info['last']:#x}] 写{info['writes']} 读{info['reads']}")

    if want_res:
        res = resources(recs)
        print("\n; ===== 资源占用 =====")
        print(f"; 指令字: {res['instr_words']}, 最大地址寄存器: {res['max_addr_reg']}, 最大事件寄存器: {res['max_evt_reg']}")
        for k, c in sorted(res["kind_histogram"].items(), key=lambda x: -x[1]):
            print(f";   {k}: {c}")

if __name__ == "__main__":
    main()
