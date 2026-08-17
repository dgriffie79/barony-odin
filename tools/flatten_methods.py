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

Modes:
  analyze:  print report + generated snippets (no edits)
  apply:    insert forwarders after method definitions in their .cpps,
            relabel private->public in the header, accumulate into a
            consolidated Odin foreign block.

Usage:
  python3 tools/flatten_methods.py prng.hpp BaronyRNG

"""
import sys, os, glob, re
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

import clang.cindex as ci
from clang.cindex import CursorKind, AccessSpecifier

LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

OPERATOR_SUFFIX = {
    'operator==': 'eq', 'operator!=': 'ne', 'operator<': 'lt', 'operator<=': 'le',
    'operator>': 'gt', 'operator>=': 'ge', 'operator+': 'add', 'operator-': 'sub',
    'operator*': 'mul', 'operator/': 'div', 'operator%': 'mod',
    'operator+=': 'add_assign', 'operator-=': 'sub_assign', 'operator*=': 'mul_assign',
    'operator/=': 'div_assign', 'operator%=': 'mod_assign', 'operator=': 'assign',
    'operator[]': 'index', 'operator()': 'call', 'operator<<': 'shl', 'operator>>': 'shr',
    'operator!': 'not', 'operator&&': 'and', 'operator||': 'or',
}

# Classes already ported to Odin (methods are @(export) procs, NOT C++-owned).
# The flatten prep must skip them to avoid duplicate symbols.
PORTED_CLASSES = {'BaronyRNG'}

# (header, class) pairs whose private-member relabel is handled by
# fix_flat_forwarders.py instead (the generic regex misfires on overloads).
SKIP_RELABEL = {('json.hpp', 'FileInterface')}

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

def is_field_access_expr(cursor):
    if cursor.kind in (CursorKind.DECL_REF_EXPR, CursorKind.MEMBER_REF_EXPR,
                       CursorKind.UNEXPOSED_EXPR, CursorKind.ARRAY_SUBSCRIPT_EXPR):
        return True
    if cursor.kind in (CursorKind.PAREN_EXPR, CursorKind.CSTYLE_CAST_EXPR):
        inner = list(cursor.get_children())
        if len(inner) == 1:
            return is_field_access_expr(inner[0])
    return False

def is_trivial_accessor(method):
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

def base_flat_name(cls, method):
    sp = method.spelling
    if sp in OPERATOR_SUFFIX:
        return f"{cls}_{OPERATOR_SUFFIX[sp]}"
    if sp.startswith('operator'):
        body = sp[len('operator'):]
        body = ''.join(c if c.isalnum() else '_' for c in body)
        return f"{cls}_op_{body}"
    return f"{cls}_{sp}"

def assign_flat_names(cls, methods):
    groups = {}
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
        # If the type is a nested type of the class, canonical gives full path.
        canon = p.type.get_canonical().spelling
        if '::' in canon and 'std::' not in canon:
            t = canon
        name = p.spelling or f"a{len(cparams)}"
        cparams.append((t, name))
    return cparams

def result_c_type(method):
    t = method.result_type.spelling
    canon = method.result_type.get_canonical().spelling
    if '::' in canon and 'std::' not in canon:
        return canon
    return t

def method_needs_self(method):
    return not method.is_static_method()

def class_qualifier(method):
    """Full qualified path of the method's class (Outer::Inner)."""
    parts = []
    p = method.semantic_parent
    while p and p.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL,
                           CursorKind.NAMESPACE):
        parts.append(p.spelling)
        p = p.semantic_parent
    return '::'.join(reversed(parts))

def self_c_type(method):
    qual = class_qualifier(method)
    return (f"const {qual}*" if method.is_const_method() else f"{qual}*")

def qualify_type(method, t):
    """Qualify a C++ type spelling if it's a nested type of the method's
    class chain. Returns the source spelling when it's already global."""
    if '::' in t:
        return t  # already qualified
    # check each param/return type's declaration home
    return t

def emit_forwarder(cls, method, fn):
    """Flavor A: method still owned by C++. extern "C" flat fn forwards to it."""
    ret = result_c_type(method)
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
        qual = class_qualifier(method)
        call_expr = f"{qual}::{method.spelling}({', '.join(call_args)})"
    else:
        call_expr = f"self->{method.spelling}({', '.join(call_args)})"
    return f"extern \"C\" {decl} {{ return {call_expr}; }}"

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

# ---------------------------------------------------------------------------
# Analysis (print-only)
# ---------------------------------------------------------------------------

def get_class(tu, clsname):
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.CLASS_DECL and node.spelling == clsname and node.is_definition():
            return node
    return None

def analyze(header, clsname):
    tu = parse(header)
    target = get_class(tu, clsname)
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
    print("--- ODIN foreign block ---\n")
    print("foreign _barony {")
    for m in flatten:
        print(f"    {odin_foreign_decl(clsname, m, names[m])}")
    print("}")
    return flatten, names

# ---------------------------------------------------------------------------

# ---------------------------------------------------------------------------

def find_method_end_bytes(data, brace_pos):
    """Return byte index AFTER the matching close brace, string/comment-aware.

    Braces inside string literals, char literals, and // and /* */ comments
    are skipped so they don't unbalance the counter.
    """
    depth = 0
    i = brace_pos
    n = len(data)
    state = 'code'  # code | line_comment | block_comment | str | char
    while i < n:
        c = data[i:i+1]
        nxt = data[i+1:i+2] if i+1 < n else b''
        if state == 'code':
            if c == b'{':
                depth += 1
            elif c == b'}':
                depth -= 1
                if depth == 0:
                    return i + 1
            elif c == b'/' and nxt == b'/':
                state = 'line_comment'; i += 2; continue
            elif c == b'/' and nxt == b'*':
                state = 'block_comment'; i += 2; continue
            elif c == b'"':
                state = 'str'
            elif c == b"'":
                state = 'char'
        elif state == 'line_comment':
            if c == b'\n':
                state = 'code'
        elif state == 'block_comment':
            if c == b'*' and nxt == b'/':
                state = 'code'; i += 2; continue
        elif state == 'str':
            if c == b'\\':
                i += 2; continue
            if c == b'"':
                state = 'code'
        elif state == 'char':
            if c == b'\\':
                i += 2; continue
            if c == b"'":
                state = 'code'
        i += 1
    return -1

def find_def_in_cpps(clsname, method_name, cpp_paths):
    """Find (cpp_path, byte_offset_of_definition_start, byte_offset_of_body_end)
    for the FIRST out-of-line definition of Class::method across cpp_paths.

    A definition starts at line-start (after optional whitespace):
    `\n[ \t]*Ret Class::method(`. Call sites (inside expressions) never match
    because they are not at line start.
    """
    for cp in cpp_paths:
        try:
            data = open(cp, 'rb').read()
        except FileNotFoundError:
            continue
        # Match line-start definition: `\n[ \t]*[Ret ][Outer::]Class::method(...) [const] {`
        # The ')' must be followed (after optional const/noexcept/ref) by '{' -
        # NOT by ';' (a call site) or ',' (call arg). The optional return type
        # must NOT contain '::' (so it can't swallow Class:: itself). Nested
        # classes get an optional Outer:: prefix (e.g. GameModeManager_t::Tutorial_t).
        pat = re.compile(
            rb'(?m)^[ \t]*'                       # line start + indent
            rb'(?:[A-Za-z_][A-Za-z0-9_:<>*&, ]*?[ \t]+)?'  # return type
            + rb'(?:[A-Za-z_][A-Za-z0-9_]*::)*'   # optional outer qualifiers
            + re.escape(clsname.encode()) + rb'::' + re.escape(method_name.encode())
            + rb'\s*\([^;]*?\)'                 # params (no ';' inside)
            rb'\s*(?://[^\n]*)?'                  # optional trailing comment
            rb'\s*(?:const|noexcept|override|final)*\s*\{'  # qualifiers then body-open
        )
        m = pat.search(data)
        if m:
            # The '{' is the last char of the match; find its position.
            brace_pos = m.end() - 1
            sig_start = m.start()
            end = find_method_end_bytes(data, brace_pos)
            if end != -1:
                return cp, sig_start, end
    return None

def find_method_end(text, brace_pos):
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

def apply_one(tu, header, clsname, cpp_paths, dry=False, target=None,
              global_inserts=None, apply_now=False):
    """Flatten one class: insert forwarders, relabel header.
    Pass target=None to look up by name; pass a cursor to skip the lookup."""
    if target is None:
        target = get_class(tu, clsname)
    if target is None:
        print(f"  class {clsname} not found in {header}")
        return 0, 0

    methods = [c for c in target.get_children() if c.kind == CursorKind.CXX_METHOD]
    flatten = []
    private_to_relabel = []
    for m in methods:
        if m.is_deleted_method() or m.is_default_method() or is_trivial_accessor(m):
            continue
        if m.access_specifier == AccessSpecifier.PRIVATE and (header, clsname) not in SKIP_RELABEL:
            private_to_relabel.append(m.spelling)
        flatten.append(m)
    names = assign_flat_names(clsname, flatten)

    # Group insertions per cpp file.
    per_cpp = {}  # cpp_path -> [(body_end, fwd_bytes)]
    for m in flatten:
        fwd = emit_forwarder(clsname, m, names[m]).encode()
        found = find_def_in_cpps(clsname, m.spelling, cpp_paths)
        if found is None:
            print(f"    WARN: no definition found for {clsname}::{m.spelling}")
            continue
        cp, _, end = found
        per_cpp.setdefault(cp, []).append((end, fwd))

    # Accumulate into the GLOBAL per-cpp map (applied once at the end by --all
    # to avoid byte-offset shifting across classes).
    for cp, items in per_cpp.items():
        global_inserts.setdefault(cp, []).extend(items)

    inserted = sum(len(items) for items in per_cpp.values())
    if not dry and apply_now:
        # Single-class apply (no global coordination): strip + insert here.
        for cp, items in per_cpp.items():
            data = open(cp, 'rb').read()
            data = re.sub(rb'(?m)^extern "C" .*\{[^}]*\}\s*$\n?', b'', data)
            for end, fwd in sorted(items, key=lambda x: -x[0]):
                if fwd in data:
                    continue
                data = data[:end] + b'\n\n' + fwd + b'\n' + data[end:]
            open(cp, 'wb').write(data)

    if not dry:
        # Relabel private methods -> public in the header (both modes).
        hdr_path = os.path.join(SRC, header)
        hdr = open(hdr_path, encoding='utf-8', newline='').read()
        for name in private_to_relabel:
            # Declaration-anchored: match a line that is a member declaration
            # (type + name + '('), NOT a call site inside a function body.
            pat = re.compile(r'(\n[ \t]*)([A-Za-z_][A-Za-z0-9_:<>*&, ]*?[ \t]+' + re.escape(name) + r'\s*\()')
            m2 = pat.search(hdr)
            if m2:
                indent = m2.group(1)
                hdr = hdr[:m2.start()] + f"\n{indent}public:" + hdr[m2.start():]
        open(hdr_path, 'w', encoding='utf-8', newline='').write(hdr)

    print(f"  {clsname}: inserted {inserted} forwarders, relabeled {len(private_to_relabel)}")
    return inserted, len(private_to_relabel)

if __name__ == '__main__':
    dry = '--dry-run' in sys.argv
    args = [a for a in sys.argv[1:] if a != '--dry-run']

    if '--all' in args:
        # Process every project class.
        headers = [os.path.relpath(h, SRC).replace('\\', '/')
                   for h in glob.glob(SRC+'/*.hpp') + glob.glob(SRC+'/*/*.hpp')]
        # All cpp files for definition discovery.
        cpp_paths = [os.path.relpath(f, ROOT).replace('\\', '/')
                     for f in glob.glob(SRC+'/*.cpp') + glob.glob(SRC+'/*/*.cpp')]
        global_inserts = {}  # cp_path -> [(end, fwd)]

        # Global strip pre-pass: remove ALL previously generated forwarders.
        if not dry:
            for cp in cpp_paths:
                try:
                    data = open(cp, 'rb').read()
                except FileNotFoundError:
                    continue
                new = re.sub(rb'(?m)^extern "C" .*\{[^}]*\}\s*$\n?', b'', data)
                if new != data:
                    open(cp, 'wb').write(new)

        total_ins = 0
        seen = set()
        for h in headers:
            tu = parse(h)
            hp = norm(os.path.join(SRC, h))
            for n in tu.cursor.walk_preorder():
                if n.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL) and n.is_definition():
                    loc = n.location.file
                    if not loc:
                        continue
                    if norm(loc.name) != hp:
                        continue
                    ckey = (n.spelling, n.location.line)
                    if ckey in seen:
                        continue
                    seen.add(ckey)
                    n_methods = [c for c in n.get_children() if c.kind == CursorKind.CXX_METHOD]
                    if not n_methods or n.spelling in PORTED_CLASSES:
                        continue
                    ins, rel = apply_one(tu, h, n.spelling, cpp_paths,
                                         dry=dry, target=n,
                                         global_inserts=global_inserts, apply_now=False)
                    total_ins += ins

        # Apply all insertions per file once (offsets are against the
        # already-stripped file; applying sorted-desc per file is safe).
        if not dry:
            for cp, items in global_inserts.items():
                data = open(cp, 'rb').read()
                for end, fwd in sorted(items, key=lambda x: -x[0]):
                    if fwd in data:
                        continue
                    data = data[:end] + b'\n\n' + fwd + b'\n' + data[end:]
                open(cp, 'wb').write(data)
        print(f"\nTOTAL inserted forwarders: {total_ins}")
    else:
        analyze(args[0], args[1])
