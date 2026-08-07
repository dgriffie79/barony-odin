// main.odin — Odin driver entry point for Barony (game or editor).
//
// Mirrors main.cpp (the process entry / globals in C++). Odin owns the real
// process main(); it calls into the C++ barony_game.dll (or barony_editor.dll
// with -define:EDITOR=true), which runs the whole game (init -> loop ->
// shutdown) exactly like the original C++ main() did. As the port progresses,
// code moves from the DLLs into Odin and this driver grows.
package main

import "core:c"
import "core:fmt"
import "core:mem"
import "core:os"

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
