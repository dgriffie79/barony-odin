// dynamic_map.odin -- C-ABI shim for maps, callable from C++.
//
// Replaces std::map / std::unordered_map in shared structs. Uses ODIN'S NATIVE
// map (hash map) -- not a hand-rolled container -- so growth/hash/equal are the
// battle-tested builtins. The C++ mirror is Raw_Map (32 bytes on x64).
//
// GENERIC (since D3l): the per-value-type proc families (strworldicon_*,
// strspecialnpc_*, ... ~22 families, 207 exports) are replaced by TWO key-type
// families (str-key + [4]byte-key) whose procs take a value_kind and dispatch
// through a polymorphic core (`$V: typeid`). The exported proc "c" is a thin
// switch; the map mechanics live ONCE in the polymorphic core. Value free/copy
// (for owned values) go through the shared Element_Ops table.
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
// Shared global string interner (key ownership for string-keyed maps)
// ---------------------------------------------------------------------------
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



LightDef :: struct {
	radius:      i32,
	r, g, b, a:  f32,
	falloff_exp: f32,
	shadows:     bool,
}

IconEntryTextMap_t :: struct {
	text:       DynamicString,       // Raw_String {data, len}
	highlights: map[i32]struct{},    // Raw_Map (32B)
}

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

WorldIconEntry_t :: struct {
	pathDefault:  DynamicString,
	pathPlayer1:  DynamicString,
	pathPlayer2:  DynamicString,
	pathPlayer3:  DynamicString,
	pathPlayer4:  DynamicString,
	pathPlayerX:  DynamicString,
	id:           i32,
}

DiscoveryAnim_t :: struct {
	startTicks:      u32,
	processedOnTick: u32,
	name:            DynamicString,
}

SpecialNPCEntry_t :: struct {
	internalName: DynamicString,
	name:         DynamicString,
	shortname:    DynamicString,
	modelIndexes: map[i32]struct{},
	baseModel:    i32,
	uniqueIcon:   DynamicString,
}

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

ItemLocalization_t :: struct {
	name_identified:   DynamicString,
	name_unidentified: DynamicString,
}

Achievement_t :: struct {
	name:       DynamicString,
	unlocked:   bool,
	unlockTime: i64,
}

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

IconEntryCallout_t :: struct {
	name:            DynamicString,
	id:              i32,
	path:            DynamicString,
	path_hover:      DynamicString,
	path_active:     DynamicString,
	path_active_hover: DynamicString,
	icon_offsetx:    i32,
	icon_offsety:    i32,
	text_map:        map[string]IconEntryText_t,
}

binding_t :: struct {
	input:       DynamicString,
	analog:      f32,
	binary:      bool,
	consumed:    bool,
	heldTicks:   u32,
	_type:       i32,   // bindtype_t enum (INVALID..NUM)
	keycode:     i64,   // SDL_Keycode
	padIndex:    i32,
	pad:         rawptr,  // SDL_GameController* (non-owning)
	padAxis:     i32,     // SDL_GameControllerAxis
	padButton:   i32,     // SDL_GameControllerButton
	padAxisNegative: bool,
	joystick:    rawptr,  // SDL_Joystick* (non-owning)
	joystickAxis: i32,
	joystickAxisNegative: bool,
	joystickButton: i32,
	joystickHat: i32,
	joystickHatState: u8,
	mouseButton: i32,
}

Class_t :: struct {
	dlc:               i32,
	image:             rawptr,  // const char* (non-owning)
	image_highlighted: rawptr,
	image_locked:      rawptr,
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

achievement_data_free :: proc(v: ^AchievementData_t) {
	strings := [?]^DynamicString{ &v.name, &v.desc, &v.desc_formatted, &v.category }
	for s in strings {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
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

achievement_t_free :: proc(v: ^Achievement_t) {
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
}

binding_t_copy :: proc(dst: ^binding_t, src: ^binding_t) {
	dst^ = src^
	dst.input = DynamicString{}
	if src.input.len > 0 {
		buf, _ := mem.alloc(src.input.len + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, src.input.data, src.input.len)
			(^u8)(uintptr(buf) + uintptr(src.input.len))^ = 0
			dst.input = DynamicString{ data = buf, len = src.input.len }
		}
	}
}

binding_t_free :: proc(v: ^binding_t) {
	if v.input.data != nil {
		mem.free(v.input.data)
		v.input.data = nil
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

discovery_anim_free :: proc(v: ^DiscoveryAnim_t) {
	if v.name.data != nil {
		mem.free(v.name.data)
		v.name.data = nil
	}
}

icon_entry_callout_copy :: proc(dst: ^IconEntryCallout_t, src: ^IconEntryCallout_t) {
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
		dst.text_map = make(map[string]IconEntryText_t)
		for key in src.text_map {
			_, vp, _, err := map_entry(&src.text_map, key)
			if err == nil && vp != nil {
				new_val: IconEntryText_t
				icon_entry_text_copy(&new_val, vp)
				dst.text_map[key] = new_val
			}
		}
	}
}

icon_entry_callout_free :: proc(v: ^IconEntryCallout_t) {
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
				icon_entry_text_free(vp)
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

world_icon_entry_free :: proc(v: ^WorldIconEntry_t) {
	fields := [?]^DynamicString{ &v.pathDefault, &v.pathPlayer1, &v.pathPlayer2, &v.pathPlayer3, &v.pathPlayer4, &v.pathPlayerX }
	for s in fields {
		if s.data != nil {
			mem.free(s.data)
			s.data = nil
		}
	}
}

@(export)
barony_dynamic_set_i32_init :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	s^ = nil
}

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

@(export)
barony_dynamic_set_i32_contains :: proc "c" (s: ^map[i32]struct{}, value: i32) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, ok := s[value]
	return ok
}

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

@(export)
barony_dynamic_set_i32_clear :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		clear(&s^)
	}
}

@(export)
barony_dynamic_set_i32_len :: proc "c" (s: ^map[i32]struct{}) -> i32 {
	context = runtime.default_context()
	if s^ == nil {
		return 0
	}
	return i32(len(s^))
}

@(export)
barony_dynamic_set_i32_destroy :: proc "c" (s: ^map[i32]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		delete(s^)
		s^ = nil
	}
}

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

@(export)
barony_dynamic_set_str_init :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	s^ = nil
}

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

@(export)
barony_dynamic_set_str_contains :: proc "c" (s: ^map[string]struct{}, value: string) -> bool {
	context = runtime.default_context()
	if s^ == nil {
		return false
	}
	_, ok := s[value]
	return ok
}

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

@(export)
barony_dynamic_set_str_clear :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		clear(&s^)
	}
}

@(export)
barony_dynamic_set_str_len :: proc "c" (s: ^map[string]struct{}) -> i32 {
	context = runtime.default_context()
	if s^ == nil {
		return 0
	}
	return i32(len(s^))
}

@(export)
barony_dynamic_set_str_destroy :: proc "c" (s: ^map[string]struct{}) {
	context = runtime.default_context()
	if s^ != nil {
		delete(s^)
		s^ = nil
	}
}

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
// GENERIC map families (replaces the per-value-type families).
// Two key-type families: barony_dynamic_map_str_* (string keys) and
// barony_dynamic_map_i32_* ([4]byte keys). Each exported proc "c" takes
// value_kind and dispatches through a polymorphic core ($V: typeid) that
// writes the map mechanics ONCE. Value free/copy (owned values) come from the
// shared Element_Ops table (see dynamic_array.odin).
//
// value_kind mapping (must match value_kind_of<V> on the C++ side):
//   0  i32            1  f32            2  u32            3  string
//   4  LightDef       5  IconEntryTextMap_t  6  IconEntryText_t
//   7  WorldIconEntry_t  8  DiscoveryAnim_t  9  SpecialNPCEntry_t
//   10 ColliderDmgProperties_t  11 ItemLocalization_t  12 Achievement_t
//   13 AchievementData_t  14 IconEntry_t  15 IconEntryCallout_t
//   16 binding_t      17 Class_t        18 Raw_Dynamic_Array (strarrstr)
// ---------------------------------------------------------------------------

// ops-table alias (defined in dynamic_array.odin)
Value_Ops :: Element_Ops

// ---- polymorphic cores: string-keyed maps ----

str_map_put :: proc(m: rawptr, key: string, value: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		mm^ = make(map[string]V)
	}
	k := intern_string(key)
	if old, had := mm[k]; had {
		if ops.free != nil {
			ops.free(&old)
		}
	}
	new_val: V
	if ops.copy != nil {
		ops.copy(&new_val, value)
	} else {
		new_val = (^V)(value)^
	}
	mm[k] = new_val
}

str_map_get :: proc(m: rawptr, key: string, out: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		return false
	}
	v, ok := mm[key]
	if ok {
		if ops.copy != nil {
			ops.copy(out, &v)
		} else {
			(^V)(out)^ = v
		}
	}
	return ok
}

str_map_len :: proc(m: rawptr, $V: typeid) -> i32 {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		return 0
	}
	return i32(len(mm^))
}

str_map_clear :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[string]V)(m)
	if mm^ != nil {
		if ops.free != nil {
			for k in mm^ {
				_, vp, _, err := map_entry(mm, k)
				if err == nil && vp != nil {
					ops.free(vp)
				}
			}
		}
		clear(&mm^)
	}
}

str_map_destroy :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[string]V)(m)
	if mm^ != nil {
		if ops.free != nil {
			for k in mm^ {
				_, vp, _, err := map_entry(mm, k)
				if err == nil && vp != nil {
					ops.free(vp)
				}
			}
		}
		delete(mm^)
		mm^ = nil
	}
}

str_map_entry :: proc(m: rawptr, key: string, $V: typeid) -> rawptr {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		mm^ = make(map[string]V)
	}
	k := intern_string(key)
	_, vp, _, err := map_entry(mm, k)
	if err != nil {
		return nil
	}
	return vp
}

str_map_entries :: proc(m: rawptr, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: rawptr, count: i32, $V: typeid, ops: Value_Ops) -> i32 {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, &value in mm^ {
		if n >= count {
			break
		}
		key_ptrs[n] = raw_data(key)
		key_lens[n] = i32(len(key))
		if ops.copy != nil {
			ops.copy(([^]u8)(uintptr(val_ptrs) + uintptr(n) * uintptr(size_of(V))), &value)
		} else {
			(^V)(([^]u8)(uintptr(val_ptrs) + uintptr(n) * uintptr(size_of(V))))^ = value
		}
		n += 1
	}
	return n
}

// ---- polymorphic cores: [4]byte-keyed maps ----

i32_map_put :: proc(m: rawptr, key: rawptr, value: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		mm^ = make(map[[4]byte]V)
	}
	k := (^[4]byte)(key)^
	if old, had := mm[k]; had {
		if ops.free != nil {
			ops.free(&old)
		}
	}
	new_val: V
	if ops.copy != nil {
		ops.copy(&new_val, value)
	} else {
		new_val = (^V)(value)^
	}
	mm[k] = new_val
}

i32_map_get :: proc(m: rawptr, key: rawptr, out: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		return false
	}
	k := (^[4]byte)(key)^
	v, ok := mm[k]
	if ok {
		if ops.copy != nil {
			ops.copy(out, &v)
		} else {
			(^V)(out)^ = v
		}
	}
	return ok
}

i32_map_len :: proc(m: rawptr, $V: typeid) -> i32 {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		return 0
	}
	return i32(len(mm^))
}

i32_map_clear :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ != nil {
		if ops.free != nil {
			for k in mm^ {
				_, vp, _, err := map_entry(mm, k)
				if err == nil && vp != nil {
					ops.free(vp)
				}
			}
		}
		clear(&mm^)
	}
}

i32_map_destroy :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ != nil {
		if ops.free != nil {
			for k in mm^ {
				_, vp, _, err := map_entry(mm, k)
				if err == nil && vp != nil {
					ops.free(vp)
				}
			}
		}
		delete(mm^)
		mm^ = nil
	}
}

i32_map_entry :: proc(m: rawptr, key: rawptr, $V: typeid) -> rawptr {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		mm^ = make(map[[4]byte]V)
	}
	k := (^[4]byte)(key)^
	_, vp, _, err := map_entry(mm, k)
	if err != nil {
		return nil
	}
	return vp
}

i32_map_entries :: proc(m: rawptr, key_ptrs: [^][4]byte, val_ptrs: rawptr, count: i32, $V: typeid, ops: Value_Ops) -> i32 {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil || count <= 0 {
		return 0
	}
	n := i32(0)
	for key, &value in mm^ {
		if n >= count {
			break
		}
		key_ptrs[n] = key
		if ops.copy != nil {
			ops.copy(([^]u8)(uintptr(val_ptrs) + uintptr(n) * uintptr(size_of(V))), &value)
		} else {
			(^V)(([^]u8)(uintptr(val_ptrs) + uintptr(n) * uintptr(size_of(V))))^ = value
		}
		n += 1
	}
	return n
}

// ---- element ops per value kind (must match value_kind_of<V> on the C++ side) ----
// POD kinds (no free/copy): 0=i32 1=f32 2=u32 4=LightDef 17=Class_t
// string (3) and the struct kinds get deep free/copy. string values are OWNED
// (alloc'd on put, freed on destroy) exactly like DynamicString elements.

string_value_free :: proc(p: rawptr) {
	s := (^string)(p)
	if raw_data(s^) != nil {
		mem.free(raw_data(s^))
	}
	s^ = ""
}

string_value_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^string)(dst)
	s := (^string)(src)
	if raw_data(d^) != nil {
		mem.free(raw_data(d^))
	}
	d^ = ""
	if len(s^) > 0 {
		buf, _ := mem.alloc(len(s^) + 1, align_of(u8))
		if buf != nil {
			runtime.mem_copy(buf, raw_data(s^), len(s^))
			(^u8)(uintptr(buf) + uintptr(len(s^)))^ = 0
			d^ = string(([^]u8)(buf)[:len(s^)])
		}
	}
}

// strarrstr value = Raw_Dynamic_Array of DynamicString (deep array)
dynarrstr_value_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(DynamicString), Kind_DynamicString)
}
dynarrstr_value_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(DynamicString), Kind_DynamicString)
}

dynarrs32_value_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(i32), Kind_POD)
}
dynarrs32_value_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(i32), Kind_POD)
}

StatueLimb_t :: struct {
	x:      f32,
	y:      f32,
	z:      f32,
	pitch:  f32,
	roll:   f32,
	yaw:    f32,
	focalx: f32,
	focaly: f32,
	focalz: f32,
	sprite: i32,
	visible: b32,
}

dynarr_statuelimb_value_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(StatueLimb_t), Kind_POD)
}
dynarr_statuelimb_value_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(StatueLimb_t), Kind_POD)
}

dynarr_storeslots_value_free :: proc(p: rawptr) {
	barony_dynamic_array_elem_destroy((^Raw_Dynamic_Array)(p), size_of(StoreSlots_t), Kind_StoreSlots)
}
dynarr_storeslots_value_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_array_elem_copy((^Raw_Dynamic_Array)(dst), (^Raw_Dynamic_Array)(src), size_of(StoreSlots_t), Kind_StoreSlots)
}

MonsterTrapIgnoreEntities_t :: struct {
	ignoreEntities: Raw_Map,  // map[i32]struct{} (DynamicSetI32)
	parent:         u32,
}

monster_trap_ignore_free :: proc(p: rawptr) {
	v := (^MonsterTrapIgnoreEntities_t)(p)
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(&v.ignoreEntities))
}

monster_trap_ignore_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^MonsterTrapIgnoreEntities_t)(dst)
	s := (^MonsterTrapIgnoreEntities_t)(src)
	d.parent = s.parent
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(&d.ignoreEntities), transmute(^map[i32]struct{})(&s.ignoreEntities))
}

set_i32_value_free :: proc(p: rawptr) {
	barony_dynamic_set_i32_destroy(transmute(^map[i32]struct{})(p))
}

set_i32_value_copy :: proc(dst: rawptr, src: rawptr) {
	barony_dynamic_set_i32_copy(transmute(^map[i32]struct{})(dst), transmute(^map[i32]struct{})(src))
}

// game Item — 56-byte POD (verified via static_assert(sizeof(Item)==56))
Item_Game :: struct {
	bytes: [56]u8,
}

// game Stat::Lootbag_t — {i32 spawn_x, i32 spawn_y, bool spawnedOnGround,
// bool looted, Raw_Dynamic_Array items (POD Item bytes)}
Lootbag_t :: struct {
	spawn_x:         i32,
	spawn_y:         i32,
	spawnedOnGround: bool,
	looted:          bool,
	items:           Raw_Dynamic_Array,
}

lootbag_free :: proc(p: rawptr) {
	v := (^Lootbag_t)(p)
	barony_dynamic_array_elem_destroy(&v.items, size_of(Item_Game), Kind_POD)
}

lootbag_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Lootbag_t)(dst)
	s := (^Lootbag_t)(src)
	d.spawn_x = s.spawn_x
	d.spawn_y = s.spawn_y
	d.spawnedOnGround = s.spawnedOnGround
	d.looted = s.looted
	barony_dynamic_array_elem_copy(&d.items, &s.items, size_of(Item_Game), Kind_POD)
}

// EnemyHPDamageBarHandler::EnemyHPDetails — POD fields + one DynamicString
BarAnimator_t :: struct {
	foregroundValue: f32,
	backgroundValue: f32,
	previousSetpoint: f32,
	setpoint: i32,
	animateTicks: u32,
	damageTaken: i32,
	widthMultiplier: f32,
	maxValue: f32,
	currentOpacity: f32,
	fadeOut: f32,
	fadeIn: f32,
	skullOpacities: [4]f32,
	damageFrameOpacity: f32,
}

EnemyHPDetails_t :: struct {
	barType: i32,
	animator: BarAnimator_t,
	enemy_name: DynamicString,
	enemy_hp: i32,
	enemy_maxhp: i32,
	enemy_oldhp: i32,
	enemy_timer: u32,
	enemy_uid: u32,
	enemy_statusEffects1: u32,
	enemy_statusEffects2: u32,
	enemy_statusEffects3: u32,
	enemy_statusEffects4: u32,
	enemy_statusEffects5: u32,
	enemy_statusEffectsLowDuration1: u32,
	enemy_statusEffectsLowDuration2: u32,
	enemy_statusEffectsLowDuration3: u32,
	enemy_statusEffectsLowDuration4: u32,
	enemy_statusEffectsLowDuration5: u32,
	lowPriorityTick: bool,
	shouldDisplay: bool,
	hasDistanceCheck: bool,
	displayOnHUD: bool,
	expired: bool,
	detectMonsterCheckStatus: bool,
	depletionAnimationPercent: f32,
}

enemy_hp_details_free :: proc(p: rawptr) {
	v := (^EnemyHPDetails_t)(p)
	dynamic_string_free_elem(rawptr(&v.enemy_name))
}

enemy_hp_details_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^EnemyHPDetails_t)(dst)
	s := (^EnemyHPDetails_t)(src)
	d^ = s^
	d.enemy_name = DynamicString{}
	dynamic_string_copy_elem(rawptr(&d.enemy_name), rawptr(&s.enemy_name))
}

// GlyphRenderer_t::GlyphData_t — 8 DynamicStrings + 3 ints (owning)
GlyphData_t :: struct {
	keyname: DynamicString,
	folder: DynamicString,
	fullpath: DynamicString,
	pressedRenderedFullpath: DynamicString,
	unpressedRenderedFullpath: DynamicString,
	filename: DynamicString,
	unpressedGlyphPath: DynamicString,
	pressedGlyphPath: DynamicString,
	render_offsetx: i32,
	render_offsety: i32,
	keycode: i32,
}

glyph_data_free :: proc(p: rawptr) {
	g := (^GlyphData_t)(p)
	fields := [?]^DynamicString{ &g.keyname, &g.folder, &g.fullpath, &g.pressedRenderedFullpath, &g.unpressedRenderedFullpath, &g.filename, &g.unpressedGlyphPath, &g.pressedGlyphPath }
	for f in fields {
		if f.data != nil { mem.free(f.data); f.data = nil }
		f.len = 0
	}
}

glyph_data_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^GlyphData_t)(dst)
	s := (^GlyphData_t)(src)
	d^ = s^
	srcs := [?]^DynamicString{ &s.keyname, &s.folder, &s.fullpath, &s.pressedRenderedFullpath, &s.unpressedRenderedFullpath, &s.filename, &s.unpressedGlyphPath, &s.pressedGlyphPath }
	dsts := [?]^DynamicString{ &d.keyname, &d.folder, &d.fullpath, &d.pressedRenderedFullpath, &d.unpressedRenderedFullpath, &d.filename, &d.unpressedGlyphPath, &d.pressedGlyphPath }
	for i in 0..<len(srcs) {
		dynamic_string_copy_elem(rawptr(dsts[i]), rawptr(srcs[i]))
	}
}

// SteamAchievements_t::Statistic_t — 1 DynamicString + int (owning)
Statistic_t :: struct {
	name: DynamicString,
	value: i32,
}

statistic_free :: proc(p: rawptr) {
	s := (^Statistic_t)(p)
	if s.name.data != nil { mem.free(s.name.data); s.name.data = nil }
	s.name.len = 0
}

statistic_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Statistic_t)(dst)
	s := (^Statistic_t)(src)
	d^ = s^
	dynamic_string_copy_elem(rawptr(&d.name), rawptr(&s.name))
}

// kind -> ops lookup. POD kinds use {nil, nil} (raw byte copy, no free).

icon_entry_text_map_free_raw :: proc(p: rawptr) {
	icon_entry_text_map_free((^IconEntryTextMap_t)(p))
}

icon_entry_text_map_copy_raw :: proc(dst: rawptr, src: rawptr) {
	icon_entry_text_map_copy((^IconEntryTextMap_t)(dst), (^IconEntryTextMap_t)(src))
}

icon_entry_text_free_raw :: proc(p: rawptr) {
	icon_entry_text_free((^IconEntryText_t)(p))
}

icon_entry_text_copy_raw :: proc(dst: rawptr, src: rawptr) {
	icon_entry_text_copy((^IconEntryText_t)(dst), (^IconEntryText_t)(src))
}

world_icon_entry_free_raw :: proc(p: rawptr) {
	world_icon_entry_free((^WorldIconEntry_t)(p))
}

world_icon_entry_copy_raw :: proc(dst: rawptr, src: rawptr) {
	world_icon_entry_copy((^WorldIconEntry_t)(dst), (^WorldIconEntry_t)(src))
}

discovery_anim_free_raw :: proc(p: rawptr) {
	discovery_anim_free((^DiscoveryAnim_t)(p))
}

discovery_anim_copy_raw :: proc(dst: rawptr, src: rawptr) {
	discovery_anim_copy((^DiscoveryAnim_t)(dst), (^DiscoveryAnim_t)(src))
}

special_npc_free_raw :: proc(p: rawptr) {
	special_npc_free((^SpecialNPCEntry_t)(p))
}

special_npc_copy_raw :: proc(dst: rawptr, src: rawptr) {
	special_npc_copy((^SpecialNPCEntry_t)(dst), (^SpecialNPCEntry_t)(src))
}

collider_dmg_free_raw :: proc(p: rawptr) {
	collider_dmg_free((^ColliderDmgProperties_t)(p))
}

collider_dmg_copy_raw :: proc(dst: rawptr, src: rawptr) {
	collider_dmg_copy((^ColliderDmgProperties_t)(dst), (^ColliderDmgProperties_t)(src))
}

item_localization_free_raw :: proc(p: rawptr) {
	item_localization_free((^ItemLocalization_t)(p))
}

item_localization_copy_raw :: proc(dst: rawptr, src: rawptr) {
	item_localization_copy((^ItemLocalization_t)(dst), (^ItemLocalization_t)(src))
}

achievement_t_free_raw :: proc(p: rawptr) {
	achievement_t_free((^Achievement_t)(p))
}

achievement_t_copy_raw :: proc(dst: rawptr, src: rawptr) {
	achievement_t_copy((^Achievement_t)(dst), (^Achievement_t)(src))
}

achievement_data_free_raw :: proc(p: rawptr) {
	achievement_data_free((^AchievementData_t)(p))
}

achievement_data_copy_raw :: proc(dst: rawptr, src: rawptr) {
	achievement_data_copy((^AchievementData_t)(dst), (^AchievementData_t)(src))
}

icon_entry_free_raw :: proc(p: rawptr) {
	icon_entry_free((^IconEntry_t)(p))
}

icon_entry_copy_raw :: proc(dst: rawptr, src: rawptr) {
	icon_entry_copy((^IconEntry_t)(dst), (^IconEntry_t)(src))
}

icon_entry_callout_free_raw :: proc(p: rawptr) {
	icon_entry_callout_free((^IconEntryCallout_t)(p))
}

icon_entry_callout_copy_raw :: proc(dst: rawptr, src: rawptr) {
	icon_entry_callout_copy((^IconEntryCallout_t)(dst), (^IconEntryCallout_t)(src))
}

binding_t_free_raw :: proc(p: rawptr) {
	binding_t_free((^binding_t)(p))
}

binding_t_copy_raw :: proc(dst: rawptr, src: rawptr) {
	binding_t_copy((^binding_t)(dst), (^binding_t)(src))
}

value_ops_for :: proc(kind: i32) -> Value_Ops {
	switch kind {
	case 0, 1, 2, 4, 17, 26:
		return Value_Ops{}
	case 3:
		return Value_Ops{ free = string_value_free, copy = string_value_copy }
	case 5:
		return Value_Ops{ free = icon_entry_text_map_free_raw, copy = icon_entry_text_map_copy_raw }
	case 6:
		return Value_Ops{ free = icon_entry_text_free_raw, copy = icon_entry_text_copy_raw }
	case 7:
		return Value_Ops{ free = world_icon_entry_free_raw, copy = world_icon_entry_copy_raw }
	case 8:
		return Value_Ops{ free = discovery_anim_free_raw, copy = discovery_anim_copy_raw }
	case 9:
		return Value_Ops{ free = special_npc_free_raw, copy = special_npc_copy_raw }
	case 10:
		return Value_Ops{ free = collider_dmg_free_raw, copy = collider_dmg_copy_raw }
	case 11:
		return Value_Ops{ free = item_localization_free_raw, copy = item_localization_copy_raw }
	case 12:
		return Value_Ops{ free = achievement_t_free_raw, copy = achievement_t_copy_raw }
	case 13:
		return Value_Ops{ free = achievement_data_free_raw, copy = achievement_data_copy_raw }
	case 14:
		return Value_Ops{ free = icon_entry_free_raw, copy = icon_entry_copy_raw }
	case 15:
		return Value_Ops{ free = icon_entry_callout_free_raw, copy = icon_entry_callout_copy_raw }
	case 16:
		return Value_Ops{ free = binding_t_free_raw, copy = binding_t_copy_raw }
	case 18:
		return Value_Ops{ free = dynarrstr_value_free, copy = dynarrstr_value_copy }
	case 19:
		return Value_Ops{ free = dynarrs32_value_free, copy = dynarrs32_value_copy }
	case 20:
		return Value_Ops{ free = dynarr_statuelimb_value_free, copy = dynarr_statuelimb_value_copy }
	case 21:
		return Value_Ops{ free = dynarr_storeslots_value_free, copy = dynarr_storeslots_value_copy }
	case 22:
		return Value_Ops{ free = monster_trap_ignore_free, copy = monster_trap_ignore_copy }
	case 23:
		return Value_Ops{ free = set_i32_value_free, copy = set_i32_value_copy }
	case 24:
		return Value_Ops{ free = lootbag_free, copy = lootbag_copy }
	case 25:
		return Value_Ops{ free = enemy_hp_details_free, copy = enemy_hp_details_copy }
	case 27:
		return Value_Ops{ free = glyph_data_free, copy = glyph_data_copy }
	case 28:
		return Value_Ops{ free = statistic_free, copy = statistic_copy }
	}
	return Value_Ops{}
}

// ---- exported str-key family (ONE per op, value_kind-dispatched) ----

@(export)
barony_dynamic_map_str_init :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	(^[32]byte)(m)^ = 0
}

@(export)
barony_dynamic_map_str_put :: proc "c" (m: rawptr, key: string, value: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  str_map_put(m, key, value, i32, ops)
	case 1:  str_map_put(m, key, value, f32, ops)
	case 2:  str_map_put(m, key, value, u32, ops)
	case 3:  str_map_put(m, key, value, string, ops)
	case 4:  str_map_put(m, key, value, LightDef, ops)
	case 5:  str_map_put(m, key, value, IconEntryTextMap_t, ops)
	case 6:  str_map_put(m, key, value, IconEntryText_t, ops)
	case 7:  str_map_put(m, key, value, WorldIconEntry_t, ops)
	case 8:  str_map_put(m, key, value, DiscoveryAnim_t, ops)
	case 9:  str_map_put(m, key, value, SpecialNPCEntry_t, ops)
	case 10: str_map_put(m, key, value, ColliderDmgProperties_t, ops)
	case 11: str_map_put(m, key, value, ItemLocalization_t, ops)
	case 12: str_map_put(m, key, value, Achievement_t, ops)
	case 13: str_map_put(m, key, value, AchievementData_t, ops)
	case 14: str_map_put(m, key, value, IconEntry_t, ops)
	case 15: str_map_put(m, key, value, IconEntryCallout_t, ops)
	case 16: str_map_put(m, key, value, binding_t, ops)
	case 17: str_map_put(m, key, value, Class_t, ops)
	case 18: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 19: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 20: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 21: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	}
}

@(export)
barony_dynamic_map_str_get :: proc "c" (m: rawptr, key: string, out: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return str_map_get(m, key, out, i32, ops)
	case 1:  return str_map_get(m, key, out, f32, ops)
	case 2:  return str_map_get(m, key, out, u32, ops)
	case 3:  return str_map_get(m, key, out, string, ops)
	case 4:  return str_map_get(m, key, out, LightDef, ops)
	case 5:  return str_map_get(m, key, out, IconEntryTextMap_t, ops)
	case 6:  return str_map_get(m, key, out, IconEntryText_t, ops)
	case 7:  return str_map_get(m, key, out, WorldIconEntry_t, ops)
	case 8:  return str_map_get(m, key, out, DiscoveryAnim_t, ops)
	case 9:  return str_map_get(m, key, out, SpecialNPCEntry_t, ops)
	case 10: return str_map_get(m, key, out, ColliderDmgProperties_t, ops)
	case 11: return str_map_get(m, key, out, ItemLocalization_t, ops)
	case 12: return str_map_get(m, key, out, Achievement_t, ops)
	case 13: return str_map_get(m, key, out, AchievementData_t, ops)
	case 14: return str_map_get(m, key, out, IconEntry_t, ops)
	case 15: return str_map_get(m, key, out, IconEntryCallout_t, ops)
	case 16: return str_map_get(m, key, out, binding_t, ops)
	case 17: return str_map_get(m, key, out, Class_t, ops)
	case 18: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 19: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 20: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 21: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	}
	return false
}

@(export)
barony_dynamic_map_str_len :: proc "c" (m: rawptr, value_kind: i32) -> i32 {
	context = runtime.default_context()
	switch value_kind {
	case 0:  return str_map_len(m, i32)
	case 1:  return str_map_len(m, f32)
	case 2:  return str_map_len(m, u32)
	case 3:  return str_map_len(m, string)
	case 4:  return str_map_len(m, LightDef)
	case 5:  return str_map_len(m, IconEntryTextMap_t)
	case 6:  return str_map_len(m, IconEntryText_t)
	case 7:  return str_map_len(m, WorldIconEntry_t)
	case 8:  return str_map_len(m, DiscoveryAnim_t)
	case 9:  return str_map_len(m, SpecialNPCEntry_t)
	case 10: return str_map_len(m, ColliderDmgProperties_t)
	case 11: return str_map_len(m, ItemLocalization_t)
	case 12: return str_map_len(m, Achievement_t)
	case 13: return str_map_len(m, AchievementData_t)
	case 14: return str_map_len(m, IconEntry_t)
	case 15: return str_map_len(m, IconEntryCallout_t)
	case 16: return str_map_len(m, binding_t)
	case 17: return str_map_len(m, Class_t)
	case 18: return str_map_len(m, Raw_Dynamic_Array)
	case 19: return str_map_len(m, Raw_Dynamic_Array)
	case 20: return str_map_len(m, Raw_Dynamic_Array)
	case 21: return str_map_len(m, Raw_Dynamic_Array)
	}
	return 0
}

@(export)
barony_dynamic_map_str_clear :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  str_map_clear(m, i32, ops)
	case 1:  str_map_clear(m, f32, ops)
	case 2:  str_map_clear(m, u32, ops)
	case 3:  str_map_clear(m, string, ops)
	case 4:  str_map_clear(m, LightDef, ops)
	case 5:  str_map_clear(m, IconEntryTextMap_t, ops)
	case 6:  str_map_clear(m, IconEntryText_t, ops)
	case 7:  str_map_clear(m, WorldIconEntry_t, ops)
	case 8:  str_map_clear(m, DiscoveryAnim_t, ops)
	case 9:  str_map_clear(m, SpecialNPCEntry_t, ops)
	case 10: str_map_clear(m, ColliderDmgProperties_t, ops)
	case 11: str_map_clear(m, ItemLocalization_t, ops)
	case 12: str_map_clear(m, Achievement_t, ops)
	case 13: str_map_clear(m, AchievementData_t, ops)
	case 14: str_map_clear(m, IconEntry_t, ops)
	case 15: str_map_clear(m, IconEntryCallout_t, ops)
	case 16: str_map_clear(m, binding_t, ops)
	case 17: str_map_clear(m, Class_t, ops)
	case 18: str_map_clear(m, Raw_Dynamic_Array, ops)
	case 19: str_map_clear(m, Raw_Dynamic_Array, ops)
	case 20: str_map_clear(m, Raw_Dynamic_Array, ops)
	case 21: str_map_clear(m, Raw_Dynamic_Array, ops)
	}
}

@(export)
barony_dynamic_map_str_destroy :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  str_map_destroy(m, i32, ops)
	case 1:  str_map_destroy(m, f32, ops)
	case 2:  str_map_destroy(m, u32, ops)
	case 3:  str_map_destroy(m, string, ops)
	case 4:  str_map_destroy(m, LightDef, ops)
	case 5:  str_map_destroy(m, IconEntryTextMap_t, ops)
	case 6:  str_map_destroy(m, IconEntryText_t, ops)
	case 7:  str_map_destroy(m, WorldIconEntry_t, ops)
	case 8:  str_map_destroy(m, DiscoveryAnim_t, ops)
	case 9:  str_map_destroy(m, SpecialNPCEntry_t, ops)
	case 10: str_map_destroy(m, ColliderDmgProperties_t, ops)
	case 11: str_map_destroy(m, ItemLocalization_t, ops)
	case 12: str_map_destroy(m, Achievement_t, ops)
	case 13: str_map_destroy(m, AchievementData_t, ops)
	case 14: str_map_destroy(m, IconEntry_t, ops)
	case 15: str_map_destroy(m, IconEntryCallout_t, ops)
	case 16: str_map_destroy(m, binding_t, ops)
	case 17: str_map_destroy(m, Class_t, ops)
	case 18: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case 19: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case 20: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case 21: str_map_destroy(m, Raw_Dynamic_Array, ops)
	}
}

@(export)
barony_dynamic_map_str_entry :: proc "c" (m: rawptr, key: string, value_kind: i32) -> rawptr {
	context = runtime.default_context()
	switch value_kind {
	case 0:  return str_map_entry(m, key, i32)
	case 1:  return str_map_entry(m, key, f32)
	case 2:  return str_map_entry(m, key, u32)
	case 3:  return str_map_entry(m, key, string)
	case 4:  return str_map_entry(m, key, LightDef)
	case 5:  return str_map_entry(m, key, IconEntryTextMap_t)
	case 6:  return str_map_entry(m, key, IconEntryText_t)
	case 7:  return str_map_entry(m, key, WorldIconEntry_t)
	case 8:  return str_map_entry(m, key, DiscoveryAnim_t)
	case 9:  return str_map_entry(m, key, SpecialNPCEntry_t)
	case 10: return str_map_entry(m, key, ColliderDmgProperties_t)
	case 11: return str_map_entry(m, key, ItemLocalization_t)
	case 12: return str_map_entry(m, key, Achievement_t)
	case 13: return str_map_entry(m, key, AchievementData_t)
	case 14: return str_map_entry(m, key, IconEntry_t)
	case 15: return str_map_entry(m, key, IconEntryCallout_t)
	case 16: return str_map_entry(m, key, binding_t)
	case 17: return str_map_entry(m, key, Class_t)
	case 18: return str_map_entry(m, key, Raw_Dynamic_Array)
	case 19: return str_map_entry(m, key, Raw_Dynamic_Array)
	case 20: return str_map_entry(m, key, Raw_Dynamic_Array)
	case 21: return str_map_entry(m, key, Raw_Dynamic_Array)
	}
	return nil
}

@(export)
barony_dynamic_map_str_entries :: proc "c" (m: rawptr, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: rawptr, count: i32, value_kind: i32) -> i32 {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, i32, ops)
	case 1:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, f32, ops)
	case 2:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, u32, ops)
	case 3:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, string, ops)
	case 4:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, LightDef, ops)
	case 5:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryTextMap_t, ops)
	case 6:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryText_t, ops)
	case 7:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, WorldIconEntry_t, ops)
	case 8:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, DiscoveryAnim_t, ops)
	case 9:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, SpecialNPCEntry_t, ops)
	case 10: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, ColliderDmgProperties_t, ops)
	case 11: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, ItemLocalization_t, ops)
	case 12: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Achievement_t, ops)
	case 13: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, AchievementData_t, ops)
	case 14: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntry_t, ops)
	case 15: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryCallout_t, ops)
	case 16: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, binding_t, ops)
	case 17: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Class_t, ops)
	case 18: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 19: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 20: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 21: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	}
	return 0
}

// erase: free the value at key (if owned), then delete the key
@(export)
barony_dynamic_map_str_erase :: proc "c" (m: rawptr, key: string, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	mm := transmute(^map[string]Raw_Dynamic_Array)(m)
	if mm^ == nil {
		return false
	}
	// polymorphic delete via the core (free first, then delete key)
	switch value_kind {
	case 0:  return str_map_erase(m, key, i32, ops)
	case 1:  return str_map_erase(m, key, f32, ops)
	case 2:  return str_map_erase(m, key, u32, ops)
	case 3:  return str_map_erase(m, key, string, ops)
	case 4:  return str_map_erase(m, key, LightDef, ops)
	case 5:  return str_map_erase(m, key, IconEntryTextMap_t, ops)
	case 6:  return str_map_erase(m, key, IconEntryText_t, ops)
	case 7:  return str_map_erase(m, key, WorldIconEntry_t, ops)
	case 8:  return str_map_erase(m, key, DiscoveryAnim_t, ops)
	case 9:  return str_map_erase(m, key, SpecialNPCEntry_t, ops)
	case 10: return str_map_erase(m, key, ColliderDmgProperties_t, ops)
	case 11: return str_map_erase(m, key, ItemLocalization_t, ops)
	case 12: return str_map_erase(m, key, Achievement_t, ops)
	case 13: return str_map_erase(m, key, AchievementData_t, ops)
	case 14: return str_map_erase(m, key, IconEntry_t, ops)
	case 15: return str_map_erase(m, key, IconEntryCallout_t, ops)
	case 16: return str_map_erase(m, key, binding_t, ops)
	case 17: return str_map_erase(m, key, Class_t, ops)
	case 18: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 19: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 20: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 21: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	}
	return false
}

str_map_erase :: proc(m: rawptr, key: string, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		return false
	}
	if v, had := mm[key]; had {
		if ops.free != nil {
			ops.free(&v)
		}
		runtime.delete_key(mm, key)
		return true
	}
	return false
}

// ---- exported [4]byte-key family ----

@(export)
barony_dynamic_map_i32_init :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	(^[32]byte)(m)^ = 0
}

@(export)
barony_dynamic_map_i32_put :: proc "c" (m: rawptr, key: rawptr, value: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  i32_map_put(m, key, value, i32, ops)
	case 2:  i32_map_put(m, key, value, u32, ops)
	case 26: i32_map_put(m, key, value, u64, ops)
	case 3:  i32_map_put(m, key, value, string, ops)
	case 18: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 19: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 20: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 21: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case 22: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case 23: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case 24: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case 25: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case 27: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case 28: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_get :: proc "c" (m: rawptr, key: rawptr, out: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return i32_map_get(m, key, out, i32, ops)
	case 2:  return i32_map_get(m, key, out, u32, ops)
	case 26: return i32_map_get(m, key, out, u64, ops)
	case 3:  return i32_map_get(m, key, out, string, ops)
	case 18: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 19: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 20: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 21: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case 22: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case 23: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case 24: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case 25: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case 27: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case 28: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	}
	return false
}

@(export)
barony_dynamic_map_i32_len :: proc "c" (m: rawptr, value_kind: i32) -> i32 {
	context = runtime.default_context()
	switch value_kind {
	case 0:  return i32_map_len(m, i32)
	case 2:  return i32_map_len(m, u32)
	case 26: return i32_map_len(m, u32)
	case 3:  return i32_map_len(m, string)
	case 18: return i32_map_len(m, Raw_Dynamic_Array)
	case 19: return i32_map_len(m, Raw_Dynamic_Array)
	case 20: return i32_map_len(m, Raw_Dynamic_Array)
	case 21: return i32_map_len(m, Raw_Dynamic_Array)
	case 22: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case 23: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case 24: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case 25: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case 27: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case 28: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	}
	return 0
}

@(export)
barony_dynamic_map_i32_clear :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  i32_map_clear(m, i32, ops)
	case 2:  i32_map_clear(m, u32, ops)
	case 26: i32_map_clear(m, u64, ops)
	case 3:  i32_map_clear(m, string, ops)
	case 18: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case 19: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case 20: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case 21: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case 22: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case 23: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case 24: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case 25: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case 27: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case 28: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_destroy :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  i32_map_destroy(m, i32, ops)
	case 2:  i32_map_destroy(m, u32, ops)
	case 26: i32_map_destroy(m, u64, ops)
	case 3:  i32_map_destroy(m, string, ops)
	case 18: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case 19: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case 20: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case 21: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case 22: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case 23: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case 24: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case 25: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case 27: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case 28: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_entry :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> rawptr {
	context = runtime.default_context()
	switch value_kind {
	case 0:  return i32_map_entry(m, key, i32)
	case 2:  return i32_map_entry(m, key, u32)
	case 26: return i32_map_entry(m, key, u32)
	case 3:  return i32_map_entry(m, key, string)
	case 18: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case 19: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case 20: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case 21: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case 22: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case 23: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case 24: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case 25: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case 27: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case 28: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	}
	return nil
}

@(export)
barony_dynamic_map_i32_entries :: proc "c" (m: rawptr, key_ptrs: [^][4]byte, val_ptrs: rawptr, count: i32, value_kind: i32) -> i32 {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return i32_map_entries(m, key_ptrs, val_ptrs, count, i32, ops)
	case 2:  return i32_map_entries(m, key_ptrs, val_ptrs, count, u32, ops)
	case 26: return i32_map_entries(m, key_ptrs, val_ptrs, count, u64, ops)
	case 3:  return i32_map_entries(m, key_ptrs, val_ptrs, count, string, ops)
	case 18: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 19: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 20: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 21: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case 22: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case 23: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case 24: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case 25: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case 27: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case 28: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	}
	return 0
}

@(export)
barony_dynamic_map_i32_erase :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return i32_map_erase(m, key, i32, ops)
	case 2:  return i32_map_erase(m, key, u32, ops)
	case 26: return i32_map_erase(m, key, u64, ops)
	case 3:  return i32_map_erase(m, key, string, ops)
	case 18: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 19: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 20: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 21: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case 22: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case 23: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case 24: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case 25: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case 27: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case 28: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	}
	return false
}

i32_map_erase :: proc(m: rawptr, key: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		return false
	}
	k := (^[4]byte)(key)^
	if v, had := mm[k]; had {
		if ops.free != nil {
			ops.free(&v)
		}
		runtime.delete_key(mm, k)
		return true
	}
	return false
}

// find (non-mutating): deep-copies the value out; returns the interned key
@(export)
barony_dynamic_map_str_find :: proc "c" (m: rawptr, key: string, out_key: ^rawptr, out_key_len: ^i32, out_val: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return str_map_find(m, key, out_key, out_key_len, out_val, i32, ops)
	case 1:  return str_map_find(m, key, out_key, out_key_len, out_val, f32, ops)
	case 2:  return str_map_find(m, key, out_key, out_key_len, out_val, u32, ops)
	case 3:  return str_map_find(m, key, out_key, out_key_len, out_val, string, ops)
	case 4:  return str_map_find(m, key, out_key, out_key_len, out_val, LightDef, ops)
	case 5:  return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryTextMap_t, ops)
	case 6:  return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryText_t, ops)
	case 7:  return str_map_find(m, key, out_key, out_key_len, out_val, WorldIconEntry_t, ops)
	case 8:  return str_map_find(m, key, out_key, out_key_len, out_val, DiscoveryAnim_t, ops)
	case 9:  return str_map_find(m, key, out_key, out_key_len, out_val, SpecialNPCEntry_t, ops)
	case 10: return str_map_find(m, key, out_key, out_key_len, out_val, ColliderDmgProperties_t, ops)
	case 11: return str_map_find(m, key, out_key, out_key_len, out_val, ItemLocalization_t, ops)
	case 12: return str_map_find(m, key, out_key, out_key_len, out_val, Achievement_t, ops)
	case 13: return str_map_find(m, key, out_key, out_key_len, out_val, AchievementData_t, ops)
	case 14: return str_map_find(m, key, out_key, out_key_len, out_val, IconEntry_t, ops)
	case 15: return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryCallout_t, ops)
	case 16: return str_map_find(m, key, out_key, out_key_len, out_val, binding_t, ops)
	case 17: return str_map_find(m, key, out_key, out_key_len, out_val, Class_t, ops)
	case 18: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case 19: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case 20: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case 21: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	}
	return false
}

str_map_find :: proc(m: rawptr, key: string, out_key: ^rawptr, out_key_len: ^i32, out_val: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		return false
	}
	v, ok := mm[key]
	if !ok {
		return false
	}
	if out_key != nil && out_key_len != nil {
		out_key^ = raw_data(key)
		out_key_len^ = i32(len(key))
	}
	if ops.copy != nil {
		ops.copy(out_val, &v)
	} else {
		(^V)(out_val)^ = v
	}
	return true
}

@(export)
barony_dynamic_map_i32_find :: proc "c" (m: rawptr, key: rawptr, out_val: rawptr, out_val_len: ^i32, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	switch value_kind {
	case 0:  return i32_map_find(m, key, out_val, out_val_len, i32, ops)
	case 2:  return i32_map_find(m, key, out_val, out_val_len, u32, ops)
	case 26: return i32_map_find(m, key, out_val, out_val_len, u64, ops)
	case 3:  return i32_map_find(m, key, out_val, out_val_len, string, ops)
	case 18: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case 19: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case 20: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case 21: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case 22: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case 23: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case 24: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case 25: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case 27: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case 28: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	}
	return false
}

i32_map_find :: proc(m: rawptr, key: rawptr, out_val: rawptr, out_val_len: ^i32, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		return false
	}
	k := (^[4]byte)(key)^
	v, ok := mm[k]
	if !ok {
		return false
	}
	if ops.copy != nil {
		ops.copy(out_val, &v)
	} else {
		(^V)(out_val)^ = v
	}
	if out_val_len != nil {
		out_val_len^ = i32(size_of(V))
	}
	return true
}

// set copy: snapshot src entries, insert into dst (dst must be empty/fresh)
@(export)
barony_dynamic_set_i32_copy :: proc "c" (dst: ^map[i32]struct{}, src: ^map[i32]struct{}) {
	context = runtime.default_context()
	if dst^ != nil {
		barony_dynamic_set_i32_destroy(dst)
	}
	if src^ == nil {
		return
	}
	values: [512]i32
	n := barony_dynamic_set_i32_entries(src, &values[0], 512)
	for i in 0..<int(n) {
		barony_dynamic_set_i32_insert(dst, values[i])
	}
}
