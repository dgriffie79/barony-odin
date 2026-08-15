// menu.odin — Odin mirrors of menu.hpp.
package main

import "containers"

// struct SaveGameListEntry_t — 16 bytes
// { int lastModified; int multiplayerType; int fileEntry; int description; }
SaveGameListEntry_t :: struct {
	last_modified:   i32,
	multiplayer_type: i32,
	file_entry:      i32,
	description:     i32,
}
#assert(size_of(SaveGameListEntry_t) == 16)

// struct resolution — 12 bytes
resolution :: struct {
	x:  i32,
	y:  i32,
	hz: i32,
}
#assert(size_of(resolution) == 12)

// struct LastCreatedCharacter — 120 bytes
// { int characterClass[6]; int characterAppearance[6]; int characterSex[6];
//   int characterRace[6]; DynamicString characterName[6]; }
LastCreatedCharacter :: struct {
	character_class:       [6]i32,
	character_appearance:  [6]i32,
	character_sex:         [6]i32,
	character_race:        [6]i32,
	character_name:        [6]containers.DynamicString, // 6 x 16B = 96B
}
#assert(size_of(LastCreatedCharacter) == 192)
