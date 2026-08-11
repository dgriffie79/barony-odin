// dynamic_map.odin -- C-ABI shim for maps, callable from C++.
//
// Replaces std::map / std::unordered_map in shared structs. Uses ODIN'S NATIVE
// map (hash map) -- not a hand-rolled container -- so growth/hash/equal are the
// battle-tested builtins. The C++ mirror is Raw_Map {data,len,allocator} (32
// bytes on x64).
//
// Key strategy (from the de-STL ordering: strings are replaced BEFORE maps):
//   - numeric keys (int/Uint32/enum, 4 bytes): map[[4]byte]V  -- C++ passes the
//     key as 4 raw bytes
//   - string keys (Item.attributes etc.): map[string]V -- C++ passes a
//     DynamicString {data,len}, which is ABI-identical to Odin's string, so the
//     shim receives it directly as `string`.
//
// STRING KEY OWNERSHIP (critical):
// Odin's map[string]V stores keys as VIEWS (16-byte header copied, character
// buffer NOT copied). std::map<std::string,V> deep-copies keys. To match
// std::map semantics when called from C++, string keys are INTERNED into a
// SHARED GLOBAL interner (strings.intern) on put. The map stores a view to the
// interned copy, which lives for the process lifetime (bounded key sets:
// attribute names, binding names, config keys). Repeated keys dedup to one
// copy. Ported Odin code uses the same idiom (map[string]V + strings.intern).
// The interner is lazily init'd; population is single-threaded (startup/game).
package containers

import "core:runtime"
import "core:mem"
import "core:strings"
import "core:slice"

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

// string -> i32 (for Stat/Item attributes: map<string, Sint32>) -- interned key
@(export)
barony_dynamic_map_stri32_init :: proc "c" (m: ^map[string]i32) {
	context = runtime.default_context()
	m^ = nil
}
@(export)
barony_dynamic_map_stri32_put :: proc "c" (m: ^map[string]i32, key: string, value: i32) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]i32)
	}
	k := intern_string(key)
	m[k] = value
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


// ---------------------------------------------------------------------------
// Shared global string interner (key ownership for string-keyed maps)
// ---------------------------------------------------------------------------
// Odin's map[string]V stores keys as views; std::map deep-copies them. We
// intern keys into ONE process-lifetime interner so map keys are stable and
// deduplicated. Keys are never freed (bounded set -- attribute/binding names).
_global_interner: strings.Intern
_global_interner_init: bool

intern_string :: proc(key: string) -> string {
	context = runtime.default_context()
	if !_global_interner_init {
		strings.intern_init(&_global_interner)
		_global_interner_init = true
	}
	s, _ := strings.intern_get(&_global_interner, key)
	return s
}

// map[string]i32 -- operator[] stable value pointer (std::map::operator[]).
// Returns pointer to the value slot; inserts default (0) if missing.
// The caller must copy the value out (ptr valid until next mutation).
@(export)
barony_dynamic_map_stri32_entry :: proc "c" (m: ^map[string]i32, key: string) -> ^i32 {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]i32)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

// map[string]i32 -- snapshot all entries for C++ copy/iteration.
// key_ptrs = array of (const char*) into interned storage, key_lens = array
// of i32 lengths, val_ptrs = array of i32 values. Each array has `count`
// slots (caller passes len(m)); returns entries written.
@(export)
barony_dynamic_map_stri32_entries :: proc "c" (m: ^map[string]i32, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]i32, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, value in m^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key) // view into the interned copy
		key_lens[n] = i32(len(key))
		val_ptrs[n] = value
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> string (map[string]string) -- BOTH keys AND values interned.
// std::map<std::string,std::string> deep-copies both; Odin stores views. The
// values are often temporaries (std::to_string results) so they must be
// interned too, or they'd dangle. Values dedup like keys (shared global
// interner).
// ---------------------------------------------------------------------------

// string -> string: init
@(export)
barony_dynamic_map_strstr_init :: proc "c" (m: ^map[string]string) {
	context = runtime.default_context()
	m^ = nil
}

// string -> string: put (interns key AND value)
@(export)
barony_dynamic_map_strstr_put :: proc "c" (m: ^map[string]string, key: string, value: string) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]string)
	}
	k := intern_string(key)
	// deep-copy the VALUE into map-owned memory (NOT interned). The C++ side
	// gets a DynamicString& to the value slot and RAII-assigns into it --
	// from_cstr would destroy an interned view (shared buffer) -> double-free.
	// Owned values free correctly.
	buf, _ := mem.alloc(len(value) + 1, align_of(u8))
	if buf == nil {
		return
	}
	if len(value) > 0 {
		runtime.mem_copy(buf, raw_data(value), len(value))
	}
	(^u8)(uintptr(buf) + uintptr(len(value)))^ = 0
	m[k] = transmute(string)DynamicString{ data = buf, len = len(value) }
}

// string -> string: get
@(export)
// string -> string: get -- DEEP-COPIES the value into fresh memory.
// CRITICAL: out is a C++ DynamicString (RAII -- frees on destruction). If we
// assign a VIEW into interned storage, the C++ dtor frees the interner's
// memory -> double-free/corruption. Copy instead: the C++ side owns the copy.
barony_dynamic_map_strstr_get :: proc "c" (m: ^map[string]string, key: string, out: ^string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		// deep-copy into fresh memory (out is a C++ RAII DynamicString -- it will free this)
	buf, _ := mem.alloc(len(v) + 1, align_of(u8))
	if buf == nil {
		return false
	}
	runtime.mem_copy(buf, raw_data(v), len(v))
	(^u8)(uintptr(buf) + uintptr(len(v)))^ = 0
	out^ = transmute(string)DynamicString{ data = buf, len = len(v) }
	}
	return ok
}

// string -> string: erase
@(export)
barony_dynamic_map_strstr_erase :: proc "c" (m: ^map[string]string, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key]
	runtime.delete_key(m, key)
	return had
}

// string -> string: clear
@(export)
barony_dynamic_map_strstr_clear :: proc "c" (m: ^map[string]string) {
	context = runtime.default_context()
	if m^ != nil {
		// free the owned value buffers (values are map-owned deep copies)
		for _, value in m^ {
			if raw_data(value) != nil {
				mem.free(raw_data(value))
			}
		}
		clear(&m^)
	}
}

// string -> string: len
@(export)
barony_dynamic_map_strstr_len :: proc "c" (m: ^map[string]string) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

// string -> string: destroy
@(export)
barony_dynamic_map_strstr_destroy :: proc "c" (m: ^map[string]string) {
	context = runtime.default_context()
	if m^ != nil {
		for _, value in m^ {
			if raw_data(value) != nil {
				mem.free(raw_data(value))
			}
		}
		delete(m^)
		m^ = nil
	}
}

// string -> string: operator[] stable value ptr (returns ^string)
@(export)
barony_dynamic_map_strstr_entry :: proc "c" (m: ^map[string]string, key: string) -> ^string {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]string)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

// string -> string: snapshot entries (key_ptr, key_len, value_ptr, value_len)
@(export)
barony_dynamic_map_strstr_entries :: proc "c" (m: ^map[string]string, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]rawptr, val_lens: [^]i32, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, value in m^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		val_ptrs[n] = raw_data(value)
		val_lens[n] = i32(len(value))
		n += 1
	}
	return n
}

// map[string]i32 -- get the STORED (interned) key pointer + value for a key.
// Used by C++ find() so iterators point at process-lifetime interned storage
// (std::map::find iterator semantics -- key stays valid while the map lives).
@(export)
barony_dynamic_map_stri32_find :: proc "c" (m: ^map[string]i32, key: string, out_key: ^rawptr, out_key_len: ^i32, out_val: ^i32) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	kp, vp, inserted, err := map_entry(m, key)
	if err != nil {
		return false
	}
	if inserted {
		// map_entry inserts if missing -- roll back to keep find() non-mutating
		runtime.delete_key(m, key)
		return false
	}
	out_key^ = raw_data(kp^)
	out_key_len^ = i32(len(kp^))
	out_val^ = vp^
	return true
}

// ---------------------------------------------------------------------------
// string -> f32 (map<string,float>) -- GameUI heightOffsets/screenDistanceOffsets
// Values are floats (no ownership). Keys interned like the others.
// ---------------------------------------------------------------------------

// string -> f32: init
@(export)
barony_dynamic_map_strf32_init :: proc "c" (m: ^map[string]f32) {
	context = runtime.default_context()
	m^ = nil
}

// string -> f32: put (interns key)
@(export)
barony_dynamic_map_strf32_put :: proc "c" (m: ^map[string]f32, key: string, value: f32) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]f32)
	}
	k := intern_string(key)
	m[k] = value
}

// string -> f32: get
@(export)
barony_dynamic_map_strf32_get :: proc "c" (m: ^map[string]f32, key: string, out: ^f32) -> bool {
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

// string -> f32: erase
@(export)
barony_dynamic_map_strf32_erase :: proc "c" (m: ^map[string]f32, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key]
	runtime.delete_key(m, key)
	return had
}

// string -> f32: clear
@(export)
barony_dynamic_map_strf32_clear :: proc "c" (m: ^map[string]f32) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}

// string -> f32: len
@(export)
barony_dynamic_map_strf32_len :: proc "c" (m: ^map[string]f32) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

// string -> f32: destroy
@(export)
barony_dynamic_map_strf32_destroy :: proc "c" (m: ^map[string]f32) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}

// string -> f32: operator[] stable value ptr
@(export)
barony_dynamic_map_strf32_entry :: proc "c" (m: ^map[string]f32, key: string) -> ^f32 {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]f32)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

// string -> f32: snapshot entries
@(export)
barony_dynamic_map_strf32_entries :: proc "c" (m: ^map[string]f32, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]f32, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, value in m^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		val_ptrs[n] = value
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> LightDef (map<string,LightDef>) -- light.hpp lightDefs.
// LightDef is a POD (i32 + 5xf32 + bool) -- layout matches C++ exactly.
// Values copied by value (POD, no ownership); keys interned.
// ---------------------------------------------------------------------------
LightDef :: struct {
	radius:      i32,
	r, g, b, a:  f32,
	falloff_exp: f32,
	shadows:     bool,
}

@(export)
barony_dynamic_map_strlightdef_init :: proc "c" (m: ^map[string]LightDef) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strlightdef_put :: proc "c" (m: ^map[string]LightDef, key: string, value: LightDef) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]LightDef)
	}
	k := intern_string(key)
	m[k] = value
}

@(export)
barony_dynamic_map_strlightdef_get :: proc "c" (m: ^map[string]LightDef, key: string, out: ^LightDef) -> bool {
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
barony_dynamic_map_strlightdef_erase :: proc "c" (m: ^map[string]LightDef, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	_, had := m[key]
	runtime.delete_key(m, key)
	return had
}

@(export)
barony_dynamic_map_strlightdef_clear :: proc "c" (m: ^map[string]LightDef) {
	context = runtime.default_context()
	if m^ != nil {
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strlightdef_len :: proc "c" (m: ^map[string]LightDef) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strlightdef_destroy :: proc "c" (m: ^map[string]LightDef) {
	context = runtime.default_context()
	if m^ != nil {
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strlightdef_entry :: proc "c" (m: ^map[string]LightDef, key: string) -> ^LightDef {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]LightDef)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strlightdef_entries :: proc "c" (m: ^map[string]LightDef, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]LightDef, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, value in m^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		val_ptrs[n] = value
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// i32 -> string (map[[4]byte]string) -- ID-to-name maps (main.hpp entries,
// mod_tools itemIDToString etc). Int keys are stored as [4]byte (no interning
// needed); VALUES are map-owned deep copies (same ownership rule as strstr:
// the C++ side wraps the value slot in DynamicString& and RAII-assigns, so
// freeing must be safe).
// ---------------------------------------------------------------------------

// i32 -> string: init
@(export)
barony_dynamic_map_i32str_init :: proc "c" (m: ^map[[4]byte]string) {
	context = runtime.default_context()
	m^ = nil
}

// i32 -> string: put (deep-copies value into map-owned memory)
@(export)
barony_dynamic_map_i32str_put :: proc "c" (m: ^map[[4]byte]string, key: ^[4]byte, value: string) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[[4]byte]string)
	}
	buf, _ := mem.alloc(len(value) + 1, align_of(u8))
	if buf == nil {
		return
	}
	if len(value) > 0 {
		runtime.mem_copy(buf, raw_data(value), len(value))
	}
	(^u8)(uintptr(buf) + uintptr(len(value)))^ = 0
	m[key^] = transmute(string)DynamicString{ data = buf, len = len(value) }
}

// i32 -> string: get -- DEEP-COPY into fresh memory (out is C++ DynamicString RAII)
@(export)
barony_dynamic_map_i32str_get :: proc "c" (m: ^map[[4]byte]string, key: ^[4]byte, out: ^string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	value, ok := m[key^]
	if !ok {
		return false
	}
	if len(value) > 0 {
		buf, _ := mem.alloc(len(value) + 1, align_of(u8))
		if buf == nil {
			return false
		}
		runtime.mem_copy(buf, raw_data(value), len(value))
		(^u8)(uintptr(buf) + uintptr(len(value)))^ = 0
		out^ = transmute(string)DynamicString{ data = buf, len = len(value) }
	} else {
		out^ = ""
	}
	return true
}

// i32 -> string: erase (frees the owned value)
@(export)
barony_dynamic_map_i32str_erase :: proc "c" (m: ^map[[4]byte]string, key: ^[4]byte) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	value, ok := m[key^]
	if ok {
		if raw_data(value) != nil {
			mem.free(raw_data(value))
		}
		runtime.delete_key(m, key^)
	}
	return ok
}

// i32 -> string: clear (frees all owned values)
@(export)
barony_dynamic_map_i32str_clear :: proc "c" (m: ^map[[4]byte]string) {
	context = runtime.default_context()
	if m^ != nil {
		for _, value in m^ {
			if raw_data(value) != nil {
				mem.free(raw_data(value))
			}
		}
		clear(&m^)
	}
}

// i32 -> string: len
@(export)
barony_dynamic_map_i32str_len :: proc "c" (m: ^map[[4]byte]string) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

// i32 -> string: destroy (frees all owned values + the map)
@(export)
barony_dynamic_map_i32str_destroy :: proc "c" (m: ^map[[4]byte]string) {
	context = runtime.default_context()
	if m^ != nil {
		for _, value in m^ {
			if raw_data(value) != nil {
				mem.free(raw_data(value))
			}
		}
		delete(m^)
		m^ = nil
	}
}

// i32 -> string: entry (mutable value slot for operator[]; O(1))
@(export)
barony_dynamic_map_i32str_entry :: proc "c" (m: ^map[[4]byte]string, key: ^[4]byte) -> ^string {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[[4]byte]string)
	}
	_, vp, _, err := map_entry(m, key^)
	if err != nil {
		return nil
	}
	return vp
}

// i32 -> string: entries (snapshot of keys + values)
@(export)
barony_dynamic_map_i32str_entries :: proc "c" (m: ^map[[4]byte]string, key_ptrs: [^][4]byte, val_ptrs: [^]rawptr, val_lens: [^]i32, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, value in m^ {
		if n >= count {
			break
		}
		key_ptrs[n] = key
		val_ptrs[n] = raw_data(value)
		val_lens[n] = i32(len(value))
		n += 1
	}
	return n
}

// i32 -> string: find (non-mutating; DEEP-COPIES the value. out is a C++
// DynamicString (RAII) that will free it, so we must not hand it a view into
// map-owned storage.)
@(export)
barony_dynamic_map_i32str_find :: proc "c" (m: ^map[[4]byte]string, key: ^[4]byte, out_val: ^string, out_val_len: ^i32) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	value, ok := m[key^]
	if !ok {
		return false
	}
	if len(value) > 0 {
		buf, _ := mem.alloc(len(value) + 1, align_of(u8))
		if buf == nil {
			return false
		}
		runtime.mem_copy(buf, raw_data(value), len(value))
		(^u8)(uintptr(buf) + uintptr(len(value)))^ = 0
		out_val^ = transmute(string)DynamicString{ data = buf, len = len(value) }
	} else {
		out_val^ = ""
	}
	out_val_len^ = i32(len(value))
	return true
}

// ---------------------------------------------------------------------------
// DynamicSet — std::set replacement.
// Odin idiom: map[T]struct{} (same Raw_Map layout as the maps). C++ mirrors
// DynamicMapRaw (32B). Values are unit (no ownership); keys own themselves
// (ints) or are interned (strings — process-lifetime stable, never freed).
// ---------------------------------------------------------------------------

// set<int>: init
@(export)
barony_dynamic_set_i32_init :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	s^ = nil
}

// set<int>: insert (returns true if newly inserted)
@(export)
barony_dynamic_set_i32_insert :: proc "c" (s: ^map[i32]struct{}, value: i32) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		s^ = make(map[i32]struct{})
	}
	_, was_present := s[value]
	s[value] = {}
	return !was_present
}

// set<int>: contains
@(export)
barony_dynamic_set_i32_contains :: proc "c" (s: ^map[i32]struct{}, value: i32) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, ok := s[value]
	return ok
}

// set<int>: erase (returns true if present)
@(export)
barony_dynamic_set_i32_erase :: proc "c" (s: ^map[i32]struct{}, value: i32) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, had := s[value]
	runtime.delete_key(s, value)
	return had
}

// set<int>: clear
@(export)
barony_dynamic_set_i32_clear :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		clear(&s^)
	}
}

// set<int>: len
@(export)
barony_dynamic_set_i32_len :: proc "c" (s: ^map[i32]struct{}) -> i32 {
	context = runtime.default_context()
	if s^ == nil {
		return 0
	}
	return i32(len(s^))
}

// set<int>: destroy
@(export)
barony_dynamic_set_i32_destroy :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		delete(s^)
		s^ = nil
	}
}

// set<int>: entries (snapshot for iteration/copy). values = array of int.
@(export)
barony_dynamic_set_i32_entries :: proc "c" (s: ^map[i32]struct{}, values: [^]i32, count: i32) -> i32 {
	context = runtime.default_context()
	if s^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in s^ {
		if n >= count {
			break
		}
		values[n] = key
		n += 1
	}
	return n
}

// set<string>: init
@(export)
barony_dynamic_set_str_init :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	s^ = nil
}

// set<string>: insert (interns key; returns true if newly inserted)
@(export)
barony_dynamic_set_str_insert :: proc "c" (s: ^map[string]struct{}, value: string) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		s^ = make(map[string]struct{})
	}
	k := intern_string(value)
	_, was_present := s[k]
	s[k] = {}
	return !was_present
}

// set<string>: contains
@(export)
barony_dynamic_set_str_contains :: proc "c" (s: ^map[string]struct{}, value: string) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, ok := s[value]
	return ok
}

// set<string>: erase (returns true if present)
@(export)
barony_dynamic_set_str_erase :: proc "c" (s: ^map[string]struct{}, value: string) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, had := s[value]
	runtime.delete_key(s, value)
	return had
}

// set<string>: clear
@(export)
barony_dynamic_set_str_clear :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		clear(&s^)
	}
}

// set<string>: len
@(export)
barony_dynamic_set_str_len :: proc "c" (s: ^map[string]struct{}) -> i32 {
	context = runtime.default_context()
	if s^ == nil {
		return 0
	}
	return i32(len(s^))
}

// set<string>: destroy
@(export)
barony_dynamic_set_str_destroy :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		delete(s^)
		s^ = nil
	}
}

// set<string>: entries (snapshot; keys are interned views — stable, never freed)
@(export)
barony_dynamic_set_str_entries :: proc "c" (s: ^map[string]struct{}, key_ptrs: [^]rawptr, key_lens: [^]i32, count: i32) -> i32 {
	context = runtime.default_context()
	if s^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in s^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> IconEntryTextMap_t (map<string, IconEntryTextMap_t>) -- the
// triple-nested IconEntry::text_map (was map<string, pair<string, set<int>>>).
// The value OWNS members: text is a DynamicString (Raw_String {data,len}) and
// highlights is a DynamicSetI32 (Raw_Map over map[i32]struct{}). Both are
// heap-backed, so put/get/erase/clear/destroy must deep-copy / deep-free —
// never shallow-copy the struct (double-free / leak).
// ---------------------------------------------------------------------------
IconEntryTextMap_t :: struct {
	text:       DynamicString,       // Raw_String {data, len}
	highlights: map[i32]struct{},    // Raw_Map (32B)
}

// deep-free the value's owned members (string buffer + set map)
icon_entry_text_map_free :: proc(v: ^IconEntryTextMap_t) {
	if v.text.data != nil {
		mem.free(v.text.data)
		v.text.data = nil
	}
	if v.highlights != nil {
		for key in v.highlights {
			_ = key
		}
		delete(v.highlights)
		v.highlights = nil
	}
}

// deep-copy src into dst (dst must be zeroed / freshly allocated)
icon_entry_text_map_copy :: proc(dst: ^IconEntryTextMap_t, src: ^IconEntryTextMap_t) {
	// copy the string (owned)
	if src.text.len > 0 {
		buf, _ := mem.alloc(src.text.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, src.text.data, src.text.len)
			(^u8)(uintptr(buf) + uintptr(src.text.len))^ = 0
			dst.text = DynamicString{ data = buf, len = src.text.len }
		}
	}
	// copy the set (owned)
	if src.highlights != nil {
		dst.highlights = make(map[i32]struct{})
		for key in src.highlights {
			dst.highlights[key] = {}
		}
	}
}

@(export)
barony_dynamic_map_striconentry_init :: proc "c" (m: ^map[string]IconEntryTextMap_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_striconentry_put :: proc "c" (m: ^map[string]IconEntryTextMap_t, key: string, value: ^IconEntryTextMap_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]IconEntryTextMap_t)
	}
	k := intern_string(key)
	// if key exists, free the old owned value first
	if old, had := m[k]; had {
		icon_entry_text_map_free(&old)
	}
	new_val: IconEntryTextMap_t
	icon_entry_text_map_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_striconentry_get :: proc "c" (m: ^map[string]IconEntryTextMap_t, key: string, out: ^IconEntryTextMap_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		// deep-copy into out (out is a fresh C++ struct)
		icon_entry_text_map_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_striconentry_erase :: proc "c" (m: ^map[string]IconEntryTextMap_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		icon_entry_text_map_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_striconentry_clear :: proc "c" (m: ^map[string]IconEntryTextMap_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_text_map_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_striconentry_len :: proc "c" (m: ^map[string]IconEntryTextMap_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_striconentry_destroy :: proc "c" (m: ^map[string]IconEntryTextMap_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_text_map_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

// entry: returns a pointer to a FRESH deep-copied value (C++ owns it; the
// caller must write it back via put). We cannot return a pointer into map
// storage (the C++ side would mutate the shared members). operator[] in the
// C++ class routes through get + put.
@(export)
barony_dynamic_map_striconentry_entry :: proc "c" (m: ^map[string]IconEntryTextMap_t, key: string, out: ^IconEntryTextMap_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]IconEntryTextMap_t)
	}
	k := intern_string(key)
	if v, had := m[k]; had {
		icon_entry_text_map_copy(out, &v)
		return true
	}
	return false
}

@(export)
barony_dynamic_map_striconentry_entries :: proc "c" (m: ^map[string]IconEntryTextMap_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]IconEntryTextMap_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		icon_entry_text_map_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> IconEntryText_t (map<string, IconEntryText_t>) — Callout's
// IconEntry::text_map. The value OWNS 8 DynamicStrings + a DynamicSetI32,
// so put/get/erase/clear/destroy deep-copy / deep-free (never shallow-copy).
// ---------------------------------------------------------------------------
IconEntryText_t :: struct {
	bannerText:          DynamicString,
	bannerHighlights:    map[i32]struct{},
	worldMsgSays:        DynamicString,
	worldMsg:            DynamicString,
	worldMsgEmote:       DynamicString,
	worldMsgEmoteYou:    DynamicString,
	worldMsgEmoteToYou:  DynamicString,
	worldIconTag:        DynamicString,
	worldIconTagMini:    DynamicString,
}

icon_entry_text_free :: proc(v: ^IconEntryText_t) {
	strings_to_free := [?]^DynamicString{
		&v.bannerText, &v.worldMsgSays, &v.worldMsg, &v.worldMsgEmote,
		&v.worldMsgEmoteYou, &v.worldMsgEmoteToYou, &v.worldIconTag, &v.worldIconTagMini,
	}
	for s in strings_to_free {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
	if v.bannerHighlights != nil {
		delete(v.bannerHighlights)
		v.bannerHighlights = nil
	}
}

icon_entry_text_copy :: proc(dst: ^IconEntryText_t, src: ^IconEntryText_t) {
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.bannerText, &src.bannerText },
		{ &dst.worldMsgSays, &src.worldMsgSays },
		{ &dst.worldMsg, &src.worldMsg },
		{ &dst.worldMsgEmote, &src.worldMsgEmote },
		{ &dst.worldMsgEmoteYou, &src.worldMsgEmoteYou },
		{ &dst.worldMsgEmoteToYou, &src.worldMsgEmoteToYou },
		{ &dst.worldIconTag, &src.worldIconTag },
		{ &dst.worldIconTagMini, &src.worldIconTagMini },
	}
	for f in fields {
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	if src.bannerHighlights != nil {
		dst.bannerHighlights = make(map[i32]struct{})
		for key in src.bannerHighlights {
			dst.bannerHighlights[key] = {}
		}
	}
}

@(export)
barony_dynamic_map_striconentrytext_init :: proc "c" (m: ^map[string]IconEntryText_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_striconentrytext_put :: proc "c" (m: ^map[string]IconEntryText_t, key: string, value: ^IconEntryText_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]IconEntryText_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		icon_entry_text_free(&old)
	}
	new_val: IconEntryText_t
	icon_entry_text_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_striconentrytext_get :: proc "c" (m: ^map[string]IconEntryText_t, key: string, out: ^IconEntryText_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		icon_entry_text_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_striconentrytext_erase :: proc "c" (m: ^map[string]IconEntryText_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		icon_entry_text_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_striconentrytext_clear :: proc "c" (m: ^map[string]IconEntryText_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_text_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_striconentrytext_len :: proc "c" (m: ^map[string]IconEntryText_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_striconentrytext_destroy :: proc "c" (m: ^map[string]IconEntryText_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_text_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_striconentrytext_entries :: proc "c" (m: ^map[string]IconEntryText_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]IconEntryText_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		icon_entry_text_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> WorldIconEntry_t (map<string, WorldIconEntry_t>) — callout world
// icons. Value owns 7 DynamicStrings + an int. entry() returns the map's
// stored slot for in-place mutation (C++ assigns fields; each field's
// DynamicString op= frees the old map-owned buffer + allocates new — safe).
// get/put deep-copy for by-value use.
// ---------------------------------------------------------------------------
WorldIconEntry_t :: struct {
	pathDefault:  DynamicString,
	pathPlayer1:  DynamicString,
	pathPlayer2:  DynamicString,
	pathPlayer3:  DynamicString,
	pathPlayer4:  DynamicString,
	pathPlayerX:  DynamicString,
	id:           i32,
}

world_icon_entry_free :: proc(v: ^WorldIconEntry_t) {
	fields := [?]^DynamicString{ &v.pathDefault, &v.pathPlayer1, &v.pathPlayer2, &v.pathPlayer3, &v.pathPlayer4, &v.pathPlayerX }
	for s in fields {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
}

world_icon_entry_copy :: proc(dst: ^WorldIconEntry_t, src: ^WorldIconEntry_t) {
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.pathDefault, &src.pathDefault },
		{ &dst.pathPlayer1, &src.pathPlayer1 },
		{ &dst.pathPlayer2, &src.pathPlayer2 },
		{ &dst.pathPlayer3, &src.pathPlayer3 },
		{ &dst.pathPlayer4, &src.pathPlayer4 },
		{ &dst.pathPlayerX, &src.pathPlayerX },
	}
	for f in fields {
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	dst.id = src.id
}

@(export)
barony_dynamic_map_strworldicon_init :: proc "c" (m: ^map[string]WorldIconEntry_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strworldicon_put :: proc "c" (m: ^map[string]WorldIconEntry_t, key: string, value: ^WorldIconEntry_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]WorldIconEntry_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		world_icon_entry_free(&old)
	}
	new_val: WorldIconEntry_t
	world_icon_entry_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strworldicon_get :: proc "c" (m: ^map[string]WorldIconEntry_t, key: string, out: ^WorldIconEntry_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		world_icon_entry_copy(out, &v)
	}
	return ok
}

// entry: mutable slot into map storage (in-place field mutation)
@(export)
barony_dynamic_map_strworldicon_entry :: proc "c" (m: ^map[string]WorldIconEntry_t, key: string) -> ^WorldIconEntry_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]WorldIconEntry_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strworldicon_erase :: proc "c" (m: ^map[string]WorldIconEntry_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		world_icon_entry_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strworldicon_clear :: proc "c" (m: ^map[string]WorldIconEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				world_icon_entry_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strworldicon_len :: proc "c" (m: ^map[string]WorldIconEntry_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strworldicon_destroy :: proc "c" (m: ^map[string]WorldIconEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				world_icon_entry_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strworldicon_entries :: proc "c" (m: ^map[string]WorldIconEntry_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]WorldIconEntry_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		world_icon_entry_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> DiscoveryAnim_t (map<string, DiscoveryAnim_t>) — featherGUI
// label discoveries. Value owns 1 DynamicString + 2 Uint32. entry() for
// in-place mutation; get/put deep-copy.
// ---------------------------------------------------------------------------
DiscoveryAnim_t :: struct {
	startTicks:      u32,
	processedOnTick: u32,
	name:            DynamicString,
}

discovery_anim_free :: proc(v: ^DiscoveryAnim_t) {
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
}

discovery_anim_copy :: proc(dst: ^DiscoveryAnim_t, src: ^DiscoveryAnim_t) {
	dst.startTicks = src.startTicks
	dst.processedOnTick = src.processedOnTick
	if src.name.len > 0 {
		buf, _ := mem.alloc(src.name.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, src.name.data, src.name.len)
			(^u8)(uintptr(buf) + uintptr(src.name.len))^ = 0
			dst.name = DynamicString{ data = buf, len = src.name.len }
		}
	}
}

@(export)
barony_dynamic_map_strdiscoveryanim_init :: proc "c" (m: ^map[string]DiscoveryAnim_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strdiscoveryanim_put :: proc "c" (m: ^map[string]DiscoveryAnim_t, key: string, value: ^DiscoveryAnim_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]DiscoveryAnim_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		discovery_anim_free(&old)
	}
	new_val: DiscoveryAnim_t
	discovery_anim_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strdiscoveryanim_get :: proc "c" (m: ^map[string]DiscoveryAnim_t, key: string, out: ^DiscoveryAnim_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		discovery_anim_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_strdiscoveryanim_entry :: proc "c" (m: ^map[string]DiscoveryAnim_t, key: string) -> ^DiscoveryAnim_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]DiscoveryAnim_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strdiscoveryanim_erase :: proc "c" (m: ^map[string]DiscoveryAnim_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		discovery_anim_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strdiscoveryanim_clear :: proc "c" (m: ^map[string]DiscoveryAnim_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				discovery_anim_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strdiscoveryanim_len :: proc "c" (m: ^map[string]DiscoveryAnim_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strdiscoveryanim_destroy :: proc "c" (m: ^map[string]DiscoveryAnim_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				discovery_anim_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strdiscoveryanim_entries :: proc "c" (m: ^map[string]DiscoveryAnim_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]DiscoveryAnim_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		discovery_anim_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> SpecialNPCEntry_t (map<string, SpecialNPCEntry_t>) — monster
// special NPCs. Value owns 4 DynamicStrings + DynamicSetI32 + int.
// entry() for in-place mutation; get/put deep-copy.
// ---------------------------------------------------------------------------
SpecialNPCEntry_t :: struct {
	internalName: DynamicString,
	name:         DynamicString,
	shortname:    DynamicString,
	modelIndexes: map[i32]struct{},
	baseModel:    i32,
	uniqueIcon:   DynamicString,
}

special_npc_free :: proc(v: ^SpecialNPCEntry_t) {
	strings := [?]^DynamicString{ &v.internalName, &v.name, &v.shortname, &v.uniqueIcon }
	for s in strings {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
	if v.modelIndexes != nil {
		delete(v.modelIndexes)
		v.modelIndexes = nil
	}
}

special_npc_copy :: proc(dst: ^SpecialNPCEntry_t, src: ^SpecialNPCEntry_t) {
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.internalName, &src.internalName },
		{ &dst.name, &src.name },
		{ &dst.shortname, &src.shortname },
		{ &dst.uniqueIcon, &src.uniqueIcon },
	}
	for f in fields {
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	if src.modelIndexes != nil {
		dst.modelIndexes = make(map[i32]struct{})
		for key in src.modelIndexes {
			dst.modelIndexes[key] = {}
		}
	}
	dst.baseModel = src.baseModel
}

@(export)
barony_dynamic_map_strspecialnpc_init :: proc "c" (m: ^map[string]SpecialNPCEntry_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strspecialnpc_put :: proc "c" (m: ^map[string]SpecialNPCEntry_t, key: string, value: ^SpecialNPCEntry_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]SpecialNPCEntry_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		special_npc_free(&old)
	}
	new_val: SpecialNPCEntry_t
	special_npc_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strspecialnpc_get :: proc "c" (m: ^map[string]SpecialNPCEntry_t, key: string, out: ^SpecialNPCEntry_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		special_npc_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_strspecialnpc_entry :: proc "c" (m: ^map[string]SpecialNPCEntry_t, key: string) -> ^SpecialNPCEntry_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]SpecialNPCEntry_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strspecialnpc_erase :: proc "c" (m: ^map[string]SpecialNPCEntry_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		special_npc_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strspecialnpc_clear :: proc "c" (m: ^map[string]SpecialNPCEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				special_npc_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strspecialnpc_len :: proc "c" (m: ^map[string]SpecialNPCEntry_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strspecialnpc_destroy :: proc "c" (m: ^map[string]SpecialNPCEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				special_npc_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strspecialnpc_entries :: proc "c" (m: ^map[string]SpecialNPCEntry_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]SpecialNPCEntry_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		special_npc_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> ColliderDmgProperties_t (map<string, ColliderDmgProperties_t>)
// Value owns 2 DynamicSetI32 + 8 bools. entry() for in-place mutation;
// get/put deep-copy (the sets must be copied, not shared).
// ---------------------------------------------------------------------------
ColliderDmgProperties_t :: struct {
	burnable:                  bool,
	minotaurPathThroughAndBreak: bool,
	meleeAffects:              bool,
	magicAffects:              bool,
	bombsAttach:               bool,
	boulderDestroys:           bool,
	showAsWallOnMinimap:       bool,
	allowNPCPathing:           bool,
	proficiencyBonusDamage:    map[i32]struct{},
	proficiencyResistDamage:   map[i32]struct{},
}

collider_dmg_free :: proc(v: ^ColliderDmgProperties_t) {
	if v.proficiencyBonusDamage != nil {
		delete(v.proficiencyBonusDamage)
		v.proficiencyBonusDamage = nil
	}
	if v.proficiencyResistDamage != nil {
		delete(v.proficiencyResistDamage)
		v.proficiencyResistDamage = nil
	}
}

collider_dmg_copy :: proc(dst: ^ColliderDmgProperties_t, src: ^ColliderDmgProperties_t) {
	dst^ = src^
	dst.proficiencyBonusDamage = nil
	dst.proficiencyResistDamage = nil
	if src.proficiencyBonusDamage != nil {
		dst.proficiencyBonusDamage = make(map[i32]struct{})
		for key in src.proficiencyBonusDamage {
			dst.proficiencyBonusDamage[key] = {}
		}
	}
	if src.proficiencyResistDamage != nil {
		dst.proficiencyResistDamage = make(map[i32]struct{})
		for key in src.proficiencyResistDamage {
			dst.proficiencyResistDamage[key] = {}
		}
	}
}

@(export)
barony_dynamic_map_strcolliderdmg_init :: proc "c" (m: ^map[string]ColliderDmgProperties_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strcolliderdmg_put :: proc "c" (m: ^map[string]ColliderDmgProperties_t, key: string, value: ^ColliderDmgProperties_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]ColliderDmgProperties_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		collider_dmg_free(&old)
	}
	new_val: ColliderDmgProperties_t
	collider_dmg_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strcolliderdmg_get :: proc "c" (m: ^map[string]ColliderDmgProperties_t, key: string, out: ^ColliderDmgProperties_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		collider_dmg_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_strcolliderdmg_entry :: proc "c" (m: ^map[string]ColliderDmgProperties_t, key: string) -> ^ColliderDmgProperties_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]ColliderDmgProperties_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strcolliderdmg_erase :: proc "c" (m: ^map[string]ColliderDmgProperties_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		collider_dmg_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strcolliderdmg_clear :: proc "c" (m: ^map[string]ColliderDmgProperties_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				collider_dmg_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strcolliderdmg_len :: proc "c" (m: ^map[string]ColliderDmgProperties_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strcolliderdmg_destroy :: proc "c" (m: ^map[string]ColliderDmgProperties_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				collider_dmg_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strcolliderdmg_entries :: proc "c" (m: ^map[string]ColliderDmgProperties_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]ColliderDmgProperties_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		collider_dmg_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> ItemLocalization_t (map<string, ItemLocalization_t>) — item name
// localizations. Value owns 2 DynamicStrings. entry() for in-place mutation;
// get/put deep-copy.
// ---------------------------------------------------------------------------
ItemLocalization_t :: struct {
	name_identified:   DynamicString,
	name_unidentified: DynamicString,
}

item_localization_free :: proc(v: ^ItemLocalization_t) {
	if v.name_identified.data != nil {
		mem.free(v.name_identified.data)
		v.name_identified.data = nil
	}
	if v.name_unidentified.data != nil {
		mem.free(v.name_unidentified.data)
		v.name_unidentified.data = nil
	}
}

item_localization_copy :: proc(dst: ^ItemLocalization_t, src: ^ItemLocalization_t) {
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.name_identified, &src.name_identified },
		{ &dst.name_unidentified, &src.name_unidentified },
	}
	for f in fields {
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

@(export)
barony_dynamic_map_stritemloc_init :: proc "c" (m: ^map[string]ItemLocalization_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_stritemloc_put :: proc "c" (m: ^map[string]ItemLocalization_t, key: string, value: ^ItemLocalization_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]ItemLocalization_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		item_localization_free(&old)
	}
	new_val: ItemLocalization_t
	item_localization_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_stritemloc_get :: proc "c" (m: ^map[string]ItemLocalization_t, key: string, out: ^ItemLocalization_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		item_localization_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_stritemloc_entry :: proc "c" (m: ^map[string]ItemLocalization_t, key: string) -> ^ItemLocalization_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]ItemLocalization_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_stritemloc_erase :: proc "c" (m: ^map[string]ItemLocalization_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		item_localization_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_stritemloc_clear :: proc "c" (m: ^map[string]ItemLocalization_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				item_localization_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_stritemloc_len :: proc "c" (m: ^map[string]ItemLocalization_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_stritemloc_destroy :: proc "c" (m: ^map[string]ItemLocalization_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				item_localization_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_stritemloc_entries :: proc "c" (m: ^map[string]ItemLocalization_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]ItemLocalization_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		item_localization_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> Achievement_t (map<string, Achievement_t>) — compendium
// achievements. Value owns 1 DynamicString + bool + i64. entry() for
// in-place mutation; get/put deep-copy.
// ---------------------------------------------------------------------------
Achievement_t :: struct {
	name:       DynamicString,
	unlocked:   bool,
	unlockTime: i64,
}

achievement_t_free :: proc(v: ^Achievement_t) {
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
}

achievement_t_copy :: proc(dst: ^Achievement_t, src: ^Achievement_t) {
	dst.unlocked = src.unlocked
	dst.unlockTime = src.unlockTime
	if src.name.len > 0 {
		buf, _ := mem.alloc(src.name.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, src.name.data, src.name.len)
			(^u8)(uintptr(buf) + uintptr(src.name.len))^ = 0
			dst.name = DynamicString{ data = buf, len = src.name.len }
		}
	}
}

@(export)
barony_dynamic_map_strachievement_init :: proc "c" (m: ^map[string]Achievement_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strachievement_put :: proc "c" (m: ^map[string]Achievement_t, key: string, value: ^Achievement_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]Achievement_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		achievement_t_free(&old)
	}
	new_val: Achievement_t
	achievement_t_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strachievement_get :: proc "c" (m: ^map[string]Achievement_t, key: string, out: ^Achievement_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		achievement_t_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_strachievement_entry :: proc "c" (m: ^map[string]Achievement_t, key: string) -> ^Achievement_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]Achievement_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strachievement_erase :: proc "c" (m: ^map[string]Achievement_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		achievement_t_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strachievement_clear :: proc "c" (m: ^map[string]Achievement_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				achievement_t_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strachievement_len :: proc "c" (m: ^map[string]Achievement_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strachievement_destroy :: proc "c" (m: ^map[string]Achievement_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				achievement_t_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strachievement_entries :: proc "c" (m: ^map[string]Achievement_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]Achievement_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		achievement_t_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> AchievementData_t (map<string, AchievementData_t>) — compendium
// achievements. Value owns 4 DynamicStrings + bool + enum + int + i64 + int.
// entry() for in-place mutation; get/put deep-copy.
// ---------------------------------------------------------------------------
AchievementData_t :: struct {
	name:             DynamicString,
	desc:             DynamicString,
	desc_formatted:   DynamicString,
	hidden:           bool,
	dlcType:          i32,
	category:         DynamicString,
	lorePoints:       i32,
	unlockTime:       i64,
	unlocked:         bool,
	achievementProgress: i32,
}

achievement_data_free :: proc(v: ^AchievementData_t) {
	strings := [?]^DynamicString{ &v.name, &v.desc, &v.desc_formatted, &v.category }
	for s in strings {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
}

achievement_data_copy :: proc(dst: ^AchievementData_t, src: ^AchievementData_t) {
	dst.hidden = src.hidden
	dst.dlcType = src.dlcType
	dst.lorePoints = src.lorePoints
	dst.unlockTime = src.unlockTime
	dst.unlocked = src.unlocked
	dst.achievementProgress = src.achievementProgress
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.name, &src.name },
		{ &dst.desc, &src.desc },
		{ &dst.desc_formatted, &src.desc_formatted },
		{ &dst.category, &src.category },
	}
	for f in fields {
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

@(export)
barony_dynamic_map_strachdata_init :: proc "c" (m: ^map[string]AchievementData_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_strachdata_put :: proc "c" (m: ^map[string]AchievementData_t, key: string, value: ^AchievementData_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]AchievementData_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		achievement_data_free(&old)
	}
	new_val: AchievementData_t
	achievement_data_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_strachdata_get :: proc "c" (m: ^map[string]AchievementData_t, key: string, out: ^AchievementData_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		achievement_data_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_strachdata_entry :: proc "c" (m: ^map[string]AchievementData_t, key: string) -> ^AchievementData_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]AchievementData_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_strachdata_erase :: proc "c" (m: ^map[string]AchievementData_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		achievement_data_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_strachdata_clear :: proc "c" (m: ^map[string]AchievementData_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				achievement_data_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_strachdata_len :: proc "c" (m: ^map[string]AchievementData_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_strachdata_destroy :: proc "c" (m: ^map[string]AchievementData_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				achievement_data_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_strachdata_entries :: proc "c" (m: ^map[string]AchievementData_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]AchievementData_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		achievement_data_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}

// ---------------------------------------------------------------------------
// string -> IconEntry (map<string, IconEntry>) — follower/callout radial menu
// icons. Value owns 6 DynamicStrings + 2 i32 + a NESTED map
// (text_map: map[string]IconEntryTextMap_t, itself deep-owned). entry() for
// in-place mutation; get/put deep-copy (nested map cloned, not shared).
// ---------------------------------------------------------------------------
IconEntry_t :: struct {
	name:            DynamicString,
	id:              i32,
	path:            DynamicString,
	path_hover:      DynamicString,
	path_active:     DynamicString,
	path_active_hover: DynamicString,
	icon_offsetx:    i32,
	icon_offsety:    i32,
	text_map:        map[string]IconEntryTextMap_t,
}

icon_entry_free :: proc(v: ^IconEntry_t) {
	strings := [?]^DynamicString{ &v.name, &v.path, &v.path_hover, &v.path_active, &v.path_active_hover }
	for s in strings {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
	if v.text_map != nil {
		for key in v.text_map {
			_, vp, _, err := map_entry(&v.text_map, key)
			if err == nil && vp != nil {
				icon_entry_text_map_free(vp)
			}
		}
		delete(v.text_map)
		v.text_map = nil
	}
}

icon_entry_copy :: proc(dst: ^IconEntry_t, src: ^IconEntry_t) {
	dst.id = src.id
	dst.icon_offsetx = src.icon_offsetx
	dst.icon_offsety = src.icon_offsety
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &dst.name, &src.name },
		{ &dst.path, &src.path },
		{ &dst.path_hover, &src.path_hover },
		{ &dst.path_active, &src.path_active },
		{ &dst.path_active_hover, &src.path_active_hover },
	}
	for f in fields {
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	if src.text_map != nil {
		dst.text_map = make(map[string]IconEntryTextMap_t)
		for key in src.text_map {
			_, vp, _, err := map_entry(&src.text_map, key)
			if err == nil && vp != nil {
				new_val: IconEntryTextMap_t
				icon_entry_text_map_copy(&new_val, vp)
				dst.text_map[key] = new_val
			}
		}
	}
}

@(export)
barony_dynamic_map_striconentrylist_init :: proc "c" (m: ^map[string]IconEntry_t) {
	context = runtime.default_context()
	m^ = nil
}

@(export)
barony_dynamic_map_striconentrylist_put :: proc "c" (m: ^map[string]IconEntry_t, key: string, value: ^IconEntry_t) {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]IconEntry_t)
	}
	k := intern_string(key)
	if old, had := m[k]; had {
		icon_entry_free(&old)
	}
	new_val: IconEntry_t
	icon_entry_copy(&new_val, value)
	m[k] = new_val
}

@(export)
barony_dynamic_map_striconentrylist_get :: proc "c" (m: ^map[string]IconEntry_t, key: string, out: ^IconEntry_t) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, ok := m[key]
	if ok {
		icon_entry_copy(out, &v)
	}
	return ok
}

@(export)
barony_dynamic_map_striconentrylist_entry :: proc "c" (m: ^map[string]IconEntry_t, key: string) -> ^IconEntry_t {
	context = runtime.default_context()
	if m^ == nil {
		m^ = make(map[string]IconEntry_t)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(m, k)
	if err != nil {
		return nil
	}
	return vp
}

@(export)
barony_dynamic_map_striconentrylist_erase :: proc "c" (m: ^map[string]IconEntry_t, key: string) -> bool {
	context = runtime.default_context()
	if m^ == nil {
		return false
	}
	v, had := m[key]
	if had {
		icon_entry_free(&v)
		runtime.delete_key(m, key)
	}
	return had
}

@(export)
barony_dynamic_map_striconentrylist_clear :: proc "c" (m: ^map[string]IconEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_free(vp)
			}
		}
		clear(&m^)
	}
}

@(export)
barony_dynamic_map_striconentrylist_len :: proc "c" (m: ^map[string]IconEntry_t) -> i32 {
	context = runtime.default_context()
	if m^ == nil {
		return 0
	}
	return i32(len(m^))
}

@(export)
barony_dynamic_map_striconentrylist_destroy :: proc "c" (m: ^map[string]IconEntry_t) {
	context = runtime.default_context()
	if m^ != nil {
		for key in m^ {
			_, vp, _, err := map_entry(m, key)
			if err == nil && vp != nil {
				icon_entry_free(vp)
			}
		}
		delete(m^)
		m^ = nil
	}
}

@(export)
barony_dynamic_map_striconentrylist_entries :: proc "c" (m: ^map[string]IconEntry_t, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: [^]IconEntry_t, count: i32) -> i32 {
	context = runtime.default_context()
	if m^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key in m^ {
		if n >= count {
			break
		}
		_, vp, _, err := map_entry(m, key)
		if err != nil || vp == nil {
			continue
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		icon_entry_copy(&val_ptrs[n], vp)
		n += 1
	}
	return n
}
