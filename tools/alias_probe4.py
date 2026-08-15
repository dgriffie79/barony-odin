#!/usr/bin/env python3
import clang.cindex as ci
from clang.cindex import CursorKind
import os, json, shlex

SRC = 'src'
cc = json.load(open('builddir/compile_commands.json', encoding='utf-8'))

def parse_args(entry):
    parts = shlex.split(entry['command'])
    args = []
    for p in parts:
        if p.endswith('cl.exe') or p == 'cl':
            continue
        if p.startswith('/Fo') or p.startswith('/Fd') or p.startswith('/c') or p.startswith('/F'):
            continue
        if 'entity_shared.cpp' in p or 'entity.cpp' in p:
            continue
        args.append(p)
    def norm(a):
        if a.startswith('/I'): return '-I' + a[2:]
        if a.startswith('/D'): return '-D' + a[2:]
        if a.startswith('/std:'): return '-std=' + a[5:]
        if a.startswith('/'): return None
        return a
    args = [x for x in (norm(a) for a in args) if x]
    args += ['-x','c++']
    return args

# find entity.cpp entry
entry = next(e for e in cc if 'entity.cpp' in e['file'])
args = parse_args(entry)
idx = ci.Index.create()
tu = idx.parse(os.path.join(SRC, 'entity.cpp'), args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

# find entityLightAfterReductions method and inspect the 'sneaking' refs
seen = 0
for node in tu.cursor.walk_preorder():
    if node.location.file and node.location.file.name.endswith('entity.cpp'):
        if node.kind in (CursorKind.MEMBER_REF_EXPR, CursorKind.DECL_REF_EXPR, CursorKind.VAR_DECL):
            sp = node.spelling
            if sp == 'sneaking':
                seen += 1
                ref = node.referenced
                print(f"line {node.location.line:4d} kind={str(node.kind):25s} spelling={sp:10s} referenced={str(ref.kind) if ref else None} refspelling={ref.spelling if ref else ''}")
                if seen > 20: break
