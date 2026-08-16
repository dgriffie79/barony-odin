#!/usr/bin/env python3
"""
outline_inline_methods.py - move inline (in-class) method bodies out of
headers into the owning .cpp, leaving a clean declaration.

This is the mechanical prep for flattening: a method body must live in a .cpp
before tools/flatten_methods.py can place an extern "C" forwarder next to it
(a header forwarder would violate the ODR).

Rules:
  * Only non-trivial inline methods (skip single `return field;` accessors -
    they emit no symbol and Odin reads fields directly).
  * Skip constructors/destructors/deleted/defaulted (not flattenable).
  * Target .cpp = <Header>.cpp by convention, with explicit overrides for the
    two exceptions (interface/ui.hpp -> interface/ui_general.cpp,
    monster.hpp -> actmonster.cpp) and classes that are all-inline (no owning
    .cpp -> collected into a generated <Header>_inline.cpp).

Usage:
  python3 tools/outline_inline_methods.py <header> [--dry-run]
  python3 tools/outline_inline_methods.py --all [--dry-run]
"""
import sys, os, re, glob
from collections import defaultdict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

import clang.cindex as ci
from clang.cindex import CursorKind, AccessSpecifier

LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

# Header -> owning .cpp for the out-of-lined bodies.
# Default: <Header>.cpp. Overrides for exceptions.
CPP_TARGET = {
    'interface/ui.hpp': 'interface/ui_general.cpp',
    'monster.hpp': 'actmonster.cpp',
}

def norm(p):
    return os.path.normpath(p).replace('\\', '/').lower()

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

def has_body(m):
    for c in m.get_children():
        if c.kind == CursorKind.COMPOUND_STMT:
            return True
    return False

def is_trivial(m):
    if not has_body(m):
        return False
    for c in m.get_children():
        if c.kind == CursorKind.COMPOUND_STMT:
            stmts = list(c.get_children())
            if len(stmts) == 1 and stmts[0].kind == CursorKind.RETURN_STMT:
                rk = list(stmts[0].get_children())
                if len(rk) == 1 and rk[0].kind in (
                    CursorKind.DECL_REF_EXPR,
                    CursorKind.MEMBER_REF_EXPR,
                    CursorKind.UNEXPOSED_EXPR,
                    CursorKind.ARRAY_SUBSCRIPT_EXPR,
                ):
                    return True
    return False

def is_flattenable(m):
    """True if this method should be out-of-lined for later flattening."""
    if m.kind != CursorKind.CXX_METHOD:
        return False  # not a constructor/destructor
    if m.is_deleted_method() or m.is_default_method():
        return False
    if is_trivial(m):
        return False
    return has_body(m)

def get_signature_and_body(m, file_text):
    """Return (signature, body, start_offset, end_offset) for an in-class method.

    Uses the method's TOKENS to find the true boundaries (m.extent is unreliable
    for in-class definitions - it starts mid-signature).
    """
    toks = list(m.get_tokens())
    if not toks:
        return None, None, 0, 0
    first = toks[0]
    last = toks[-1]
    start = first.extent.start.offset
    end = last.extent.end.offset

    # Find the body-opening '{' token (first '{' in the token stream).
    brace_idx = None
    for i, t in enumerate(toks):
        if t.spelling == '{':
            brace_idx = i
            break
    if brace_idx is None:
        return None, None, start, end

    sig = file_text[start:toks[brace_idx].extent.start.offset].rstrip()
    body = file_text[toks[brace_idx].extent.start.offset:end]
    return sig, body, start, end

def method_decl(m, cls):
    """Build the out-of-line definition signature `Ret Class::name(params) [const]`."""
    ret = m.result_type.spelling
    name = m.spelling
    params = ', '.join(
        f'{p.type.spelling} {p.spelling or "a"+str(i)}'
        for i, p in enumerate(m.get_arguments())
    )
    const = ' const' if m.is_const_method() else ''
    return f'{ret} {cls}::{name}({params}){const}'

def outline_header(header, dry_run=True):
    """Process a single header. Returns list of (class, method, target_cpp, sig, body)."""
    tu = parse(header)
    hdr_path = os.path.join(SRC, header)
    # CRITICAL: newline='' preserves CRLF so Python offsets match libclang's
    # byte offsets (libclang counts the raw CRLF bytes).
    text = open(hdr_path, encoding='utf-8', newline='').read()
    rel = header.replace('\\', '/')

    moves = []  # (class, method, sig, body, start, end)
    for n in tu.cursor.walk_preorder():
        if n.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL) and n.is_definition():
            loc = n.location.file
            if not loc:
                continue
            if norm(loc.name) != norm(hdr_path):
                continue
            for m in n.get_children():
                if not is_flattenable(m):
                    continue
                sig, body, start, end = get_signature_and_body(m, text)
                if sig is None:
                    continue
                moves.append((n.spelling, m, sig, body, start, end))

    if not moves:
        return []

    target = CPP_TARGET.get(rel, os.path.splitext(rel)[0] + '.cpp')
    target_path = os.path.join(SRC, target)

    if dry_run:
        print(f'=== {header} ({len(moves)} methods) -> {target} ===')
        for cls, m, sig, body, start, end in moves:
            print(f'  {cls}::{m.spelling}')
            print(f'    DECL:  {method_decl(m, cls)}')
        return moves

    # Apply: remove bodies from header, append definitions to cpp.
    # Remove from END to START so earlier offsets stay valid.
    new_text = text
    for cls, m, sig, body, start, end in sorted(moves, key=lambda x: -x[4]):
        new_text = new_text[:start] + sig + ';' + new_text[end:]

    open(hdr_path, 'w', encoding='utf-8', newline='').write(new_text)

    # Append definitions to target cpp (preserve CRLF: read/write newline='')
    if not os.path.exists(target_path):
        cpp_text = (
            f'// Auto-generated out-of-line definitions for {header}\n'
            f'#include "{os.path.basename(header)}"\n\n'
        )
    else:
        cpp_text = open(target_path, encoding='utf-8', newline='').read()
        if not cpp_text.endswith('\n'):
            cpp_text += '\n'

    added = []
    for cls, m, sig, body, start, end in moves:
        decl = method_decl(m, cls)
        defn = f'{decl} {body}\n'
        if defn not in cpp_text:
            cpp_text += '\n' + defn
            added.append(f'{cls}::{m.spelling}')

    open(target_path, 'w', encoding='utf-8', newline='').write(cpp_text)
    print(f'{header}: moved {len(moves)} methods to {target}: {", ".join(added)}')
    return moves

if __name__ == '__main__':
    dry = '--dry-run' in sys.argv
    args = [a for a in sys.argv[1:] if a != '--dry-run']
    if not args or args[0] == '--all':
        headers = [
            os.path.relpath(h, SRC).replace('\\', '/')
            for h in glob.glob(SRC + '/*.hpp') + glob.glob(SRC + '/*/*.hpp')
        ]
        for h in headers:
            outline_header(h, dry_run=dry)
    else:
        outline_header(args[0], dry_run=dry)
