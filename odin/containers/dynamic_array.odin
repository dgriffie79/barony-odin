// containers.odin — C-ABI shim for Odin dynamic arrays, callable from C++.
//
// The C++ reference is being de-STL'd: std::vector members become a plain
// `{data,len,cap,alloc}` struct matching Odin's Raw_Dynamic_Array layout, and
// C++ calls these exported procs (one-way C++ -> Odin) instead of carrying its
// own vector implementation. When a C++ file is ported to Odin, its shim calls
// become native `append(...)` etc. — identical behavior, so there is no drift.
//
// CRITICAL: these procs are called from C++ where the Odin runtime context is
// NOT initialized, so each proc sets `context = runtime.default_context()` first
// (matching how Odin's own entry points do it). This makes context.allocator
// valid for native append growth.
//
// The C++ side must NEVER grow the array itself (no realloc) — it only reads
// .data/.len/.cap and calls these shims to grow. Odin owns the allocator.
package containers

import "core:mem"
import "base:runtime"

// Matches Odin's Raw_Dynamic_Array (base/runtime/core.odin) — C++ mirrors this.
Raw_Dynamic_Array :: struct {
	data:      rawptr,
	len:       int,
	cap:       int,
	allocator: mem.Allocator,
}

@(export)
barony_dynamic_array_init :: proc "c" (arr: ^Raw_Dynamic_Array) {
	arr^ = Raw_Dynamic_Array{}
}

// Append one element (elem_size bytes) — uses native append on a transmuted
// [dynamic]u8, so growth + allocator exactly match the ported Odin code.
@(export)
barony_dynamic_array_append :: proc "c" (arr: ^Raw_Dynamic_Array, elem: rawptr, elem_size: int) -> i32 {
	context = runtime.default_context()
	if elem == nil || elem_size <= 0 {
		return 0
	}
	a := transmute(^[dynamic]u8)(arr)
	append(a, ..([^]u8)(elem)[:elem_size])
	return i32(arr.len)
}

// Free the array via native delete (stored allocator).
@(export)
barony_dynamic_array_destroy :: proc "c" (arr: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	runtime.delete_dynamic_array(transmute([dynamic]u8)arr^)
	arr^ = Raw_Dynamic_Array{}
}

// Deep copy: dst becomes an independent copy of src's entire buffer.
// std::vector copies deep; a plain {data,len,cap} struct copies shallow, so
// without this shim, dst.data == src.data would double-free. Copying via
// native append keeps the allocator field consistent with the other shims.
@(export)
barony_dynamic_array_copy :: proc "c" (dst: ^Raw_Dynamic_Array, src: ^Raw_Dynamic_Array) -> i32 {
	context = runtime.default_context()
	// free any existing dst buffer first (std::vector copy-assign clears dst)
	if dst.data != nil {
		runtime.delete_dynamic_array(transmute([dynamic]u8)dst^)
	}
	dst^ = Raw_Dynamic_Array{}
	if src == nil || src.len == 0 || src.data == nil {
		return 1
	}
	d := transmute(^[dynamic]u8)(dst)
	s := ([^]u8)(src.data)
	append(d, ..s[:src.len])
	return 1
}

// Erase element at index (preserves order, like std::vector::erase).
// Elements after index shift left; the next element lands at the same index,
// so callers stay at `index` after erasing (no return needed).
@(export)
barony_dynamic_array_erase :: proc "c" (arr: ^Raw_Dynamic_Array, index: i32, elem_size: int) {
	context = runtime.default_context()
	a := transmute(^[dynamic]u8)(arr)
	runtime.remove_range_dynamic_array(a, int(index) * elem_size, (int(index) + 1) * elem_size)
}

// Insert element at index (preserves order, like std::vector::insert).
// Odin has no insert builtin, so grow via append then memmove right.
@(export)
barony_dynamic_array_insert :: proc "c" (arr: ^Raw_Dynamic_Array, index: i32, elem: rawptr, elem_size: int) -> i32 {
	context = runtime.default_context()
	if elem == nil || elem_size <= 0 {
		return 0
	}
	a := transmute(^[dynamic]u8)(arr)
	n := int(index)
	if n < 0 {
		n = 0
	}
	if n > arr.len {
		n = arr.len
	}
	// append elem_size placeholder bytes to grow the buffer, then shift right
	placeholders: [16]u8 = 0
	for append_byte in 0..<elem_size {
		append(a, placeholders[append_byte % 16])
	}
	// memmove elements [n .. len-2] right by one elem_size
	if n < arr.len - 1 {
		dst := ([^]u8)(uintptr(arr.data) + uintptr(n+1) * uintptr(elem_size))
		src := ([^]u8)(uintptr(arr.data) + uintptr(n) * uintptr(elem_size))
		runtime.mem_copy(dst, src, elem_size * (arr.len - n - 1))
	}
	// write elem at n
	dst := ([^]u8)(uintptr(arr.data) + uintptr(n) * uintptr(elem_size))
	mem.copy(dst, elem, elem_size)
	return i32(arr.len)
}

// Clear: set len=0, keep capacity (std::vector::clear).
@(export)
barony_dynamic_array_clear :: proc "c" (arr: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	a := transmute(^[dynamic]u8)(arr)
	runtime.clear_dynamic_array(a)
}

// Resize to new_len (grow zero-fills, shrink truncates — std::vector::resize).
@(export)
barony_dynamic_array_resize :: proc "c" (arr: ^Raw_Dynamic_Array, elem_size: int, new_len: i32) -> i32 {
	context = runtime.default_context()
	a := transmute(^[dynamic]u8)(arr)
	runtime.resize_dynamic_array(a, int(new_len) * elem_size)
	return i32(arr.len)
}

// Pop back: remove last element (std::vector::pop_back).
@(export)
barony_dynamic_array_pop_back :: proc "c" (arr: ^Raw_Dynamic_Array, elem_size: int) {
	context = runtime.default_context()
	if arr.len >= elem_size {
		arr.len -= elem_size
	}
}

// Test/verification: sum the first `count` i32s — proves Odin reads what C++
// wrote (and vice versa).
@(export)
barony_dynamic_array_sum_ints :: proc "c" (arr: ^Raw_Dynamic_Array, count: i32) -> i64 {
	if arr.data == nil {
		return 0
	}
	data := ([^]i32)(arr.data)
	sum: i64 = 0
	for i in 0..<count {
		sum += i64(data[i])
	}
	return sum
}
