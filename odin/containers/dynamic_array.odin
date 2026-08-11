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
// GENERIC element-aware array family (replaces the per-type str/icon/option/
// entryvar families).
//
// One exported family of procs handles ANY element type: the C++ side passes
// elem_size (sizeof(T)) + value_kind (an int from kind_of<T>). The Odin side
// looks up the element's free/copy procs in Element_Ops and applies them while
// walking the raw byte buffer. POD types (kind 0) have nil free/copy -> raw
// byte semantics, identical to the base shims above.
//
// This is exactly the Odin idiom `defer { for item in arr { destroy(item) };
// delete(arr) }` — parameterized so the destroy/copy live in ONE table per
// element type instead of a whole exported proc family per type.
// ---------------------------------------------------------------------------

// element kinds (must match kind_of<T> on the C++ side)
Kind_POD            :: 0
Kind_DynamicString  :: 1
Kind_Icon           :: 2
Kind_Option         :: 3
Kind_EntryVar       :: 4

// element free/copy procs, rawptr-based so the generic walkers can use them
dynamic_string_free_elem :: proc(p: rawptr) {
	s := (^DynamicString)(p)
	if s.data != nil {
		mem.free(s.data)
		s.data = nil
	}
}

dynamic_string_copy_elem :: proc(dst: rawptr, src: rawptr) {
	d := (^DynamicString)(dst)
	s := (^DynamicString)(src)
	d^ = DynamicString{}
	if s.len > 0 {
		buf, _ := mem.alloc(s.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.data, s.len)
			(^u8)(uintptr(buf) + uintptr(s.len))^ = 0
			d^ = DynamicString{ data = buf, len = s.len }
		}
	}
}

ItemTooltipIcons_t :: struct {
	iconPath:             DynamicString,
	text:                 DynamicString,
	textColor:            u32,
	conditionalAttribute: DynamicString,
}

icon_free :: proc(p: rawptr) {
	v := (^ItemTooltipIcons_t)(p)
	fields := [?]^DynamicString{ &v.iconPath, &v.text, &v.conditionalAttribute }
	for s in fields {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
}

icon_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^ItemTooltipIcons_t)(dst)
	s := (^ItemTooltipIcons_t)(src)
	d.textColor = s.textColor
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.iconPath, &s.iconPath },
		{ &d.text, &s.text },
		{ &d.conditionalAttribute, &s.conditionalAttribute },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
}

DropdownOption_t :: struct {
	text:             DynamicString,
	keyboardGlyph:    DynamicString,
	controllerGlyph:  DynamicString,
	action:           DynamicString,
}

dropdown_option_free :: proc(p: rawptr) {
	v := (^DropdownOption_t)(p)
	fields := [?]^DynamicString{ &v.text, &v.keyboardGlyph, &v.controllerGlyph, &v.action }
	for s in fields {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
}

dropdown_option_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^DropdownOption_t)(dst)
	s := (^DropdownOption_t)(src)
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.text, &s.text },
		{ &d.keyboardGlyph, &s.keyboardGlyph },
		{ &d.controllerGlyph, &s.controllerGlyph },
		{ &d.action, &s.action },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
}

EntryVariable_t :: struct {
	_type:        i32,
	value:        DynamicString,
	numericValue: i32,
	sizex:        i32,
	sizey:        i32,
}

entry_var_free :: proc(p: rawptr) {
	v := (^EntryVariable_t)(p)
	if v.value.data != nil {
		mem.free(v.value.data)
		v.value.data = nil
	}
}

entry_var_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^EntryVariable_t)(dst)
	s := (^EntryVariable_t)(src)
	d._type = s._type
	d.numericValue = s.numericValue
	d.sizex = s.sizex
	d.sizey = s.sizey
	if d.value.data != nil { mem.free(d.value.data); d.value.data = nil }
	if s.value.len > 0 {
		buf, _ := mem.alloc(s.value.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.value.data, s.value.len)
			(^u8)(uintptr(buf) + uintptr(s.value.len))^ = 0
			d.value = DynamicString{ data = buf, len = s.value.len }
		}
	}
}

// kind -> {free, copy} ops table. POD (kind 0) = nil/nil = raw bytes.
Element_Ops :: struct {
	free: proc(rawptr),
	copy: proc(dst: rawptr, src: rawptr),
}

element_ops := [5]Element_Ops{
	0 = { free = nil,                   copy = nil },
	1 = { free = dynamic_string_free_elem, copy = dynamic_string_copy_elem },
	2 = { free = icon_free,             copy = icon_copy },
	3 = { free = dropdown_option_free,  copy = dropdown_option_copy },
	4 = { free = entry_var_free,        copy = entry_var_copy },
}

@(export)
barony_dynamic_array_elem_init :: proc "c" (a: ^Raw_Dynamic_Array) {
	context = runtime.default_context()
	a^ = Raw_Dynamic_Array{}
}

// append one element (elem_size bytes, kind's copy applied if any)
@(export)
barony_dynamic_array_elem_append :: proc "c" (a: ^Raw_Dynamic_Array, elem: rawptr, elem_size: int, value_kind: i32) -> i32 {
	context = runtime.default_context()
	if elem == nil || elem_size <= 0 {
		return 0
	}
	ops := element_ops[value_kind]
	if a.cap - a.len < elem_size {
		runtime.reserve_dynamic_array(transmute(^[dynamic]u8)(a), a.len + elem_size)
	}
	slot := ([^]u8)(uintptr(a.data) + uintptr(a.len))
	if ops.copy != nil {
		ops.copy(slot, elem)
	} else {
		runtime.mem_copy(slot, elem, elem_size)
	}
	a.len += elem_size
	return i32(a.len / elem_size)
}

// get: copy the element at index into out (out is C++ RAII / caller-owned)
@(export)
barony_dynamic_array_elem_get :: proc "c" (a: ^Raw_Dynamic_Array, index: i32, out: rawptr, elem_size: int, value_kind: i32) -> bool {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*elem_size >= a.len {
		return false
	}
	ops := element_ops[value_kind]
	slot := ([^]u8)(uintptr(a.data) + uintptr(int(index) * elem_size))
	if ops.copy != nil {
		ops.copy(out, slot)
	} else {
		runtime.mem_copy(out, slot, elem_size)
	}
	return true
}

// set: replace element at index (frees old, copies new)
@(export)
barony_dynamic_array_elem_set :: proc "c" (a: ^Raw_Dynamic_Array, index: i32, elem: rawptr, elem_size: int, value_kind: i32) {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*elem_size >= a.len {
		return
	}
	ops := element_ops[value_kind]
	slot := ([^]u8)(uintptr(a.data) + uintptr(int(index) * elem_size))
	if ops.free != nil {
		ops.free(slot)
	}
	if ops.copy != nil {
		ops.copy(slot, elem)
	} else {
		runtime.mem_copy(slot, elem, elem_size)
	}
}

// erase: free the element at index, shift left
@(export)
barony_dynamic_array_elem_erase :: proc "c" (a: ^Raw_Dynamic_Array, index: i32, elem_size: int, value_kind: i32) {
	context = runtime.default_context()
	if a.data == nil || index < 0 || int(index)*elem_size >= a.len {
		return
	}
	ops := element_ops[value_kind]
	slot := ([^]u8)(uintptr(a.data) + uintptr(int(index) * elem_size))
	if ops.free != nil {
		ops.free(slot)
	}
	n := a.len / elem_size
	for i := int(index); i < n - 1; i += 1 {
		runtime.mem_copy(([^]u8)(uintptr(a.data) + uintptr(i) * uintptr(elem_size)), ([^]u8)(uintptr(a.data) + uintptr(i+1) * uintptr(elem_size)), elem_size)
	}
	a.len -= elem_size
}

// clear: free all elements, keep capacity
@(export)
barony_dynamic_array_elem_clear :: proc "c" (a: ^Raw_Dynamic_Array, elem_size: int, value_kind: i32) {
	context = runtime.default_context()
	if a.data != nil {
		ops := element_ops[value_kind]
		if ops.free != nil {
			n := a.len / elem_size
			for i in 0..<n {
				ops.free(([^]u8)(uintptr(a.data) + uintptr(i) * uintptr(elem_size)))
			}
		}
	}
	a.len = 0
}

// destroy: free all elements + the buffer
@(export)
barony_dynamic_array_elem_destroy :: proc "c" (a: ^Raw_Dynamic_Array, elem_size: int, value_kind: i32) {
	context = runtime.default_context()
	if a.data != nil {
		ops := element_ops[value_kind]
		if ops.free != nil {
			n := a.len / elem_size
			for i in 0..<n {
				ops.free(([^]u8)(uintptr(a.data) + uintptr(i) * uintptr(elem_size)))
			}
		}
	}
	runtime.delete_dynamic_array(transmute([dynamic]u8)a^)
	a^ = Raw_Dynamic_Array{}
}

@(export)
barony_dynamic_array_elem_len :: proc "c" (a: ^Raw_Dynamic_Array, elem_size: int) -> i32 {
	context = runtime.default_context()
	return i32(a.len / elem_size)
}

// copy: deep-copy all elements (dst must be empty/fresh)
@(export)
barony_dynamic_array_elem_copy :: proc "c" (dst: ^Raw_Dynamic_Array, src: ^Raw_Dynamic_Array, elem_size: int, value_kind: i32) {
	context = runtime.default_context()
	if dst.data != nil {
		barony_dynamic_array_elem_destroy(dst, elem_size, value_kind)
	}
	if src.data == nil || src.len == 0 {
		return
	}
	ops := element_ops[value_kind]
	n := src.len / elem_size
	for i in 0..<n {
		if dst.cap - dst.len < elem_size {
			runtime.reserve_dynamic_array(transmute(^[dynamic]u8)(dst), dst.len + elem_size)
		}
		slot := ([^]u8)(uintptr(dst.data) + uintptr(dst.len))
		src_slot := ([^]u8)(uintptr(src.data) + uintptr(i) * uintptr(elem_size))
		if ops.copy != nil {
			ops.copy(slot, src_slot)
		} else {
			runtime.mem_copy(slot, src_slot, elem_size)
		}
		dst.len += elem_size
	}
}

// entries: copy all elements into val_ptrs (caller owns/frees)
@(export)
barony_dynamic_array_elem_entries :: proc "c" (a: ^Raw_Dynamic_Array, val_ptrs: rawptr, count: i32, elem_size: int, value_kind: i32) -> i32 {
	context = runtime.default_context()
	if a.data == nil || count <= 0 {
		return 0
	}
	ops := element_ops[value_kind]
	n := a.len / elem_size
	if n > int(count) {
		n = int(count)
	}
	out := ([^]u8)(val_ptrs)
	for i in 0..<n {
		slot := ([^]u8)(uintptr(a.data) + uintptr(i) * uintptr(elem_size))
		if ops.copy != nil {
			ops.copy(([^]u8)(uintptr(out) + uintptr(i) * uintptr(elem_size)), slot)
		} else {
			runtime.mem_copy(([^]u8)(uintptr(out) + uintptr(i) * uintptr(elem_size)), slot, elem_size)
		}
	}
	return i32(n)
}
