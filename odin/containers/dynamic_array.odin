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

// ---------------------------------------------------------------------------
// DynamicArray of DynamicString (std::vector<DynamicString> replacement).
// Each element is a DynamicString {data,len} — the element OWNS its buffer.
// Raw DynamicArray ops just move bytes; these shims deep-free elements on
// clear/erase/destroy/pop_back and deep-copy on copy/insert, so ownership
// is correct on both sides. Element size is always 16 (Raw_String).
// ---------------------------------------------------------------------------
dynamic_string_free_elem :: proc(s: ^DynamicString) {
	if s.data != nil {
		mem.free(s.data)
		s.data = nil
	}
}

dynamic_string_copy_elem :: proc(dst: ^DynamicString, src: ^DynamicString) {
	dst^ = DynamicString{}
	if src.len > 0 {
		buf, _ := mem.alloc(src.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, src.data, src.len)
			(^u8)(uintptr(buf) + uintptr(src.len))^ = 0
			dst^ = DynamicString{ data = buf, len = src.len }
		}
	}
}

arrstr_free_elements :: proc(a: ^Raw_Dynamic_Array, from: int) {
	if a.data == nil {
		return
	}
	elems := ([^]DynamicString)(a.data)
	n := a.len / size_of(DynamicString)
	for i := from; i < n; i += 1 {
		dynamic_string_free_elem(&elems[i])
	}
}

@(export)
barony_dynamic_array_str_init :: proc "c" (a: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	a^ = Raw_Dynamic_Array{}
}

// append one DynamicString element (deep-copied into array-owned storage)
@(export)
barony_dynamic_array_str_append :: proc "c" (a: ^Raw_Dynamic_Array, elem: ^DynamicString) {
	context = runtime.default_context()
	if elem == nil {
		return
	}
	raw := transmute(^[dynamic]u8)(a)
	// grow by one DynamicString slot
	if a.cap - a.len < size_of(DynamicString) {
		runtime.reserve_dynamic_array(transmute(^[dynamic]u8)(a), a.len + size_of(DynamicString))
	}
	// deep-copy the element into the new slot
	slot := ([^]DynamicString)(uintptr(a.data) + uintptr(a.len))
	dynamic_string_copy_elem(slot, elem)
	a.len += size_of(DynamicString)
}

// get: deep-copy the element at index into out (out is C++ RAII)
@(export)
barony_dynamic_array_str_get :: proc "c" (a: ^Raw_Dynamic_Array, index: i32, out: ^DynamicString) -> bool {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*size_of(DynamicString) >= a.len {
		return false
	}
	elems := ([^]DynamicString)(a.data)
	dynamic_string_copy_elem(out, &elems[index])
	return true
}

// set: replace element at index (frees old, deep-copies new)
@(export)
barony_dynamic_array_str_set :: proc "c" (a: ^Raw_Dynamic_Array, index: i32, elem: ^DynamicString) {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*size_of(DynamicString) >= a.len {
		return
	}
	elems := ([^]DynamicString)(a.data)
	dynamic_string_free_elem(&elems[index])
	dynamic_string_copy_elem(&elems[index], elem)
}

// erase: free the element at index, shift left
@(export)
barony_dynamic_array_str_erase :: proc "c" (a: ^Raw_Dynamic_Array, index: i32) {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*size_of(DynamicString) >= a.len {
		return
	}
	elems := ([^]DynamicString)(a.data)
	dynamic_string_free_elem(&elems[index])
	n := a.len / size_of(DynamicString)
	for i := int(index); i < n - 1; i += 1 {
		elems[i] = elems[i + 1]
	}
	a.len -= size_of(DynamicString)
}

// clear: free all elements, keep capacity
@(export)
barony_dynamic_array_str_clear :: proc "c" (a: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	arrstr_free_elements(a, 0)
	a.len = 0
}

// destroy: free all elements + the buffer
@(export)
barony_dynamic_array_str_destroy :: proc "c" (a: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	arrstr_free_elements(a, 0)
	runtime.delete_dynamic_array(transmute([dynamic]u8)a^)
	a^ = Raw_Dynamic_Array{}
}

@(export)
barony_dynamic_array_str_len :: proc "c" (a: ^Raw_Dynamic_Array) -> i32 {
	context = runtime.default_context()
	return i32(a.len / size_of(DynamicString))
}

// copy: deep-copy all elements (dst must be empty/fresh)
@(export)
barony_dynamic_array_str_copy :: proc "c" (dst: ^Raw_Dynamic_Array, src: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	if dst.data != nil {
		arrstr_free_elements(dst, 0)
		runtime.delete_dynamic_array(transmute([dynamic]u8)dst^)
		dst^ = Raw_Dynamic_Array{}
	}
	if src.data == nil || src.len == 0 {
		return
	}
	src_elems := ([^]DynamicString)(src.data)
	n := src.len / size_of(DynamicString)
	raw := transmute(^[dynamic]u8)(dst)
	for i in 0..<n {
		if dst.cap - dst.len < size_of(DynamicString) {
			runtime.reserve_dynamic_array(transmute(^[dynamic]u8)(dst), dst.len + size_of(DynamicString))
		}
		slot := ([^]DynamicString)(uintptr(dst.data) + uintptr(dst.len))
		dynamic_string_copy_elem(slot, &src_elems[i])
		dst.len += size_of(DynamicString)
	}
}

// entries: deep-copy all elements into val_ptrs (caller frees/owns)
@(export)
barony_dynamic_array_str_entries :: proc "c" (a: ^Raw_Dynamic_Array, val_ptrs: [^]DynamicString, count: i32) -> i32 {
	context = runtime.default_context()
	if a.data == nil || count <= 0 {
		return 0
	}
	elems := ([^]DynamicString)(a.data)
	n := a.len / size_of(DynamicString)
	if n > int(count) {
		n = int(count)
	}
	for i in 0..<n {
		dynamic_string_copy_elem(&val_ptrs[i], &elems[i])
	}
	return i32(n)
}
