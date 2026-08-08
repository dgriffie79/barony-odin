// dynamic_string.odin — C-ABI shim for a {data,len} string, callable from C++.
//
// Replaces std::string in shared structs. Layout matches Odin's `string`
// ({data: [^]byte, len: int}, 16 bytes). The data is always NUL-terminated
// (data[len] == 0), so c_str() is O(1) and stable while the string isn't
// mutated.
//
// CRITICAL: these procs are called from C++ where the Odin runtime context is
// NOT initialized, so every allocating proc sets
// `context = runtime.default_context()` first. mem.alloc uses
// context.allocator by default, so without this the allocator is garbage.
package containers

import "core:mem"
import "core:runtime"

DynamicString :: struct {
	data: rawptr, // [^]u8, always has data[len] == 0 (NUL-terminated)
	len:  int,
}

// Zero the struct (no allocation). Mirrors default-construction.
@(export)
barony_dynamic_string_init :: proc "c" (s: ^DynamicString) {
	s^ = DynamicString{}
}

// Make from a NUL-terminated C string. Copies (destroys old contents first).
@(export)
barony_dynamic_string_from_cstr :: proc "c" (s: ^DynamicString, cstr: cstring) {
	context = runtime.default_context()
	barony_dynamic_string_destroy(s)
	if cstr == nil {
		return
	}
	n := runtime.cstring_len(cstr) // bytes before NUL
	buf, _ := mem.alloc(n + 1, align_of(u8))
	if buf == nil {
		return
	}
	runtime.mem_copy(buf, rawptr(cstr), n)
	(^u8)(uintptr(buf) + uintptr(n))^ = 0
	s.data = buf
	s.len = n
}

// Make from a raw byte range (not NUL-terminated input). Copies.
@(export)
barony_dynamic_string_from_bytes :: proc "c" (s: ^DynamicString, bytes: rawptr, n: int) {
	context = runtime.default_context()
	barony_dynamic_string_destroy(s)
	if bytes == nil || n <= 0 {
		return
	}
	buf, _ := mem.alloc(n + 1, align_of(u8))
	if buf == nil {
		return
	}
	runtime.mem_copy(buf, bytes, n)
	(^u8)(uintptr(buf) + uintptr(n))^ = 0
	s.data = buf
	s.len = n
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
	// grow: alloc to len + n + 1
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

// Clear: free the buffer and zero the struct (std::string::clear keeps
// capacity, but we have no cap field — freeing is the safe equivalent).
@(export)
barony_dynamic_string_clear :: proc "c" (s: ^DynamicString) {
	context = runtime.default_context()
	if s.data != nil {
		mem.free(s.data)
	}
	s^ = DynamicString{}
}

// Deep copy: dst becomes an independent copy of src (std::string copy-assign).
// If dst has its own buffer, it's freed first (copy-assign semantics).
@(export)
barony_dynamic_string_copy :: proc "c" (dst: ^DynamicString, src: ^DynamicString) {
	context = runtime.default_context()
	if dst == src {
		return
	}
	barony_dynamic_string_destroy(dst)
	if src == nil || src.data == nil || src.len == 0 {
		dst^ = DynamicString{}
		return
	}
	buf, _ := mem.alloc(src.len + 1, align_of(u8))
	if buf == nil {
		return
	}
	runtime.mem_copy(buf, src.data, src.len)
	(^u8)(uintptr(buf) + uintptr(src.len))^ = 0
	dst.data = buf
	dst.len = src.len
}

// Equal: compare contents (std::string::operator==).
@(export)
barony_dynamic_string_equal :: proc "c" (a: ^DynamicString, b: ^DynamicString) -> bool {
	if a == nil || b == nil {
		return a == b
	}
	if a.len != b.len {
		return false
	}
	if a.len == 0 {
		return true
	}
	return mem.compare_byte_ptrs((^byte)(a.data), (^byte)(b.data), a.len) == 0
}

// Compare to a NUL-terminated C string (std::string == "literal").
@(export)
barony_dynamic_string_equal_cstr :: proc "c" (a: ^DynamicString, b: cstring) -> bool {
	if a == nil {
		return b == nil
	}
	if b == nil {
		return a.len == 0
	}
	n := runtime.cstring_len(b)
	if a.len != n {
		return false
	}
	if n == 0 {
		return true
	}
	return mem.compare_byte_ptrs((^byte)(a.data), (^byte)(b), n) == 0
}

// Compare contents (std::string::compare / <, >, <=, >= for map keys).
// Returns <0, 0, >0.
@(export)
barony_dynamic_string_compare :: proc "c" (a: ^DynamicString, b: ^DynamicString) -> i32 {
	if a == nil || b == nil {
		return 0
	}
	min_len := min(a.len, b.len)
	if min_len > 0 {
		c := mem.compare_byte_ptrs((^byte)(a.data), (^byte)(b.data), min_len)
		if c != 0 {
			return i32(c)
		}
	}
	if a.len < b.len {
		return -1
	} else if a.len > b.len {
		return 1
	}
	return 0
}

// Find first occurrence of `needle` (bytes) starting at `start`.
// Returns byte offset or -1 (std::string::find).
@(export)
barony_dynamic_string_find :: proc "c" (s: ^DynamicString, needle: rawptr, n: int, start: int) -> i64 {
	if s == nil || needle == nil || n <= 0 || start < 0 {
		return -1
	}
	if start > s.len {
		return -1
	}
	if n > s.len - start {
		return -1
	}
	hay := ([^]u8)(s.data)
	ned := ([^]u8)(needle)
	for i in start..<(s.len - n + 1) {
		match := true
		for j in 0..<n {
			if hay[i + j] != ned[j] {
				match = false
				break
			}
		}
		if match {
			return i64(i)
		}
	}
	return -1
}

// Substring: copy [start, start+n) into dst (std::string::substr).
@(export)
barony_dynamic_string_substr :: proc "c" (dst: ^DynamicString, src: ^DynamicString, start: int, n: int) {
	context = runtime.default_context()
	barony_dynamic_string_destroy(dst)
	if src == nil || src.data == nil || start < 0 || n <= 0 {
		dst^ = DynamicString{}
		return
	}
	if start >= src.len {
		dst^ = DynamicString{}
		return
	}
	clamp_n := min(n, src.len - start)
	buf, _ := mem.alloc(clamp_n + 1, align_of(u8))
	if buf == nil {
		return
	}
	runtime.mem_copy(buf, ([^]u8)(uintptr(src.data) + uintptr(start)), clamp_n)
	(^u8)(uintptr(buf) + uintptr(clamp_n))^ = 0
	dst.data = buf
	dst.len = clamp_n
}

@(export)
barony_dynamic_string_destroy :: proc "c" (s: ^DynamicString) {
	context = runtime.default_context()
	if s.data != nil {
		mem.free(s.data)
	}
	s^ = DynamicString{}
}
