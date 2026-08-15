#!/usr/bin/env python3
"""Dump struct/class field layouts from the real headers via libclang."""
import sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
sys.path.insert(0, os.path.join(ROOT, 'builddir'))

import clang.cindex as ci
from clang.cindex import CursorKind

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
                continue
            print(f'=== {clsname} sizeof={node.type.get_size()} ===')
            for f in kids:
                if f.kind == CursorKind.FIELD_DECL:
                    sz = f.type.get_size()
                    print(f'  {f.spelling}: {f.type.spelling} (size {sz})')
            return
    print(f'class {clsname} not found')

if __name__ == '__main__':
    for arg in sys.argv[1:]:
        h, c = arg.split(':')
        dump_class(h, c)
