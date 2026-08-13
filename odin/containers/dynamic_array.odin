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

// 32 bytes on x64 — mirrors Odin's native map header (C++ Raw_Map).
// Only used as an opaque slot inside owning structs (attributes map).
Raw_Map :: struct {
	data:      rawptr,
	len:       int,
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
Kind_FollowerDetails :: 5
Kind_LevelT          :: 6
Kind_CodexItem       :: 7
Kind_ShopkeeperItem  :: 8
Kind_VariantPair     :: 9
Kind_MonsterCurveEntry :: 10
Kind_LevelCurve      :: 11
Kind_TmpItem         :: 12
Kind_MapGeneration   :: 13
Kind_HotbarEntry     :: 14
Kind_HotbarEntryArray :: 15
Kind_DynArrayStrArray :: 16
Kind_SkillEffect      :: 17
Kind_SkillEntry       :: 18
Kind_PanelEntry       :: 19
Kind_AssistNotifPair  :: 20
Kind_AlchNotifPair    :: 21
Kind_CalloutPanel    :: 22
Kind_HiscoreStat     :: 23
Kind_HiscoreLootbag  :: 24
Kind_HiscorePlayer  :: 25
Kind_Book            :: 26
Kind_StoreSlots      :: 27
Kind_StatusEffectQueueEntry :: 28
Kind_HiscoreAttributesPair   :: 29
Kind_HiscorePlayerEquipPair  :: 30
Kind_HiscoreNpcEquipPair     :: 31
Kind_HiscoreLootbagPair      :: 32
Kind_HiscoreCompendiumPair   :: 33
Kind_FollowerBarPair         :: 34
Kind_I32Map          :: 13

Book_t :: struct {
	text:           DynamicString,
	default_name:   DynamicString,
	formattedPages: Raw_Dynamic_Array,
}

book_free :: proc(p: rawptr) {
	b := (^Book_t)(p)
	dynamic_string_free_elem(rawptr(&b.text))
	dynamic_string_free_elem(rawptr(&b.default_name))
	barony_dynamic_array_elem_destroy(&b.formattedPages, size_of(DynamicString), Kind_DynamicString)
}

book_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Book_t)(dst)
	s := (^Book_t)(src)
	dynamic_string_copy_elem(rawptr(&d.text), rawptr(&s.text))
	dynamic_string_copy_elem(rawptr(&d.default_name), rawptr(&s.default_name))
	barony_dynamic_array_elem_copy(&d.formattedPages, &s.formattedPages, size_of(DynamicString), Kind_DynamicString)
}

StoreSlots_t :: struct {
	slotTradingReq: i32,
	itemEntries:    Raw_Dynamic_Array,
}

store_slots_free :: proc(p: rawptr) {
	s := (^StoreSlots_t)(p)
	barony_dynamic_array_elem_destroy(&s.itemEntries, size_of(ShopkeeperItemEntry_t), Kind_ShopkeeperItem)
}

store_slots_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^StoreSlots_t)(dst)
	s := (^StoreSlots_t)(src)
	d.slotTradingReq = s.slotTradingReq
	barony_dynamic_array_elem_copy(&d.itemEntries, &s.itemEntries, size_of(ShopkeeperItemEntry_t), Kind_ShopkeeperItem)
}

// SDL_Rect mirror (16 bytes = 4 x i32)
SDL_Rect :: struct {
	x: i32,
	y: i32,
	w: i32,
	h: i32,
}

// StatusEffectQueueEntry_t — 160B owning (navigation map; rest POD)
StatusEffectQueueEntry_t :: struct {
	animateX: f64,
	animateY: f64,
	animateW: f64,
	animateH: f64,
	animateSetpointX: i32,
	animateSetpointY: i32,
	animateSetpointW: i32,
	animateSetpointH: i32,
	animateStartX: i32,
	animateStartY: i32,
	animateStartW: i32,
	animateStartH: i32,
	pos: SDL_Rect,
	notificationTargetPosition: SDL_Rect,
	lastUpdateTick: u32,
	effect: i32,
	customVariable: u32,
	lowDuration: bool,
	notificationState: i32,
	notificationStateInit: i32,
	navigation: map[[4]byte]u64,
	index: u64,
}

status_effect_queue_entry_free :: proc(p: rawptr) {
	v := (^StatusEffectQueueEntry_t)(p)
	if v.navigation != nil {
		delete(v.navigation)
		v.navigation = nil
	}
}

status_effect_queue_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^StatusEffectQueueEntry_t)(dst)
	s := (^StatusEffectQueueEntry_t)(src)
	if d.navigation != nil { delete(d.navigation) }
	d^ = s^
	d.navigation = nil
	if s.navigation != nil {
		d.navigation = make(map[[4]byte]u64)
		for key, value in s.navigation {
			d.navigation[key] = value
		}
	}
}

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

// ---------------------------------------------------------------------------
// FollowerGenerateDetails_t (monster curve custom manager) — owns 1 DynamicString
// ---------------------------------------------------------------------------
FollowerGenerateDetails_t :: struct {
	x:            f64,
	y:            f64,
	leaderType:   i32,
	uid:          u32,
	followerName: DynamicString,
}

follower_details_free :: proc(p: rawptr) {
	v := (^FollowerGenerateDetails_t)(p)
	if v.followerName.data != nil {
		mem.free(v.followerName.data)
		v.followerName.data = nil
	}
	v.followerName.len = 0
}

follower_details_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^FollowerGenerateDetails_t)(dst)
	s := (^FollowerGenerateDetails_t)(src)
	d.x = s.x
	d.y = s.y
	d.leaderType = s.leaderType
	d.uid = s.uid
	if d.followerName.data != nil {
		mem.free(d.followerName.data)
		d.followerName.data = nil
	}
	d.followerName.len = 0
	if s.followerName.len > 0 {
		buf, _ := mem.alloc(s.followerName.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.followerName.data, s.followerName.len)
			(^u8)(uintptr(buf) + uintptr(s.followerName.len))^ = 0
			d.followerName = DynamicString{ data = buf, len = s.followerName.len }
		}
	}
}

// ---------------------------------------------------------------------------
// Level_t (tutorial scores) — owns 3 DynamicStrings
// ---------------------------------------------------------------------------
Level_t :: struct {
	filename:      DynamicString,
	title:         DynamicString,
	description:   DynamicString,
	completionTime: u32,
}

level_t_free :: proc(p: rawptr) {
	v := (^Level_t)(p)
	fields := [?]^DynamicString{ &v.filename, &v.title, &v.description }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

level_t_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Level_t)(dst)
	s := (^Level_t)(src)
	d.completionTime = s.completionTime
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.filename, &s.filename },
		{ &d.title, &s.title },
		{ &d.description, &s.description },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

// ---------------------------------------------------------------------------
// CodexItem_t (compendium items) — owns 1 DynamicString
// ---------------------------------------------------------------------------
CodexItem_t :: struct {
	name:     DynamicString,
	rotation: i32,
	spellID:  i32,
	effectID: i32,
	itemID:   i32,
}

codex_item_free :: proc(p: rawptr) {
	v := (^CodexItem_t)(p)
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
	v.name.len = 0
}

codex_item_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^CodexItem_t)(dst)
	s := (^CodexItem_t)(src)
	d.rotation = s.rotation
	d.spellID = s.spellID
	d.effectID = s.effectID
	d.itemID = s.itemID
	if d.name.data != nil {
		mem.free(d.name.data)
		d.name.data = nil
	}
	d.name.len = 0
	if s.name.len > 0 {
		buf, _ := mem.alloc(s.name.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.name.data, s.name.len)
			(^u8)(uintptr(buf) + uintptr(s.name.len))^ = 0
			d.name = DynamicString{ data = buf, len = s.name.len }
		}
	}
}

// ---------------------------------------------------------------------------
// ShopkeeperConsumables_t::ItemEntry — owns 6 DynamicArrays (deep)
// ---------------------------------------------------------------------------
ShopkeeperItemEntry_t :: struct {
	type:            Raw_Dynamic_Array,
	status:          Raw_Dynamic_Array,
	beatitude:       Raw_Dynamic_Array,
	count:           Raw_Dynamic_Array,
	appearance:      Raw_Dynamic_Array,
	identified:      Raw_Dynamic_Array,
	percentChance:   i32,
	weightedChance:  i32,
	dropChance:      i32,
	emptyItemEntry:  bool,
	dropItemOnDeath: bool,
}

shopkeeper_item_entry_free :: proc(p: rawptr) {
	v := (^ShopkeeperItemEntry_t)(p)
	barony_dynamic_array_elem_destroy(&v.type, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.status, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.beatitude, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.count, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.appearance, size_of(u32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.identified, size_of(i32), Kind_POD)
}

shopkeeper_item_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^ShopkeeperItemEntry_t)(dst)
	s := (^ShopkeeperItemEntry_t)(src)
	d.percentChance = s.percentChance
	d.weightedChance = s.weightedChance
	d.dropChance = s.dropChance
	d.emptyItemEntry = s.emptyItemEntry
	d.dropItemOnDeath = s.dropItemOnDeath
	barony_dynamic_array_elem_copy(&d.type, &s.type, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.status, &s.status, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.beatitude, &s.beatitude, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.count, &s.count, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.appearance, &s.appearance, size_of(u32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.identified, &s.identified, size_of(i32), Kind_POD)
}

// ---------------------------------------------------------------------------
// VariantPair_t (monster/follower variant name+chance) — owns 1 DynamicString
// ---------------------------------------------------------------------------
VariantPair_t :: struct {
	name:   DynamicString,
	chance: i32,
}

variant_pair_free :: proc(p: rawptr) {
	v := (^VariantPair_t)(p)
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
	v.name.len = 0
}

variant_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^VariantPair_t)(dst)
	s := (^VariantPair_t)(src)
	d.chance = s.chance
	if d.name.data != nil {
		mem.free(d.name.data)
		d.name.data = nil
	}
	d.name.len = 0
	if s.name.len > 0 {
		buf, _ := mem.alloc(s.name.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.name.data, s.name.len)
			(^u8)(uintptr(buf) + uintptr(s.name.len))^ = 0
			d.name = DynamicString{ data = buf, len = s.name.len }
		}
	}
}

// ---------------------------------------------------------------------------
// MonsterCurveEntry_t + LevelCurve_t (custom monster curves) — recursive owns
// ---------------------------------------------------------------------------
MonsterCurveEntry_t :: struct {
	monsterType:         i32,
	levelmin:            i32,
	levelmax:            i32,
	chance:              i32,
	fallbackMonsterType: i32,
	variants:            Raw_Dynamic_Array,   // DynamicArray of VariantPair_t
}

monster_curve_entry_free :: proc(p: rawptr) {
	v := (^MonsterCurveEntry_t)(p)
	barony_dynamic_array_elem_destroy(&v.variants, size_of(VariantPair_t), Kind_VariantPair)
}

monster_curve_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^MonsterCurveEntry_t)(dst)
	s := (^MonsterCurveEntry_t)(src)
	d.monsterType = s.monsterType
	d.levelmin = s.levelmin
	d.levelmax = s.levelmax
	d.chance = s.chance
	d.fallbackMonsterType = s.fallbackMonsterType
	barony_dynamic_array_elem_copy(&d.variants, &s.variants, size_of(VariantPair_t), Kind_VariantPair)
}

LevelCurve_t :: struct {
	mapName:     DynamicString,
	monsterCurve: Raw_Dynamic_Array,   // DynamicArray of MonsterCurveEntry_t
	fixedSpawns:  Raw_Dynamic_Array,
}

level_curve_free :: proc(p: rawptr) {
	v := (^LevelCurve_t)(p)
	if v.mapName.data != nil {
		mem.free(v.mapName.data)
		v.mapName.data = nil
	}
	v.mapName.len = 0
	barony_dynamic_array_elem_destroy(&v.monsterCurve, size_of(MonsterCurveEntry_t), Kind_MonsterCurveEntry)
	barony_dynamic_array_elem_destroy(&v.fixedSpawns, size_of(MonsterCurveEntry_t), Kind_MonsterCurveEntry)
}

level_curve_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^LevelCurve_t)(dst)
	s := (^LevelCurve_t)(src)
	if d.mapName.data != nil { mem.free(d.mapName.data); d.mapName.data = nil }
	d.mapName.len = 0
	if s.mapName.len > 0 {
		buf, _ := mem.alloc(s.mapName.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.mapName.data, s.mapName.len)
			(^u8)(uintptr(buf) + uintptr(s.mapName.len))^ = 0
			d.mapName = DynamicString{ data = buf, len = s.mapName.len }
		}
	}
	barony_dynamic_array_elem_copy(&d.monsterCurve, &s.monsterCurve, size_of(MonsterCurveEntry_t), Kind_MonsterCurveEntry)
	barony_dynamic_array_elem_copy(&d.fixedSpawns, &s.fixedSpawns, size_of(MonsterCurveEntry_t), Kind_MonsterCurveEntry)
}

// ---------------------------------------------------------------------------
// TmpItem_t (ItemTooltips data table) — owns strings + array + map
// ---------------------------------------------------------------------------
TmpItem_t :: struct {
	internalName:  DynamicString,
	itemId:        i32,
	fpIndex:       i32,
	tpIndex:       i32,
	tpShortIndex:  i32,
	gold:          i32,
	weight:        i32,
	itemLevel:     i32,
	category:      DynamicString,
	equipSlot:     DynamicString,
	imagePaths:    Raw_Dynamic_Array,   // DynamicArrayStr
	attributes:    Raw_Map,             // map[string]i32
	tooltip:       DynamicString,
	iconLabelPath: DynamicString,
}

tmp_item_free :: proc(p: rawptr) {
	v := (^TmpItem_t)(p)
	fields := [?]^DynamicString{ &v.internalName, &v.category, &v.equipSlot, &v.tooltip, &v.iconLabelPath }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
	barony_dynamic_array_elem_destroy(&v.imagePaths, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_map_str_destroy(&v.attributes, Kind_I32Map)
}

tmp_item_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^TmpItem_t)(dst)
	s := (^TmpItem_t)(src)
	d.itemId = s.itemId
	d.fpIndex = s.fpIndex
	d.tpIndex = s.tpIndex
	d.tpShortIndex = s.tpShortIndex
	d.gold = s.gold
	d.weight = s.weight
	d.itemLevel = s.itemLevel
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.internalName, &s.internalName },
		{ &d.category, &s.category },
		{ &d.equipSlot, &s.equipSlot },
		{ &d.tooltip, &s.tooltip },
		{ &d.iconLabelPath, &s.iconLabelPath },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	barony_dynamic_array_elem_copy(&d.imagePaths, &s.imagePaths, size_of(DynamicString), Kind_DynamicString)
	if d.attributes.data != nil {
		barony_dynamic_map_str_destroy(&d.attributes, 0)
	}
	if s.attributes.data != nil {
		barony_dynamic_map_str_init(&d.attributes, 0)
		key_ptrs: [256]rawptr
		key_lens: [256]i32
		val_ptrs: [256]i32
		n := barony_dynamic_map_str_entries(&s.attributes, &key_ptrs[0], &key_lens[0], &val_ptrs[0], 256, 0)
		for i in 0..<int(n) {
			key_str := string(([^]u8)(key_ptrs[i])[:key_lens[i]])
			barony_dynamic_map_str_put(&d.attributes, key_str, &val_ptrs[i], 0)
		}
	}
}

// ---------------------------------------------------------------------------
// MapGeneration_t (map generation config) — owns string + array + 4 sets
// ---------------------------------------------------------------------------
MapGeneration_t :: struct {
	mapName:        DynamicString,
	trapTypes:      Raw_Dynamic_Array,   // DynamicArrayStr
	minoFloors:     Raw_Map,             // map[i32]struct{}
	darkFloors:     Raw_Map,
	shopFloors:     Raw_Map,
	npcSpawnFloors: Raw_Map,
	usingTrapTypes: bool,
	minoPercent:    i32,
	shopPercent:    i32,
	darkPercent:    i32,
	npcSpawnPercent: i32,
}

map_generation_free :: proc(p: rawptr) {
	v := (^MapGeneration_t)(p)
	if v.mapName.data != nil {
		mem.free(v.mapName.data)
		v.mapName.data = nil
	}
	v.mapName.len = 0
	barony_dynamic_array_elem_destroy(&v.trapTypes, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(&v.minoFloors))
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(&v.darkFloors))
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(&v.shopFloors))
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(&v.npcSpawnFloors))
}

map_generation_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^MapGeneration_t)(dst)
	s := (^MapGeneration_t)(src)
	d.usingTrapTypes = s.usingTrapTypes
	d.minoPercent = s.minoPercent
	d.shopPercent = s.shopPercent
	d.darkPercent = s.darkPercent
	d.npcSpawnPercent = s.npcSpawnPercent
	if d.mapName.data != nil { mem.free(d.mapName.data); d.mapName.data = nil }
	d.mapName.len = 0
	if s.mapName.len > 0 {
		buf, _ := mem.alloc(s.mapName.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.mapName.data, s.mapName.len)
			(^u8)(uintptr(buf) + uintptr(s.mapName.len))^ = 0
			d.mapName = DynamicString{ data = buf, len = s.mapName.len }
		}
	}
	barony_dynamic_array_elem_copy(&d.trapTypes, &s.trapTypes, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(&d.minoFloors), transmute(^map[i32]struct{})(&s.minoFloors))
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(&d.darkFloors), transmute(^map[i32]struct{})(&s.darkFloors))
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(&d.shopFloors), transmute(^map[i32]struct{})(&s.shopFloors))
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(&d.npcSpawnFloors), transmute(^map[i32]struct{})(&s.npcSpawnFloors))
}

// ---------------------------------------------------------------------------
// HotbarEntry_t (class hotbar config) — owns 2 DynamicArrayS32
// ---------------------------------------------------------------------------
HotbarEntry_t :: struct {
	itemTypes:     Raw_Dynamic_Array,   // DynamicArrayS32
	itemCategories: Raw_Dynamic_Array,
	slotnum:       i32,
}

hotbar_entry_free :: proc(p: rawptr) {
	v := (^HotbarEntry_t)(p)
	barony_dynamic_array_elem_destroy(&v.itemTypes, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.itemCategories, size_of(i32), Kind_POD)
}

hotbar_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HotbarEntry_t)(dst)
	s := (^HotbarEntry_t)(src)
	d.slotnum = s.slotnum
	barony_dynamic_array_elem_copy(&d.itemTypes, &s.itemTypes, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.itemCategories, &s.itemCategories, size_of(i32), Kind_POD)
}

// "array of HotbarEntry_t" element ops (for the nested hotbar_alternates)
hotbar_entry_array_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(HotbarEntry_t), Kind_HotbarEntry)
}
hotbar_entry_array_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(HotbarEntry_t), Kind_HotbarEntry)
}

// "array of DynamicArrayStr" element ops (for nested pages)
dynarrstr_array_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(DynamicString), Kind_DynamicString)
}
dynarrstr_array_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(DynamicString), Kind_DynamicString)
}

// matches src/main.hpp MAXPLAYERS
MAXPLAYERS :: 4
NUM_CONDUCT_CHALLENGES :: 32
NUM_GAMEPLAY_STATISTICS :: 64
NUM_HOTBAR_SLOTS :: 10
NUM_HOTBAR_ALTERNATES :: 5

// ---------------------------------------------------------------------------
// SkillEffect_t + SkillEntry_t (player skill sheet) — recursive owns
// ---------------------------------------------------------------------------
SkillEffect_t :: struct {
	tag:                       DynamicString,
	title:                     DynamicString,
	titleShort:                DynamicString,
	rawValue:                  DynamicString,
	value:                     DynamicString,
	valueCustomWidthOffset:    i32,
	bAllowAutoResizeValue:     bool,
	bAllowRealtimeUpdate:      bool,
	marquee:                   [MAXPLAYERS]f64,
	marqueeTicks:              [MAXPLAYERS]u32,
	marqueeCompleted:          [MAXPLAYERS]bool,
	effectUpdatedAtSkillLevel: i32,
	effectUpdatedAtBaseSkillLevel: i32,
	effectUpdatedAtMonsterType: i32,
	cachedWidth:               i32,
}

skill_effect_free :: proc(p: rawptr) {
	v := (^SkillEffect_t)(p)
	fields := [?]^DynamicString{ &v.tag, &v.title, &v.titleShort, &v.rawValue, &v.value }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

skill_effect_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^SkillEffect_t)(dst)
	s := (^SkillEffect_t)(src)
	d.valueCustomWidthOffset = s.valueCustomWidthOffset
	d.bAllowAutoResizeValue = s.bAllowAutoResizeValue
	d.bAllowRealtimeUpdate = s.bAllowRealtimeUpdate
	d.marquee = s.marquee
	d.marqueeTicks = s.marqueeTicks
	d.marqueeCompleted = s.marqueeCompleted
	d.effectUpdatedAtSkillLevel = s.effectUpdatedAtSkillLevel
	d.effectUpdatedAtBaseSkillLevel = s.effectUpdatedAtBaseSkillLevel
	d.effectUpdatedAtMonsterType = s.effectUpdatedAtMonsterType
	d.cachedWidth = s.cachedWidth
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.tag, &s.tag },
		{ &d.title, &s.title },
		{ &d.titleShort, &s.titleShort },
		{ &d.rawValue, &s.rawValue },
		{ &d.value, &s.value },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

SkillEntry_t :: struct {
	skillName:               DynamicString,
	skillShortName:          DynamicString,
	skillId:                 i32,
	skillIconPath:           DynamicString,
	skillIconPathLegend:     DynamicString,
	skillIconPath32px:       DynamicString,
	skillIconPathLegend32px: DynamicString,
	statIconPath:            DynamicString,
	description:             DynamicString,
	legendaryDescription:    DynamicString,
	skillSfx:                i32,
	effectStartOffsetX:      i32,
	effectBackgroundOffsetX: i32,
	effectBackgroundWidth:   i32,
	effects:                 Raw_Dynamic_Array,   // DynamicArray of SkillEffect_t
}

skill_entry_free :: proc(p: rawptr) {
	v := (^SkillEntry_t)(p)
	fields := [?]^DynamicString{ &v.skillName, &v.skillShortName, &v.skillIconPath, &v.skillIconPathLegend, &v.skillIconPath32px, &v.skillIconPathLegend32px, &v.statIconPath, &v.description, &v.legendaryDescription }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
	barony_dynamic_array_elem_destroy(&v.effects, size_of(SkillEffect_t), Kind_SkillEffect)
}

skill_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^SkillEntry_t)(dst)
	s := (^SkillEntry_t)(src)
	d.skillId = s.skillId
	d.skillSfx = s.skillSfx
	d.effectStartOffsetX = s.effectStartOffsetX
	d.effectBackgroundOffsetX = s.effectBackgroundOffsetX
	d.effectBackgroundWidth = s.effectBackgroundWidth
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.skillName, &s.skillName },
		{ &d.skillShortName, &s.skillShortName },
		{ &d.skillIconPath, &s.skillIconPath },
		{ &d.skillIconPathLegend, &s.skillIconPathLegend },
		{ &d.skillIconPath32px, &s.skillIconPath32px },
		{ &d.skillIconPathLegend32px, &s.skillIconPathLegend32px },
		{ &d.statIconPath, &s.statIconPath },
		{ &d.description, &s.description },
		{ &d.legendaryDescription, &s.legendaryDescription },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
		if f.s.len > 0 {
			buf, _ := mem.alloc(f.s.len + 1, align_of(u8))
			if buf != nil {
				runtime.mem_copy(buf, f.s.data, f.s.len)
				(^u8)(uintptr(buf) + uintptr(f.s.len))^ = 0
				f.d^ = DynamicString{ data = buf, len = f.s.len }
			}
		}
	}
	barony_dynamic_array_elem_copy(&d.effects, &s.effects, size_of(SkillEffect_t), Kind_SkillEffect)
}

// ---------------------------------------------------------------------------
// PanelEntry_t (follower radial menu) — owns 4 DynamicStrings
// ---------------------------------------------------------------------------
PanelEntry_t :: struct {
	x:              i32,
	y:              i32,
	path:           DynamicString,
	path_locked:    DynamicString,
	path_hover:     DynamicString,
	path_locked_hover: DynamicString,
	icon_offsetx:   i32,
	icon_offsety:   i32,
}

panel_entry_free :: proc(p: rawptr) {
	v := (^PanelEntry_t)(p)
	fields := [?]^DynamicString{ &v.path, &v.path_locked, &v.path_hover, &v.path_locked_hover }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

panel_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^PanelEntry_t)(dst)
	s := (^PanelEntry_t)(src)
	d.x = s.x
	d.y = s.y
	d.icon_offsetx = s.icon_offsetx
	d.icon_offsety = s.icon_offsety
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.path, &s.path },
		{ &d.path_locked, &s.path_locked },
		{ &d.path_hover, &s.path_hover },
		{ &d.path_locked_hover, &s.path_locked_hover },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

// ---------------------------------------------------------------------------
// AssistNotificationPair (pair<Uint32, AssistNotification_t> owning value)
// ---------------------------------------------------------------------------
AssistNotification_t :: struct {
	img:              DynamicString,
	title:            DynamicString,
	body:             DynamicString,
	lifetime:         u32,
	notificationType: i32,
	animx:            f64,
	state:            i32,
}

AssistNotificationPair_t :: struct {
	first:  u32,
	second: AssistNotification_t,
}

assist_notif_free :: proc(p: rawptr) {
	v := (^AssistNotificationPair_t)(p)
	fields := [?]^DynamicString{ &v.second.img, &v.second.title, &v.second.body }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

assist_notif_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^AssistNotificationPair_t)(dst)
	s := (^AssistNotificationPair_t)(src)
	d.first = s.first
	d.second.lifetime = s.second.lifetime
	d.second.notificationType = s.second.notificationType
	d.second.animx = s.second.animx
	d.second.state = s.second.state
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.second.img, &s.second.img },
		{ &d.second.title, &s.second.title },
		{ &d.second.body, &s.second.body },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

// ---------------------------------------------------------------------------
// AlchNotificationPair (pair<Uint32, AlchNotification_t> owning value)
// ---------------------------------------------------------------------------
AlchNotification_t :: struct {
	img:    DynamicString,
	title:  DynamicString,
	body:   DynamicString,
	animx:  f64,
	state:  i32,
}

AlchNotificationPair_t :: struct {
	first:  u32,
	second: AlchNotification_t,
}

alch_notif_free :: proc(p: rawptr) {
	v := (^AlchNotificationPair_t)(p)
	fields := [?]^DynamicString{ &v.second.img, &v.second.title, &v.second.body }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

alch_notif_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^AlchNotificationPair_t)(dst)
	s := (^AlchNotificationPair_t)(src)
	d.first = s.first
	d.second.animx = s.second.animx
	d.second.state = s.second.state
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.second.img, &s.second.img },
		{ &d.second.title, &s.second.title },
		{ &d.second.body, &s.second.body },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

// ---------------------------------------------------------------------------
// CalloutRadialMenu::PanelEntry — owns 2 DynamicStrings
// ---------------------------------------------------------------------------
CalloutPanelEntry_t :: struct {
	x:            i32,
	y:            i32,
	path:         DynamicString,
	path_hover:   DynamicString,
	icon_offsetx: i32,
	icon_offsety: i32,
}

callout_panel_entry_free :: proc(p: rawptr) {
	v := (^CalloutPanelEntry_t)(p)
	fields := [?]^DynamicString{ &v.path, &v.path_hover }
	for f in fields {
		if f.data != nil {
			mem.free(f.data)
			f.data = nil
		}
		f.len = 0
	}
}

callout_panel_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^CalloutPanelEntry_t)(dst)
	s := (^CalloutPanelEntry_t)(src)
	d.x = s.x
	d.y = s.y
	d.icon_offsetx = s.icon_offsetx
	d.icon_offsety = s.icon_offsety
	fields := [?]struct{ d: ^DynamicString, s: ^DynamicString }{
		{ &d.path, &s.path },
		{ &d.path_hover, &s.path_hover },
	}
	for f in fields {
		if f.d.data != nil { mem.free(f.d.data); f.d.data = nil }
		f.d.len = 0
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

// ---------------------------------------------------------------------------
// HiscoreStat_t + HiscorePlayer_t (online hiscore save) — recursive owns
// ---------------------------------------------------------------------------
HiscoreItem_t :: struct {
	type:       u32,
	status:     u32,
	appearance: u32,
	beatitude:  i32,
	count:      i32,
	identified: bool,
	x:          i32,
	y:          i32,
}

HiscoreLootbag_t :: struct {
	spawn_x:         i32,
	spawn_y:         i32,
	spawnedOnGround: bool,
	looted:          bool,
	items:           Raw_Dynamic_Array,   // HiscoreItem_t
}

// Owning hiscore pair element mirrors (deep free/copy). Layouts match the
// C++ std::pair<...> members exactly on MSVC x64.
HiscoreAttributesPair_t :: struct {
	first:  DynamicString,
	second: DynamicString,
}
HiscorePlayerEquipPair_t :: struct {
	first:  DynamicString,
	second: u32,
}
HiscoreNpcEquipPair_t :: struct {
	first:  DynamicString,
	second: HiscoreItem_t,
}
HiscoreLootbagPair_t :: struct {
	first:  u32,
	second: HiscoreLootbag_t,
}
HiscoreCompendiumEventPair_t :: struct {
	first:  DynamicString,
	second: Raw_Dynamic_Array,   // i32
}

HiscoreStat_t :: struct {
	name:                DynamicString,
	type:                u32,
	sex:                 u32,
	statscore_appearance: u32,
	HP:                  i32,
	maxHP:               i32,
	MP:                  i32,
	maxMP:               i32,
	STR:                 i32,
	DEX:                 i32,
	CON:                 i32,
	INT:                 i32,
	PER:                 i32,
	CHR:                 i32,
	EXP:                 i32,
	LVL:                 i32,
	GOLD:                i32,
	HUNGER:              i32,
	PROFICIENCIES:       Raw_Dynamic_Array,  // i32
	EFFECTS:             Raw_Dynamic_Array,
	EFFECTS_TIMERS:      Raw_Dynamic_Array,
	EFFECTS_ACCRETION_TIME: Raw_Dynamic_Array,
	MISC_FLAGS:          Raw_Dynamic_Array,
	attributes:          Raw_Dynamic_Array,  // pair<DynamicString,DynamicString>
	player_equipment:    Raw_Dynamic_Array,  // pair<DynamicString,u32>
	npc_equipment:       Raw_Dynamic_Array,  // pair<DynamicString,HiscoreItem_t>
	inventory:           Raw_Dynamic_Array,  // HiscoreItem_t
	void_chest_inventory: Raw_Dynamic_Array,
	player_lootbags:     Raw_Dynamic_Array,  // pair<u32,HiscoreLootbag_t>
}


// hiscore stat free/copy: free the name + all arrays (recursive via POD/elem kinds)
hiscore_stat_free :: proc(p: rawptr) {
	v := (^HiscoreStat_t)(p)
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
	v.name.len = 0
	barony_dynamic_array_elem_destroy(&v.PROFICIENCIES, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.EFFECTS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.EFFECTS_TIMERS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.EFFECTS_ACCRETION_TIME, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.MISC_FLAGS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.attributes, size_of(HiscoreAttributesPair_t), Kind_HiscoreAttributesPair)
	barony_dynamic_array_elem_destroy(&v.player_equipment, size_of(HiscorePlayerEquipPair_t), Kind_HiscorePlayerEquipPair)
	barony_dynamic_array_elem_destroy(&v.npc_equipment, size_of(HiscoreNpcEquipPair_t), Kind_HiscoreNpcEquipPair)
	barony_dynamic_array_elem_destroy(&v.inventory, size_of(HiscoreItem_t), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.void_chest_inventory, size_of(HiscoreItem_t), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.player_lootbags, size_of(HiscoreLootbagPair_t), Kind_HiscoreLootbagPair)
}

hiscore_stat_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreStat_t)(dst)
	s := (^HiscoreStat_t)(src)
	d.type = s.type
	d.sex = s.sex
	d.statscore_appearance = s.statscore_appearance
	d.HP = s.HP
	d.maxHP = s.maxHP
	d.MP = s.MP
	d.maxMP = s.maxMP
	d.STR = s.STR
	d.DEX = s.DEX
	d.CON = s.CON
	d.INT = s.INT
	d.PER = s.PER
	d.CHR = s.CHR
	d.EXP = s.EXP
	d.LVL = s.LVL
	d.GOLD = s.GOLD
	d.HUNGER = s.HUNGER
	if d.name.data != nil { mem.free(d.name.data); d.name.data = nil }
	d.name.len = 0
	if s.name.len > 0 {
		buf, _ := mem.alloc(s.name.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, s.name.data, s.name.len)
			(^u8)(uintptr(buf) + uintptr(s.name.len))^ = 0
			d.name = DynamicString{ data = buf, len = s.name.len }
		}
	}
	barony_dynamic_array_elem_copy(&d.PROFICIENCIES, &s.PROFICIENCIES, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.EFFECTS, &s.EFFECTS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.EFFECTS_TIMERS, &s.EFFECTS_TIMERS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.EFFECTS_ACCRETION_TIME, &s.EFFECTS_ACCRETION_TIME, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.MISC_FLAGS, &s.MISC_FLAGS, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.attributes, &s.attributes, size_of(HiscoreAttributesPair_t), Kind_HiscoreAttributesPair)
	barony_dynamic_array_elem_copy(&d.player_equipment, &s.player_equipment, size_of(HiscorePlayerEquipPair_t), Kind_HiscorePlayerEquipPair)
	barony_dynamic_array_elem_copy(&d.npc_equipment, &s.npc_equipment, size_of(HiscoreNpcEquipPair_t), Kind_HiscoreNpcEquipPair)
	barony_dynamic_array_elem_copy(&d.inventory, &s.inventory, size_of(HiscoreItem_t), Kind_POD)
	barony_dynamic_array_elem_copy(&d.void_chest_inventory, &s.void_chest_inventory, size_of(HiscoreItem_t), Kind_POD)
	barony_dynamic_array_elem_copy(&d.player_lootbags, &s.player_lootbags, size_of(HiscoreLootbagPair_t), Kind_HiscoreLootbagPair)
}

// HiscoreLootbag element ops (owns items array)
hiscore_lootbag_free :: proc(p: rawptr) {
	v := (^HiscoreLootbag_t)(p)
	barony_dynamic_array_elem_destroy(&v.items, size_of(HiscoreItem_t), Kind_POD)
}
hiscore_lootbag_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreLootbag_t)(dst)
	s := (^HiscoreLootbag_t)(src)
	d.spawn_x = s.spawn_x
	d.spawn_y = s.spawn_y
	d.spawnedOnGround = s.spawnedOnGround
	d.looted = s.looted
	barony_dynamic_array_elem_copy(&d.items, &s.items, size_of(HiscoreItem_t), Kind_POD)
}

// ---- owning hiscore pair free/copy procs ----
hiscore_attributes_pair_free :: proc(p: rawptr) {
	v := (^HiscoreAttributesPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.first))
	dynamic_string_free_elem(rawptr(&v.second))
}
hiscore_attributes_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreAttributesPair_t)(dst)
	s := (^HiscoreAttributesPair_t)(src)
	dynamic_string_copy_elem(rawptr(&d.first), rawptr(&s.first))
	dynamic_string_copy_elem(rawptr(&d.second), rawptr(&s.second))
}

hiscore_player_equip_pair_free :: proc(p: rawptr) {
	v := (^HiscorePlayerEquipPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.first))
}
hiscore_player_equip_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscorePlayerEquipPair_t)(dst)
	s := (^HiscorePlayerEquipPair_t)(src)
	d.second = s.second
	dynamic_string_copy_elem(rawptr(&d.first), rawptr(&s.first))
}

hiscore_npc_equip_pair_free :: proc(p: rawptr) {
	v := (^HiscoreNpcEquipPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.first))
}
hiscore_npc_equip_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreNpcEquipPair_t)(dst)
	s := (^HiscoreNpcEquipPair_t)(src)
	d.second = s.second
	dynamic_string_copy_elem(rawptr(&d.first), rawptr(&s.first))
}

hiscore_lootbag_pair_free :: proc(p: rawptr) {
	v := (^HiscoreLootbagPair_t)(p)
	hiscore_lootbag_free(rawptr(&v.second))
}
hiscore_lootbag_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreLootbagPair_t)(dst)
	s := (^HiscoreLootbagPair_t)(src)
	d.first = s.first
	hiscore_lootbag_copy(rawptr(&d.second), rawptr(&s.second))
}

hiscore_compendium_pair_free :: proc(p: rawptr) {
	v := (^HiscoreCompendiumEventPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.first))
	barony_dynamic_array_elem_destroy(&v.second, size_of(i32), Kind_POD)
}
hiscore_compendium_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscoreCompendiumEventPair_t)(dst)
	s := (^HiscoreCompendiumEventPair_t)(src)
	dynamic_string_copy_elem(rawptr(&d.first), rawptr(&s.first))
	barony_dynamic_array_elem_copy(&d.second, &s.second, size_of(i32), Kind_POD)
}

// ---------------------------------------------------------------------------
// HiscorePlayer_t (online hiscore save Player) — recursive owns
// ---------------------------------------------------------------------------
HiscorePlayer_t :: struct {
	char_class:            u32,
	race:                  u32,
	kills:                 Raw_Dynamic_Array,   // i32
	conductPenniless:      bool,
	conductFoodless:       bool,
	conductVegetarian:     bool,
	conductIlliterate:     bool,
	additionalConducts:    [NUM_CONDUCT_CHALLENGES]i32,
	gameStatistics:        [NUM_GAMEPLAY_STATISTICS]i32,
	hotbar:                [NUM_HOTBAR_SLOTS]u32,
	hotbar_alternate:      [NUM_HOTBAR_ALTERNATES][NUM_HOTBAR_SLOTS]u32,
	selected_spell:        u32,
	selected_spell_alternate: [NUM_HOTBAR_ALTERNATES]u32,
	selected_spell_last_appearance: i32,
	spells:                Raw_Dynamic_Array,   // u32
	known_recipes:         Raw_Dynamic_Array,   // recipe_t (3 ints)
	known_scrolls:         Raw_Dynamic_Array,   // i32
	shopkeeperHostility:   Raw_Dynamic_Array,   // pair<int,PlayerRaceHostility_t>
	compendium_item_events: Raw_Dynamic_Array,  // pair<DynamicString, i32 array>
	itemDegradeRNG:        Raw_Dynamic_Array,
	escalatingRngRolls:    Raw_Dynamic_Array,
	escalatingSpellRngRolls: Raw_Dynamic_Array,
	appraisal_item_progress: Raw_Dynamic_Array,
	learnedSpells:         Raw_Dynamic_Array,   // i32
	sustainedSpellIDCounter: Raw_Dynamic_Array,  // pair<int,int> (POD)
	ducksInARow:             Raw_Dynamic_Array,  // pair<int,int> (POD)
	favoriteBooksAchievement: Raw_Dynamic_Array, // pair<int,int> (POD)
	sustainedSpellMPUsedSorcery:  i32,
	sustainedSpellMPUsedMysticism: i32,
	sustainedSpellMPUsedThaumaturgy: i32,
	baseSpellMPUsedSorcery:  i32,
	baseSpellMPUsedMysticism: i32,
	baseSpellMPUsedThaumaturgy: i32,
	stats:                 HiscoreStat_t,
	followers:             Raw_Dynamic_Array,   // HiscoreStat_t
}

// Player::HUD_t::Bar_t — all POD (real_t=f64). 88 bytes.
HudBar_t :: struct {
	animateValue: f64,
	animateValue2: f64,
	animatePreviousSetpoint: f64,
	animateSetpoint: i32,
	animateTicks: u32,
	animateState: i32,
	xpLevelups: u32,
	maxValue: f64,
	fadeIn: f64,
	fadeOut: f64,
	widthMultiplier: f64,
	flashTicks: u32,
	flashProcessedOnTick: u32,
	flashAnimState: i32,
	flashType: i32,
}

// Player::HUD_t::FollowerBar_t — POD + two owned DynamicStrings. 280 bytes.
FollowerBar_t :: struct {
	hpBar: HudBar_t,
	mpBar: HudBar_t,
	animx: f64,
	animy: f64,
	expired: bool,
	expiredTicks: u32,
	animFade: f64,
	animFadeScroll: f64,
	animFadeScrollDummy: f64,
	bInit: bool,
	name: DynamicString,
	customPortraitPath: DynamicString,
	level: i32,
	model: i32,
	monsterType: i32,
	selected: bool,
	dummy: bool,
}

// std::pair<Uint32, FollowerBar_t> — 4-byte key + 4 padding + value. 288 bytes.
FollowerBarPair_t :: struct {
	first: u32,
	_pad: u32,
	second: FollowerBar_t,
}

#assert(size_of(HudBar_t) == 88)
#assert(size_of(FollowerBar_t) == 280)
#assert(size_of(FollowerBarPair_t) == 288)

follower_bar_pair_free :: proc(p: rawptr) {
	v := (^FollowerBarPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.second.name))
	dynamic_string_free_elem(rawptr(&v.second.customPortraitPath))
}

follower_bar_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^FollowerBarPair_t)(dst)
	s := (^FollowerBarPair_t)(src)
	d^ = s^
	d.second.name = DynamicString{}
	d.second.customPortraitPath = DynamicString{}
	dynamic_string_copy_elem(rawptr(&d.second.name), rawptr(&s.second.name))
	dynamic_string_copy_elem(rawptr(&d.second.customPortraitPath), rawptr(&s.second.customPortraitPath))
}

// Layout guards — must match the C++ SaveGameInfo::Player / stat_t sizeof
// exactly (MSVC x64, real_t=double). Catch any field drift at compile time.
#assert(size_of(HiscoreItem_t) == 32)
#assert(size_of(HiscoreLootbag_t) == 56)
#assert(size_of(HiscoreStat_t) == 528)
#assert(size_of(HiscorePlayer_t) == 1816)
#assert(size_of(HiscoreAttributesPair_t) == 32)
#assert(size_of(HiscorePlayerEquipPair_t) == 24)
#assert(size_of(HiscoreNpcEquipPair_t) == 48)
#assert(size_of(HiscoreLootbagPair_t) == 64)
#assert(size_of(HiscoreCompendiumEventPair_t) == 56)

// Non-hiscore array-element mirrors — lock to C++ sizeof.
#assert(size_of(Book_t) == 72)
#assert(size_of(StoreSlots_t) == 48)
#assert(size_of(SDL_Rect) == 16)
#assert(size_of(StatusEffectQueueEntry_t) == 160)
#assert(size_of(ItemTooltipIcons_t) == 56)
#assert(size_of(DropdownOption_t) == 64)
#assert(size_of(EntryVariable_t) == 40)
#assert(size_of(FollowerGenerateDetails_t) == 40)
#assert(size_of(Level_t) == 56)
#assert(size_of(CodexItem_t) == 32)
#assert(size_of(ShopkeeperItemEntry_t) == 256)
#assert(size_of(VariantPair_t) == 24)
#assert(size_of(MonsterCurveEntry_t) == 64)
#assert(size_of(LevelCurve_t) == 96)
#assert(size_of(TmpItem_t) == 184)
#assert(size_of(MapGeneration_t) == 208)
#assert(size_of(HotbarEntry_t) == 88)
#assert(size_of(SkillEffect_t) == 160)
#assert(size_of(SkillEntry_t) == 208)
#assert(size_of(PanelEntry_t) == 80)
#assert(size_of(AssistNotification_t) == 72)
#assert(size_of(AssistNotificationPair_t) == 80)
#assert(size_of(AlchNotification_t) == 64)
#assert(size_of(AlchNotificationPair_t) == 72)
#assert(size_of(CalloutPanelEntry_t) == 48)

hiscore_player_free :: proc(p: rawptr) {
	v := (^HiscorePlayer_t)(p)
	barony_dynamic_array_elem_destroy(&v.kills, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.spells, size_of(u32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.known_recipes, 12, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.known_scrolls, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.shopkeeperHostility, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.compendium_item_events, size_of(HiscoreCompendiumEventPair_t), Kind_HiscoreCompendiumPair)
	barony_dynamic_array_elem_destroy(&v.itemDegradeRNG, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.escalatingRngRolls, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.escalatingSpellRngRolls, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.appraisal_item_progress, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.learnedSpells, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.sustainedSpellIDCounter, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.ducksInARow, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_destroy(&v.favoriteBooksAchievement, size_of(i32)*2, Kind_POD)
	hiscore_stat_free(&v.stats)
	barony_dynamic_array_elem_destroy(&v.followers, size_of(HiscoreStat_t), Kind_HiscoreStat)
}

hiscore_player_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^HiscorePlayer_t)(dst)
	s := (^HiscorePlayer_t)(src)
	d.char_class = s.char_class
	d.race = s.race
	d.conductPenniless = s.conductPenniless
	d.conductFoodless = s.conductFoodless
	d.conductVegetarian = s.conductVegetarian
	d.conductIlliterate = s.conductIlliterate
	d.additionalConducts = s.additionalConducts
	d.gameStatistics = s.gameStatistics
	d.hotbar = s.hotbar
	d.hotbar_alternate = s.hotbar_alternate
	d.selected_spell = s.selected_spell
	d.selected_spell_alternate = s.selected_spell_alternate
	d.selected_spell_last_appearance = s.selected_spell_last_appearance
	barony_dynamic_array_elem_copy(&d.kills, &s.kills, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.spells, &s.spells, size_of(u32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.known_recipes, &s.known_recipes, 12, Kind_POD)
	barony_dynamic_array_elem_copy(&d.known_scrolls, &s.known_scrolls, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.shopkeeperHostility, &s.shopkeeperHostility, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.compendium_item_events, &s.compendium_item_events, size_of(HiscoreCompendiumEventPair_t), Kind_HiscoreCompendiumPair)
	barony_dynamic_array_elem_copy(&d.itemDegradeRNG, &s.itemDegradeRNG, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.escalatingRngRolls, &s.escalatingRngRolls, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.escalatingSpellRngRolls, &s.escalatingSpellRngRolls, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.appraisal_item_progress, &s.appraisal_item_progress, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.learnedSpells, &s.learnedSpells, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.sustainedSpellIDCounter, &s.sustainedSpellIDCounter, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.ducksInARow, &s.ducksInARow, size_of(i32)*2, Kind_POD)
	barony_dynamic_array_elem_copy(&d.favoriteBooksAchievement, &s.favoriteBooksAchievement, size_of(i32)*2, Kind_POD)
	d.sustainedSpellMPUsedSorcery = s.sustainedSpellMPUsedSorcery
	d.sustainedSpellMPUsedMysticism = s.sustainedSpellMPUsedMysticism
	d.sustainedSpellMPUsedThaumaturgy = s.sustainedSpellMPUsedThaumaturgy
	d.baseSpellMPUsedSorcery = s.baseSpellMPUsedSorcery
	d.baseSpellMPUsedMysticism = s.baseSpellMPUsedMysticism
	d.baseSpellMPUsedThaumaturgy = s.baseSpellMPUsedThaumaturgy
	hiscore_stat_copy(&d.stats, &s.stats)
	barony_dynamic_array_elem_copy(&d.followers, &s.followers, size_of(HiscoreStat_t), Kind_HiscoreStat)
}

// kind -> {free, copy} ops table. POD (kind 0) = nil/nil = raw bytes.
Element_Ops :: struct {
	free: proc(rawptr),
	copy: proc(dst: rawptr, src: rawptr),
}

element_ops := [35]Element_Ops{
	0 = { free = nil,                   copy = nil },
	1 = { free = dynamic_string_free_elem, copy = dynamic_string_copy_elem },
	2 = { free = icon_free,             copy = icon_copy },
	3 = { free = dropdown_option_free,  copy = dropdown_option_copy },
	4 = { free = entry_var_free,        copy = entry_var_copy },
	5 = { free = follower_details_free, copy = follower_details_copy },
	6 = { free = level_t_free,          copy = level_t_copy },
	7 = { free = codex_item_free,       copy = codex_item_copy },
	8 = { free = shopkeeper_item_entry_free, copy = shopkeeper_item_entry_copy },
	9 = { free = variant_pair_free,     copy = variant_pair_copy },
	10 = { free = monster_curve_entry_free, copy = monster_curve_entry_copy },
	11 = { free = level_curve_free,     copy = level_curve_copy },
	12 = { free = tmp_item_free,        copy = tmp_item_copy },
	13 = { free = map_generation_free,  copy = map_generation_copy },
	14 = { free = hotbar_entry_free,    copy = hotbar_entry_copy },
	15 = { free = hotbar_entry_array_free, copy = hotbar_entry_array_copy },
	16 = { free = dynarrstr_array_free, copy = dynarrstr_array_copy },
	17 = { free = skill_effect_free,    copy = skill_effect_copy },
	18 = { free = skill_entry_free,     copy = skill_entry_copy },
	19 = { free = panel_entry_free,     copy = panel_entry_copy },
	20 = { free = assist_notif_free,    copy = assist_notif_copy },
	21 = { free = alch_notif_free,      copy = alch_notif_copy },
	22 = { free = callout_panel_entry_free, copy = callout_panel_entry_copy },
	23 = { free = hiscore_stat_free,    copy = hiscore_stat_copy },
	24 = { free = hiscore_lootbag_free, copy = hiscore_lootbag_copy },
	25 = { free = hiscore_player_free,  copy = hiscore_player_copy },
	26 = { free = book_free,            copy = book_copy },
	27 = { free = store_slots_free,    copy = store_slots_copy },
	28 = { free = status_effect_queue_entry_free, copy = status_effect_queue_entry_copy },
	29 = { free = hiscore_attributes_pair_free, copy = hiscore_attributes_pair_copy },
	30 = { free = hiscore_player_equip_pair_free, copy = hiscore_player_equip_pair_copy },
	31 = { free = hiscore_npc_equip_pair_free, copy = hiscore_npc_equip_pair_copy },
	32 = { free = hiscore_lootbag_pair_free, copy = hiscore_lootbag_pair_copy },
	33 = { free = hiscore_compendium_pair_free, copy = hiscore_compendium_pair_copy },
	34 = { free = follower_bar_pair_free, copy = follower_bar_pair_copy },
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
