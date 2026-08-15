// collision.odin — Odin mirrors of collision.hpp.
package main

import "containers"

// struct MonsterTrapIgnoreEntities_t — 40 bytes
// { DynamicSetI32 ignoreEntities (32B); Uint32 parent; }
MonsterTrap_Ignore_Entities_t :: struct {
	ignore_entities: containers.Raw_Map, // DynamicSetI32 (32B)
	parent:          u32,
}

#assert(size_of(MonsterTrap_Ignore_Entities_t) == 40)
