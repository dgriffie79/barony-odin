// collision.odin — Odin mirrors of collision.hpp.
package main


// struct MonsterTrapIgnoreEntities_t — 40 bytes
// { DynamicSetI32 ignoreEntities (32B); Uint32 parent; }
MonsterTrap_Ignore_Entities_t :: struct {
	ignore_entities: map[i32]struct{}, // DynamicSetI32 (32B)
	parent:          u32,
}

#assert(size_of(MonsterTrap_Ignore_Entities_t) == 40)


// ---------------------------------------------------------------------------
// Globals owned by Odin (C++ references via extern "C")
// ---------------------------------------------------------------------------
@(export)
monsterTrapIgnoreEntities : map[[4]byte]MonsterTrap_Ignore_Entities_t
