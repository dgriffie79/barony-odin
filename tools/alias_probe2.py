#!/usr/bin/env python3
import clang.cindex as ci
from clang.cindex import CursorKind
import os, json, shlex

SRC = 'src'
cc = json.load(open('builddir/compile_commands.json', encoding='utf-8'))
entry = next(e for e in cc if 'entity_shared.cpp' in e['file'])
parts = shlex.split(entry['command'])
args = []
for p in parts:
    if p.endswith('cl.exe') or p == 'cl':
        continue
    if p.startswith('/Fo') or p.startswith('/Fd') or p.startswith('/c') or p.startswith('/F'):
        continue
    if p.endswith('entity_shared.cpp') or p.endswith('..\\src\\entity_shared.cpp'):
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

idx = ci.Index.create()
tu = idx.parse(os.path.join(SRC,'entity_shared.cpp'), args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

def walk(node, depth=0):
    if node.kind == CursorKind.CONSTRUCTOR and node.spelling == 'Entity':
        print('ctor at line', node.location.line)
        for c in node.get_children():
            print('  ' + str(c.kind) + ' spelling=' + repr(c.spelling) + ' type=' + c.type.spelling + ' line=' + str(c.location.line))
            if c.kind == CursorKind.INIT_LIST_EXPR:
                for i in c.get_children():
                    print('     init ' + str(i.kind) + ' ' + repr(i.spelling))
    for c in node.get_children():
        walk(c, depth+1)
walk(tu.cursor)
