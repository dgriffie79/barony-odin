// dynamic_string.odin — C-ABI shim for a {data,len} string, callable from C++.
//
// Replaces std::string in shared structs. Layout matches Odin's `string`
// ({data: [^]byte, len: int}, 16 bytes). The data is NOT NUL-terminated by
// itself; c_str() provides a NUL-terminated view via a scratch buffer.
//
// C++ callers that need a `const char*` for C functions use
// barony_dynamic_string_c_str, which ensures data[len] is a NUL (reserving
// one extra byte at alloc time) — so c_str() is O(1) and stable while the
// string isn't mutated.
package containers

import "core:mem"
import "core:runtime"

DynamicString :: struct {
	data: rawptr, // [^]u8, always has data[len] == 0 (NUL-terminated)
	len:  int,
}

@(export)
barony_dynamic_string_init :: proc "c" (s: ^DynamicString) {
	s^ = DynamicString{}
}

// Make from a NUL-terminated C string. Copies.
@(export)
barony_dynamic_string_from_cstr :: proc "c" (s: ^DynamicString, cstr: cstring) {
	context = runtime.default_context()
	barony_dynamic_string_destroy(s)
	if cstr == nil {
		return
	}
	n := runtime.cstring_len(cstr) // bytes before NUL
	// allocate n + 1 (room for NUL)
	buf, _ := mem.alloc(n + 1, align_of(u8))
	if buf == nil {
		return
	}
	runtime.mem_copy(buf, rawptr(cstr), n)
	(^u8)(uintptr(buf) + uintptr(n))^ = 0
	s.data = buf
	s.len = n
}

// Set length (for strings built byte-by-byte). Caller must have reserved
// capacity via from_cstr/with_capacity; we just track len and keep NUL.
@(export)
barony_dynamic_string_set_len :: proc "c" (s: ^DynamicString, new_len: i32) {
	l := new_len
	if l < 0 {
		l = 0
	}
	s.len = int(l)
	if s.data != nil {
		(^u8)(uintptr(s.data) + uintptr(s.len))^ = 0
	}
}

// c_str(): returns the NUL-terminated buffer (stable while not mutated).
@(export)
barony_dynamic_string_c_str :: proc "c" (s: ^DynamicString) -> cstring {
	return cstring(s.data)
}

// Append bytes from a C buffer. Grows as needed. Keeps NUL termination.
@(export)
barony_dynamic_string_append :: proc "c" (s: ^DynamicString, bytes: rawptr, n: int) {
	context = runtime.default_context()
	if bytes == nil || n <= 0 {
		return
	}
	// grow: realloc to len + n + 1
	new_len := s.len + n
	buf, _ := mem.alloc(new_len + 1, align_of(u8))
	if buf == nil {
		return
	}
	if s.data != nil {
		runtime.mem_copy(buf, s.data, s.len)
		mem.free(s.data)
	}
	runtime.mem_copy(([^]u8)(uintptr(buf) + uintptr(s.len)), bytes, n)
	(^u8)(uintptr(buf) + uintptr(new_len))^ = 0
	s.data = buf
	s.len = new_len
}

@(export)
barony_dynamic_string_destroy :: proc "c" (s: ^DynamicString) {
	context = runtime.default_context()
	if s.data != nil {
		mem.free(s.data)
	}
	s^ = DynamicString{}
}
