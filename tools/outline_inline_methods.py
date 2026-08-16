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
  * Skip constructors/destructors/deleted/defaulted/template/variadic.
  * Target .cpp = <Header>.cpp by convention, with explicit overrides for the
    exceptions (interface/ui.hpp -> interface/ui_general.cpp,
    monster.hpp -> actmonster.cpp).

IMPORTANT: all offset math is done on RAW BYTES. libclang offsets are byte
offsets; decoding to str first would drift on any non-ASCII byte. We read
bytes, splice bytes, and only decode for the final write.

Usage:
  python3 tools/outline_inline_methods.py <header> [--dry-run]
  python3 tools/outline_inline_methods.py --all [--dry-run]
"""
import sys, os, glob, re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

import clang.cindex as ci
from clang.cindex import CursorKind

LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

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
                    CursorKind.DECL_REF_EXPR, CursorKind.MEMBER_REF_EXPR,
                    CursorKind.UNEXPOSED_EXPR, CursorKind.ARRAY_SUBSCRIPT_EXPR):
                    return True
    return False

def is_template_method(m):
    p = m.semantic_parent
    while p and p.kind not in (CursorKind.TRANSLATION_UNIT,):
        if p.kind in (CursorKind.FUNCTION_TEMPLATE, CursorKind.CLASS_TEMPLATE,
                      CursorKind.CLASS_TEMPLATE_PARTIAL_SPECIALIZATION):
            return True
        p = p.semantic_parent
    toks = [t.spelling for t in m.get_tokens()]
    if 'template' in toks or '...' in toks:
        return True
    if any('<' in a.type.spelling and '>' in a.type.spelling for a in m.get_arguments()):
        return True
    return False

def is_flattenable(m):
    if m.kind != CursorKind.CXX_METHOD:
        return False
    if m.is_deleted_method() or m.is_default_method():
        return False
    if is_trivial(m):
        return False
    if not has_body(m):
        return False
    if is_template_method(m):
        return False
    return True

def get_signature_and_body(m, data):
    """Return (sig_bytes, body_bytes, name_offset, start, end).

    sig_bytes = exact source bytes before the body brace.
    name_offset = byte offset of the method-name token within sig_bytes.
    """
    toks = list(m.get_tokens())
    if not toks:
        return None, None, 0, 0, 0
    first, last = toks[0], toks[-1]
    start = first.extent.start.offset
    end = last.extent.end.offset

    brace_idx = None
    name_idx = None
    for i, t in enumerate(toks):
        if t.spelling == '{' and brace_idx is None:
            brace_idx = i
        if t.spelling == m.spelling and name_idx is None and i > 0:
            name_idx = i
    if brace_idx is None or name_idx is None:
        return None, None, 0, start, end

    brace_off = toks[brace_idx].extent.start.offset
    name_off = toks[name_idx].extent.start.offset
    sig = data[start:brace_off].rstrip()
    body = data[brace_off:end]
    return sig, body, name_off - start, start, end

def class_qualifier(m):
    """Build the fully-qualified path for a method (Ns::Outer::Inner::Method).
    Walks the semantic-parent chain collecting nested class AND namespace names."""
    parts = []
    p = m.semantic_parent
    while p and p.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL,
                           CursorKind.NAMESPACE):
        parts.append(p.spelling)
        p = p.semantic_parent
    return '::'.join(reversed(parts))

def strip_default_args(decl_bytes):
    """Remove `= expr` default-arg initializers from a member definition.
    Default args live in the header declaration only; repeating them at the
    definition is a redefinition-of-default-argument error.
    """
    open_idx = decl_bytes.find(b'(')
    if open_idx == -1:
        return decl_bytes
    depth = 0
    j = open_idx
    while j < len(decl_bytes):
        c = decl_bytes[j:j+1]
        if c == b'(':
            depth += 1
        elif c == b')':
            depth -= 1
            if depth == 0:
                break
        j += 1
    if j >= len(decl_bytes):
        return decl_bytes
    close_idx = j
    params = decl_bytes[open_idx:close_idx+1]
    out = bytearray()
    d = 0
    k = 0
    n = len(params)
    while k < n:
        c = params[k:k+1]
        if c == b'(':
            d += 1
            out += c
        elif c == b')':
            d -= 1
            out += c
        elif c == b'=' and d == 1:
            # skip until top-level ',' or ')'
            k += 1
            dd = d
            while k < n:
                cc = params[k:k+1]
                if cc in (b'(', b'{', b'['):
                    dd += 1
                elif cc in (b')', b'}', b']'):
                    dd -= 1
                if cc == b',' and dd == 1:
                    out += cc
                    break
                if cc == b')' and dd == 0:
                    out += cc
                    break
                k += 1
            k += 1
            continue
        else:
            out += c
        k += 1
    return decl_bytes[:open_idx] + bytes(out) + decl_bytes[close_idx+1:]

def make_defn(cls, m, sig, name_offset, body):
    """Build the out-of-line definition from the exact signature bytes.

    Inserts the fully-qualified class path (Outer::Inner::) before the method
    name. Also qualifies the RETURN TYPE when it is an unqualified nested type,
    and strips `static` (illegal on file-scope member definitions).
    """
    qual = class_qualifier(m).encode()
    decl = sig[:name_offset] + qual + b'::' + sig[name_offset:]

    # Strip 'static', 'virtual', and 'inline' from the definition (the header
    # keeps them; file-scope member definitions must not repeat them, and
    # 'inline' would suppress the emitted symbol).
    decl = re.sub(rb'\bstatic\s+', b'', decl, count=1)
    decl = re.sub(rb'\bvirtual\s+', b'', decl, count=1)
    decl = re.sub(rb'\binline\s+', b'', decl, count=1)

    # Strip default argument values (= ...) from parameters: they live in the
    # header declaration only; repeating them at the definition is a
    # redefinition-of-default-argument error.
    decl = strip_default_args(decl)

    # Qualify an unqualified nested return type (e.g. `Iterator`, `VirtualMouse*`,
    # `time_point`). Detection: the canonical spelling differs from the source
    # spelling AND contains `::` (meaning the type's real home is qualified).
    src_ret = sig[:name_offset].strip()
    canon_ret = m.result_type.get_canonical().spelling.strip()
    if canon_ret and b'::' in canon_ret.encode() and canon_ret != src_ret.decode('utf-8', 'replace'):
        # Replace the source return-type prefix with the canonical one.
        # decl = sig-with-qualifier; the return type is the leading part before
        # the (now-qualified) method name. Rebuild cleanly:
        return_type_end = name_offset
        prefix = canon_ret.encode() + b' '
        decl = prefix + sig[return_type_end:name_offset].lstrip() + qual + b'::' + sig[name_offset:]
        decl = strip_default_args(decl)
        return decl + b' ' + body

    return decl + b' ' + body

def outline_header(header, dry_run=True):
    tu = parse(header)
    hdr_path = os.path.join(SRC, header)
    rel = header.replace('\\', '/')

    # RAW BYTES for offset math (libclang offsets are byte offsets).
    data = open(hdr_path, 'rb').read()

    moves = []  # (class, method, sig, body, name_off, start, end)
    seen_classes = set()  # (spelling, def_line) dedup: typedef struct creates two cursors
    for n in tu.cursor.walk_preorder():
        if n.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL) and n.is_definition():
            loc = n.location.file
            if not loc:
                continue
            if norm(loc.name) != norm(hdr_path):
                continue
            ckey = (n.spelling, n.location.line)
            if ckey in seen_classes:
                continue
            seen_classes.add(ckey)
            for m in n.get_children():
                if not is_flattenable(m):
                    continue
                sig, body, name_off, start, end = get_signature_and_body(m, data)
                if sig is None:
                    continue
                moves.append((n.spelling, m, sig, body, name_off, start, end))

    if not moves:
        return []

    target = CPP_TARGET.get(rel, os.path.splitext(rel)[0] + '.cpp')
    target_path = os.path.join(SRC, target)

    if dry_run:
        print(f'=== {header} ({len(moves)} methods) -> {target} ===')
        for cls, m, sig, body, name_off, start, end in moves:
            defn = make_defn(cls, m, sig, name_off, body)
            print(f'  {cls}::{m.spelling}')
            print(f'    DECL:  {defn[:90].decode("utf-8", "replace")}')
        return moves

    # Remove bodies from header (END to START). Also strip `inline` from the
    # leftover declaration so the out-of-lined definition emits a symbol
    # (a header `inline` would suppress it).
    new_data = data
    for cls, m, sig, body, name_off, start, end in sorted(moves, key=lambda x: -x[5]):
        decl_sig = re.sub(rb'\binline\s+', b'', sig, count=1)
        new_data = new_data[:start] + decl_sig + b';' + new_data[end:]

    open(hdr_path, 'wb').write(new_data)

    # Append definitions to target cpp.
    if not os.path.exists(target_path):
        cpp_bytes = (
            f'// Auto-generated out-of-line definitions for {header}\n'
            f'#include "{os.path.basename(header)}"\n\n'
        ).encode()
    else:
        cpp_bytes = open(target_path, 'rb').read()
        if not cpp_bytes.endswith(b'\n'):
            cpp_bytes += b'\n'

    added = []
    for cls, m, sig, body, name_off, start, end in moves:
        defn = make_defn(cls, m, sig, name_off, body)
        if defn not in cpp_bytes:
            cpp_bytes += b'\n' + defn + b'\n'
            added.append(f'{cls}::{m.spelling}')

    open(target_path, 'wb').write(cpp_bytes)
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
