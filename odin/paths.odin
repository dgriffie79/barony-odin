// paths.odin - Odin mirrors of paths.hpp.
package main

// typedef struct pathnode - 24 bytes
// { Sint32 x,y; Uint32 g,h; Sint32 px,py; }
pathnode_t :: struct {
	x:  i32,
	y:  i32,
	g:  u32,
	h:  u32,
	px: i32,
	py: i32,
}

#assert(size_of(pathnode_t) == 24)

Generate_Path_Types :: enum i32 {
	GENERATE_PATH_DEFAULT,
	GENERATE_PATH_SOKOBAN,
	GENERATE_PATH_TREASURE_ROOM,
}


// ---------------------------------------------------------------------------
// Globals owned by Odin (C++ references via extern "C")
// ---------------------------------------------------------------------------
@(export)
pathMapFlying : ^i32

@(export)
pathMapGrounded : ^i32

@(export)
pathMapZone : i32

@(export)
lastGeneratePathTries : i32
