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
        if 'shared.cpp' in p:
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

def backing_for(file):
    entry = next(e for e in cc if os.path.basename(e['file']).replace('..\\src\\','') == file or file in e['file'])
    args = parse_args(entry)
    idx = ci.Index.create()
    tu = idx.parse(os.path.join(SRC, file), args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    out = {}
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.MEMBER_REF and '&' in node.type.spelling:
            # its sibling ARRAY_SUBSCRIPT_EXPR is the backing
            for sib in node.get_children():
                pass
            # The backing is a following ARRAY_SUBSCRIPT_EXPR; get its source extent
            # We get source by walking tokens. Easier: the MEMBER_REF's next sibling in the init-list
        if node.kind == CursorKind.CONSTRUCTOR and node.spelling == 'Entity':
            # iterate children in order: MEMBER_REF then expr
            prev = None
            for c in node.get_children():
                if c.kind == CursorKind.MEMBER_REF and '&' in c.type.spelling:
                    prev = c
                elif prev is not None:
                    # the backing expr source
                    start = c.extent.start.offset
                    end = c.extent.end.offset
                    src = open(os.path.join(SRC, file), encoding='utf-8').read()
                    out[prev.spelling] = src[start:end].strip()
                    prev = None
    return out

for f in ['entity_shared.cpp','stat_shared.cpp']:
    b = backing_for(f)
    print(f, len(b), 'backings')
    for k,v in list(b.items())[:8]:
        print('  ', k, '->', v)
