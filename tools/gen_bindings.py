#!/usr/bin/env python3
"""
gen_bindings.py - generate odin/bindings.odin: the foreign _barony block
declaring every extern "C" free function so Odin can call the C++ side.

Free functions were given extern "C" linkage by gen_free_bindings.py.
This generator reads their signatures from libclang and emits Odin
`proc "c"` declarations with `---` bodies.

Type mapping (CXX_TO_ODIN):
  * Builtins + C shims (Uint32->u32, real_t->f64, SDL/GL/OPENAL, ...)
  * Mirrored structs/enums -> their Odin names (Entity, Stat, Item, ...)
  * Unmirrored types -> rawptr (refined as mirrors land)
  * Pointers T* -> ^T, references T& -> ^T (Odin has no refs; C++ caller
    passes address), const char* -> cstring.
  * Nested types Frame::result_t -> last component.

Fallback: camelCase -> snake_case heuristic (ClipResult -> Clip_Result,
BaronyRNG -> Barony_RNG); if that name exists in odin/ use it, else rawptr.

Overloads / variadics / templates are SKIPPED (same rules as the flatten;
variadic free fns handled via va_list split, none left in the survey).

Modes:
  analyze:  print report
  apply:    write odin/bindings.odin

Usage:
  python3 tools/gen_bindings.py            # analyze
  python3 tools/gen_bindings.py --apply    # write odin/bindings.odin
"""
import sys, os, glob, re
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')
ODIN = os.path.join(ROOT, 'odin')

import clang.cindex as ci
from clang.cindex import CursorKind
LIB = r'C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/libclang.dll'
ci.Config.set_library_file(LIB)

# ---------------------------------------------------------------------------
# Odin type inventory (for fallback name resolution)
# ---------------------------------------------------------------------------
def load_odin_types():
    types = set()
    for f in glob.glob(os.path.join(ODIN, '*.odin')):
        for line in open(f, encoding='utf-8', errors='ignore'):
            m = re.match(r'^([A-Za-z_][A-Za-z0-9_]*) :: (struct|enum|distinct|map|#)', line)
            if m:
                types.add(m.group(1))
    return types

ODIN_TYPES = load_odin_types()

def load_ported():
    """Names of @(export) proc "c" implementations in odin/ (Odin-owned).

    These are the ported seam - the C++ side forwards to them. bindings.odin
    must NOT redeclare them (they're not C++-owned free functions anymore)."""
    ported = set()
    for f in glob.glob(os.path.join(ODIN, '*.odin')):
        if os.path.basename(f) == 'bindings.odin':
            continue
        src = open(f, encoding='utf-8', errors='ignore').read()
        # @(export) on the line before "Name ::"
        for m in re.finditer(r'@\(export\)\s*\n\s*(\w+)\s*::', src):
            ported.add(m.group(1))
    return ported

PORTED = load_ported()


# ---------------------------------------------------------------------------
# Type mapping
# ---------------------------------------------------------------------------
CXX_TO_ODIN = {
    # builtins
    'void': '', 'bool': 'bool', 'int': 'i32', 'unsigned int': 'u32',
    'float': 'f32', 'double': 'f64', 'real_t': 'f64', 'size_t': 'uint',
    'char': 'u8', 'short': 'i16', 'long': 'i64', 'unsigned': 'u32',
    # stdint / SDL
    'uint8_t': 'u8', 'uint16_t': 'u16', 'uint32_t': 'u32', 'uint64_t': 'u64',
    'int8_t': 'i8', 'int16_t': 'i16', 'int32_t': 'i32', 'int64_t': 'i64',
    'Uint8': 'u8', 'Uint16': 'u16', 'Uint32': 'u32', 'Uint64': 'u64',
    'Sint8': 'i8', 'Sint16': 'i16', 'Sint32': 'i32', 'Sint64': 'i64',
    'FILE': 'rawptr',
    # SDL / GL / OPENAL C-API
    'SDL_Surface': 'rawptr', 'SDL_RWops': 'rawptr', 'SDL_Cursor': 'rawptr',
    'SDL_Window': 'rawptr', 'SDL_Rect': 'rawptr', 'SDL_Event': 'rawptr',
    'GLenum': 'u32', 'GLuint': 'u32', 'GLint': 'i32', 'GLfloat': 'f32',
    'GLdouble': 'f64', 'GLsizei': 'i32', 'GLboolean': 'bool',
    'GLchar': 'u8', 'GLvoid': 'rawptr', 'GLubyte': 'u8',
    'OPENAL_BUFFER': 'u32', 'OPENAL_CHANNELGROUP': 'u32', 'OPENAL_SOUND': 'u32',
    'OPENAL_FLOAT': 'f32', 'OPENAL_INT': 'i32', 'OPENAL_BOOL': 'bool',
    'ALuint': 'u32', 'ALint': 'i32', 'ALfloat': 'f32', 'ALboolean': 'bool',
    'va_list': 'c.va_list',
    # container shims
    'DynamicArray': 'rawptr',  # bare DynamicArray<T>& - opaque (T unknown)
    'DynamicArrayS32': '[dynamic]i32',
    'DynamicArrayStr': '[dynamic]string',
    'DynamicString': 'string',
    'DynamicMapRaw': 'rawptr',  # raw map buffer - opaque
    # mirrored structs (exact name match)
    'Entity': 'Entity', 'Stat': 'Stat', 'Item': 'Item', 'Player': 'Player',
    'File': 'File', 'FileInterface': 'FileInterface', 'Chunk': 'Chunk',
    'Shader': 'Shader', 'Frame': 'Frame', 'Button': 'Button', 'Field': 'Field',
    'Slider': 'Slider', 'Widget': 'Widget', 'Font': 'Font', 'Image': 'Image',
    'Text': 'Text', 'Directory': 'Directory', 'Menu': 'Menu',
    'Monster': 'Monster', 'Spell': 'Spell', 'Level': 'Level',
    'UDPsocket': 'rawptr', 'UDPpacket': 'rawptr', 'TCPsocket': 'rawptr',
    # enums -> i32 (or their Odin mirror)
    'ItemType': 'ItemType', 'EntityClickType': 'i32',
    'EntityHungerIntervals': 'i32', 'SaveFileType': 'i32',
    'HolidayTheme': 'i32', 'SteamGlobalStatIndexes': 'i32',
    'NetworkingLobbyJoinRequestResult': 'i32',
    'GeneratePathTypes': 'i32', 'Category': 'i32',
    'EquipItemResult': 'i32', 'EquipItemSendToServerSlot': 'i32',
    'DIR': 'i32', 'ClipResult': 'Clip_Result',
    'AttackHoverText_t': 'AttackHoverText_T', 'EffectType': 'i32',
    'Iterator': 'rawptr',
}

def snake(camel):
    """camelCase -> snake_case (BaronyRNG -> Barony_RNG)."""
    s = re.sub(r'(?<!^)(?=[A-Z])', '_', camel)
    return s

def cxx_to_odin(t, odin_types=ODIN_TYPES):
    """Map a C++ type spelling to Odin. Returns Odin type string or 'rawptr'."""
    t = t.strip()
    # reference -> pointer
    is_ref = '&' in t
    # pointer depth
    depth = t.count('*')
    t = t.replace('*', '').replace('&', '').replace('const ', '').strip()
    # nested types: Frame::result_t -> result_t
    if '::' in t:
        t = t.split('::')[-1]
    # template args: DynamicArrayT<X> -> DynamicArray (rawptr if unknown)
    t = re.sub(r'<.*>', '', t).strip()
    if t in ('void',):
        return '' if not depth else 'rawptr'
    if t in CXX_TO_ODIN:
        base = CXX_TO_ODIN[t]
    elif t in odin_types:
        base = t
    else:
        # heuristic: snake_case
        s = snake(t)
        if s in odin_types:
            base = s
        else:
            base = 'rawptr'
    if is_ref and base != 'cstring':
        base = base  # refs stay as-is; caller passes ^ already? No - T& means caller passes pointer
    if depth:
        if base == 'cstring':
            return 'cstring' if depth == 1 else 'rawptr'
        return '^' * depth + base
    if is_ref:
        return '^' + base
    return base

# ---------------------------------------------------------------------------
# Free function collection (shared with gen_free_bindings.py)
# ---------------------------------------------------------------------------
def parse(header):
    if os.path.isabs(header):
        full = header
    else:
        full = os.path.join(SRC, header)
    args = ['-x','c++','-std=c++17','-fms-compatibility-version=19.51',
            '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
            '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION',
            '-include', os.path.join(SRC, 'main.hpp'),
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
            continue
        if is_inside_class(c):
            continue
        if c.type.is_function_variadic():
            continue
        if any(k.kind == CursorKind.TEMPLATE_TYPE_PARAMETER for k in c.get_children()):
            continue
        out.append(c)
    return out

# ---------------------------------------------------------------------------
# Emission
# ---------------------------------------------------------------------------
OPERATOR_SUFFIX = {
    'operator==': 'eq', 'operator!=': 'ne', 'operator<': 'lt', 'operator<=': 'le',
    'operator>': 'gt', 'operator>=': 'ge', 'operator+': 'add', 'operator-': 'sub',
    'operator*': 'mul', 'operator/': 'div', 'operator%': 'mod',
    'operator+=': 'add_assign', 'operator-=': 'sub_assign', 'operator*=': 'mul_assign',
    'operator/=': 'div_assign', 'operator%=': 'mod_assign', 'operator=': 'assign',
    'operator[]': 'index', 'operator()': 'call', 'operator<<': 'shl', 'operator>>': 'shr',
    'operator!': 'not', 'operator&&': 'and', 'operator||': 'or',
}

def flat_fn_name(spelling):
    """operator== -> eq, operator!= -> ne, etc. (flatten tool scheme)."""
    if spelling in OPERATOR_SUFFIX:
        return OPERATOR_SUFFIX[spelling]
    if spelling.startswith('operator'):
        body = spelling[len('operator'):]
        body = ''.join(c if c.isalnum() else '_' for c in body)
        return 'op_' + body
    return spelling

def odin_name_for(cursor):
    return flat_fn_name(cursor.spelling)

RESERVED = {'map', 'type', 'in', 'out', 'distinct', 'struct', 'enum', 'union', 'proc', 'foreign', 'import', 'package', 'when', 'if', 'else', 'for', 'return', 'var', 'const', 'break', 'continue', 'not', 'or', 'and', 'do', 'defer', 'fallthrough', 'switch', 'case', 'using', 'where', 'asm', 'context', 'bool', 'string', 'nil', 'true', 'false', 'i8', 'i16', 'i32', 'i64', 'u8', 'u16', 'u32', 'u64', 'f32', 'f64', 'int', 'uint', 'rawptr', 'any', 'cstring'}

def proc_decl(cursor):
    """Emit one `name :: proc "c" (...) -> ret ---` line."""
    name = flat_fn_name(cursor.spelling)
    args = []
    used = set()
    for a in cursor.get_arguments():
        at = cxx_to_odin(a.type.spelling)
        an = a.spelling or 'arg'
        if an in RESERVED or an in used:
            an = an + '_'
        used.add(an)
        args.append(f"{an}: {at}")
    rt = cxx_to_odin(cursor.result_type.spelling)
    ret = f" -> {rt}" if rt else ""
    return f"    {name} :: proc \"c\" ({', '.join(args)}){ret} ---"

def main():
    apply = '--apply' in sys.argv
    headers = sorted(glob.glob(os.path.join(SRC, '**', '*.hpp'), recursive=True))
    decls = []
    skipped_overload = []
    skipped_other = []
    for h in headers:
        h_abs = os.path.abspath(h).replace(os.sep, '/')
        h_rel = os.path.relpath(h, SRC).replace(os.sep, '/')
        try:
            tu = parse(h_rel)
        except Exception as e:
            print(f"ERR {h_rel}: {e}")
            continue
        fns = free_functions(tu, h_abs)
        if not fns:
            continue
        # dedupe cursors (libclang emits decl + redecl at same line as 2 cursors)
        seen_loc = set()
        uniq_fns = []
        for f in fns:
            key = (f.spelling, f.location.line)
            if key in seen_loc:
                continue
            seen_loc.add(key)
            uniq_fns.append(f)
        fns = uniq_fns
        from collections import Counter
        ncount = Counter(f.spelling for f in fns)
        overloaded = {n for n, c in ncount.items() if c > 1}
        for f in fns:
            if f.spelling in overloaded:
                skipped_overload.append(f.spelling)
                continue
            if f.spelling in PORTED:
                continue  # Odin-owned @(export) impl; not a C++-owned free fn
            decls.append(proc_decl(f))
    # dedupe (same fn may be declared in multiple headers) - keep first
    seen = set()
    uniq = []
    for d in decls:
        m = re.match(r'^\s*(\w+) ::', d)
        name = m.group(1) if m else d
        if name in seen:
            continue
        seen.add(name)
        uniq.append(d)
    print(f"free fns to declare: {len(uniq)}")
    print(f"overloads skipped: {len(set(skipped_overload))}")
    if skipped_overload:
        print(f"  {sorted(set(skipped_overload))}")
    if not apply:
        for d in uniq[:10]:
            print(d)
        print("...")
        return
    out = ['// bindings.odin - generated by tools/gen_bindings.py. Do not edit by hand.',
           '//',
           '// Foreign declarations for every extern "C" free function in the C++',
           '// game code. As a function is ported to Odin (its @(export) proc "c"',
           '// implementation), remove its decl from this file and move the body',
           '// into the ported file - the name is the seam.',
           '//',
           '// The foreign import _barony lives in game.odin/editor.odin (when-gated);',
           '// the block below mirrors their guards so _barony is in scope.',
           '',
           'package main',
           '',
           'when !#config(EDITOR, false) {',
           '	foreign import _barony "../builddir/src/libbarony_game.a"',
           '	foreign _barony {']
    out += uniq
    out += ['	}', '}',
            '',
            'when #config(EDITOR, false) {',
            '	foreign import _barony "../builddir/src/libbarony_editor.a"',
            '	foreign _barony {']
    out += uniq
    out += ['	}', '}']
    with open(os.path.join(ODIN, 'bindings.odin'), 'w', encoding='utf-8', newline='') as f:
        f.write('\n'.join(out) + '\n')
    print(f"wrote odin/bindings.odin ({len(uniq)} decls)")

if __name__ == '__main__':
    main()
