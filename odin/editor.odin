// editor.odin — Odin bindings to the C++ editor code.
//
// Mirrors editor.cpp: that file's `main()` was renamed to barony_main and
// given C linkage. The C++ editor code is compiled into a static library
// (libbarony_editor.a) that is linked into this executable via /WHOLEARCHIVE
// (see src/meson.build). The foreign import references the archive so Odin
// emits the symbol reference; /WHOLEARCHIVE guarantees all objects are pulled
// in (so ported Odin procs can be called from C++ and vice versa).
package main

when #config(EDITOR, false) {
	foreign import _barony "../builddir/src/libbarony_editor.a"

	foreign _barony {
		barony_main :: proc(argc: i32, argv: [^]cstring) -> i32 ---
	}
}
