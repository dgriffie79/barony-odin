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
def pristine(path):
    """Read a file from git HEAD (the pristine pre-rewrite state), so the
    script is idempotent even if the working tree is already partially
    rewritten."""
    import subprocess
    rel = os.path.relpath(path, ROOT).replace('\\', '/')
    try:
        out = subprocess.run(['git', 'show', 'HEAD:' + rel],
                             capture_output=True, text=True, cwd=ROOT)
        if out.returncode == 0:
            return out.stdout
    except Exception:
        pass
    # fallback: current disk state
    return open(path, encoding='utf-8', newline='').read()

def alias_set():
    """Extract reference-member fields from pristine entity.hpp/stat.hpp."""
    names = set()
    for h in ('entity.hpp', 'stat.hpp'):
        txt = pristine(os.path.join(SRC, h))
        import re
        for m in re.finditer(r'^\s*(?:Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*[;=]', txt, re.M):
            names.add(m.group(1))
    return names

def backing_map():
    """From ctor init-lists (entity_shared.cpp/stat_shared.cpp) AND in-class
    initializers (entity.hpp/stat.hpp: `T& name = arr[idx];`)."""
    out = {}
    import re
    # 1. ctor init-lists (pristine)
    for f in ('entity_shared.cpp', 'stat_shared.cpp'):
        txt = pristine(os.path.join(SRC, f))
        for m in re.finditer(r'^\s*(\w+)\s*\(\s*(?:(\w+)\s*\[\s*(\d+)\s*\]|(\w+))\s*\)\s*,?', txt, re.M):
            name = m.group(1)
            if m.group(2):
                out[name] = '%s[%s]' % (m.group(2), m.group(3))
            elif m.group(4):
                out[name] = m.group(4)
    # 2. in-class initializers (pristine)
    for f in ('entity.hpp', 'stat.hpp'):
        txt = pristine(os.path.join(SRC, f))
        for m in re.finditer(r'^\s*(?:Sint32|Uint32|int|bool|real_t|float|double)\s*&\s*(\w+)\s*=\s*(\w+)\s*\[\s*(\d+)\s*\]\s*;', txt, re.M):
            out[m.group(1)] = '%s[%s]' % (m.group(2), m.group(3))
    return out

# ---------------------------------------------------------------------------
# clang helpers
# ---------------------------------------------------------------------------
def compile_args(entry):
    cwd = entry['directory']
    parts = shlex.split(entry['command'])
    args = []
    for p in parts:
        if p.endswith('cl.exe') or p == 'cl':
            continue
        if p.startswith('/Fo') or p.startswith('/Fd') or p.startswith('/c') or p.startswith('/F'):
            continue
        if p.endswith('.cpp') or ('.cpp' in p and os.path.sep in p):
            continue
        args.append(p)
    def norm(a):
        inc = None
        if a.startswith('/I'):
            inc = a[2:]
        elif a.startswith('-I') and len(a) > 2:
            inc = a[2:]
        if inc is not None:
            if not os.path.isabs(inc):
                inc = os.path.normpath(os.path.join(cwd, inc))
            return '-I' + inc
        if a.startswith('/D'): return '-D' + a[2:]
        if a.startswith('/std:'): return '-std=' + a[5:]
        if a.startswith('/'): return None
        return a
    args = [x for x in (norm(a) for a in args) if x is not None]
    # Clang-on-MSVC-STL compatibility: this clang lacks __builtin_verbose_trap
    # (the MSVC STL's clang-20+ intrinsic), which truncates the AST. The STL's
    # documented escape hatch is to use abort() as the doom function.
    args += ['-x', 'c++', '-fms-compatibility-version=19.51',
             '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
             '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION']
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
    # allow a trailing comment after the init-list entry. Use [^\S\r\n]* for
    # trailing whitespace so \s* can't cross newlines and eat the next lines.
    new = re.sub(r'^[ \t]*(\w+)[ \t]*\([ \t]*(?:(?:\w+)[ \t]*\[[ \t]*\d+[ \t]*\]|\w+)[ \t]*\)[ \t]*,?[ \t]*(?=$|//)',
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
    # Read as raw bytes: libclang's loc.offset is a BYTE offset into the raw
    # file. Decoding to str would shift offsets at every non-ASCII (UTF-8
    # multi-byte) character. We operate on bytes throughout.
    txt = open(file, 'rb').read()
    edits = []
    for node in tu.cursor.walk_preorder():
        if node.kind == CursorKind.MEMBER_REF_EXPR:
            ref = node.referenced
            # after phase1 the alias field became a method (CXX_METHOD); before
            # it is FIELD_DECL. Accept both.
            if ref is not None and ref.kind in (CursorKind.FIELD_DECL, CursorKind.CXX_METHOD) and node.spelling in aliases:
                # must be a member of Entity/Stat, not a same-named member of
                # another struct (e.g. PlayerRaceHostility_t::playerRace)
                owner = None
                try:
                    owner = ref.semantic_parent.spelling if ref.semantic_parent else None
                except Exception:
                    owner = None
                if owner not in ('Entity', 'Stat'):
                    continue
                loc = node.location
                if loc is None or loc.file is None:
                    continue
                if os.path.basename(loc.file.name) != os.path.basename(file):
                    continue
                end = loc.offset + len(node.spelling)
                # guard 1: not a prefix of a longer identifier
                if end < len(txt) and (bytes([txt[end]]).isalnum() or txt[end] == ord('_')):
                    continue
                # guard 2: skip if already a call (followed by '(')
                k = end
                while k < len(txt) and txt[k] in (32, 9):  # ' ', '\t'
                    k += 1
                if k < len(txt) and txt[k] == ord('('):
                    continue
                # guard 3: never edit inside a preprocessor line
                line_start = txt.rfind(b'\n', 0, loc.offset) + 1
                if txt[line_start:line_start+1] == b'#':
                    continue
                edits.append((end, end, b'()'))
        # '&alias' where alias is now a method: parse fails (cannot create
        # pointer to member) — clang reports OVERLOADED_DECL_REF under a
        # UNARY_OPERATOR '&'. Rewrite to '&alias()'.
        if node.kind == CursorKind.UNARY_OPERATOR and node.spelling == '&':
            for ch in node.get_children():
                if ch.kind == CursorKind.OVERLOADED_DECL_REF and ch.spelling in aliases:
                    # confirm it's an Entity/Stat method
                    try:
                        owner = ch.referenced.semantic_parent.spelling if ch.referenced and ch.referenced.semantic_parent else None
                    except Exception:
                        owner = None
                    if owner not in ('Entity', 'Stat'):
                        continue
                    loc = ch.location
                    if loc is None or loc.file is None:
                        continue
                    if os.path.basename(loc.file.name) != os.path.basename(file):
                        continue
                    end = loc.offset + len(ch.spelling)
                    k = end
                    while k < len(txt) and txt[k] in (32, 9):
                        k += 1
                    if k < len(txt) and txt[k] == ord('('):
                        continue
                    edits.append((end, end, b'()'))
    if not edits:
        return
    # dedupe and sort desc
    edits = sorted(set(edits), key=lambda e: e[0], reverse=True)
    for (s, e, ins) in edits:
        txt = txt[:s] + ins + txt[e:]
    if not dry:
        open(file, 'wb').write(txt)
    print('phase3:', file, len(edits), 'edits')

def phase4_addr_of(file, aliases, dry):
    """Text pass: rewrite `&<receiver>->alias` and `&<receiver>.alias` to
    append `()`, since after phase1 the alias is a method and clang fails to
    parse the address-of-method form (truncating the AST). Pattern is
    unambiguous: '&' immediately before receiver + '->'/'.' + alias name."""
    import re
    txt = open(file, encoding='utf-8', newline='').read()
    changed = False
    for name in sorted(aliases, key=len, reverse=True):
        # find '&' .. receiver .. '->'/'.' .. name (name not already a call)
        # We only rewrite when the name is directly after '->' or '.', and the
        # '&' is within the preceding few tokens (no ';' or newline between).
        idx = 0
        while True:
            found = txt.find(name, idx)
            if found == -1:
                break
            idx = found + len(name)
            # skip if it's a prefix of a longer identifier or already a call
            after = txt[found + len(name):]
            if after[:1].isalnum() or after[:1] == '_':
                continue
            k = 0
            while k < len(after) and after[k] in ' \t':
                k += 1
            if k < len(after) and after[k] == '(':
                continue
            # walk back to the '&': must be <receiver>->name or <receiver>.name
            before = txt[max(0, found - 60):found]
            m = re.search(r'&([^\n;{}]*?)(?:->|\.)\s*$', before)
            if not m:
                continue
            # make sure the receiver between & and ->/. has no '(' (not a call arg)
            recv = m.group(1)
            if '(' in recv:
                continue
            # ensure this is not a pointer-to-member context; simple receivers only
            if '&' in recv[:-1]:
                continue
            # apply edit: insert '()' after name
            txt = txt[:found + len(name)] + '()' + txt[found + len(name):]
            changed = True
            idx = found + len(name) + 2
    if changed and not dry:
        open(file, 'w', encoding='utf-8', newline='').write(txt)
    return changed

def main():
    dry = '--dry-run' in sys.argv
    aliases = alias_set()
    backing = backing_map()
    print('aliases:', len(aliases), 'backings:', len(backing))

    # Phase 3+4 FIRST, against the PRISTINE headers (aliases are still fields,
    # so clang parses `stats[player]->playerSummonLVLHP` cleanly). If we
    # rewrote the headers first (phase1), the aliases become methods and clang
    # errors on field-style uses, truncating the AST.
    stl_args = ['-x','c++','-fms-compatibility-version=19.51',
                '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH',
                '-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION']
    hdr_args = stl_args + ['-I'+SRC,'-I'+os.path.join(SRC,'magic'),
                '-I'+os.path.join(SRC,'interface'),'-I'+os.path.join(SRC,'ui'),
                '-I'+os.path.join(SRC,'engine'),'-I'+os.path.join(SRC,'engine','audio'),
                '-I'+os.path.join(ROOT,'odin','containers'),
                '-I'+os.path.join(ROOT,'builddir'),
                '-IC:/dev/vcpkg/installed/x64-windows/include',
                '-IC:/dev/vcpkg/installed/x64-windows/include/SDL2']
    phase3(os.path.join(SRC, 'entity.hpp'), aliases, hdr_args, dry)
    phase3(os.path.join(SRC, 'stat.hpp'), aliases, hdr_args, dry)

    cc = json.load(open(os.path.join(ROOT, 'builddir', 'compile_commands.json'), encoding='utf-8'))
    cpp_files = []
    seen_files = set()
    for e in cc:
        f = e['file']
        if f.startswith('../'):
            f = os.path.normpath(os.path.join(e['directory'], f))
        f = os.path.normpath(os.path.abspath(f))
        if f.endswith('.cpp'):
            if f in seen_files:
                continue
            seen_files.add(f)
            cpp_files.append((f, compile_args(e)))

    for f, args in cpp_files:
        if not os.path.exists(f):
            continue
        phase3(f, aliases, args, dry)
        phase4_addr_of(f, aliases, dry)

    # Phase 1 + 2 (rewrite headers + ctors) LAST, after all references updated.
    phase1_header(os.path.join(SRC, 'entity.hpp'), 'entity', aliases, backing, dry)
    phase1_header(os.path.join(SRC, 'stat.hpp'), 'stat', aliases, backing, dry)
    phase2_ctor(os.path.join(SRC, 'entity_shared.cpp'), aliases, backing, dry)
    phase2_ctor(os.path.join(SRC, 'stat_shared.cpp'), aliases, backing, dry)

if __name__ == '__main__':
    main()
