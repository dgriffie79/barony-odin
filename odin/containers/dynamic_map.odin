// dynamic_map.odin — C-ABI shim for maps, callable from C++.
//
// Replaces std::map / std::unordered_map in shared structs. Uses ODIN'S NATIVE
// map (hash map) — not a hand-rolled container — so growth/hash/equal are the
// battle-tested builtins. The C++ mirror is Raw_Map {data,len,allocator} (32
// bytes on x64).
//
// Key strategy (from the de-STL ordering: strings are replaced BEFORE maps):
//   - numeric keys (int/Uint32/enum, 4 bytes): map[[4]byte]V  — C++ passes the
//     key as 4 raw bytes
//   - string keys (Item.attributes etc.): map[string]V — C++ passes a
//     DynamicString {data,len}, which is ABI-identical to Odin's string, so the
//     shim receives it directly as `string`.
package containers

import "core:runtime"

// ---------------------------------------------------------------------------
// Integer-keyed maps (key = 4-byte blob). Value types get one instantiation
// each. C++ mirrors Raw_Map (32 bytes).
// ---------------------------------------------------------------------------

// int -> int
@(export)
barony_dynamic_map_i32i32_init :: proc "c" (m: ^map[[4]byte]int) {
	context = runtime.default_context()
	m^ = nil
}
@(export)
barony_dynamic_map_i32i32_put :: proc "c" (m: ^map[[4]byte]int, key: ^[4]byte, value: int) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[[4]byte]int)
	}
	m[key^] = value
}
@(export)
barony_dynamic_map_i32i32_get :: proc "c" (m: ^map[[4]byte]int, key: ^[4]byte, out: ^int) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key^]
	if ok {
		out^ = v
	}
	return ok
}
@(export)
barony_dynamic_map_i32i32_erase :: proc "c" (m: ^map[[4]byte]int, key: ^[4]byte) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key^]
	runtime.delete_key(m, key^)
	return had
}
@(export)
barony_dynamic_map_i32i32_clear :: proc "c" (m: ^map[[4]byte]int) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}
@(export)
barony_dynamic_map_i32i32_len :: proc "c" (m: ^map[[4]byte]int) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}
@(export)
barony_dynamic_map_i32i32_destroy :: proc "c" (m: ^map[[4]byte]int) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}

// Uint32 -> Uint32
@(export)
barony_dynamic_map_u32u32_put :: proc "c" (m: ^map[[4]byte]u32, key: ^[4]byte, value: u32) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[[4]byte]u32)
	}
	m[key^] = value
}
@(export)
barony_dynamic_map_u32u32_get :: proc "c" (m: ^map[[4]byte]u32, key: ^[4]byte, out: ^u32) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key^]
	if ok {
		out^ = v
	}
	return ok
}
@(export)
barony_dynamic_map_u32u32_erase :: proc "c" (m: ^map[[4]byte]u32, key: ^[4]byte) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key^]
	runtime.delete_key(m, key^)
	return had
}
@(export)
barony_dynamic_map_u32u32_clear :: proc "c" (m: ^map[[4]byte]u32) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}
@(export)
barony_dynamic_map_u32u32_len :: proc "c" (m: ^map[[4]byte]u32) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}
@(export)
barony_dynamic_map_u32u32_destroy :: proc "c" (m: ^map[[4]byte]u32) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}

// ---------------------------------------------------------------------------
// String-keyed maps (key = DynamicString {data,len} == Odin string).
// C++ passes the 16-byte DynamicString by value; it IS an Odin string.
// ---------------------------------------------------------------------------

// string -> int
@(export)
barony_dynamic_map_strint_init :: proc "c" (m: ^map[string]int) {
	context = runtime.default_context()
	m^ = nil
}
@(export)
barony_dynamic_map_strint_put :: proc "c" (m: ^map[string]int, key: string, value: int) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]int)
	}
	m[key] = value
}
@(export)
barony_dynamic_map_strint_get :: proc "c" (m: ^map[string]int, key: string, out: ^int) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		out^ = v
	}
	return ok
}
@(export)
barony_dynamic_map_strint_erase :: proc "c" (m: ^map[string]int, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key]
	runtime.delete_key(m, key)
	return had
}
@(export)
barony_dynamic_map_strint_clear :: proc "c" (m: ^map[string]int) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}
@(export)
barony_dynamic_map_strint_len :: proc "c" (m: ^map[string]int) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}
@(export)
barony_dynamic_map_strint_destroy :: proc "c" (m: ^map[string]int) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}

// string -> i32 (for Stat/Item attributes: map<string, Sint32>)
@(export)
barony_dynamic_map_stri32_put :: proc "c" (m: ^map[string]i32, key: string, value: i32) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]i32)
	}
	m[key] = value
}
@(export)
barony_dynamic_map_stri32_get :: proc "c" (m: ^map[string]i32, key: string, out: ^i32) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		out^ = v
	}
	return ok
}
@(export)
barony_dynamic_map_stri32_erase :: proc "c" (m: ^map[string]i32, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key]
	runtime.delete_key(m, key)
	return had
}
@(export)
barony_dynamic_map_stri32_clear :: proc "c" (m: ^map[string]i32) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}
@(export)
barony_dynamic_map_stri32_len :: proc "c" (m: ^map[string]i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}
@(export)
barony_dynamic_map_stri32_destroy :: proc "c" (m: ^map[string]i32) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}
