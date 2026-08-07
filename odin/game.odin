// game.odin — Odin bindings to the C++ game library (barony_game.dll).
//
// Mirrors game.cpp: that file's `main()` was renamed to barony_main and
// exported extern "C". This file declares the binding and imports the game
// import lib. Only compiled into the game build (not the editor).
package main

when !#config(EDITOR, false) {
	foreign import _barony "../builddir/src/barony_game.lib"

	foreign _barony {
		barony_main :: proc(argc: i32, argv: [^]cstring) -> i32 ---
	}
}
