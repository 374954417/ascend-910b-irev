#!/usr/bin/env python3
"""为单个算子生成宏级 tiling stub(结构体由 global_tiling_stub.h 提供)
解析顺序: 本算子 op_host 的 REGISTER_TILING_DATA_CLASS → 全局注册表(按 CamelCase 名)
"""
import re
import sys
import glob
import os
import json

REG_PATH = "/tmp/op_corpus/stubs/registry.json"
REGISTRY = json.load(open(REG_PATH)) if os.path.exists(REG_PATH) else {}

def camelize(op_name):
    return "".join(p.capitalize() for p in re.split(r"[_\-]", op_name) if p)

def resolve_struct(op_dir, op_name):
    host_dir = os.path.join(op_dir, "op_host")
    cn = camelize(op_name)
    if os.path.isdir(host_dir):
        pairs = []
        for h in glob.glob(os.path.join(host_dir, "*.*")):
            try:
                txt = open(h, encoding="utf-8", errors="ignore").read()
            except OSError:
                continue
            pairs += re.findall(r"REGISTER_TILING_DATA_CLASS\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)", txt)
        # 优先按算子名精确匹配
        for cls, st in pairs:
            if cls.lower() == cn.lower():
                return st
        for cls, st in pairs:  # 退化: 包含关系
            if cn.lower() in cls.lower() or cls.lower() in cn.lower():
                return st
        if pairs:
            return pairs[0][1]
    for cand in (camelize(op_name), camelize(op_name).replace("Grad", "")):
        if cand in REGISTRY:
            return REGISTRY[cand]
    cn = camelize(op_name).lower()
    for k, v in REGISTRY.items():
        if k.lower() == cn or k.lower().replace("grad", "") == cn:
            return v
    return None

def main():
    op_dir, op_name, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
    st = resolve_struct(op_dir, op_name)
    if not st:
        return
    lines = ["#pragma once", ""]
    lines.append("#ifndef GET_TILING_DATA")
    lines.append("#define GET_TILING_DATA(tiling_data, tiling_arg) \\")
    lines.append(f"    {st} tiling_data;")
    lines.append("#endif")
    lines.append("#ifndef GET_TILING_DATA_WITH_STRUCT")
    lines.append("#define GET_TILING_DATA_WITH_STRUCT(struct_name, tiling_data, tiling_arg) \\")
    lines.append("    struct_name tiling_data;")
    lines.append("#endif")
    open(out_path, "w").write("\n".join(lines))
    print(st)

if __name__ == "__main__":
    main()
