import sys, os
sys.path.insert(0, 'builddir'); sys.path.insert(0, 'tools')
import clang.cindex as ci
from clang.cindex import CursorKind
ROOT='C:/dev/barony-odin'; SRC=os.path.join(ROOT,'src')
args = ['-x','c++','-std=c++17','-fms-compatibility-version=19.51',
        '-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH','-D_MSVC_STL_USE_ABORT_AS_DOOM_FUNCTION',
        '-I'+SRC,'-I'+os.path.join(SRC,'magic'),'-I'+os.path.join(SRC,'interface'),
        '-I'+os.path.join(SRC,'ui'),'-I'+os.path.join(SRC,'engine'),'-I'+os.path.join(SRC,'engine','audio'),
        '-I'+os.path.join(ROOT,'odin','containers'),'-I'+os.path.join(ROOT,'builddir'),
        '-IC:/dev/vcpkg/installed/x64-windows/include','-IC:/dev/vcpkg/installed/x64-windows/include/SDL2']
idx=ci.Index.create()
tu=idx.parse(os.path.join(SRC,'player.hpp'), args=args, options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

def is_from_player(n):
    loc = n.location
    try:
        f = loc.file
        return f and f.name.replace('\\','/').endswith('player.hpp')
    except:
        return False

for n in tu.cursor.walk_preorder():
    if n.kind in (CursorKind.CLASS_DECL, CursorKind.STRUCT_DECL) and is_from_player(n):
        name = n.spelling or '<anon>'
        if not name or name=='<anon>': continue
        kids = list(n.get_children())
        fields = [k for k in kids if k.kind==CursorKind.FIELD_DECL]
        if not fields: continue
        print('=== {} sizeof={} ==='.format(name, n.type.get_size()))
        for f in fields:
            print('  {}: {} (size {})'.format(f.spelling, f.type.spelling, f.type.get_size()))
