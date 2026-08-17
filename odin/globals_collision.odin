// globals_collision.odin — Odin owners of the extern globals declared in src/collision.hpp.
package main

// extern "C" DynamicMapI32T<MonsterTrapIgnoreEntities_t> monsterTrapIgnoreEntities;
// (C++ definition deleted from src/actbeartrap.cpp; Odin is the single owner now.)
@(export)
monsterTrapIgnoreEntities : map[[4]byte]MonsterTrap_Ignore_Entities_t
