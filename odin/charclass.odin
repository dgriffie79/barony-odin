// charclass.odin -- Odin mirror of charclass.hpp.
package main

// class PlayerCharacterClassManager - wraps a Stat* + int characterClass.
// The class has no vtable and no other data (methods are inline in the header).
PlayerCharacterClassManager :: struct {
	class_stats:    ^Stat, // Stat*
	character_class: i32,  // int
}
#assert(size_of(PlayerCharacterClassManager) == 16)
