#!/usr/bin/env python3
"""Dump Stat/Entity field layouts from the real headers via libclang."""
import sys, os, json, shlex
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'tools'))
import clang.cindex as ci
from clang.cindex import CursorKind

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

cc = json.load(open(os.path.join(ROOT, 'builddir', 'compile_commands.json'), encoding='utf-8'))

def compile_args(entry):
    cwd = entry['directory']
    parts = shlex.split(entry['command'])
    args = []
    for p in parts:
        if p.endswith('cl.exe') or p == 'cl': continue
        if p.startswith('/Fo') or p.startswith('/Fd') or p.startswith('/c') or p.startswith('/F'): continue
        if p.endswith('.cpp') or ('.cpp' in p and os.path.sep in p): continue
        args.append(p)
    def norm(a):
        inc = None
        if a.startswith('/I'): inc = a[2:]
        elif a.startswith('-I') and len(a) > 2: inc = a[2:]
        if inc is not None:
            if not os.path.isabs(inc): inc = os.path.normpath(os.path.join(cwd, inc))
            return '-I' + inc
        if a.startswith('/D'): return '-D' + a[2:]
        if a.startswith('/std:'): return '-std=' + a[5:]
        if a.startswith('/'): return None
        return a
    args = [x for x in (norm(a) for a in args) if x is not None]
    args += ['-x','c++','-fms-compatibility-version=19.51',
             '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
             '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION']
    return args

def parse(header):
    args = ['-x','c++','-std=c++17','-fms-compatibility-version=19.51',
            '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
            '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION',
            '-I'+SRC,'-I'+os.path.join(SRC,'magic'),'-I'+os.path.join(SRC,'interface'),
            '-I'+os.path.join(SRC,'ui'),'-I'+os.path.join(SRC,'engine'),'-I'+os.path.join(SRC,'engine','audio'),
            '-I'+os.path.join(ROOT,'odin','containers'),'-I'+os.path.join(ROOT,'builddir'),
            '-IC:/dev/vcpkg/installed/x64-windows/include','-IC:/dev/vcpkg/installed/x64-windows/include/SDL2']
    idx = ci.Index.create()
    return idx.parse(os.path.join(SRC, header), args=args,
                     options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

def dump_class(header, clsname):
    tu = parse(header)
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.CLASS_DECL and node.spelling == clsname:
            kids = list(node.get_children())
            if not any(k.kind == CursorKind.FIELD_DECL for k in kids):
                continue  # forward decl
            print(f'=== {clsname} sizeof={node.type.get_size()} ===')
            for f in kids:
                if f.kind == CursorKind.FIELD_DECL:
                    sz = f.type.get_size()
                    print(f'  {f.spelling}: {f.type.spelling} (size {sz})')
            return
    print(f'class {clsname} not found')

if __name__ == '__main__':
    dump_class("items.hpp", "ItemGeneric")
