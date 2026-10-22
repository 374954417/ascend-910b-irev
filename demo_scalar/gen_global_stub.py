#!/usr/bin/env python3
"""扫描 cann-ops 全树, 生成全局 tiling 结构 stub + 算子->结构注册表
产物:
  /tmp/op_corpus/stubs/global_tiling_stub.h   全部 tiling 结构体 + __aicore__ Init 函数
  /tmp/op_corpus/stubs/registry.json          OpClassName -> TilingStruct 映射
"""
import re
import glob
import json
import os

import sys
SRC = sys.argv[1] if len(sys.argv) > 1 else "/tmp/cann_ops/src"
OUT_DIR = sys.argv[2] if len(sys.argv) > 2 else "/tmp/op_corpus/stubs"
FIELD_RE = re.compile(r"TILING_DATA_FIELD_DEF(_ARR|_STRUCT)?\s*\(([^)]+)\)")
KNOWN_TYPES = {"int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t",
               "uint64_t", "float", "double", "bool", "char", "half", "int",
               "uint8", "uint16", "uint32", "uint64", "int8", "int16", "int32", "int64"}

def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    structs = {}
    order = []
    registry = {}
    # 工具链自带的设备侧 tiling 结构(adv_api/kernel_tiling.h)
    lib_types = set()
    kt = "/usr/local/Ascend/ascend-toolkit/latest/aarch64-linux/asc/include/adv_api/kernel_tiling.h"
    if os.path.exists(kt):
        lib_types = set(re.findall(r"^(?:struct|class) (\w+)", open(kt).read(), re.M))
    for h in glob.glob(f"{SRC}/**/*.h", recursive=True) + glob.glob(f"{SRC}/**/*.cpp", recursive=True):
        try:
            txt = open(h, encoding="utf-8", errors="ignore").read()
        except OSError:
            continue
        for m in re.finditer(r"REGISTER_TILING_DATA_CLASS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)", txt):
            registry.setdefault(m.group(1), m.group(2))
        if "BEGIN_TILING_DATA_DEF" not in txt:
            continue
        for m in re.finditer(r"BEGIN_TILING_DATA_DEF\s*\(\s*(\w+)\s*\)(.*?)END_TILING_DATA_DEF",
                             txt, re.S):
            name, body = m.group(1), m.group(2)
            if name in structs:
                continue
            fields = []
            for fm in FIELD_RE.finditer(body):
                variant = fm.group(1)
                is_arr = variant == "_ARR"
                parts = [p.strip() for p in fm.group(2).split(",")]
                if variant == "_STRUCT" and len(parts) >= 2:
                    typ, arrn, fld = parts[0], None, parts[1]   # _STRUCT: (类型, 成员名)
                elif is_arr and len(parts) >= 3:
                    typ, arrn, fld = parts[0], parts[1], parts[2]  # _ARR: (类型, 长度, 字段名)
                elif not is_arr and len(parts) >= 2:
                    typ, arrn, fld = parts[0], None, parts[1]
                else:
                    continue
                if (typ not in KNOWN_TYPES and typ not in structs and typ not in lib_types
                        and not typ.endswith("TilingData") and not typ.endswith("Tiling")):
                    typ = "int64_t"     # 未知类型降级( tiling 结构/库类型保留原名)
                if arrn and not arrn.isdigit():
                    arrn = "8"          # 非数字数组长度降级
                fields.append((typ, fld, arrn))
            structs[name] = fields
            order.append(name)

    # 拓扑排序: 被依赖的结构体先输出
    deps = {n: {t for t, _, _ in structs[n] if t in structs} for n in order}
    topo = []
    remaining = set(order)
    while remaining:
        ready = [n for n in order if n in remaining and deps[n] <= set(topo)]
        if not ready:  # 环或缺定义, 放弃排序
            topo += [n for n in order if n in remaining]
            break
        topo += ready
        remaining -= set(ready)
    order = topo
    lines = ["#pragma once", '#include "kernel_operator.h"',
             '#include "adv_api/kernel_tiling.h"', ""]
    for name in order:
        lines.append(f"struct {name} {{")
        seen = set()
        for typ, fld, arrn in structs[name]:
            if fld in seen:
                continue
            seen.add(fld)
            lines.append(f"    {typ} {fld}[{arrn}];" if arrn else f"    {typ} {fld};")
        lines.append("};")
        lines.append(f"__aicore__ inline void Init{name}(__gm__ uint8_t* t, {name}* d) "
                     "{ (void)t; (void)d; }")
        lines.append("")
    open(f"{OUT_DIR}/global_tiling_stub.h", "w").write("\n".join(lines))
    json.dump(registry, open(f"{OUT_DIR}/registry.json", "w"))
    print(f"结构体 {len(order)} 个, 注册表 {len(registry)} 条")

if __name__ == "__main__":
    main()
