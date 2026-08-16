#!/usr/bin/env python3
"""
flatten_methods.py - generate the C++ prep step that flattens class methods
into extern "C" free functions (Class_method naming) so Odin can call any
method at any time, regardless of port order.

Rules (locked in):
  * NO barony_ prefix - the flat name is ClassName_methodName, single owner.
  * Same-name @(export) proc "c" on the Odin side; C++ links to it.
  * Access-specifier relabel (private: -> public:) is LAYOUT-SAFE (verified:
    clang record layouts identical). Never drop/reorder members.
  * Trivial inline accessors (single `return field;`) emit no symbol and are
    SKIPPED - Odin reads fields directly.
  * Deleted/defaulted ctors, dtors, operator=, and conversion operators are
    SKIPPED (no user code to forward).
  * Static methods forward WITHOUT a `self` pointer; const methods take a
    `const Class* self`.
  * OPERATORS get readable flat names: operator== -> Class_eq, operator()
    -> Class_call, operator[] -> Class_index, operator= -> Class_assign, etc.
  * OVERLOADS are auto-disambiguated: first keeps the base name, then _2,
    _3, ... in declaration order (deterministic).

This script only ANALYZES and PRINTS generated snippets + a report. It does
NOT edit files (review first). Usage:

    python3 tools/flatten_methods.py prng.hpp BaronyRNG
    python3 tools/flatten_methods.py input.hpp Input

Output:
  1. REPORT  - methods to flatten, skip (accessors/ctors), and why.
  2. C++      - extern "C" forwarder bodies (flavor A: method still in C++) to
                paste next to each method definition.
  3. ODIN     - the consolidated `foreign _barony { ... }` block for methods
                that remain C++-owned, so Odin can call them.
"""
import sys, os
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

import clang.cindex as ci
from clang.cindex import CursorKind, AccessSpecifier

LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

# ---------------------------------------------------------------------------
# Operator -> readable flat-name suffix
# ---------------------------------------------------------------------------
OPERATOR_SUFFIX = {
    'operator==': 'eq',
    'operator!=': 'ne',
    'operator<':  'lt',
    'operator<=': 'le',
    'operator>':  'gt',
    'operator>=': 'ge',
    'operator+':  'add',
    'operator-':  'sub',
    'operator*':  'mul',
    'operator/':  'div',
    'operator%':  'mod',
    'operator+=': 'add_assign',
    'operator-=': 'sub_assign',
    'operator*=': 'mul_assign',
    'operator/=': 'div_assign',
    'operator%=': 'mod_assign',
    'operator=':  'assign',
    'operator[]': 'index',
    'operator()': 'call',
    'operator<<': 'shl',
    'operator>>': 'shr',
    'operator!':  'not',
    'operator&&': 'and',
    'operator||': 'or',
}

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

def has_body(method):
    for c in method.get_children():
        if c.kind == CursorKind.COMPOUND_STMT:
            return True
    return False

def is_trivial_accessor(method):
    """A single `return <field access>;` body - emit-no-symbol accessor.

    Recognizes the cursor shapes libclang reports for field accessors:
      DECL_REF_EXPR        (return someIdent;)
      MEMBER_REF_EXPR      (return member;)
      UNEXPOSED_EXPR       (return disabled; - resolved lazily)
      ARRAY_SUBSCRIPT_EXPR (return skill[28];)
      PAREN_EXPR           (return (x); - unwrap to inner)
      CSTYLE_CAST_EXPR     (return (T)x; - unwrap to inner)
    """
    if not has_body(method):
        return False
    for c in method.get_children():
        if c.kind == CursorKind.COMPOUND_STMT:
            stmts = list(c.get_children())
            if len(stmts) == 1 and stmts[0].kind == CursorKind.RETURN_STMT:
                ret_kids = list(stmts[0].get_children())
                if len(ret_kids) == 1:
                    return is_field_access_expr(ret_kids[0])
    return False

def is_field_access_expr(cursor):
    """True if cursor is a field access, array subscript, or a transparent
    wrapper (paren/cast) around one. Recursive so `return (T)field[3];` works."""
    if cursor.kind in (
        CursorKind.DECL_REF_EXPR,
        CursorKind.MEMBER_REF_EXPR,
        CursorKind.UNEXPOSED_EXPR,
        CursorKind.ARRAY_SUBSCRIPT_EXPR,
    ):
        return True
    if cursor.kind in (CursorKind.PAREN_EXPR, CursorKind.CSTYLE_CAST_EXPR):
        inner = list(cursor.get_children())
        if len(inner) == 1:
            return is_field_access_expr(inner[0])
    return False

def base_flat_name(cls, method):
    """Class + method/operator base name, BEFORE overload disambiguation."""
    sp = method.spelling
    if sp in OPERATOR_SUFFIX:
        return f"{cls}_{OPERATOR_SUFFIX[sp]}"
    # unknown operator -> sanitize (strip 'operator', replace non-alnum with _)
    if sp.startswith('operator'):
        body = sp[len('operator'):]
        body = ''.join(c if c.isalnum() else '_' for c in body)
        return f"{cls}_op_{body}"
    return f"{cls}_{sp}"

def assign_flat_names(cls, methods):
    """Return {method_cursor: flat_name} with deterministic overload suffixes.
    Groups by base name; first keeps base, then _2, _3 in declaration order.
    """
    groups = {}  # base -> [methods in order]
    for m in methods:
        groups.setdefault(base_flat_name(cls, m), []).append(m)
    result = {}
    for base, ms in groups.items():
        if len(ms) == 1:
            result[ms[0]] = base
        else:
            for i, m in enumerate(ms):
                result[m] = base if i == 0 else f"{base}_{i+1}"
    return result

def param_list(method):
    cparams = []
    for p in method.get_arguments():
        t = p.type.spelling
        name = p.spelling or f"a{len(cparams)}"
        cparams.append((t, name))
    return cparams

def method_needs_self(method):
    return not method.is_static_method()

def self_c_type(method):
    """`const Class*` for const methods, else `Class*`."""
    return f"const {method.lexical_parent.spelling}*" if method.is_const_method() else f"{method.lexical_parent.spelling}*"

def emit_forwarder(cls, method, fn):
    """Flavor A: method still owned by C++. extern "C" flat fn forwards to it."""
    ret = method.result_type.spelling
    params = param_list(method)
    decl = f"{ret} {fn}("
    if method_needs_self(method):
        decl += f"{self_c_type(method)} self"
        if params:
            decl += ", "
    decl += ", ".join(f"{t} {n}" for t, n in params)
    decl += ")"

    call_args = [n for _, n in params]
    if method.is_static_method():
        call_expr = f"{cls}::{method.spelling}({', '.join(call_args)})"
    else:
        call_expr = f"self->{method.spelling}({', '.join(call_args)})"
    return f"extern \"C\" {decl} {{ return {call_expr}; }}"

def odin_foreign_decl(cls, method, fn):
    """Odin `foreign _barony { ... }` line for a C++-owned method's flat fn."""
    ret = method.result_type.spelling
    params = param_list(method)
    odin_ret = c_to_odin(ret)
    odin_params = []
    if method_needs_self(method):
        odin_params.append(f"self: ^{cls}")
    for t, n in params:
        odin_params.append(f"{n}: {c_to_odin(t)}")
    ret_part = f" -> {odin_ret}" if odin_ret else ""
    return f"{fn} :: proc \"c\" ({', '.join(odin_params)}){ret_part} ---"

C_TO_ODIN = {
    'void': '', 'bool': 'bool', 'int': 'i32', 'unsigned int': 'u32',
    'float': 'f32', 'double': 'f64', 'real_t': 'f64', 'size_t': 'uint',
    'uint8_t': 'u8', 'uint16_t': 'u16', 'uint32_t': 'u32', 'uint64_t': 'u64',
    'int8_t': 'i8', 'int16_t': 'i16', 'int32_t': 'i32', 'int64_t': 'i64',
    'Uint32': 'u32', 'Sint32': 'i32', 'char': 'u8', 'const char*': 'cstring',
    'char*': 'cstring',
}

def c_to_odin(t):
    t = t.strip().replace(' *', '*')
    if t == 'void':
        return ''
    if t in C_TO_ODIN:
        return C_TO_ODIN[t]
    if t.endswith('*'):
        base = t[:-1].strip().replace('const ', '').strip()
        if base == 'void' or base == '':
            return 'rawptr'
        if base in C_TO_ODIN:
            return f'^{C_TO_ODIN[base]}'
        return 'rawptr'
    if t.startswith('const ') and t.endswith('*'):
        return 'cstring'
    return 'rawptr'

def analyze(header, clsname):
    tu = parse(header)
    target = None
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.CLASS_DECL and node.spelling == clsname and node.is_definition():
            target = node
            break
    if target is None:
        print(f"class {clsname} not found in {header}")
        return

    methods = [c for c in target.get_children() if c.kind == CursorKind.CXX_METHOD]
    skip_accessors = []
    skip_special = []
    flatten = []
    private_to_relabel = set()

    for m in methods:
        if m.is_deleted_method():
            skip_special.append(m.spelling + ' (deleted)')
            continue
        if m.is_default_method():
            skip_special.append(m.spelling + ' (= default)')
            continue
        if is_trivial_accessor(m):
            skip_accessors.append(m.spelling)
            continue
        if m.access_specifier == AccessSpecifier.PRIVATE:
            private_to_relabel.add(m.spelling)
        flatten.append(m)

    names = assign_flat_names(clsname, flatten)

    print(f"=== {clsname} in {header} ===\n")
    print(f"methods to FLATTEN ({len(flatten)}):")
    for m in flatten:
        static = 'static ' if m.is_static_method() else ''
        const = ' const' if m.is_const_method() else ''
        print(f"  {static}{m.result_type.spelling} {m.spelling}(...){const}  ->  {names[m]}")
    print(f"\nskipped accessors ({len(skip_accessors)}): {', '.join(skip_accessors) or '(none)'}")
    if skip_special:
        print(f"skipped special ({len(skip_special)}): {', '.join(skip_special)}")
    if private_to_relabel:
        print(f"\nprivate methods to make public: {', '.join(sorted(private_to_relabel))}")

    print("\n--- C++ forwarders (paste next to each definition) ---\n")
    for m in flatten:
        print(emit_forwarder(clsname, m, names[m]))
        print()

    print("--- ODIN foreign block (for C++-owned methods Odin must call) ---\n")
    print("foreign _barony {")
    for m in flatten:
        print(f"    {odin_foreign_decl(clsname, m, names[m])}")
    print("}")
    return flatten, names


# ---------------------------------------------------------------------------
# --apply mode: actually edit the .cpp (insert forwarders) and .hpp (relabel)
# ---------------------------------------------------------------------------

def find_method_end(text, brace_pos):
    """Given a position at an opening brace, return the index AFTER its match."""
    depth = 0
    i = brace_pos
    while i < len(text):
        if text[i] == '{':
            depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return -1

def apply_mode(header, clsname, cpp_path, odin_bindings_path):
    """Insert forwarders into cpp_path, relabel private methods in header,
    and write the Odin foreign block to odin_bindings_path."""
    tu = parse(header)
    target = None
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.CLASS_DECL and node.spelling == clsname and node.is_definition():
            target = node
            break
    if target is None:
        print(f"class {clsname} not found in {header}")
        return

    methods = [c for c in target.get_children() if c.kind == CursorKind.CXX_METHOD]
    flatten = []
    private_to_relabel = []
    for m in methods:
        if m.is_deleted_method() or m.is_default_method() or is_trivial_accessor(m):
            continue
        if m.access_specifier == AccessSpecifier.PRIVATE:
            private_to_relabel.append(m.spelling)
        flatten.append(m)
    names = assign_flat_names(clsname, flatten)

    # 1. Insert forwarders into the .cpp
    cpp_text = open(cpp_path, encoding='utf-8').read()
    insertions = []  # (pos, text)
    for m in flatten:
        fwd = emit_forwarder(clsname, m, names[m])
        # Find the method DEFINITION in the .cpp: ClassName::methodName(
        # Skip declarations (no body). Match the first definition occurrence.
        sig = f"{clsname}::{m.spelling}"
        idx = 0
        while True:
            idx = cpp_text.find(sig, idx)
            if idx == -1:
                print(f"  WARN: definition not found for {sig} in {cpp_path}")
                break
            # find '{' within the next 300 chars (definition), else it's a decl
            window = cpp_text[idx:idx+300]
            b = window.find('{')
            if b != -1:
                end = find_method_end(cpp_text, idx + b)
                if end != -1:
                    insertions.append((end, f"\n\n{fwd}\n"))
                break
            idx += len(sig)
    insertions.sort(key=lambda x: -x[0])
    for pos, text in insertions:
        cpp_text = cpp_text[:pos] + text + cpp_text[pos:]
    open(cpp_path, 'w', encoding='utf-8').write(cpp_text)
    print(f"  inserted {len(insertions)} forwarders into {cpp_path}")

    # 2. Relabel private methods -> public in the header.
    hdr_path = os.path.join(SRC, header)
    hdr = open(hdr_path, encoding='utf-8').read()
    if private_to_relabel:
        for name in private_to_relabel:
            # Relabel: change `private:` before this method's declaration.
            # Robust approach: insert `public:` immediately before the method decl.
            # Match the method declaration line (name followed by '(').
            import re as _re
            pat = _re.compile(r'(\n[ \t]*)((?:static[ \t]+)?[^\n;]*\b' + _re.escape(name) + r'\s*\()')
            m2 = pat.search(hdr)
            if m2:
                # insert `public:` just before, on its own line at same indent
                indent = m2.group(1)
                hdr = hdr[:m2.start()] + f"\n{indent}public:" + hdr[m2.start():]
        open(hdr_path, 'w', encoding='utf-8').write(hdr)
        print(f"  relabeled {len(private_to_relabel)} private method(s) -> public in {header}: {', '.join(private_to_relabel)}")
    else:
        print(f"  no private methods to relabel in {header}")

    # 3. Write the Odin foreign block.
    decls = "\n".join(f"    {odin_foreign_decl(clsname, m, names[m])}" for m in flatten)
    block = f"// Auto-generated: flatten_methods.py --apply {header} {clsname}\nforeign _barony {{\n{decls}\n}}\n"
    open(odin_bindings_path, 'w', encoding='utf-8').write(block)
    print(f"  wrote Odin foreign block to {odin_bindings_path}")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    if '--apply' in sys.argv:
        # usage: flatten_methods.py <header> <class> --apply <cpp_path> <odin_bindings_path>
        a = sys.argv.index('--apply')
        cpp_path = sys.argv[a+1]
        odin_path = sys.argv[a+2]
        apply_mode(sys.argv[1], sys.argv[2], cpp_path, odin_path)
    else:
        analyze(sys.argv[1], sys.argv[2])
