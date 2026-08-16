// ui_text.odin — Odin mirror of ui/Text.hpp.
package main

// class Text — 80 bytes
Text :: struct {
	name:              string, // DynamicString (16B)
	texid:             u32,    // GLuint
	// pad 4
	surf:              rawptr, // SDL_Surface*
	width:             i32,
	height:            i32,
	num_text_lines:    i32,
	// pad 4
	words_to_highlight: map[[4]byte]u32, // DynamicMapI32T<Uint32> (32B)
}
#assert(size_of(Text) == 80)
