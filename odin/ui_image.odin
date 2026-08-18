// ui_image.odin - Odin mirror of ui/Image.hpp.
package main

// class Image - 56 bytes (has virtuals -> vtable ptr first)
Image :: struct {
	vtable:        rawptr, // vftable pointer (Image is polymorphic)
	name:          string, // DynamicString (16B)
	texid:         u32,    // GLuint
	// pad 4
	surf:          rawptr, // SDL_Surface*
	outline_surf:  rawptr, // SDL_Surface*
	clamp:         bool,
	point:         bool,
	// pad 6
}
#assert(size_of(Image) == 56)
