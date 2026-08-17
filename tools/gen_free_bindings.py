#!/usr/bin/env python3
"""
gen_free_bindings.py - add `extern "C"` to free-function declarations in
headers, so Odin's `foreign _barony` can see them (the free-function analog
of flatten_methods.py for class methods).

Free functions cross the C++<->Odin boundary unmangled. A header decl like
    real_t entityDist(Entity* my, Entity* your);
becomes
    extern "C" real_t entityDist(Entity* my, Entity* your);

Rules:
  * Only FREE functions (FUNCTION_DECL at namespace scope, not inside a class
    body) declared in the target header itself (not included system headers).
  * static / inline / constexpr free functions are skipped (no exported
    symbol / Odin can't call them).
  * Already-extern decls are idempotent (skipped).
  * Overloads: C-linkage can't have two same-named symbols, but the survey
    showed zero overloads among the real free functions, so plain `extern "C"`
    prefixing is safe. If an overload ever appears, it needs a flat rename
    (the Class_method _2/_3 scheme) - detect and bail.

Modes:
  analyze:  print report (no edits)
  apply:    rewrite headers in place

Usage:
  python3 tools/gen_free_bindings.py            # analyze all
  python3 tools/gen_free_bindings.py --apply    # rewrite all
"""
import sys, os, glob, re
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

import clang.cindex as ci
from clang.cindex import CursorKind
LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

def parse(header):
    if os.path.isabs(header):
        full = header
    else:
        full = os.path.join(SRC, header)
    args = ['-x','c++','-std=c++17','-fms-compatibility-version=19.51',
            '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
            '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION',
            '-I'+SRC,'-I'+os.path.join(SRC,'magic'),'-I'+os.path.join(SRC,'interface'),
            '-I'+os.path.join(SRC,'ui'),'-I'+os.path.join(SRC,'engine'),'-I'+os.path.join(SRC,'engine','audio'),
            '-I'+os.path.join(ROOT,'odin','containers'),'-I'+os.path.join(ROOT,'builddir'),
            '-IC:/dev/vcpkg/installed/x64-windows/include','-IC:/dev/vcpkg/installed/x64-windows/include/SDL2']
    idx = ci.Index.create()
    return idx.parse(full, args=args,
                     options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

def normp(p):
    return os.path.normpath(p).replace(os.sep, '/')

def is_inside_class(cursor):
    c = cursor.semantic_parent
    while c is not None and c.kind != CursorKind.TRANSLATION_UNIT:
        if c.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL):
            return True
        c = c.semantic_parent
    return False

def free_functions(tu, header):
    out = []
    header = normp(header)
    for c in tu.cursor.walk_preorder():
        if c.kind != CursorKind.FUNCTION_DECL:
            continue
        loc = c.location
        if not loc.file or normp(loc.file.name) != header:
            continue  # only decls in THIS header, not included system headers
        if is_inside_class(c):
            continue
        if c.type.is_function_variadic():
            continue  # variadic free fns handled separately (va_list split)
        if any(k.kind == CursorKind.TEMPLATE_TYPE_PARAMETER for k in c.get_children()):
            continue
        out.append(c)
    return out

def decl_start_line(lines, f):
    """Return 0-based line index where the decl begins (handles multi-line)."""
    ln = f.location.line - 1
    while ln > 0:
        prev = lines[ln - 1].strip()
        # strip trailing // comment before terminator check
        prev_code = prev.split('//')[0].rstrip() if '//' in prev else prev
        # stop at blank lines, comments, and statement terminators
        if not prev or prev.startswith(('//', '/*', '*', '#', 'public:', 'private:', 'protected:')):
            break
        if prev_code.endswith((';', '{', '}')) or re.search(r'[;{}]$', prev_code):
            break
        ln -= 1
    return ln

def rewrite_header(path, fns):
    """Prepend 'extern "C" ' to each free-fn decl in fns. Returns count."""
    with open(path, encoding='utf-8', newline='') as fh:
        text = fh.read()
    lines = text.split('\n')
    insert = []
    # group by name to detect overloads -> skip whole group (C linkage can't overload)
    from collections import Counter
    name_counts = Counter(f.spelling for f in fns)
    overloaded = {n for n, c in name_counts.items() if c > 1}
    for f in fns:
        if f.spelling in overloaded:
            continue  # overloads: C linkage illegal; skip (hand-handle)
        ln = decl_start_line(lines, f)
        stripped = lines[ln].strip()
        if re.match(r'^(static|inline|constexpr)\b', stripped):
            continue
        if 'friend' in stripped.split('(')[0].split(';')[0].split('{')[0][:40]:
            continue  # friend operator/fn inside class body
        if re.search(r'^extern\s+"C"', stripped):
            continue  # already extern
        # skip decls already inside an extern "C" { ... } block
        in_block = False
        depth = 0
        for li in range(ln + 1):
            s = lines[li]
            if re.search(r'extern\s+"C"\s*\{', s):
                in_block = True
            if in_block:
                depth += s.count('{') - s.count('}')
                if depth <= 0:
                    in_block = False
        if in_block:
            continue
        insert.append(ln)
    insert = sorted(set(insert), reverse=True)
    for ln in insert:
        lines[ln] = 'extern "C" ' + lines[ln]
    with open(path, 'w', encoding='utf-8', newline='') as fh:
        fh.write('\n'.join(lines))
    return len(insert)

def main():
    apply = '--apply' in sys.argv
    only = [os.path.abspath(a) for a in sys.argv[1:] if not a.startswith('-')]
    headers = only or sorted(glob.glob(os.path.join(SRC, '**', '*.hpp'), recursive=True))
    total = 0
    for h in headers:
        h_abs = os.path.abspath(h).replace(os.sep, '/')
        try:
            tu = parse(h)
        except Exception as e:
            print(f"ERR {h}: {e}")
            continue
        fns = free_functions(tu, h_abs)
        if not fns:
            continue
        from collections import Counter
        ncount = Counter(f.spelling for f in fns)
        overloaded = {n for n, c in ncount.items() if c > 1}
        if overloaded:
            print(f"{h}: OVERLOADS SKIPPED: {sorted(overloaded)}")
        if apply:
            n = rewrite_header(h, fns)
            print(f"{h}: rewrote {n}")
            total += n
        else:
            print(f"{h}: {len(fns)} free fns")
            for f in fns[:3]:
                print(f"    {f.type.spelling} {f.spelling}  [{f.location.line}]")
            total += len(fns)
    print(f"\nTOTAL: {total}")

if __name__ == '__main__':
    main()
