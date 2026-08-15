// game.odin — Odin bindings to the C++ game code + mirrors of game.hpp types.
//
// Mirrors game.cpp: that file's `main()` was renamed to barony_main and given
// C linkage. The C++ game code is compiled into a static library
// (libbarony_game.a) that is linked into this executable via /WHOLEARCHIVE
// (see src/meson.build). The foreign import references the archive so Odin
// emits the symbol reference; /WHOLEARCHIVE guarantees all objects are pulled
// in (so ported Odin procs can be called from C++ and vice versa).
package main
import "containers"
import "json_shim"

when !#config(EDITOR, false) {
	foreign import _barony "../builddir/src/libbarony_game.a"

	foreign _barony {
		barony_main :: proc(argc: i32, argv: [^]cstring) -> i32 ---
	}
}

// ---------------------------------------------------------------------------
// game.hpp types
// ---------------------------------------------------------------------------

// typedef struct packetsend_t — 40 bytes
packetsend_t :: struct {
	sock:    rawptr, // UDPsocket
	channel: i32,
	packet:  rawptr, // UDPpacket*
	num:     i32,
	tries:   i32,
	hostnum: i32,
}
#assert(size_of(packetsend_t) == 40)

// class TileEntityListHandler — 262144 bytes
// { int gridEntities[256][256]; }
TileEntityListHandler :: struct {
	grid_entities: [256][256]i32,
}
#assert(size_of(TileEntityListHandler) == 262144)

// class TimerExperiments — statics only (no per-instance data). State and
// EntityStates are defined in entity.odin.
