// main.odin — Odin driver entry point for Barony (game or editor) + mirrors of main.hpp.
//
// The driver owns the real process main() and calls into the C++
// barony_game.dll (or barony_editor.dll with -define:EDITOR=true), which runs
// the whole game (init -> loop -> shutdown) exactly like the original C++
// main() did. As the port progresses, code moves from the DLLs into Odin
// and this driver grows.
//
// The type mirrors (constants, enums, core structs from main.hpp) live here
// in the same file — one header, one odin file.

package main

import "core:c"
import "core:fmt"
import "core:mem"
import "core:os"
import "containers"

main :: proc() {
	rc := run_barony()
	if rc != 0 {
		fmt.eprintf("barony exited with code %d\n", rc)
	}
	os.exit(rc)
}

// run_barony drives barony_main (declared in game.odin or editor.odin, the
// active binding for this build) with the real process argv. Odin strings are
// not NUL-terminated, so each arg is copied into a NUL-terminated cstring
// buffer (the C++ main() expects a normal char**). Returns the exit code.
run_barony :: proc() -> int {
	n := len(os.args)
	argv_buf := make([]cstring, n + 1, context.allocator)
	defer delete(argv_buf, context.allocator)

	arg_copies := make([][]byte, n, context.allocator)
	defer {
		for copy in arg_copies {
			delete(copy, context.allocator)
		}
		delete(arg_copies, context.allocator)
	}

	for arg, i in os.args {
		arg_copies[i] = make([]byte, len(arg) + 1, context.allocator)
		mem.copy(raw_data(arg_copies[i]), raw_data(arg), len(arg))
		arg_copies[i][len(arg)] = 0
		argv_buf[i] = cstring(raw_data(arg_copies[i]))
	}
	argv_buf[n] = nil

	rc := barony_main(c.int(len(os.args)), raw_data(argv_buf))
	return int(rc)
}

// ---------------------------------------------------------------------------
// Constants (mirror main.hpp #defines)
// ---------------------------------------------------------------------------
PI             :: 3.14159265358979323846
MAXPLAYERS     :: 4 // BARONY_SUPER_MULTIPLAYER not defined in this build
CLIPNEAR       :: 2
CLIPFAR        :: 4000
TEXTURESIZE    :: 32
TEXTUREPOWER   :: 5
MAXTRIES       :: 6
MAXDELETES     :: 2
HORIZONTAL     :: 1
VERTICAL       :: 2

// input indices (IN_* from main.hpp)
IN_FORWARD :: 0; IN_LEFT :: 1; IN_BACK :: 2; IN_RIGHT :: 3; IN_TURNL :: 4; IN_TURNR :: 5
IN_UP :: 6; IN_DOWN :: 7; IN_CHAT :: 8; IN_COMMAND :: 9; IN_STATUS :: 10; IN_SPELL_LIST :: 11
IN_CAST_SPELL :: 12; IN_DEFEND :: 13; IN_ATTACK :: 14; IN_USE :: 15; IN_AUTOSORT :: 16; IN_MINIMAPSCALE :: 17
IN_TOGGLECHATLOG :: 18; IN_FOLLOWERMENU :: 19; IN_FOLLOWERMENU_LASTCMD :: 20; IN_FOLLOWERMENU_CYCLENEXT :: 21
IN_HOTBAR_SCROLL_LEFT :: 22; IN_HOTBAR_SCROLL_RIGHT :: 23; IN_JOYPAD_LEFT :: 24; IN_JOYPAD_RIGHT :: 25

// map_t tile attribute flags (map_t::TILE_ATTRIBUTE_*)
TILE_ATTRIBUTE_NODIG          :: u32(1) << 0
TILE_ATTRIBUTE_SLIPPERY       :: u32(1) << 1
TILE_ATTRIBUTE_SLOW           :: u32(1) << 2
TILE_ATTRIBUTE_GREASE         :: u32(1) << 3
TILE_ATTRIBUTE_TREASURE_ROOM  :: u32(1) << 4

// ---------------------------------------------------------------------------
// Enums (mirror main.hpp)
// ---------------------------------------------------------------------------
Light_Modifier_Values :: enum i32 {
	GLOBAL_LIGHT_MODIFIER_STOPPED,
	GLOBAL_LIGHT_MODIFIER_INUSE,
	GLOBAL_LIGHT_MODIFIER_DISSIPATING,
}

Door_Dir :: enum i32 { // enum DoorDir : Sint32
	DIR_EAST,
	DIR_SOUTH,
	DIR_WEST,
	DIR_NORTH,
}

Door_Edge :: enum i32 { // enum DoorEdge : Sint32
	EDGE_EAST,
	EDGE_SOUTHEAST,
	EDGE_SOUTH,
	EDGE_SOUTHWEST,
	EDGE_WEST,
	EDGE_NORTHWEST,
	EDGE_NORTH,
	EDGE_NORTHEAST,
}

// ---------------------------------------------------------------------------
// Core structs (mirror main.hpp)
// ---------------------------------------------------------------------------

// typedef struct vec4 { float x,y,z,w; } vec4_t;  — 16 bytes
vec4_t :: struct {
	x: f32,
	y: f32,
	z: f32,
	w: f32,
}

// typedef struct mat4x4 { vec4_t x,y,z,w; } mat4x4_t;  — 64 bytes
mat4x4_t :: struct {
	x: vec4_t,
	y: vec4_t,
	z: vec4_t,
	w: vec4_t,
}

// typedef struct node_t — 48 bytes
node_t :: struct {
	next:          ^node_t,
	prev:          ^node_t,
	list:          ^list_t,
	element:       rawptr,
	deconstructor: proc(rawptr), // void (*deconstructor)(void* data)
	size:          u32,
}

// typedef struct list_t { node_t* first; node_t* last; }  — 16 bytes
list_t :: struct {
	first: ^node_t,
	last:  ^node_t,
}

// typedef struct map_t  — 480 bytes
map_t :: struct {
	name:                  [32]u8, // char name[32]
	author:                [32]u8, // char author[32]
	width:                 u32,
	height:                u32,
	skybox:                u32,
	flags:                 [16]i32, // Sint32 flags[16]
	tiles:                 ^i32,    // Sint32* tiles
	entities_map:          map[[4]byte]^node_t, // DynamicMapI32T<node_t*> (i32-keyed)
	entities:              ^list_t,
	creatures:             ^list_t,
	worldUI:               ^list_t,
	trapexcludelocations:  ^bool,
	monsterexcludelocations: ^bool,
	lootexcludelocations:  ^bool,
	liquidSfxPlayedTiles:  map[i32]struct{}, // DynamicSetI32
	tileAttributes:        map[[4]byte]u32, // DynamicMapI32T<Uint32> (i32-keyed)
	filename:              [256]u8, // char filename[256]
}

// typedef struct deleteent_t { Uint32 uid; Uint32 tries; }  — 8 bytes
deleteent_t :: struct {
	uid:   u32,
	tries: u32,
}

// typedef struct hit_t  — 40 bytes
hit_t :: struct {
	x:      f64, // real_t
	y:      f64,
	mapx:   i32,
	mapy:   i32,
	entity: ^Entity,
	side:   i32,
}

// typedef struct button_t  — 80 bytes
button_t :: struct {
	label:    [32]u8, // char label[32]
	x:        i32,
	y:        i32,
	sizex:    u32,
	sizey:    u32,
	visible:  u8,
	focused:  u8,
	key:      i32, // SDL_Keycode (int32)
	joykey:   i32, // int
	pressed:  bool,
	needclick: bool,
	outline:  bool,
	node:     ^node_t,
	action:   proc(^button_t), // void (*action)(struct button_t* my)
}

// typedef struct voxel_t  — 792 bytes
voxel_t :: struct {
	sizex:   i32,
	sizey:   i32,
	sizez:   i32,
	data:    ^u8,
	palette: [256][3]u8,
}

// typedef struct vertex_t { real_t x,y,z; }  — 24 bytes
vertex_t :: struct {
	x: f64,
	y: f64,
	z: f64,
}

// typedef struct polyquad_t  — 104 bytes
polyquad_t :: struct {
	vertex: [4]vertex_t,
	r: u8,
	g: u8,
	b: u8,
	side: i32,
}

// typedef struct polytriangle_t  — 104 bytes
polytriangle_t :: struct {
	vertex: [3]vertex_t,
	normal: vertex_t,
	r: u8,
	g: u8,
	b: u8,
}

// typedef struct polymodel_t  — 32 bytes
polymodel_t :: struct {
	faces:     ^polytriangle_t,
	numfaces:  u64,
	vao:       u32, // GLuint
	positions: u32,
	colors:    u32,
	normals:   u32,
}

// typedef struct string_t  — 40 bytes
string_t :: struct {
	lines:  u32,
	data:   ^u8, // char* data
	node:   ^node_t,
	color:  u32,
	time:   u32,
	player: i32, // int player = -1
}

// typedef struct door_t  — 16 bytes
door_t :: struct {
	x:    i32,
	y:    i32,
	dir:  Door_Dir,  // enum DoorDir : Sint32
	edge: Door_Edge, // enum DoorEdge : Sint32
}

// struct cameravars_t  — 24 bytes
cameravars_t :: struct {
	shakex:  f64, // real_t
	shakex2: f64,
	shakey:  i32,
	shakey2: i32,
}

// struct AnimatedTile { int indices[8]; }  — 32 bytes
Animated_Tile :: struct {
	indices: [8]i32,
}


// ---------------------------------------------------------------------------
// Layout parity checks (C++ sizeof from the real headers, verified by probe)
// ---------------------------------------------------------------------------
#assert(size_of(vec4_t) == 16)
#assert(size_of(mat4x4_t) == 64)
#assert(size_of(node_t) == 48)
#assert(size_of(list_t) == 16)
#assert(size_of(deleteent_t) == 8)
#assert(size_of(hit_t) == 40)
#assert(size_of(button_t) == 80)
#assert(size_of(voxel_t) == 792)
#assert(size_of(vertex_t) == 24)
#assert(size_of(polyquad_t) == 104)
#assert(size_of(polytriangle_t) == 104)
#assert(size_of(polymodel_t) == 32)
#assert(size_of(string_t) == 40)
#assert(size_of(door_t) == 16)
#assert(size_of(cameravars_t) == 24)
#assert(size_of(Animated_Tile) == 32)
// map_t size assert is below (map members are native map[[4]byte]V / map[i32]struct{})
// #assert(size_of(map_t) == 552)
