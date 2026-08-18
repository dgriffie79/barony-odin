// ui_font.odin - Odin mirror of ui/Font.hpp.
package main

// class Font - 32 bytes
Font :: struct {
	name:        string, // DynamicString (16B)
	font:        rawptr, // TTF_Font*
	point_size:  i32,    // default 16
	outline_size: i32,   // default 0
}
#assert(size_of(Font) == 32)
