#!/usr/bin/env python3
"""alias_rewrite.py — clang-AST-driven rewrite of Entity/Stat reference members.

Replaces reference members (`T& name;`) with inline reference-returning getters
and rewrites every reference to those members (bare implicit `this->`, `->`,
`.`) to call the getter (`name()`).

The clang AST gives exact symbol resolution: a reference to the alias is a
MEMBER_REF_EXPR whose referenced decl is the FIELD_DECL (the reference member),
while a local variable or parameter with the same name is a DECL_REF_EXPR/
VAR_DECL referencing a VAR_DECL/PARM_DECL. This resolves all the heuristic
ambiguities (shadowing locals, `>`-vs-`->`, params named like aliases).

Phases:
  1. Header: for each `T& name;` field in Entity/Stat, replace with
     `inline T& name() { return <backing>; }` + const overload.
  2. Ctor: remove the init-list entries `name(backing)`.
  3. Bodies: for each MEMBER_REF_EXPR referencing an alias FIELD_DECL,
     append `()`.

Run: python tools/alias_rewrite.py [--dry-run]
"""
import os, sys, json, shlex
import clang.cindex as ci
from clang.cindex import CursorKind

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, 'src')

# ---------------------------------------------------------------------------
# alias set + backing map (from the pristine headers' ctor init-lists)
# ---------------------------------------------------------------------------
def alias_set():
    """Extract reference-member fields from entity.hpp/stat.hpp via a simple
    textual scan of the PRISTINE headers (we run before any rewrite)."""
    names = set()
    for h in ('entity.hpp', 'stat.hpp'):
        txt = open(os.path.join(SRC, h), encoding='utf-8').read()
        import re
        for m in re.finditer(r'^\s*(?:Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*[;=]', txt, re.M):
            names.add(m.group(1))
    return names

def backing_map():
    """From ctor init-lists (entity_shared.cpp/stat_shared.cpp) AND in-class
    initializers (entity.hpp/stat.hpp: `T& name = arr[idx];`)."""
    out = {}
    import re
    # 1. ctor init-lists
    for f in ('entity_shared.cpp', 'stat_shared.cpp'):
        txt = open(os.path.join(SRC, f), encoding='utf-8').read()
        for m in re.finditer(r'^\s*(\w+)\s*\(\s*(?:(\w+)\s*\[\s*(\d+)\s*\]|(\w+))\s*\)\s*,?\s*$', txt, re.M):
            name = m.group(1)
            if m.group(2):
                out[name] = '%s[%s]' % (m.group(2), m.group(3))
            elif m.group(4):
                out[name] = m.group(4)
    # 2. in-class initializers
    for f in ('entity.hpp', 'stat.hpp'):
        txt = open(os.path.join(SRC, f), encoding='utf-8').read()
        for m in re.finditer(r'^\s*(?:Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*=\s*(\w+)\s*\[\s*(\d+)\s*\]\s*;', txt, re.M):
            out[m.group(1)] = '%s[%s]' % (m.group(2), m.group(3))
    return out

# ---------------------------------------------------------------------------
# clang helpers
# ---------------------------------------------------------------------------
def compile_args(entry):
    parts = shlex.split(entry['command'])
    args = []
    for p in parts:
        if p.endswith('cl.exe') or p == 'cl':
            continue
        if p.startswith('/Fo') or p.startswith('/Fd') or p.startswith('/c') or p.startswith('/F'):
            continue
        if p.endswith('.cpp') or '.cpp' in p and os.path.sep in p:
            continue
        args.append(p)
    def norm(a):
        if a.startswith('/I'): return '-I' + a[2:]
        if a.startswith('/D'): return '-D' + a[2:]
        if a.startswith('/std:'): return '-std=' + a[5:]
        if a.startswith('/'): return None
        return a
    args = [x for x in (norm(a) for a in args) if x is not None]
    args += ['-x', 'c++']
    return args

# ---------------------------------------------------------------------------
# Phase 1: rewrite headers (entity.hpp / stat.hpp)
# ---------------------------------------------------------------------------
def phase1_header(header, owner, aliases, backing, dry):
    txt = open(header, encoding='utf-8').read()
    import re
    def repl(m):
        t = m.group(1)
        name = m.group(2)
        if name not in aliases:
            return m.group(0)
        b = backing.get(name, '')
        T = t
        if T in ('Sint32', 'Uint32', 'int'):
            T = 'int'
        elif T == 'real_t':
            T = 'real_t'
        lines = []
        lines.append('\tinline %s& %s() { return %s; }' % (T, name, b))
        lines.append('\tinline const %s& %s() const { return %s; }' % (T, name, b))
        return '\n'.join(lines)
    # in-class `T& name;` or `T& name = arr[i];`
    new = re.sub(r'^\s*(Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*(?:=\s*\w+\s*\[\s*\d+\s*\])?\s*;',
                 repl, txt, flags=re.M)
    if new != txt:
        if not dry:
            open(header, 'w', encoding='utf-8', newline='').write(new)
        print('phase1 header:', header)
    return new

# ---------------------------------------------------------------------------
# Phase 2: remove ctor init-list entries
# ---------------------------------------------------------------------------
def phase2_ctor(cpp, aliases, backing, dry):
    txt = open(cpp, encoding='utf-8').read()
    import re
    new = re.sub(r'^\s*(\w+)\s*\(\s*(?:(?:\w+)\s*\[\s*\d+\s*\]|\w+)\s*\)\s*,?\s*$',
                 lambda m: '' if m.group(1) in aliases else m.group(0),
                 txt, flags=re.M)
    if new != txt:
        if not dry:
            open(cpp, 'w', encoding='utf-8', newline='').write(new)
        print('phase2 ctor:', cpp)

# ---------------------------------------------------------------------------
# Phase 3: rewrite references in .cpp and header inline bodies
# ---------------------------------------------------------------------------
def phase3(file, aliases, args, dry):
    idx = ci.Index.create()
    tu = idx.parse(file, args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)
    edits = []
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.MEMBER_REF_EXPR:
            ref = node.referenced
            if ref is not None and ref.kind == CursorKind.FIELD_DECL and node.spelling in aliases:
                # append '()' after the identifier
                # extent.end.offset points just past the name
                end = node.extent.end.offset
                edits.append((end, end, '()'))
    if not edits:
        return
    # dedupe and sort desc
    edits = sorted(set(edits), key=lambda e: e[0], reverse=True)
    txt = open(file, encoding='utf-8').read()
    for (s, e, ins) in edits:
        txt = txt[:s] + ins + txt[e:]
    if not dry:
        open(file, 'w', encoding='utf-8', newline='').write(txt)
    print('phase3:', file, len(edits), 'edits')

def main():
    dry = '--dry-run' in sys.argv
    aliases = alias_set()
    backing = backing_map()
    print('aliases:', len(aliases), 'backings:', len(backing))

    # Phase 1 + 2
    phase1_header(os.path.join(SRC, 'entity.hpp'), 'entity', aliases, backing, dry)
    phase1_header(os.path.join(SRC, 'stat.hpp'), 'stat', aliases, backing, dry)
    phase2_ctor(os.path.join(SRC, 'entity_shared.cpp'), aliases, backing, dry)
    phase2_ctor(os.path.join(SRC, 'stat_shared.cpp'), aliases, backing, dry)

    # Phase 3 over all .cpp (and headers for inline bodies)
    cc = json.load(open(os.path.join(ROOT, 'builddir', 'compile_commands.json'), encoding='utf-8'))
    cpp_files = []
    for e in cc:
        f = e['file']
        if f.startswith('../'):
            f = os.path.normpath(os.path.join(e['directory'], f))
        if f.endswith('.cpp'):
            cpp_files.append((f, compile_args(e)))

    for f, args in cpp_files:
        if not os.path.exists(f):
            continue
        phase3(f, aliases, args, dry)

if __name__ == '__main__':
    main()
