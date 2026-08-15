#!/usr/bin/env python3
import clang.cindex as ci
from clang.cindex import CursorKind

src = r'''
class Entity {
public:
    int skill[60];
    int circuit_status;
    int monsterState;
};

void Entity::foo() {
    circuit_status = 1;
    if (circuit_status == 0) { }
    int circuit_status = 5;
    circuit_status = 7;
    monsterState = 2;
}
'''

idx = ci.Index.create()
tu = idx.parse('probe.cpp', args=['-x','c++','-std=c++17'], unsaved_files=[('probe.cpp', src)], options=ci.TranslationUnit.PARSE_DETAILED_PROCESSING_RECORD)

def ref_kind(node):
    try:
        if node.referenced is not None:
            return node.referenced.kind
    except Exception:
        pass
    return None

for node in tu.cursor.walk_preorder():
    if node.location.file and node.location.file.name == 'probe.cpp':
        k = node.kind
        if k in (CursorKind.DECL_REF_EXPR, CursorKind.MEMBER_REF_EXPR, CursorKind.VAR_DECL):
            rk = ref_kind(node)
            print(f"{node.location.line:3d} {str(k):25s} {node.spelling:15s} referenced={rk}")
