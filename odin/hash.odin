// hash.odin — Odin mirrors of hash.hpp.
package main

import "core:c"

HASH_SIZE :: 256

// typedef struct ttfTextHash_t — 32 bytes
ttfTextHash_t :: struct {
	str:     cstring, // char*
	surf:    rawptr,  // SDL_Surface*
	font:    rawptr,  // TTF_Font*
	outline: bool,
}

#assert(size_of(ttfTextHash_t) == 32)
