#!/usr/bin/env python3
"""DWARF 归因: 把语料仓 kernel 里指定类的残余字映射回源码行
用法: dwarf_attr.py <op_dir> <kernel.cpp 相对路径> [stub...]
流程: 带 -g 重编 → 提取 .text → 找当前未识别字 → llvm-objdump -l 取行号表 → 映射
"""
import re
import subprocess
import sys
import os
import tempfile
import importlib.util
from collections import Counter

A = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux"
BISHENG = f"{A}/ccec_compiler/bin/bisheng"
OBJDUMP = f"{A}/ccec_compiler/bin/llvm-objdump"
spec = importlib.util.spec_from_file_location(
    "msd", os.path.join(os.path.dirname(os.path.abspath(__file__)), "msdisasm.py"))
msd = importlib.util.module_from_spec(spec)
spec.loader.exec_module(msd)

INCS = [f"-I{A}/asc/impl/adv_api", f"-I{A}/asc/impl/basic_api", f"-I$A/asc/impl/utils".replace("$A", A),
        f"-I{A}/asc/include", f"-I{A}/asc/include/adv_api", f"-I{A}/asc/include/basic_api",
        f"-I{A}/asc/include/utils", f"-I{A}/tikcpp/tikcfw", f"-I{A}/tikcpp/tikcfw/interface",
        f"-I{A}/tikcpp/tikcfw/impl"]
FLAGS = ["--cce-aicore-arch=dav-c220-vec", "--cce-aicore-only", "--cce-auto-sync", "-O3", "-g",
         "-std=c++17", "--cce-aicore-lang",
         f"-include", f"{A}/../include/ascendc/asc_devkit_version.h", "-D__NPU_ARCH__=2201"]

def build_addr2src(obj):
    out = subprocess.run([OBJDUMP, "--line-numbers", "--disassemble", obj],
                         capture_output=True, text=True).stdout
    addr2src = {}
    cur = None
    for l in out.splitlines():
        m = re.match(r"^(/.+|[\w./-]+\.\w+):(\d+)", l)
        if m:
            cur = f"{m.group(1).split('/')[-1]}:{m.group(2)}"
            continue
        m = re.match(r"^\s+([0-9a-f]+):", l)
        if m and cur:
            addr2src[int(m.group(1), 16)] = cur
    return addr2src

def main():
    src_file = sys.argv[1]
    stubs = sys.argv[2:]
    tmp = tempfile.mkdtemp()
    o = os.path.join(tmp, "k.o")
    cmd = [BISHENG] + FLAGS + INCS
    for s in stubs:
        cmd += ["-include", s]
    cmd += ["-I" + os.path.dirname(src_file), "-c", src_file, "-o", o]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print("COMPILE FAIL:", r.stderr[:500])
        sys.exit(1)
    addr2src = build_addr2src(o)
    # 提取 .text
    t = os.path.join(tmp, "k.text")
    subprocess.run([f"{A}/ccec_compiler/bin/llvm-objcopy", "--dump-section", f".text={t}", o],
                   check=True, capture_output=True)
    data = open(t, "rb").read()
    ws = [data[i:i+4] for i in range(0, len(data)-3, 4)]
    has_fixp = any(w[3] == 0xc2 for w in ws)
    is_cube = has_fixp or any(w[3] == 0x60 for w in ws)
    hits = Counter()
    for i, w in enumerate(ws):
        text, _ = msd.decode_word(w, allow_mad=is_cube, has_fixp=has_fixp)
        if text is None:
            hits[(f"({w[3]:#04x},{w[2]:#04x})", w.hex(), addr2src.get(i * 4, "?"))] += 1
    print(f"== {src_file.split('/')[-1]}: 未识别字 {sum(hits.values())}")
    for (cls, w, srcline), c in hits.most_common(25):
        print(f"  {cls} {w} x{c}  <- {srcline}")

if __name__ == "__main__":
    main()
