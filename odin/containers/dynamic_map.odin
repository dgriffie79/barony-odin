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
