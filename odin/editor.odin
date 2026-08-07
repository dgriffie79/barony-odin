// editor.odin — Odin bindings to the C++ editor library (barony_editor.dll).
//
// Mirrors editor.cpp: that file's `main()` was renamed to barony_main and
// exported extern "C". This file declares the binding and imports the editor
// import lib. Only compiled into the editor build (not the game).
package main

when #config(EDITOR, false) {
	foreign import _barony "../builddir/src/barony_editor.lib"

	foreign _barony {
		barony_main :: proc(argc: i32, argv: [^]cstring) -> i32 ---
	}
}
