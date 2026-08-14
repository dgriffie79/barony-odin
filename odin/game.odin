// game.odin — Odin bindings to the C++ game code.
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
