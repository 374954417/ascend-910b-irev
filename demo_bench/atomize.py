#!/usr/bin/env python3
"""拆解 AscendC 原子性: 每个变体 kernel 里有多少条计算指令
原子 = 恰好 1 条; 复合 = 多条"""
import subprocess, os, glob, sys
from collections import Counter

OC = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/ccec_compiler/bin/llvm-objcopy"
OD = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/ccec_compiler/bin/llvm-objdump"

# 计算指令类(0x83 一元/归约, 0x85/87/89/8b 二元, 0x8c cvt, 0x95/96/97 标量,
#             0x9a-9e 比较/选择/移位/位逻辑, 0x82 置换)
COMPUTE = {0x83, 0x85, 0x87, 0x89, 0x8b, 0x8c, 0x95, 0x96, 0x97,
           0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x98, 0x99, 0x82}

def kernel_ranges(elf):
    """从符号表拿每个 kernel 的 [start, end)"""
    out = subprocess.run([OD, "-t", elf], capture_output=True, text=True).stdout
    syms = []
    for l in out.splitlines():
        parts = l.split()
        # 格式: addr g F .text size name
        if len(parts) >= 6 and parts[2] == "F" and parts[3] == ".text":
            try:
                syms.append((int(parts[0], 16), int(parts[4], 16), parts[5]))
            except ValueError:
                pass
    syms.sort()
    ranges = []
    for i, (addr, size, name) in enumerate(syms):
        end = addr + size if size > 0 else (syms[i+1][0] if i+1 < len(syms) else 1 << 60)
        ranges.append((addr, end, name))
    return ranges

def count_compute(text_path, lo, hi):
    d = open(text_path, 'rb').read()
    n = 0
    for i in range(lo, min(hi, len(d) - 3), 4):
        if d[i+3] in COMPUTE:
            n += 1
    return n

def main():
    rows = []
    for elf in sorted(glob.glob("/tmp/atomize/*.elf")):
        name = os.path.basename(elf)[:-4]
        txt = elf[:-4] + ".text"
        if not os.path.exists(txt):
            continue
        for lo, hi, kname in kernel_ranges(elf):
            if not kname.startswith("bench_"):
                continue
            n = count_compute(txt, lo, hi)
            suffix = "_i" if kname.endswith("_i") else ("_d" if kname.endswith("_d") else "")
            # 独立链 8 槽, 依赖链 1 条/迭代
            per_op = n / 8 if suffix == "_i" else float(n)
            rows.append((name + suffix, n, per_op, kname))

    print(f"{'变体':<32s} {'计算指令数':>8s} {'每条op':>7s}  判定")
    for name, n, per_op, kname in sorted(rows):
        verdict = "原子" if abs(per_op - 1.0) < 0.1 else (f"复合({per_op:.1f}条)" if per_op > 1 else "?")
        print(f"{name:<32s} {n:>8d} {per_op:>7.2f}  {verdict}")

if __name__ == "__main__":
    main()
