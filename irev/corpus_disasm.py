#!/usr/bin/env python3
"""对整个语料仓跑 msdisasm, 汇总指令类覆盖统计
用法: python3 corpus_disasm.py <corpus_dir> [-v]
"""
import re
import subprocess
import sys
import glob
import os
from collections import Counter

DISASM = os.path.join(os.path.dirname(os.path.abspath(__file__)), "msdisasm.py")

def analyze(corpus_dir):
    texts = sorted(glob.glob(os.path.join(corpus_dir, "*.text")))
    total_words = 0
    total_unk = 0
    unk_classes = Counter()
    op_hist = Counter()
    for t in texts:
        out = subprocess.run(["python3", DISASM, t, "--raw"],
                             capture_output=True, text=True).stdout
        for line in out.splitlines():
            m = re.match(r"\s+[0-9a-f]{6}\s+([0-9a-f]{8})\s+(.*)", line)
            if not m:
                continue
            total_words += 1
            rest = m.group(2)
            if ".word" in rest:
                total_unk += 1
                w = bytes.fromhex(m.group(1))
                unk_classes[(w[3], w[2])] += 1
            else:
                op = rest.split()[0].split(".")[0]
                op_hist[op] += 1

    print(f"语料: {len(texts)} 个 kernel, {total_words} 个指令字")
    print(f"未识别: {total_unk} ({100 * total_unk / max(total_words, 1):.1f}%)")
    print("\n指令分布 top20:")
    for op, c in op_hist.most_common(20):
        print(f"  {op:22s} {c:6d}")
    print("\n未识别 top15 (byte3,byte2):")
    for (b3, b2), c in unk_classes.most_common(15):
        print(f"  ({b3:#04x},{b2:#04x})  x{c}")

if __name__ == "__main__":
    analyze(sys.argv[1])
