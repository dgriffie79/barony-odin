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

// Zero-copy iteration: the callback receives (key, key_len, value_ptr, userdata)
// where value_ptr points at the LIVE map slot. No value copy/free happens here,
// so this is the iteration path to use for hot/large maps. The value pointer is
// only valid for the duration of the callback (no insert/erase while iterating).
MapForeachCb :: #type proc "c" (key: rawptr, key_len: i32, value: rawptr, userdata: rawptr)

i32_map_for_each :: proc(m: rawptr, $V: typeid, cb: MapForeachCb, userdata: rawptr) {
	mm := transmute(^map[[4]byte]V)(m)
	if mm^ == nil {
		return
	}
	for key, &value in mm^ {
		k := key
		cb(rawptr(&k), 4, rawptr(&value), userdata)
	}
}

str_map_for_each :: proc(m: rawptr, $V: typeid, cb: MapForeachCb, userdata: rawptr) {
	mm := transmute(^map[string]V)(m)
	if mm^ == nil {
		return
	}
	for key, &value in mm^ {
		cb(rawptr(raw_data(key)), i32(len(key)), rawptr(&value), userdata)
	}
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
	x:      f64,
	y:      f64,
	z:      f64,
	pitch:  f64,
	roll:   f64,
	yaw:    f64,
	focalx: f64,
	focaly: f64,
	focalz: f64,
	sprite: i32,
	visible: bool,
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

// i32_map value = map[int]int (Raw_Map of i32). Owned nested map: deep-free
// on erase/clear/destroy, deep-copy on put/copy. Self-contained (no dependency
// on the exported i32 family, which is defined later in this file).
i32_map_value_free :: proc(p: rawptr) {
	mm := transmute(^map[[4]byte]i32)(p)
	if mm^ != nil {
		delete(mm^)
		mm^ = nil
	}
}

i32_map_value_copy :: proc(dst: rawptr, src: rawptr) {
	m := transmute(^map[[4]byte]i32)(dst)
	s := transmute(^map[[4]byte]i32)(src)
	if m^ != nil {
		delete(m^)
	}
	m^ = nil
	if s^ == nil {
		return
	}
	m^ = make(map[[4]byte]i32)
	for key in s^ {
		_, vp, _, err := map_entry(s, key)
		if err == nil && vp != nil {
			m^[key] = vp^
		}
	}
}

// u32_map value = map[uint]uint (Raw_Map of u32). Same shape as i32_map_value_*;
// separate kind because the inner value type differs.
// ParticleEmitterHit_t — 8B POD (Uint32 tick; int hits)
ParticleEmitterHit_t :: struct {
	tick: u32,
	hits: i32,
}

// DynamicStringPair_t — 32B owning (2 DynamicStrings). Value for
// map<string, pair<string,string>> (mapDisplayNamesDescriptions etc.).
DynamicStringPair_t :: struct {
	first:  DynamicString,
	second: DynamicString,
}

// EntityColliderData_t — owning (strings + arrays + str-keyed map + set + map).
// Mirror of EditorEntityData_t::EntityColliderData_t (methods live on the C++
// struct only; the Odin mirror holds the DATA fields, which is all the map
// value needs to own/free/copy).
EntityColliderData_t :: struct {
	gib:                     i32,
	gib_hit:                 Raw_Dynamic_Array,   // of i32
	sfxBreak:                Raw_Dynamic_Array,   // of i32
	sfxHit:                  i32,
	damageCalculationType:   DynamicString,
	name:                    DynamicString,
	hpbarLookupName:         DynamicString,
	entityLangEntry:         i32,
	hitMessageLangEntry:     i32,
	breakMessageLangEntry:   i32,
	hideMonsters:            map[string]Raw_Dynamic_Array, // str -> [dynamic]i32
	spellTriggers:           Raw_Dynamic_Array,   // of i32
	pathableMonsters:        map[i32]struct{},
	colliderJumpLangEntry:   i32,
	overrideProperties:      map[string]i32,
}

dynamic_string_pair_free :: proc(p: rawptr) {
	v := (^DynamicStringPair_t)(p)
	dynamic_string_free_elem(rawptr(&v.first))
	dynamic_string_free_elem(rawptr(&v.second))
}

dynamic_string_pair_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^DynamicStringPair_t)(dst)
	s := (^DynamicStringPair_t)(src)
	d^ = DynamicStringPair_t{}
	dynamic_string_copy_elem(rawptr(&d.first), rawptr(&s.first))
	dynamic_string_copy_elem(rawptr(&d.second), rawptr(&s.second))
}

entity_collider_data_free :: proc(p: rawptr) {
	v := (^EntityColliderData_t)(p)
	barony_dynamic_array_elem_destroy(&v.gib_hit, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.sfxBreak, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.spellTriggers, size_of(i32), Kind_POD)
	dynamic_string_free_elem(rawptr(&v.damageCalculationType))
	dynamic_string_free_elem(rawptr(&v.name))
	dynamic_string_free_elem(rawptr(&v.hpbarLookupName))
	if v.hideMonsters != nil {
		for key in v.hideMonsters {
			_, vp, _, err := map_entry(&v.hideMonsters, key)
			if err == nil && vp != nil {
				barony_dynamic_array_elem_destroy(vp, size_of(i32), Kind_POD)
			}
		}
		delete(v.hideMonsters)
		v.hideMonsters = nil
	}
	if v.pathableMonsters != nil { delete(v.pathableMonsters); v.pathableMonsters = nil }
	if v.overrideProperties != nil { delete(v.overrideProperties); v.overrideProperties = nil }
}

// spellItem_t — owning mirror of ItemTooltips_t::spellItem_t. real_t = f64 on
// x64; SpellItemTypes = i32; owns 9 DynamicStrings, 2 DynamicArrayStr, 2
// DynamicSetI32, 1 DynamicArrayS32.
SpellItem_t :: struct {
	id:                            i32,
	internalName:                  DynamicString,
	name:                          DynamicString,
	name_lowercase:                DynamicString,
	spellTypeStr:                  DynamicString,
	spellType:                     i32,
	spellbookInternalName:         DynamicString,
	magicstaffInternalName:        DynamicString,
	fociInternalName:              DynamicString,
	spellbookId:                   i32,
	magicstaffId:                  i32,
	fociId:                        i32,
	spellTagsStr:                  Raw_Dynamic_Array, // of DynamicString
	spellTags:                     map[i32]struct{},
	spellFormatTags:               Raw_Dynamic_Array, // of DynamicString
	spellbookItemIconPaddingLines: Raw_Dynamic_Array, // of i32
	spellLevelTags:                map[i32]struct{},
	hasExpandedJSON:               bool,
	damage:                        i32,
	damage2:                       i32,
	damage_mult:                   f64,
	damage2_mult:                  f64,
	duration:                      i32,
	duration_mult:                 f64,
	duration2:                     i32,
	duration2_mult:                f64,
	mana:                          i32,
	distance:                      f64,
	distance_mult:                 f64,
	life_time:                     i32,
	life_mult:                     f64,
	cast_time:                     f64,
	cast_time_mult:                f64,
	skillID:                       i32,
	difficulty:                    i32,
	sustain_mana:                  i32,
	sustain_duration:              i32,
	sustain_mult:                  f64,
	radius:                        f64,
	radius_mult:                   f64,
	drop_table:                    i32,
}

// ItemTooltip_t — owning mirror of ItemTooltips_t::ItemTooltip_t.
ItemTooltip_t :: struct {
	headingTextColor:       u32,
	descriptionTextColor:   u32,
	detailsTextColor:       u32,
	positiveTextColor:      u32,
	negativeTextColor:      u32,
	statusEffectTextColor:  u32,
	faintTextColor:         u32,
	icons:                  Raw_Dynamic_Array, // of ItemTooltipIcons_t
	descriptionText:        Raw_Dynamic_Array, // of DynamicString
	detailsText:            map[string]Raw_Dynamic_Array, // str -> [dynamic]DynamicString
	detailsTextInsertOrder: Raw_Dynamic_Array, // of DynamicString
	minWidths:              map[string]i32,
	maxWidths:              map[string]i32,
	headerMaxWidths:        map[string]i32,
}

item_tooltip_free :: proc(p: rawptr) {
	v := (^ItemTooltip_t)(p)
	barony_dynamic_array_elem_destroy(&v.icons, size_of(ItemTooltipIcons_t), Kind_Icon)
	barony_dynamic_array_elem_destroy(&v.descriptionText, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.detailsTextInsertOrder, size_of(DynamicString), Kind_DynamicString)
	if v.detailsText != nil {
		for key in v.detailsText {
			_, vp, _, err := map_entry(&v.detailsText, key)
			if err == nil && vp != nil {
				barony_dynamic_array_elem_destroy(vp, size_of(DynamicString), Kind_DynamicString)
			}
		}
		delete(v.detailsText)
		v.detailsText = nil
	}
	if v.minWidths != nil { delete(v.minWidths); v.minWidths = nil }
	if v.maxWidths != nil { delete(v.maxWidths); v.maxWidths = nil }
	if v.headerMaxWidths != nil { delete(v.headerMaxWidths); v.headerMaxWidths = nil }
}

item_tooltip_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^ItemTooltip_t)(dst)
	s := (^ItemTooltip_t)(src)
	d^ = ItemTooltip_t{}
	d.headingTextColor = s.headingTextColor
	d.descriptionTextColor = s.descriptionTextColor
	d.detailsTextColor = s.detailsTextColor
	d.positiveTextColor = s.positiveTextColor
	d.negativeTextColor = s.negativeTextColor
	d.statusEffectTextColor = s.statusEffectTextColor
	d.faintTextColor = s.faintTextColor
	barony_dynamic_array_elem_copy(&d.icons, &s.icons, size_of(ItemTooltipIcons_t), Kind_Icon)
	barony_dynamic_array_elem_copy(&d.descriptionText, &s.descriptionText, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.detailsTextInsertOrder, &s.detailsTextInsertOrder, size_of(DynamicString), Kind_DynamicString)
	if s.detailsText != nil {
		d.detailsText = make(map[string]Raw_Dynamic_Array)
		for key in s.detailsText {
			_, vp, _, err := map_entry(&s.detailsText, key)
			if err == nil && vp != nil {
				new_val: Raw_Dynamic_Array
				barony_dynamic_array_elem_copy(&new_val, vp, size_of(DynamicString), Kind_DynamicString)
				d.detailsText[key] = new_val
			}
		}
	}
	if s.minWidths != nil {
		d.minWidths = make(map[string]i32)
		for key in s.minWidths {
			_, vp, _, err := map_entry(&s.minWidths, key)
			if err == nil && vp != nil { d.minWidths[key] = vp^ }
		}
	}
	if s.maxWidths != nil {
		d.maxWidths = make(map[string]i32)
		for key in s.maxWidths {
			_, vp, _, err := map_entry(&s.maxWidths, key)
			if err == nil && vp != nil { d.maxWidths[key] = vp^ }
		}
	}
	if s.headerMaxWidths != nil {
		d.headerMaxWidths = make(map[string]i32)
		for key in s.headerMaxWidths {
			_, vp, _, err := map_entry(&s.headerMaxWidths, key)
			if err == nil && vp != nil { d.headerMaxWidths[key] = vp^ }
		}
	}
}

// Entry_t — owning mirror of ScriptTextParser_t::Entry_t (the C++ struct has
// a nested AdditionalContentProperties_t with an SDL_Rect; the mirror holds
// the same POD bytes). ObjectType_t = i32.
SDL_Rect_t :: struct {
	x: i32,
	y: i32,
	w: i32,
	h: i32,
}

EntryAdditionalContent_t :: struct {
	pos:       SDL_Rect_t,
	path:      DynamicString,
	bgPath:    DynamicString,
	imgBorder: i32,
}

Entry_t :: struct {
	name:                    DynamicString,
	rawText:                 Raw_Dynamic_Array, // of DynamicString
	variables:               Raw_Dynamic_Array, // of EntryVariable_t
	formattedText:           DynamicString,
	objectType:              i32,
	hjustify:                i32,
	vjustify:                i32,
	padPerLine:              Raw_Dynamic_Array, // of i32
	padTopY:                 i32,
	font:                    DynamicString,
	fontColor:               u32,
	fontOutlineColor:        u32,
	fontHighlightColor:      u32,
	fontHighlight2Color:     u32,
	wordHighlights:          Raw_Dynamic_Array, // of i32
	wordHighlights2:         Raw_Dynamic_Array, // of i32
	imageInlineTextAdjustX:  i32,
	signVideoContent:        EntryAdditionalContent_t,
}

entry_free :: proc(p: rawptr) {
	v := (^Entry_t)(p)
	dynamic_string_free_elem(rawptr(&v.name))
	dynamic_string_free_elem(rawptr(&v.formattedText))
	dynamic_string_free_elem(rawptr(&v.font))
	barony_dynamic_array_elem_destroy(&v.rawText, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.variables, size_of(EntryVariable_t), Kind_EntryVar)
	barony_dynamic_array_elem_destroy(&v.padPerLine, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.wordHighlights, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.wordHighlights2, size_of(i32), Kind_POD)
	dynamic_string_free_elem(rawptr(&v.signVideoContent.path))
	dynamic_string_free_elem(rawptr(&v.signVideoContent.bgPath))
}

entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Entry_t)(dst)
	s := (^Entry_t)(src)
	d^ = Entry_t{}
	dynamic_string_copy_elem(rawptr(&d.name), rawptr(&s.name))
	dynamic_string_copy_elem(rawptr(&d.formattedText), rawptr(&s.formattedText))
	dynamic_string_copy_elem(rawptr(&d.font), rawptr(&s.font))
	d.objectType = s.objectType
	d.hjustify = s.hjustify
	d.vjustify = s.vjustify
	d.padTopY = s.padTopY
	d.fontColor = s.fontColor
	d.fontOutlineColor = s.fontOutlineColor
	d.fontHighlightColor = s.fontHighlightColor
	d.fontHighlight2Color = s.fontHighlight2Color
	d.imageInlineTextAdjustX = s.imageInlineTextAdjustX
	barony_dynamic_array_elem_copy(&d.rawText, &s.rawText, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.variables, &s.variables, size_of(EntryVariable_t), Kind_EntryVar)
	barony_dynamic_array_elem_copy(&d.padPerLine, &s.padPerLine, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.wordHighlights, &s.wordHighlights, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.wordHighlights2, &s.wordHighlights2, size_of(i32), Kind_POD)
	d.signVideoContent.pos = s.signVideoContent.pos
	d.signVideoContent.imgBorder = s.signVideoContent.imgBorder
	dynamic_string_copy_elem(rawptr(&d.signVideoContent.path), rawptr(&s.signVideoContent.path))
	dynamic_string_copy_elem(rawptr(&d.signVideoContent.bgPath), rawptr(&s.signVideoContent.bgPath))
}

// DropDown_t — owning mirror of Player::GUIDropdown_t::DropDown_t.
DropDown_t :: struct {
	title:         DynamicString,
	internalName:  DynamicString,
	alignRight:    bool,
	module:        i32,
	defaultOption: i32,
	options:       Raw_Dynamic_Array, // of DropdownOption_t
}

drop_down_free :: proc(p: rawptr) {
	v := (^DropDown_t)(p)
	dynamic_string_free_elem(rawptr(&v.title))
	dynamic_string_free_elem(rawptr(&v.internalName))
	barony_dynamic_array_elem_destroy(&v.options, size_of(DropdownOption_t), Kind_Option)
}

drop_down_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^DropDown_t)(dst)
	s := (^DropDown_t)(src)
	d^ = DropDown_t{}
	dynamic_string_copy_elem(rawptr(&d.title), rawptr(&s.title))
	dynamic_string_copy_elem(rawptr(&d.internalName), rawptr(&s.internalName))
	d.alignRight = s.alignRight
	d.module = s.module
	d.defaultOption = s.defaultOption
	barony_dynamic_array_elem_copy(&d.options, &s.options, size_of(DropdownOption_t), Kind_Option)
}

// Statue_t — owning mirror of StatueManager_t::Statue_t. limbs is
// map[string][dynamic]StatueLimb_t (StatueLimb_t is POD).
Statue_t :: struct {
	id:           u32,
	limbs:        map[string]Raw_Dynamic_Array, // str -> [dynamic]StatueLimb_t
	heightOffset: f64,
}

// nested map<int, map<int, ModelOffset_t>> value: deep free/copy the inner
// map of ModelOffset_t (owning, 2 nested i32 maps each).
i32_map_modeloffset_free :: proc(p: rawptr) {
	mm := transmute(^map[[4]byte]ModelOffset_t)(p)
	if mm^ != nil {
		for key in mm^ {
			_, vp, _, err := map_entry(mm, key)
			if err == nil && vp != nil {
				model_offset_free(vp)
			}
		}
		delete(mm^)
		mm^ = nil
	}
}

i32_map_modeloffset_copy :: proc(dst: rawptr, src: rawptr) {
	m := transmute(^map[[4]byte]ModelOffset_t)(dst)
	s := transmute(^map[[4]byte]ModelOffset_t)(src)
	if m^ != nil {
		i32_map_modeloffset_free(m)
	}
	m^ = nil
	if s^ == nil {
		return
	}
	m^ = make(map[[4]byte]ModelOffset_t)
	for key in s^ {
		_, vp, _, err := map_entry(s, key)
		if err == nil && vp != nil {
			new_val: ModelOffset_t
			model_offset_copy(&new_val, vp)
			m^[key] = new_val
		}
	}
}

// nested map<string, map<string, string>> value: the inner map owns its
// DynamicString values (interned keys). Deep-free/copy.
str_map_str_value_free :: proc(p: rawptr) {
	mm := transmute(^map[string]string)(p)
	if mm^ != nil {
		for key in mm^ {
			_, vp, _, err := map_entry(mm, key)
			if err == nil && vp != nil {
				string_value_free(vp)
			}
		}
		delete(mm^)
		mm^ = nil
	}
}

str_map_str_value_copy :: proc(dst: rawptr, src: rawptr) {
	m := transmute(^map[string]string)(dst)
	s := transmute(^map[string]string)(src)
	if m^ != nil {
		str_map_str_value_free(m)
	}
	m^ = nil
	if s^ == nil {
		return
	}
	m^ = make(map[string]string)
	for key in s^ {
		_, vp, _, err := map_entry(s, key)
		if err == nil && vp != nil {
			new_val: string
			string_value_copy(&new_val, vp)
			m^[key] = new_val
		}
	}
}

statue_free :: proc(p: rawptr) {
	v := (^Statue_t)(p)
	if v.limbs != nil {
		for key in v.limbs {
			_, vp, _, err := map_entry(&v.limbs, key)
			if err == nil && vp != nil {
				barony_dynamic_array_elem_destroy(vp, size_of(StatueLimb_t), Kind_POD)
			}
		}
		delete(v.limbs)
		v.limbs = nil
	}
}

statue_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^Statue_t)(dst)
	s := (^Statue_t)(src)
	d^ = Statue_t{}
	d.id = s.id
	d.heightOffset = s.heightOffset
	if s.limbs != nil {
		d.limbs = make(map[string]Raw_Dynamic_Array)
		for key in s.limbs {
			_, vp, _, err := map_entry(&s.limbs, key)
			if err == nil && vp != nil {
				new_val: Raw_Dynamic_Array
				barony_dynamic_array_elem_copy(&new_val, vp, size_of(StatueLimb_t), Kind_POD)
				d.limbs[key] = new_val
			}
		}
	}
}

spell_item_free :: proc(p: rawptr) {
	v := (^SpellItem_t)(p)
	strings := [?]^DynamicString{
		&v.internalName, &v.name, &v.name_lowercase, &v.spellTypeStr,
		&v.spellbookInternalName, &v.magicstaffInternalName, &v.fociInternalName,
	}
	for s in strings {
		dynamic_string_free_elem(rawptr(s))
	}
	barony_dynamic_array_elem_destroy(&v.spellTagsStr, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.spellFormatTags, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.spellbookItemIconPaddingLines, size_of(i32), Kind_POD)
	if v.spellTags != nil { delete(v.spellTags); v.spellTags = nil }
	if v.spellLevelTags != nil { delete(v.spellLevelTags); v.spellLevelTags = nil }
}

spell_item_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^SpellItem_t)(dst)
	s := (^SpellItem_t)(src)
	d^ = SpellItem_t{}
	d.id = s.id
	d.spellType = s.spellType
	d.spellbookId = s.spellbookId
	d.magicstaffId = s.magicstaffId
	d.fociId = s.fociId
	d.hasExpandedJSON = s.hasExpandedJSON
	d.damage = s.damage
	d.damage2 = s.damage2
	d.damage_mult = s.damage_mult
	d.damage2_mult = s.damage2_mult
	d.duration = s.duration
	d.duration_mult = s.duration_mult
	d.duration2 = s.duration2
	d.duration2_mult = s.duration2_mult
	d.mana = s.mana
	d.distance = s.distance
	d.distance_mult = s.distance_mult
	d.life_time = s.life_time
	d.life_mult = s.life_mult
	d.cast_time = s.cast_time
	d.cast_time_mult = s.cast_time_mult
	d.skillID = s.skillID
	d.difficulty = s.difficulty
	d.sustain_mana = s.sustain_mana
	d.sustain_duration = s.sustain_duration
	d.sustain_mult = s.sustain_mult
	d.radius = s.radius
	d.radius_mult = s.radius_mult
	d.drop_table = s.drop_table
	srcs := [?]^DynamicString{ &s.internalName, &s.name, &s.name_lowercase, &s.spellTypeStr, &s.spellbookInternalName, &s.magicstaffInternalName, &s.fociInternalName }
	dsts := [?]^DynamicString{ &d.internalName, &d.name, &d.name_lowercase, &d.spellTypeStr, &d.spellbookInternalName, &d.magicstaffInternalName, &d.fociInternalName }
	for i in 0..<len(srcs) {
		dynamic_string_copy_elem(rawptr(dsts[i]), rawptr(srcs[i]))
	}
	barony_dynamic_array_elem_copy(&d.spellTagsStr, &s.spellTagsStr, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.spellFormatTags, &s.spellFormatTags, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.spellbookItemIconPaddingLines, &s.spellbookItemIconPaddingLines, size_of(i32), Kind_POD)
	if s.spellTags != nil {
		d.spellTags = make(map[i32]struct{})
		for key in s.spellTags { d.spellTags[key] = {} }
	}
	if s.spellLevelTags != nil {
		d.spellLevelTags = make(map[i32]struct{})
		for key in s.spellLevelTags { d.spellLevelTags[key] = {} }
	}
}

entity_collider_data_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^EntityColliderData_t)(dst)
	s := (^EntityColliderData_t)(src)
	d^ = EntityColliderData_t{}
	d.gib = s.gib
	d.sfxHit = s.sfxHit
	d.entityLangEntry = s.entityLangEntry
	d.hitMessageLangEntry = s.hitMessageLangEntry
	d.breakMessageLangEntry = s.breakMessageLangEntry
	d.colliderJumpLangEntry = s.colliderJumpLangEntry
	barony_dynamic_array_elem_copy(&d.gib_hit, &s.gib_hit, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.sfxBreak, &s.sfxBreak, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.spellTriggers, &s.spellTriggers, size_of(i32), Kind_POD)
	dynamic_string_copy_elem(rawptr(&d.damageCalculationType), rawptr(&s.damageCalculationType))
	dynamic_string_copy_elem(rawptr(&d.name), rawptr(&s.name))
	dynamic_string_copy_elem(rawptr(&d.hpbarLookupName), rawptr(&s.hpbarLookupName))
	if s.hideMonsters != nil {
		d.hideMonsters = make(map[string]Raw_Dynamic_Array)
		for key in s.hideMonsters {
			_, vp, _, err := map_entry(&s.hideMonsters, key)
			if err == nil && vp != nil {
				new_val: Raw_Dynamic_Array
				barony_dynamic_array_elem_copy(&new_val, vp, size_of(i32), Kind_POD)
				d.hideMonsters[key] = new_val
			}
		}
	}
	if s.pathableMonsters != nil {
		d.pathableMonsters = make(map[i32]struct{})
		for key in s.pathableMonsters {
			d.pathableMonsters[key] = {}
		}
	}
	if s.overrideProperties != nil {
		d.overrideProperties = make(map[string]i32)
		for key in s.overrideProperties {
			_, vp, _, err := map_entry(&s.overrideProperties, key)
			if err == nil && vp != nil {
				d.overrideProperties[key] = vp^
			}
		}
	}
}

emitter_hit_map_value_free :: proc(p: rawptr) {
	mm := transmute(^map[[4]byte]ParticleEmitterHit_t)(p)
	if mm^ != nil {
		delete(mm^)
		mm^ = nil
	}
}

emitter_hit_map_value_copy :: proc(dst: rawptr, src: rawptr) {
	m := transmute(^map[[4]byte]ParticleEmitterHit_t)(dst)
	s := transmute(^map[[4]byte]ParticleEmitterHit_t)(src)
	if m^ != nil {
		delete(m^)
	}
	m^ = nil
	if s^ == nil {
		return
	}
	m^ = make(map[[4]byte]ParticleEmitterHit_t)
	for key in s^ {
		_, vp, _, err := map_entry(s, key)
		if err == nil && vp != nil {
			m^[key] = vp^
		}
	}
}

u32_map_value_free :: proc(p: rawptr) {
	mm := transmute(^map[[4]byte]u32)(p)
	if mm^ != nil {
		delete(mm^)
		mm^ = nil
	}
}

u32_map_value_copy :: proc(dst: rawptr, src: rawptr) {
	m := transmute(^map[[4]byte]u32)(dst)
	s := transmute(^map[[4]byte]u32)(src)
	if m^ != nil {
		delete(m^)
	}
	m^ = nil
	if s^ == nil {
		return
	}
	m^ = make(map[[4]byte]u32)
	for key in s^ {
		_, vp, _, err := map_entry(s, key)
		if err == nil && vp != nil {
			m^[key] = vp^
		}
	}
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

// EnemyHPDamageBarHandler::EnemyHPDetails — real_t fields (f64) + one DynamicString + owning C++ pointers.
BarAnimator_t :: struct {
	foregroundValue: f64,
	backgroundValue: f64,
	previousSetpoint: f64,
	setpoint: i32,
	animateTicks: u32,
	damageTaken: i32,
	widthMultiplier: f64,
	maxValue: f64,
	currentOpacity: f64,
	fadeOut: f64,
	fadeIn: f64,
	skullOpacities: [4]f64,
	damageFrameOpacity: f64,
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
	depletionAnimationPercent: f64,
	worldX: f64,
	worldY: f64,
	worldZ: f64,
	screenDistance: f64,
	worldTexture: rawptr,
	worldSurfaceSprite: rawptr,
	worldSurfaceSpriteStatusEffects: rawptr,
}

#assert(size_of(BarAnimator_t) == 120)
#assert(size_of(EnemyHPDetails_t) == 280)

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
	// C++-owned transient caches (TempTexture / SDL_Surface). They are only ever
	// read/written through operator[] (the live map slot), never through copied
	// values; nulling them here stops snapshot/entry copies from double-freeing
	// the slot's textures/surfaces via the C++ destructor.
	d.worldTexture = nil
	d.worldSurfaceSprite = nil
	d.worldSurfaceSpriteStatusEffects = nil
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

// ---------------------------------------------------------------------------
// Case-2 value kinds (29-41): POD mirrors + owning mirrors (nested maps).
// POD kinds use nil free/copy; owning kinds deep-free/deep-copy.
// ---------------------------------------------------------------------------

// FormationInfo_t — 20B POD (MSVC x64)
FormationInfo_t :: struct {
	x:             i32,
	y:             i32,
	pathingDelay:  i32,
	tryExtendPath: i32,
	init:          bool,
	expired:       bool,
}

// AnimatedTile — 32B POD
AnimatedTile :: struct {
	indices: [8]i32,
}

// AdditionalOffset_t — 48B POD (6 x f64)
AdditionalOffset_t :: struct {
	focalx: f64,
	focaly: f64,
	focalz: f64,
	scalex: f64,
	scaley: f64,
	scalez: f64,
}

// PlayerRaceHostility_t — 40B POD
PlayerRaceHostility_t :: struct {
	numAggressions:     i32,
	numKills:           i32,
	numAccessories:     i32,
	playerRace:         i32,
	sex:                i32,
	equipment:          u8,
	type_:              i32, // C++ field `type` (Odin keyword -> type_)
	wantedLevel:        i32,
	player:             i32,
	bRequiresNetUpdate: bool,
}

// spellElement_t — 168B POD. `list_t` (two ptrs) inlined; `node` is a rawptr.
spellElement_t :: struct {
	damage:                 i32,
	damage2:                i32,
	duration2:              i32,
	damage_mult:            f64,
	damage2_mult:           f64,
	channeledMana_mult:     f64,
	duration_mult:          f64,
	duration2_mult:         f64,
	channeledMana_duration: i32,
	duration:               i32,
	element_internal_name:  [64]u8,
	elementID:              i32,
	can_be_learned:         bool,
	channeledMana:          i32,
	fociSpell:              bool,
	elements: struct {
		first: rawptr,
		last:  rawptr,
	},
	node: rawptr,
}

// ParticleTimerEffect_t::Effect_t — 40B POD
Effect_t :: struct {
	x:          f64,
	y:          f64,
	effectType: i32,
	yaw:        f64,
	sfx:        i32,
	firstEffect: bool,
}

// ParticleTimerEffect_t::EffectLocations_t — 40B POD
EffectLocations_t :: struct {
	yawOffset: f64,
	xOffset:   f64,
	seconds:   f64,
	dist:      f64,
	sfx:       i32,
}

// CalloutParticle_t — 160B POD
CalloutParticle_t :: struct {
	x:                        f64,
	y:                        f64,
	z:                        f64,
	entityUid:                u32,
	ticks:                    u32,
	lifetime:                 u32,
	creationTick:             u32,
	cmd:                      i32,
	type_:                    i32, // C++ field `type`
	expired:                  bool,
	lockOnScreen:             [4]bool,
	playerColor:              i32,
	tagID:                    i32,
	tagSmallID:               i32,
	animateState:             i32,
	animateStateInit:         i32,
	scale:                    f64,
	animateX:                 f64,
	animateScaleForPlayerView: [4]f64,
	animateBounce:            f64,
	animateY:                 f64,
	noUpdate:                 bool,
	selfCallout:              bool,
	doMessage:                bool,
	messageSentTick:          u32,
	big:                      [4]bool,
}

// ModelOffset_t — 160B owning (2 nested i32-keyed maps of AdditionalOffset_t / kind 33)
ModelOffset_t :: struct {
	focalx: f64,
	focaly: f64,
	focalz: f64,
	scalex: f64,
	scaley: f64,
	scalez: f64,
	rotation: f64,
	pitch:    f64,
	x:        f64,
	y:        f64,
	z:        f64,
	limbsIndex:  i32,
	oversizedMask: bool,
	expandToFitMask: bool,
	adjustToOversizeMask:  map[[4]byte]AdditionalOffset_t,
	adjustToExpandedHelm:  map[[4]byte]AdditionalOffset_t,
}

// StatusEffectQueue_t::EffectDefinitionEntry_t — 248B owning
EffectDefinitionEntry_t :: struct {
	effect_id: i32,
	spell_id:  i32,
	internal_name: DynamicString,
	name:          DynamicString,
	desc:          DynamicString,
	imgPath:       DynamicString,
	nameVariations:           Raw_Dynamic_Array, // of DynamicString
	descVariations:           Raw_Dynamic_Array, // of DynamicString
	useSpellIDForImgVariations: Raw_Dynamic_Array, // of i32
	imgPathVariations:        Raw_Dynamic_Array, // of DynamicString
	useSpellIDForImg: i32,
	neverDisplay:    bool,
	sustainedSpellID: i32,
	tooltipWidth:    i32,
}

// ParticleTimerEffect_t — 32B owning (nested i32-keyed map of Effect_t / kind 36)
ParticleTimerEffect_t :: struct {
	effectMap: map[[4]byte]Effect_t,
}

// MonsterData_t::MonsterDataEntry_t::IconLookup_t — 32B owning (2 DynamicStrings)
IconLookup_t :: struct {
	key:      DynamicString,
	iconPath: DynamicString,
}

// MonsterData_t::MonsterDataEntry_t — owning (strings + i32 maps + str-keyed map + sets)
MonsterDataEntry_t :: struct {
	monsterType:           i32,
	defaultIconPath:       DynamicString,
	iconSpritesAndPaths:   map[[4]byte]IconLookup_t,
	keyToSpriteLookup:     map[string]Raw_Dynamic_Array,
	modelIndexes:          map[i32]struct{},
	playerModelIndexes:    map[i32]struct{},
	defaultShortDisplayName: DynamicString,
	specialNPCs:           map[string]SpecialNPCEntry_t,
}

// MonsterAllyFormation_t::MonsterAllies_t — owning (2 i32-keyed maps of FormationInfo_t)
MonsterAllies_t :: struct {
	meleeUnits:   map[[4]byte]FormationInfo_t,
	rangedUnits:  map[[4]byte]FormationInfo_t,
	updatedOnTick: u32,
}

// Dither_t — 8B POD. Value for the pointer-keyed dithering maps:
//   Entity::Dither { int value = 0; Uint32 lastUpdateTick = 0; }
//   Chunk::Dither  { int value = MAX(10); Uint32 lastUpdateTick = 0; }
// Both are {value, lastUpdateTick}; the difference is only the non-zero
// default (handled by the ptr entry path, not the struct).
Dither_t :: struct {
	value:         i32,
	lastUpdateTick: u32,
}

// Layout guards — each mirror must match its C++ struct sizeof exactly.
#assert(size_of(LightDef) == 28)
#assert(size_of(IconEntryTextMap_t) == 48)
#assert(size_of(IconEntryText_t) == 160)
#assert(size_of(WorldIconEntry_t) == 104)
#assert(size_of(DiscoveryAnim_t) == 24)
#assert(size_of(SpecialNPCEntry_t) == 104)
#assert(size_of(ColliderDmgProperties_t) == 72)
#assert(size_of(ItemLocalization_t) == 32)
#assert(size_of(Achievement_t) == 32)
#assert(size_of(AchievementData_t) == 96)
#assert(size_of(IconEntry_t) == 128)
#assert(size_of(IconEntryCallout_t) == 128)
#assert(size_of(binding_t) == 104)
#assert(size_of(Class_t) == 32)
#assert(size_of(StatueLimb_t) == 80)
#assert(size_of(MonsterTrapIgnoreEntities_t) == 40)
#assert(size_of(Item_Game) == 56)
#assert(size_of(Lootbag_t) == 56)
#assert(size_of(BarAnimator_t) == 120)
#assert(size_of(EnemyHPDetails_t) == 280)
#assert(size_of(GlyphData_t) == 144)
#assert(size_of(Statistic_t) == 24)
#assert(size_of(FormationInfo_t) == 20)
#assert(size_of(AnimatedTile) == 32)
#assert(size_of(AdditionalOffset_t) == 48)
#assert(size_of(PlayerRaceHostility_t) == 40)
#assert(size_of(spellElement_t) == 168)
#assert(size_of(Effect_t) == 40)
#assert(size_of(EffectLocations_t) == 40)
#assert(size_of(CalloutParticle_t) == 160)
#assert(size_of(ModelOffset_t) == 160)
#assert(size_of(EffectDefinitionEntry_t) == 248)
#assert(size_of(ParticleTimerEffect_t) == 32)
#assert(size_of(IconLookup_t) == 32)
#assert(size_of(MonsterDataEntry_t) == 200)
#assert(size_of(MonsterAllies_t) == 72)
#assert(size_of(Dither_t) == 8)
#assert(size_of(ParticleEmitterHit_t) == 8)
#assert(size_of(DynamicStringPair_t) == 32)
#assert(size_of(EntityColliderData_t) == 304)
#assert(size_of(SpellItem_t) == 496)
#assert(size_of(ItemTooltip_t) == 280)
#assert(size_of(Entry_t) == 352)
#assert(size_of(Statue_t) == 48)

model_offset_free :: proc(p: rawptr) {
	v := (^ModelOffset_t)(p)
	if v.adjustToOversizeMask != nil { delete(v.adjustToOversizeMask); v.adjustToOversizeMask = nil }
	if v.adjustToExpandedHelm != nil { delete(v.adjustToExpandedHelm); v.adjustToExpandedHelm = nil }
}

copy_i32_map_additionaloffset :: proc(dst: ^map[[4]byte]AdditionalOffset_t, src: ^map[[4]byte]AdditionalOffset_t) {
	if dst^ != nil { delete(dst^) }
	dst^ = nil
	if src^ == nil { return }
	dst^ = make(map[[4]byte]AdditionalOffset_t)
	for key in src^ {
		_, vp, _, err := map_entry(src, key)
		if err == nil && vp != nil {
			// AdditionalOffset_t is POD (bytes); byte-copy the value.
			dst^[key] = vp^
		}
	}
}

model_offset_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^ModelOffset_t)(dst)
	s := (^ModelOffset_t)(src)
	d.focalx = s.focalx; d.focaly = s.focaly; d.focalz = s.focalz
	d.scalex = s.scalex; d.scaley = s.scaley; d.scalez = s.scalez
	d.rotation = s.rotation; d.pitch = s.pitch
	d.x = s.x; d.y = s.y; d.z = s.z
	d.limbsIndex = s.limbsIndex
	d.oversizedMask = s.oversizedMask
	d.expandToFitMask = s.expandToFitMask
	copy_i32_map_additionaloffset(&d.adjustToOversizeMask, &s.adjustToOversizeMask)
	copy_i32_map_additionaloffset(&d.adjustToExpandedHelm, &s.adjustToExpandedHelm)
}

copy_i32_map_effect :: proc(dst: ^map[[4]byte]Effect_t, src: ^map[[4]byte]Effect_t) {
	if dst^ != nil { delete(dst^) }
	dst^ = nil
	if src^ == nil { return }
	dst^ = make(map[[4]byte]Effect_t)
	for key in src^ {
		_, vp, _, err := map_entry(src, key)
		if err == nil && vp != nil {
			dst^[key] = vp^ // Effect_t is POD; byte-copy.
		}
	}
}

particle_timer_effect_free :: proc(p: rawptr) {
	v := (^ParticleTimerEffect_t)(p)
	if v.effectMap != nil { delete(v.effectMap); v.effectMap = nil }
}

particle_timer_effect_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^ParticleTimerEffect_t)(dst)
	s := (^ParticleTimerEffect_t)(src)
	copy_i32_map_effect(&d.effectMap, &s.effectMap)
}

icon_lookup_free :: proc(p: rawptr) {
	v := (^IconLookup_t)(p)
	dynamic_string_free_elem(rawptr(&v.key))
	dynamic_string_free_elem(rawptr(&v.iconPath))
}

icon_lookup_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^IconLookup_t)(dst)
	s := (^IconLookup_t)(src)
	d^ = IconLookup_t{}
	dynamic_string_copy_elem(rawptr(&d.key), rawptr(&s.key))
	dynamic_string_copy_elem(rawptr(&d.iconPath), rawptr(&s.iconPath))
}

monster_data_entry_free :: proc(p: rawptr) {
	v := (^MonsterDataEntry_t)(p)
	dynamic_string_free_elem(rawptr(&v.defaultIconPath))
	dynamic_string_free_elem(rawptr(&v.defaultShortDisplayName))
	if v.iconSpritesAndPaths != nil {
		for key in v.iconSpritesAndPaths {
			_, vp, _, err := map_entry(&v.iconSpritesAndPaths, key)
			if err == nil && vp != nil {
				icon_lookup_free(vp)
			}
		}
		delete(v.iconSpritesAndPaths)
		v.iconSpritesAndPaths = nil
	}
	if v.keyToSpriteLookup != nil {
		for key in v.keyToSpriteLookup {
			_, vp, _, err := map_entry(&v.keyToSpriteLookup, key)
			if err == nil && vp != nil {
				barony_dynamic_array_elem_destroy(vp, size_of(i32), Kind_POD)
			}
		}
		delete(v.keyToSpriteLookup)
		v.keyToSpriteLookup = nil
	}
	if v.modelIndexes != nil { delete(v.modelIndexes); v.modelIndexes = nil }
	if v.playerModelIndexes != nil { delete(v.playerModelIndexes); v.playerModelIndexes = nil }
	if v.specialNPCs != nil {
		for key in v.specialNPCs {
			_, vp, _, err := map_entry(&v.specialNPCs, key)
			if err == nil && vp != nil {
				special_npc_free(vp)
			}
		}
		delete(v.specialNPCs)
		v.specialNPCs = nil
	}
}

monster_data_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^MonsterDataEntry_t)(dst)
	s := (^MonsterDataEntry_t)(src)
	d^ = MonsterDataEntry_t{}
	d.monsterType = s.monsterType
	dynamic_string_copy_elem(rawptr(&d.defaultIconPath), rawptr(&s.defaultIconPath))
	dynamic_string_copy_elem(rawptr(&d.defaultShortDisplayName), rawptr(&s.defaultShortDisplayName))
	if s.iconSpritesAndPaths != nil {
		d.iconSpritesAndPaths = make(map[[4]byte]IconLookup_t)
		for key in s.iconSpritesAndPaths {
			_, vp, _, err := map_entry(&s.iconSpritesAndPaths, key)
			if err == nil && vp != nil {
				new_val: IconLookup_t
				icon_lookup_copy(&new_val, vp)
				d.iconSpritesAndPaths[key] = new_val
			}
		}
	}
	if s.keyToSpriteLookup != nil {
		d.keyToSpriteLookup = make(map[string]Raw_Dynamic_Array)
		for key in s.keyToSpriteLookup {
			_, vp, _, err := map_entry(&s.keyToSpriteLookup, key)
			if err == nil && vp != nil {
				new_val: Raw_Dynamic_Array
				barony_dynamic_array_elem_copy(&new_val, vp, size_of(i32), Kind_POD)
				d.keyToSpriteLookup[key] = new_val
			}
		}
	}
	if s.modelIndexes != nil {
		d.modelIndexes = make(map[i32]struct{})
		for key in s.modelIndexes {
			d.modelIndexes[key] = {}
		}
	}
	if s.playerModelIndexes != nil {
		d.playerModelIndexes = make(map[i32]struct{})
		for key in s.playerModelIndexes {
			d.playerModelIndexes[key] = {}
		}
	}
	if s.specialNPCs != nil {
		d.specialNPCs = make(map[string]SpecialNPCEntry_t)
		for key in s.specialNPCs {
			_, vp, _, err := map_entry(&s.specialNPCs, key)
			if err == nil && vp != nil {
				new_val: SpecialNPCEntry_t
				special_npc_copy(&new_val, vp)
				d.specialNPCs[key] = new_val
			}
		}
	}
}

monster_allies_free :: proc(p: rawptr) {
	v := (^MonsterAllies_t)(p)
	if v.meleeUnits != nil { delete(v.meleeUnits); v.meleeUnits = nil }
	if v.rangedUnits != nil { delete(v.rangedUnits); v.rangedUnits = nil }
}

monster_allies_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^MonsterAllies_t)(dst)
	s := (^MonsterAllies_t)(src)
	d^ = MonsterAllies_t{}
	d.updatedOnTick = s.updatedOnTick
	if s.meleeUnits != nil {
		d.meleeUnits = make(map[[4]byte]FormationInfo_t)
		for key in s.meleeUnits {
			_, vp, _, err := map_entry(&s.meleeUnits, key)
			if err == nil && vp != nil {
				d.meleeUnits[key] = vp^
			}
		}
	}
	if s.rangedUnits != nil {
		d.rangedUnits = make(map[[4]byte]FormationInfo_t)
		for key in s.rangedUnits {
			_, vp, _, err := map_entry(&s.rangedUnits, key)
			if err == nil && vp != nil {
				d.rangedUnits[key] = vp^
			}
		}
	}
}

effect_definition_entry_free :: proc(p: rawptr) {
	v := (^EffectDefinitionEntry_t)(p)
	strings := [?]^DynamicString{ &v.internal_name, &v.name, &v.desc, &v.imgPath }
	for s in strings {
		if s.data != nil { mem.free(s.data); s.data = nil }
		s.len = 0
	}
	barony_dynamic_array_elem_destroy(&v.nameVariations, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.descVariations, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_destroy(&v.useSpellIDForImgVariations, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_destroy(&v.imgPathVariations, size_of(DynamicString), Kind_DynamicString)
}

effect_definition_entry_copy :: proc(dst: rawptr, src: rawptr) {
	d := (^EffectDefinitionEntry_t)(dst)
	s := (^EffectDefinitionEntry_t)(src)
	d.effect_id = s.effect_id
	d.spell_id = s.spell_id
	d.useSpellIDForImg = s.useSpellIDForImg
	d.neverDisplay = s.neverDisplay
	d.sustainedSpellID = s.sustainedSpellID
	d.tooltipWidth = s.tooltipWidth
	srcs := [?]^DynamicString{ &s.internal_name, &s.name, &s.desc, &s.imgPath }
	dsts := [?]^DynamicString{ &d.internal_name, &d.name, &d.desc, &d.imgPath }
	for i in 0..<len(srcs) {
		dynamic_string_copy_elem(rawptr(dsts[i]), rawptr(srcs[i]))
	}
	barony_dynamic_array_elem_copy(&d.nameVariations, &s.nameVariations, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.descVariations, &s.descVariations, size_of(DynamicString), Kind_DynamicString)
	barony_dynamic_array_elem_copy(&d.useSpellIDForImgVariations, &s.useSpellIDForImgVariations, size_of(i32), Kind_POD)
	barony_dynamic_array_elem_copy(&d.imgPathVariations, &s.imgPathVariations, size_of(DynamicString), Kind_DynamicString)
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

// Value kinds — MUST match `enum MapValueKind` in dynamic_map.hpp.
// i32-backed on purpose: Odin's default `int` is 8 bytes on x64, but these
// cross the C ABI as int32_t. Keeping them named here makes the dispatchers
// self-documenting so `case 26`-style magic numbers (and the u32/u64 drift)
// can't happen again.
Value_Kind :: enum i32 {
	MK_I32                   = 0,
	MK_F32                   = 1,
	MK_U32                   = 2,
	MK_String                = 3,
	MK_LightDef              = 4,
	MK_IconEntryTextMap      = 5,
	MK_IconEntryText         = 6,
	MK_WorldIconEntry        = 7,
	MK_DiscoveryAnim         = 8,
	MK_SpecialNPC            = 9,
	MK_ColliderDmg           = 10,
	MK_ItemLoc               = 11,
	MK_Achievement           = 12,
	MK_AchievementData       = 13,
	MK_IconEntry             = 14,
	MK_IconEntryCallout      = 15,
	MK_Binding               = 16,
	MK_Class                 = 17,
	MK_DynArrayStr           = 18,
	MK_DynArrayS32           = 19,
	MK_StatueLimbArray       = 20,
	MK_StoreSlotsArray       = 21,
	MK_MonsterTrapIgnore     = 22,
	MK_SetOfI32              = 23,
	MK_Lootbag               = 24,
	MK_EnemyHPDetails        = 25,
	MK_U64                   = 26,
	MK_GlyphData             = 27,
	MK_Statistic             = 28,
	MK_Ptr                   = 29,
	MK_F64                   = 30,
	MK_FormationInfo         = 31,
	MK_AnimatedTile          = 32,
	MK_AdditionalOffset      = 33,
	MK_PlayerRaceHostility   = 34,
	MK_SpellElement          = 35,
	MK_Effect                = 36,
	MK_EffectLocations       = 37,
	MK_CalloutParticle       = 38,
	MK_ModelOffset           = 39,
	MK_EffectDefinitionEntry = 40,
	MK_ParticleTimerEffect   = 41,
	MK_IconLookup            = 42,
	MK_MonsterDataEntry      = 43,
	MK_MonsterAllies         = 44,
	MK_Dither                = 45,
	MK_ChunkDither           = 46,
	MK_I32Map                = 47,
	MK_Bool                  = 48,
	MK_U32Map                = 49,
	MK_U32MapEmitterHit      = 50,
	MK_StringPair            = 51,
	MK_EntityColliderData    = 52,
	MK_SpellItem             = 53,
	MK_ItemTooltip           = 54,
	MK_Entry                 = 55,
	MK_DropDown              = 56,
	MK_Statue                = 57,
	MK_I32MapModelOffset     = 58,
	MK_StrMapStr             = 59,
}

value_ops_for :: proc(kind: i32) -> Value_Ops {
	switch Value_Kind(kind) {
	case .MK_I32, .MK_F32, .MK_U32, .MK_LightDef, .MK_Class, .MK_U64, .MK_Ptr, .MK_F64, .MK_FormationInfo, .MK_AnimatedTile, .MK_AdditionalOffset, .MK_PlayerRaceHostility, .MK_SpellElement, .MK_Effect, .MK_EffectLocations, .MK_CalloutParticle, .MK_Bool:
		return Value_Ops{}
	case .MK_ModelOffset:
		return Value_Ops{ free = model_offset_free, copy = model_offset_copy }
	case .MK_EffectDefinitionEntry:
		return Value_Ops{ free = effect_definition_entry_free, copy = effect_definition_entry_copy }
	case .MK_ParticleTimerEffect:
		return Value_Ops{ free = particle_timer_effect_free, copy = particle_timer_effect_copy }
	case .MK_String:
		return Value_Ops{ free = string_value_free, copy = string_value_copy }
	case .MK_IconEntryTextMap:
		return Value_Ops{ free = icon_entry_text_map_free_raw, copy = icon_entry_text_map_copy_raw }
	case .MK_IconEntryText:
		return Value_Ops{ free = icon_entry_text_free_raw, copy = icon_entry_text_copy_raw }
	case .MK_WorldIconEntry:
		return Value_Ops{ free = world_icon_entry_free_raw, copy = world_icon_entry_copy_raw }
	case .MK_DiscoveryAnim:
		return Value_Ops{ free = discovery_anim_free_raw, copy = discovery_anim_copy_raw }
	case .MK_SpecialNPC:
		return Value_Ops{ free = special_npc_free_raw, copy = special_npc_copy_raw }
	case .MK_ColliderDmg:
		return Value_Ops{ free = collider_dmg_free_raw, copy = collider_dmg_copy_raw }
	case .MK_ItemLoc:
		return Value_Ops{ free = item_localization_free_raw, copy = item_localization_copy_raw }
	case .MK_Achievement:
		return Value_Ops{ free = achievement_t_free_raw, copy = achievement_t_copy_raw }
	case .MK_AchievementData:
		return Value_Ops{ free = achievement_data_free_raw, copy = achievement_data_copy_raw }
	case .MK_IconEntry:
		return Value_Ops{ free = icon_entry_free_raw, copy = icon_entry_copy_raw }
	case .MK_IconEntryCallout:
		return Value_Ops{ free = icon_entry_callout_free_raw, copy = icon_entry_callout_copy_raw }
	case .MK_Binding:
		return Value_Ops{ free = binding_t_free_raw, copy = binding_t_copy_raw }
	case .MK_DynArrayStr:
		return Value_Ops{ free = dynarrstr_value_free, copy = dynarrstr_value_copy }
	case .MK_DynArrayS32:
		return Value_Ops{ free = dynarrs32_value_free, copy = dynarrs32_value_copy }
	case .MK_StatueLimbArray:
		return Value_Ops{ free = dynarr_statuelimb_value_free, copy = dynarr_statuelimb_value_copy }
	case .MK_StoreSlotsArray:
		return Value_Ops{ free = dynarr_storeslots_value_free, copy = dynarr_storeslots_value_copy }
	case .MK_MonsterTrapIgnore:
		return Value_Ops{ free = monster_trap_ignore_free, copy = monster_trap_ignore_copy }
	case .MK_SetOfI32:
		return Value_Ops{ free = set_i32_value_free, copy = set_i32_value_copy }
	case .MK_I32Map:
		return Value_Ops{ free = i32_map_value_free, copy = i32_map_value_copy }
	case .MK_U32Map:
		return Value_Ops{ free = u32_map_value_free, copy = u32_map_value_copy }
	case .MK_U32MapEmitterHit:
		return Value_Ops{ free = emitter_hit_map_value_free, copy = emitter_hit_map_value_copy }
	case .MK_StringPair:
		return Value_Ops{ free = dynamic_string_pair_free, copy = dynamic_string_pair_copy }
	case .MK_EntityColliderData:
		return Value_Ops{ free = entity_collider_data_free, copy = entity_collider_data_copy }
	case .MK_SpellItem:
		return Value_Ops{ free = spell_item_free, copy = spell_item_copy }
	case .MK_ItemTooltip:
		return Value_Ops{ free = item_tooltip_free, copy = item_tooltip_copy }
	case .MK_Entry:
		return Value_Ops{ free = entry_free, copy = entry_copy }
	case .MK_DropDown:
		return Value_Ops{ free = drop_down_free, copy = drop_down_copy }
	case .MK_Statue:
		return Value_Ops{ free = statue_free, copy = statue_copy }
	case .MK_I32MapModelOffset:
		return Value_Ops{ free = i32_map_modeloffset_free, copy = i32_map_modeloffset_copy }
	case .MK_StrMapStr:
		return Value_Ops{ free = str_map_str_value_free, copy = str_map_str_value_copy }
	case .MK_Lootbag:
		return Value_Ops{ free = lootbag_free, copy = lootbag_copy }
	case .MK_EnemyHPDetails:
		return Value_Ops{ free = enemy_hp_details_free, copy = enemy_hp_details_copy }
	case .MK_GlyphData:
		return Value_Ops{ free = glyph_data_free, copy = glyph_data_copy }
	case .MK_Statistic:
		return Value_Ops{ free = statistic_free, copy = statistic_copy }
	case .MK_IconLookup:
		return Value_Ops{ free = icon_lookup_free, copy = icon_lookup_copy }
	case .MK_MonsterDataEntry:
		return Value_Ops{ free = monster_data_entry_free, copy = monster_data_entry_copy }
	case .MK_MonsterAllies:
		return Value_Ops{ free = monster_allies_free, copy = monster_allies_copy }
	case .MK_Dither, .MK_ChunkDither:
		return Value_Ops{}
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
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  str_map_put(m, key, value, i32, ops)
	case .MK_F32:  str_map_put(m, key, value, f32, ops)
	case .MK_U32:  str_map_put(m, key, value, u32, ops)
	case .MK_String:  str_map_put(m, key, value, string, ops)
	case .MK_LightDef:  str_map_put(m, key, value, LightDef, ops)
	case .MK_IconEntryTextMap:  str_map_put(m, key, value, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  str_map_put(m, key, value, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  str_map_put(m, key, value, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  str_map_put(m, key, value, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  str_map_put(m, key, value, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: str_map_put(m, key, value, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: str_map_put(m, key, value, ItemLocalization_t, ops)
	case .MK_Achievement: str_map_put(m, key, value, Achievement_t, ops)
	case .MK_AchievementData: str_map_put(m, key, value, AchievementData_t, ops)
	case .MK_IconEntry: str_map_put(m, key, value, IconEntry_t, ops)
	case .MK_IconEntryCallout: str_map_put(m, key, value, IconEntryCallout_t, ops)
	case .MK_Binding: str_map_put(m, key, value, binding_t, ops)
	case .MK_Class: str_map_put(m, key, value, Class_t, ops)
	case .MK_DynArrayStr: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: str_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_StringPair: str_map_put(m, key, value, DynamicStringPair_t, ops)
	case .MK_I32Map: str_map_put(m, key, value, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: str_map_put(m, key, value, ItemTooltip_t, ops)
	case .MK_Entry: str_map_put(m, key, value, Entry_t, ops)
	case .MK_DropDown: str_map_put(m, key, value, DropDown_t, ops)
	case .MK_StrMapStr: str_map_put(m, key, value, map[string]string, ops)
	}
}

@(export)
barony_dynamic_map_str_get :: proc "c" (m: rawptr, key: string, out: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_get(m, key, out, i32, ops)
	case .MK_F32:  return str_map_get(m, key, out, f32, ops)
	case .MK_U32:  return str_map_get(m, key, out, u32, ops)
	case .MK_String:  return str_map_get(m, key, out, string, ops)
	case .MK_LightDef:  return str_map_get(m, key, out, LightDef, ops)
	case .MK_IconEntryTextMap:  return str_map_get(m, key, out, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  return str_map_get(m, key, out, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  return str_map_get(m, key, out, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  return str_map_get(m, key, out, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  return str_map_get(m, key, out, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: return str_map_get(m, key, out, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: return str_map_get(m, key, out, ItemLocalization_t, ops)
	case .MK_Achievement: return str_map_get(m, key, out, Achievement_t, ops)
	case .MK_AchievementData: return str_map_get(m, key, out, AchievementData_t, ops)
	case .MK_IconEntry: return str_map_get(m, key, out, IconEntry_t, ops)
	case .MK_IconEntryCallout: return str_map_get(m, key, out, IconEntryCallout_t, ops)
	case .MK_Binding: return str_map_get(m, key, out, binding_t, ops)
	case .MK_Class: return str_map_get(m, key, out, Class_t, ops)
	case .MK_DynArrayStr: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return str_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_StringPair: return str_map_get(m, key, out, DynamicStringPair_t, ops)
	case .MK_I32Map: return str_map_get(m, key, out, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: return str_map_get(m, key, out, ItemTooltip_t, ops)
	case .MK_Entry: return str_map_get(m, key, out, Entry_t, ops)
	case .MK_DropDown: return str_map_get(m, key, out, DropDown_t, ops)
	case .MK_StrMapStr: return str_map_get(m, key, out, map[string]string, ops)
	}
	return false
}

@(export)
barony_dynamic_map_str_len :: proc "c" (m: rawptr, value_kind: i32) -> i32 {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_len(m, i32)
	case .MK_F32:  return str_map_len(m, f32)
	case .MK_U32:  return str_map_len(m, u32)
	case .MK_String:  return str_map_len(m, string)
	case .MK_LightDef:  return str_map_len(m, LightDef)
	case .MK_IconEntryTextMap:  return str_map_len(m, IconEntryTextMap_t)
	case .MK_IconEntryText:  return str_map_len(m, IconEntryText_t)
	case .MK_WorldIconEntry:  return str_map_len(m, WorldIconEntry_t)
	case .MK_DiscoveryAnim:  return str_map_len(m, DiscoveryAnim_t)
	case .MK_SpecialNPC:  return str_map_len(m, SpecialNPCEntry_t)
	case .MK_ColliderDmg: return str_map_len(m, ColliderDmgProperties_t)
	case .MK_ItemLoc: return str_map_len(m, ItemLocalization_t)
	case .MK_Achievement: return str_map_len(m, Achievement_t)
	case .MK_AchievementData: return str_map_len(m, AchievementData_t)
	case .MK_IconEntry: return str_map_len(m, IconEntry_t)
	case .MK_IconEntryCallout: return str_map_len(m, IconEntryCallout_t)
	case .MK_Binding: return str_map_len(m, binding_t)
	case .MK_Class: return str_map_len(m, Class_t)
	case .MK_DynArrayStr: return str_map_len(m, Raw_Dynamic_Array)
	case .MK_DynArrayS32: return str_map_len(m, Raw_Dynamic_Array)
	case .MK_StatueLimbArray: return str_map_len(m, Raw_Dynamic_Array)
	case .MK_StoreSlotsArray: return str_map_len(m, Raw_Dynamic_Array)
	case .MK_StringPair: return str_map_len(m, DynamicStringPair_t)
	case .MK_I32Map: return str_map_len(m, map[[4]byte]i32)
	case .MK_ItemTooltip: return str_map_len(m, ItemTooltip_t)
	case .MK_Entry: return str_map_len(m, Entry_t)
	case .MK_DropDown: return str_map_len(m, DropDown_t)
	case .MK_StrMapStr: return str_map_len(m, map[string]string)
	}
	return 0
}

@(export)
barony_dynamic_map_str_clear :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  str_map_clear(m, i32, ops)
	case .MK_F32:  str_map_clear(m, f32, ops)
	case .MK_U32:  str_map_clear(m, u32, ops)
	case .MK_String:  str_map_clear(m, string, ops)
	case .MK_LightDef:  str_map_clear(m, LightDef, ops)
	case .MK_IconEntryTextMap:  str_map_clear(m, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  str_map_clear(m, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  str_map_clear(m, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  str_map_clear(m, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  str_map_clear(m, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: str_map_clear(m, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: str_map_clear(m, ItemLocalization_t, ops)
	case .MK_Achievement: str_map_clear(m, Achievement_t, ops)
	case .MK_AchievementData: str_map_clear(m, AchievementData_t, ops)
	case .MK_IconEntry: str_map_clear(m, IconEntry_t, ops)
	case .MK_IconEntryCallout: str_map_clear(m, IconEntryCallout_t, ops)
	case .MK_Binding: str_map_clear(m, binding_t, ops)
	case .MK_Class: str_map_clear(m, Class_t, ops)
	case .MK_DynArrayStr: str_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: str_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: str_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: str_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_StringPair: str_map_clear(m, DynamicStringPair_t, ops)
	case .MK_I32Map: str_map_clear(m, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: str_map_clear(m, ItemTooltip_t, ops)
	case .MK_Entry: str_map_clear(m, Entry_t, ops)
	case .MK_DropDown: str_map_clear(m, DropDown_t, ops)
	case .MK_StrMapStr: str_map_clear(m, map[string]string, ops)
	}
}

@(export)
barony_dynamic_map_str_destroy :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  str_map_destroy(m, i32, ops)
	case .MK_F32:  str_map_destroy(m, f32, ops)
	case .MK_U32:  str_map_destroy(m, u32, ops)
	case .MK_String:  str_map_destroy(m, string, ops)
	case .MK_LightDef:  str_map_destroy(m, LightDef, ops)
	case .MK_IconEntryTextMap:  str_map_destroy(m, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  str_map_destroy(m, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  str_map_destroy(m, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  str_map_destroy(m, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  str_map_destroy(m, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: str_map_destroy(m, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: str_map_destroy(m, ItemLocalization_t, ops)
	case .MK_Achievement: str_map_destroy(m, Achievement_t, ops)
	case .MK_AchievementData: str_map_destroy(m, AchievementData_t, ops)
	case .MK_IconEntry: str_map_destroy(m, IconEntry_t, ops)
	case .MK_IconEntryCallout: str_map_destroy(m, IconEntryCallout_t, ops)
	case .MK_Binding: str_map_destroy(m, binding_t, ops)
	case .MK_Class: str_map_destroy(m, Class_t, ops)
	case .MK_DynArrayStr: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: str_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_StringPair: str_map_destroy(m, DynamicStringPair_t, ops)
	case .MK_I32Map: str_map_destroy(m, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: str_map_destroy(m, ItemTooltip_t, ops)
	case .MK_Entry: str_map_destroy(m, Entry_t, ops)
	case .MK_DropDown: str_map_destroy(m, DropDown_t, ops)
	case .MK_StrMapStr: str_map_destroy(m, map[string]string, ops)
	}
}

@(export)
barony_dynamic_map_str_entry :: proc "c" (m: rawptr, key: string, value_kind: i32) -> rawptr {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_entry(m, key, i32)
	case .MK_F32:  return str_map_entry(m, key, f32)
	case .MK_U32:  return str_map_entry(m, key, u32)
	case .MK_String:  return str_map_entry(m, key, string)
	case .MK_LightDef:  return str_map_entry(m, key, LightDef)
	case .MK_IconEntryTextMap:  return str_map_entry(m, key, IconEntryTextMap_t)
	case .MK_IconEntryText:  return str_map_entry(m, key, IconEntryText_t)
	case .MK_WorldIconEntry:  return str_map_entry(m, key, WorldIconEntry_t)
	case .MK_DiscoveryAnim:  return str_map_entry(m, key, DiscoveryAnim_t)
	case .MK_SpecialNPC:  return str_map_entry(m, key, SpecialNPCEntry_t)
	case .MK_ColliderDmg: return str_map_entry(m, key, ColliderDmgProperties_t)
	case .MK_ItemLoc: return str_map_entry(m, key, ItemLocalization_t)
	case .MK_Achievement: return str_map_entry(m, key, Achievement_t)
	case .MK_AchievementData: return str_map_entry(m, key, AchievementData_t)
	case .MK_IconEntry: return str_map_entry(m, key, IconEntry_t)
	case .MK_IconEntryCallout: return str_map_entry(m, key, IconEntryCallout_t)
	case .MK_Binding: return str_map_entry(m, key, binding_t)
	case .MK_Class: return str_map_entry(m, key, Class_t)
	case .MK_DynArrayStr: return str_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_DynArrayS32: return str_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_StatueLimbArray: return str_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_StoreSlotsArray: return str_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_StringPair: return str_map_entry(m, key, DynamicStringPair_t)
	case .MK_I32Map: return str_map_entry(m, key, map[[4]byte]i32)
	case .MK_ItemTooltip: return str_map_entry(m, key, ItemTooltip_t)
	case .MK_Entry: return str_map_entry(m, key, Entry_t)
	case .MK_DropDown: return str_map_entry(m, key, DropDown_t)
	case .MK_StrMapStr: return str_map_entry(m, key, map[string]string)
	}
	return nil
}

@(export)
barony_dynamic_map_str_entries :: proc "c" (m: rawptr, key_ptrs: [^]rawptr, key_lens: [^]i32, val_ptrs: rawptr, count: i32, value_kind: i32) -> i32 {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, i32, ops)
	case .MK_F32:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, f32, ops)
	case .MK_U32:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, u32, ops)
	case .MK_String:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, string, ops)
	case .MK_LightDef:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, LightDef, ops)
	case .MK_IconEntryTextMap:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, ItemLocalization_t, ops)
	case .MK_Achievement: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Achievement_t, ops)
	case .MK_AchievementData: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, AchievementData_t, ops)
	case .MK_IconEntry: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntry_t, ops)
	case .MK_IconEntryCallout: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, IconEntryCallout_t, ops)
	case .MK_Binding: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, binding_t, ops)
	case .MK_Class: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Class_t, ops)
	case .MK_DynArrayStr: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_StringPair: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, DynamicStringPair_t, ops)
	case .MK_I32Map: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, ItemTooltip_t, ops)
	case .MK_Entry: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, Entry_t, ops)
	case .MK_DropDown: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, DropDown_t, ops)
	case .MK_StrMapStr: return str_map_entries(m, key_ptrs, key_lens, val_ptrs, count, map[string]string, ops)
	}
	return 0
}

@(export)
barony_dynamic_map_str_for_each :: proc "c" (m: rawptr, value_kind: i32, cb: rawptr, userdata: rawptr) {
	context = runtime.default_context()
	f := transmute(MapForeachCb)(cb)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  str_map_for_each(m, i32, f, userdata)
	case .MK_F32:  str_map_for_each(m, f32, f, userdata)
	case .MK_U32:  str_map_for_each(m, u32, f, userdata)
	case .MK_String:  str_map_for_each(m, string, f, userdata)
	case .MK_LightDef:  str_map_for_each(m, LightDef, f, userdata)
	case .MK_IconEntryTextMap:  str_map_for_each(m, IconEntryTextMap_t, f, userdata)
	case .MK_IconEntryText:  str_map_for_each(m, IconEntryText_t, f, userdata)
	case .MK_WorldIconEntry:  str_map_for_each(m, WorldIconEntry_t, f, userdata)
	case .MK_DiscoveryAnim:  str_map_for_each(m, DiscoveryAnim_t, f, userdata)
	case .MK_SpecialNPC:  str_map_for_each(m, SpecialNPCEntry_t, f, userdata)
	case .MK_ColliderDmg: str_map_for_each(m, ColliderDmgProperties_t, f, userdata)
	case .MK_ItemLoc: str_map_for_each(m, ItemLocalization_t, f, userdata)
	case .MK_Achievement: str_map_for_each(m, Achievement_t, f, userdata)
	case .MK_AchievementData: str_map_for_each(m, AchievementData_t, f, userdata)
	case .MK_IconEntry: str_map_for_each(m, IconEntry_t, f, userdata)
	case .MK_IconEntryCallout: str_map_for_each(m, IconEntryCallout_t, f, userdata)
	case .MK_Binding: str_map_for_each(m, binding_t, f, userdata)
	case .MK_Class: str_map_for_each(m, Class_t, f, userdata)
	case .MK_DynArrayStr: str_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_DynArrayS32: str_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_StatueLimbArray: str_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_StoreSlotsArray: str_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_StringPair: str_map_for_each(m, DynamicStringPair_t, f, userdata)
	case .MK_I32Map: str_map_for_each(m, map[[4]byte]i32, f, userdata)
	case .MK_ItemTooltip: str_map_for_each(m, ItemTooltip_t, f, userdata)
	case .MK_Entry: str_map_for_each(m, Entry_t, f, userdata)
	case .MK_DropDown: str_map_for_each(m, DropDown_t, f, userdata)
	case .MK_StrMapStr: str_map_for_each(m, map[string]string, f, userdata)
	}
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
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_erase(m, key, i32, ops)
	case .MK_F32:  return str_map_erase(m, key, f32, ops)
	case .MK_U32:  return str_map_erase(m, key, u32, ops)
	case .MK_String:  return str_map_erase(m, key, string, ops)
	case .MK_LightDef:  return str_map_erase(m, key, LightDef, ops)
	case .MK_IconEntryTextMap:  return str_map_erase(m, key, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  return str_map_erase(m, key, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  return str_map_erase(m, key, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  return str_map_erase(m, key, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  return str_map_erase(m, key, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: return str_map_erase(m, key, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: return str_map_erase(m, key, ItemLocalization_t, ops)
	case .MK_Achievement: return str_map_erase(m, key, Achievement_t, ops)
	case .MK_AchievementData: return str_map_erase(m, key, AchievementData_t, ops)
	case .MK_IconEntry: return str_map_erase(m, key, IconEntry_t, ops)
	case .MK_IconEntryCallout: return str_map_erase(m, key, IconEntryCallout_t, ops)
	case .MK_Binding: return str_map_erase(m, key, binding_t, ops)
	case .MK_Class: return str_map_erase(m, key, Class_t, ops)
	case .MK_DynArrayStr: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return str_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_StringPair: return str_map_erase(m, key, DynamicStringPair_t, ops)
	case .MK_I32Map: return str_map_erase(m, key, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: return str_map_erase(m, key, ItemTooltip_t, ops)
	case .MK_Entry: return str_map_erase(m, key, Entry_t, ops)
	case .MK_DropDown: return str_map_erase(m, key, DropDown_t, ops)
	case .MK_StrMapStr: return str_map_erase(m, key, map[string]string, ops)
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
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  i32_map_put(m, key, value, i32, ops)
	case .MK_U32:  i32_map_put(m, key, value, u32, ops)
	case .MK_Bool: i32_map_put(m, key, value, bool, ops)
	case .MK_U64: i32_map_put(m, key, value, u64, ops)
	case .MK_String:  i32_map_put(m, key, value, string, ops)
	case .MK_DynArrayStr: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: i32_map_put(m, key, value, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: i32_map_put(m, key, value, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: i32_map_put(m, key, value, map[i32]struct{}, ops)
	case .MK_I32Map: i32_map_put(m, key, value, map[[4]byte]i32, ops)
	case .MK_U32Map: i32_map_put(m, key, value, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: i32_map_put(m, key, value, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: i32_map_put(m, key, value, EntityColliderData_t, ops)
	case .MK_SpellItem: i32_map_put(m, key, value, SpellItem_t, ops)
	case .MK_Statue: i32_map_put(m, key, value, Statue_t, ops)
	case .MK_I32MapModelOffset: i32_map_put(m, key, value, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: i32_map_put(m, key, value, Lootbag_t, ops)
	case .MK_EnemyHPDetails: i32_map_put(m, key, value, EnemyHPDetails_t, ops)
	case .MK_GlyphData: i32_map_put(m, key, value, GlyphData_t, ops)
	case .MK_Statistic: i32_map_put(m, key, value, Statistic_t, ops)
	case .MK_Ptr: i32_map_put(m, key, value, u64, ops)
	case .MK_F64: i32_map_put(m, key, value, f64, ops)
	case .MK_FormationInfo: i32_map_put(m, key, value, FormationInfo_t, ops)
	case .MK_AnimatedTile: i32_map_put(m, key, value, AnimatedTile, ops)
	case .MK_AdditionalOffset: i32_map_put(m, key, value, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: i32_map_put(m, key, value, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: i32_map_put(m, key, value, spellElement_t, ops)
	case .MK_Effect: i32_map_put(m, key, value, Effect_t, ops)
	case .MK_EffectLocations: i32_map_put(m, key, value, EffectLocations_t, ops)
	case .MK_CalloutParticle: i32_map_put(m, key, value, CalloutParticle_t, ops)
	case .MK_ModelOffset: i32_map_put(m, key, value, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: i32_map_put(m, key, value, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: i32_map_put(m, key, value, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: i32_map_put(m, key, value, IconLookup_t, ops)
	case .MK_MonsterDataEntry: i32_map_put(m, key, value, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: i32_map_put(m, key, value, MonsterAllies_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_get :: proc "c" (m: rawptr, key: rawptr, out: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_get(m, key, out, i32, ops)
	case .MK_U32:  return i32_map_get(m, key, out, u32, ops)
	case .MK_Bool: return i32_map_get(m, key, out, bool, ops)
	case .MK_U64: return i32_map_get(m, key, out, u64, ops)
	case .MK_String:  return i32_map_get(m, key, out, string, ops)
	case .MK_DynArrayStr: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return i32_map_get(m, key, out, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: return i32_map_get(m, key, out, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: return i32_map_get(m, key, out, map[i32]struct{}, ops)
	case .MK_I32Map: return i32_map_get(m, key, out, map[[4]byte]i32, ops)
	case .MK_U32Map: return i32_map_get(m, key, out, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: return i32_map_get(m, key, out, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: return i32_map_get(m, key, out, EntityColliderData_t, ops)
	case .MK_SpellItem: return i32_map_get(m, key, out, SpellItem_t, ops)
	case .MK_Statue: return i32_map_get(m, key, out, Statue_t, ops)
	case .MK_I32MapModelOffset: return i32_map_get(m, key, out, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: return i32_map_get(m, key, out, Lootbag_t, ops)
	case .MK_EnemyHPDetails: return i32_map_get(m, key, out, EnemyHPDetails_t, ops)
	case .MK_GlyphData: return i32_map_get(m, key, out, GlyphData_t, ops)
	case .MK_Statistic: return i32_map_get(m, key, out, Statistic_t, ops)
	case .MK_Ptr: return i32_map_get(m, key, out, u64, ops)
	case .MK_F64: return i32_map_get(m, key, out, f64, ops)
	case .MK_FormationInfo: return i32_map_get(m, key, out, FormationInfo_t, ops)
	case .MK_AnimatedTile: return i32_map_get(m, key, out, AnimatedTile, ops)
	case .MK_AdditionalOffset: return i32_map_get(m, key, out, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: return i32_map_get(m, key, out, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: return i32_map_get(m, key, out, spellElement_t, ops)
	case .MK_Effect: return i32_map_get(m, key, out, Effect_t, ops)
	case .MK_EffectLocations: return i32_map_get(m, key, out, EffectLocations_t, ops)
	case .MK_CalloutParticle: return i32_map_get(m, key, out, CalloutParticle_t, ops)
	case .MK_ModelOffset: return i32_map_get(m, key, out, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: return i32_map_get(m, key, out, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: return i32_map_get(m, key, out, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: return i32_map_get(m, key, out, IconLookup_t, ops)
	case .MK_MonsterDataEntry: return i32_map_get(m, key, out, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: return i32_map_get(m, key, out, MonsterAllies_t, ops)
	}
	return false
}

@(export)
barony_dynamic_map_i32_len :: proc "c" (m: rawptr, value_kind: i32) -> i32 {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_len(m, i32)
	case .MK_U32:  return i32_map_len(m, u32)
	case .MK_Bool: return i32_map_len(m, bool)
	case .MK_U64: return i32_map_len(m, u64)
	case .MK_String:  return i32_map_len(m, string)
	case .MK_DynArrayStr: return i32_map_len(m, Raw_Dynamic_Array)
	case .MK_DynArrayS32: return i32_map_len(m, Raw_Dynamic_Array)
	case .MK_StatueLimbArray: return i32_map_len(m, Raw_Dynamic_Array)
	case .MK_StoreSlotsArray: return i32_map_len(m, Raw_Dynamic_Array)
	case .MK_MonsterTrapIgnore: return i32_map_len(m, MonsterTrapIgnoreEntities_t)
	case .MK_SetOfI32: return i32_map_len(m, map[i32]struct{})
	case .MK_I32Map: return i32_map_len(m, map[[4]byte]i32)
	case .MK_U32Map: return i32_map_len(m, map[[4]byte]u32)
	case .MK_U32MapEmitterHit: return i32_map_len(m, map[[4]byte]ParticleEmitterHit_t)
	case .MK_EntityColliderData: return i32_map_len(m, EntityColliderData_t)
	case .MK_SpellItem: return i32_map_len(m, SpellItem_t)
	case .MK_Statue: return i32_map_len(m, Statue_t)
	case .MK_I32MapModelOffset: return i32_map_len(m, map[[4]byte]ModelOffset_t)
	case .MK_Lootbag: return i32_map_len(m, Lootbag_t)
	case .MK_EnemyHPDetails: return i32_map_len(m, EnemyHPDetails_t)
	case .MK_GlyphData: return i32_map_len(m, GlyphData_t)
	case .MK_Statistic: return i32_map_len(m, Statistic_t)
	case .MK_Ptr: return i32_map_len(m, u64)
	case .MK_F64: return i32_map_len(m, f64)
	case .MK_FormationInfo: return i32_map_len(m, FormationInfo_t)
	case .MK_AnimatedTile: return i32_map_len(m, AnimatedTile)
	case .MK_AdditionalOffset: return i32_map_len(m, AdditionalOffset_t)
	case .MK_PlayerRaceHostility: return i32_map_len(m, PlayerRaceHostility_t)
	case .MK_SpellElement: return i32_map_len(m, spellElement_t)
	case .MK_Effect: return i32_map_len(m, Effect_t)
	case .MK_EffectLocations: return i32_map_len(m, EffectLocations_t)
	case .MK_CalloutParticle: return i32_map_len(m, CalloutParticle_t)
	case .MK_ModelOffset: return i32_map_len(m, ModelOffset_t)
	case .MK_EffectDefinitionEntry: return i32_map_len(m, EffectDefinitionEntry_t)
	case .MK_ParticleTimerEffect: return i32_map_len(m, ParticleTimerEffect_t)
	case .MK_IconLookup: return i32_map_len(m, IconLookup_t)
	case .MK_MonsterDataEntry: return i32_map_len(m, MonsterDataEntry_t)
	case .MK_MonsterAllies: return i32_map_len(m, MonsterAllies_t)
	}
	return 0
}

@(export)
barony_dynamic_map_i32_clear :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  i32_map_clear(m, i32, ops)
	case .MK_U32:  i32_map_clear(m, u32, ops)
	case .MK_Bool: i32_map_clear(m, bool, ops)
	case .MK_U64: i32_map_clear(m, u64, ops)
	case .MK_String:  i32_map_clear(m, string, ops)
	case .MK_DynArrayStr: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: i32_map_clear(m, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: i32_map_clear(m, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: i32_map_clear(m, map[i32]struct{}, ops)
	case .MK_I32Map: i32_map_clear(m, map[[4]byte]i32, ops)
	case .MK_U32Map: i32_map_clear(m, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: i32_map_clear(m, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: i32_map_clear(m, EntityColliderData_t, ops)
	case .MK_SpellItem: i32_map_clear(m, SpellItem_t, ops)
	case .MK_Statue: i32_map_clear(m, Statue_t, ops)
	case .MK_I32MapModelOffset: i32_map_clear(m, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: i32_map_clear(m, Lootbag_t, ops)
	case .MK_EnemyHPDetails: i32_map_clear(m, EnemyHPDetails_t, ops)
	case .MK_GlyphData: i32_map_clear(m, GlyphData_t, ops)
	case .MK_Statistic: i32_map_clear(m, Statistic_t, ops)
	case .MK_Ptr: i32_map_clear(m, u64, ops)
	case .MK_F64: i32_map_clear(m, f64, ops)
	case .MK_FormationInfo: i32_map_clear(m, FormationInfo_t, ops)
	case .MK_AnimatedTile: i32_map_clear(m, AnimatedTile, ops)
	case .MK_AdditionalOffset: i32_map_clear(m, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: i32_map_clear(m, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: i32_map_clear(m, spellElement_t, ops)
	case .MK_Effect: i32_map_clear(m, Effect_t, ops)
	case .MK_EffectLocations: i32_map_clear(m, EffectLocations_t, ops)
	case .MK_CalloutParticle: i32_map_clear(m, CalloutParticle_t, ops)
	case .MK_ModelOffset: i32_map_clear(m, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: i32_map_clear(m, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: i32_map_clear(m, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: i32_map_clear(m, IconLookup_t, ops)
	case .MK_MonsterDataEntry: i32_map_clear(m, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: i32_map_clear(m, MonsterAllies_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_destroy :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  i32_map_destroy(m, i32, ops)
	case .MK_U32:  i32_map_destroy(m, u32, ops)
	case .MK_Bool: i32_map_destroy(m, bool, ops)
	case .MK_U64: i32_map_destroy(m, u64, ops)
	case .MK_String:  i32_map_destroy(m, string, ops)
	case .MK_DynArrayStr: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: i32_map_destroy(m, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: i32_map_destroy(m, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: i32_map_destroy(m, map[i32]struct{}, ops)
	case .MK_I32Map: i32_map_destroy(m, map[[4]byte]i32, ops)
	case .MK_U32Map: i32_map_destroy(m, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: i32_map_destroy(m, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: i32_map_destroy(m, EntityColliderData_t, ops)
	case .MK_SpellItem: i32_map_destroy(m, SpellItem_t, ops)
	case .MK_Statue: i32_map_destroy(m, Statue_t, ops)
	case .MK_I32MapModelOffset: i32_map_destroy(m, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: i32_map_destroy(m, Lootbag_t, ops)
	case .MK_EnemyHPDetails: i32_map_destroy(m, EnemyHPDetails_t, ops)
	case .MK_GlyphData: i32_map_destroy(m, GlyphData_t, ops)
	case .MK_Statistic: i32_map_destroy(m, Statistic_t, ops)
	case .MK_Ptr: i32_map_destroy(m, u64, ops)
	case .MK_F64: i32_map_destroy(m, f64, ops)
	case .MK_FormationInfo: i32_map_destroy(m, FormationInfo_t, ops)
	case .MK_AnimatedTile: i32_map_destroy(m, AnimatedTile, ops)
	case .MK_AdditionalOffset: i32_map_destroy(m, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: i32_map_destroy(m, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: i32_map_destroy(m, spellElement_t, ops)
	case .MK_Effect: i32_map_destroy(m, Effect_t, ops)
	case .MK_EffectLocations: i32_map_destroy(m, EffectLocations_t, ops)
	case .MK_CalloutParticle: i32_map_destroy(m, CalloutParticle_t, ops)
	case .MK_ModelOffset: i32_map_destroy(m, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: i32_map_destroy(m, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: i32_map_destroy(m, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: i32_map_destroy(m, IconLookup_t, ops)
	case .MK_MonsterDataEntry: i32_map_destroy(m, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: i32_map_destroy(m, MonsterAllies_t, ops)
	}
}

@(export)
barony_dynamic_map_i32_entry :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> rawptr {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_entry(m, key, i32)
	case .MK_U32:  return i32_map_entry(m, key, u32)
	case .MK_Bool: return i32_map_entry(m, key, bool)
	case .MK_U64: return i32_map_entry(m, key, u64)
	case .MK_String:  return i32_map_entry(m, key, string)
	case .MK_DynArrayStr: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_DynArrayS32: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_StatueLimbArray: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_StoreSlotsArray: return i32_map_entry(m, key, Raw_Dynamic_Array)
	case .MK_MonsterTrapIgnore: return i32_map_entry(m, key, MonsterTrapIgnoreEntities_t)
	case .MK_SetOfI32: return i32_map_entry(m, key, map[i32]struct{})
	case .MK_I32Map: return i32_map_entry(m, key, map[[4]byte]i32)
	case .MK_U32Map: return i32_map_entry(m, key, map[[4]byte]u32)
	case .MK_U32MapEmitterHit: return i32_map_entry(m, key, map[[4]byte]ParticleEmitterHit_t)
	case .MK_EntityColliderData: return i32_map_entry(m, key, EntityColliderData_t)
	case .MK_SpellItem: return i32_map_entry(m, key, SpellItem_t)
	case .MK_Statue: return i32_map_entry(m, key, Statue_t)
	case .MK_I32MapModelOffset: return i32_map_entry(m, key, map[[4]byte]ModelOffset_t)
	case .MK_Lootbag: return i32_map_entry(m, key, Lootbag_t)
	case .MK_EnemyHPDetails: return i32_map_entry(m, key, EnemyHPDetails_t)
	case .MK_GlyphData: return i32_map_entry(m, key, GlyphData_t)
	case .MK_Statistic: return i32_map_entry(m, key, Statistic_t)
	case .MK_Ptr: return i32_map_entry(m, key, u64)
	case .MK_F64: return i32_map_entry(m, key, f64)
	case .MK_FormationInfo: return i32_map_entry(m, key, FormationInfo_t)
	case .MK_AnimatedTile: return i32_map_entry(m, key, AnimatedTile)
	case .MK_AdditionalOffset: return i32_map_entry(m, key, AdditionalOffset_t)
	case .MK_PlayerRaceHostility: return i32_map_entry(m, key, PlayerRaceHostility_t)
	case .MK_SpellElement: return i32_map_entry(m, key, spellElement_t)
	case .MK_Effect: return i32_map_entry(m, key, Effect_t)
	case .MK_EffectLocations: return i32_map_entry(m, key, EffectLocations_t)
	case .MK_CalloutParticle: return i32_map_entry(m, key, CalloutParticle_t)
	case .MK_ModelOffset: return i32_map_entry(m, key, ModelOffset_t)
	case .MK_EffectDefinitionEntry: return i32_map_entry(m, key, EffectDefinitionEntry_t)
	case .MK_ParticleTimerEffect: return i32_map_entry(m, key, ParticleTimerEffect_t)
	case .MK_IconLookup: return i32_map_entry(m, key, IconLookup_t)
	case .MK_MonsterDataEntry: return i32_map_entry(m, key, MonsterDataEntry_t)
	case .MK_MonsterAllies: return i32_map_entry(m, key, MonsterAllies_t)
	}
	return nil
}

@(export)
barony_dynamic_map_i32_entries :: proc "c" (m: rawptr, key_ptrs: [^][4]byte, val_ptrs: rawptr, count: i32, value_kind: i32) -> i32 {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_entries(m, key_ptrs, val_ptrs, count, i32, ops)
	case .MK_U32:  return i32_map_entries(m, key_ptrs, val_ptrs, count, u32, ops)
	case .MK_Bool: return i32_map_entries(m, key_ptrs, val_ptrs, count, bool, ops)
	case .MK_U64: return i32_map_entries(m, key_ptrs, val_ptrs, count, u64, ops)
	case .MK_String:  return i32_map_entries(m, key_ptrs, val_ptrs, count, string, ops)
	case .MK_DynArrayStr: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return i32_map_entries(m, key_ptrs, val_ptrs, count, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: return i32_map_entries(m, key_ptrs, val_ptrs, count, map[i32]struct{}, ops)
	case .MK_I32Map: return i32_map_entries(m, key_ptrs, val_ptrs, count, map[[4]byte]i32, ops)
	case .MK_U32Map: return i32_map_entries(m, key_ptrs, val_ptrs, count, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: return i32_map_entries(m, key_ptrs, val_ptrs, count, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: return i32_map_entries(m, key_ptrs, val_ptrs, count, EntityColliderData_t, ops)
	case .MK_SpellItem: return i32_map_entries(m, key_ptrs, val_ptrs, count, SpellItem_t, ops)
	case .MK_Statue: return i32_map_entries(m, key_ptrs, val_ptrs, count, Statue_t, ops)
	case .MK_I32MapModelOffset: return i32_map_entries(m, key_ptrs, val_ptrs, count, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: return i32_map_entries(m, key_ptrs, val_ptrs, count, Lootbag_t, ops)
	case .MK_EnemyHPDetails: return i32_map_entries(m, key_ptrs, val_ptrs, count, EnemyHPDetails_t, ops)
	case .MK_GlyphData: return i32_map_entries(m, key_ptrs, val_ptrs, count, GlyphData_t, ops)
	case .MK_Statistic: return i32_map_entries(m, key_ptrs, val_ptrs, count, Statistic_t, ops)
	case .MK_Ptr: return i32_map_entries(m, key_ptrs, val_ptrs, count, u64, ops)
	case .MK_F64: return i32_map_entries(m, key_ptrs, val_ptrs, count, f64, ops)
	case .MK_FormationInfo: return i32_map_entries(m, key_ptrs, val_ptrs, count, FormationInfo_t, ops)
	case .MK_AnimatedTile: return i32_map_entries(m, key_ptrs, val_ptrs, count, AnimatedTile, ops)
	case .MK_AdditionalOffset: return i32_map_entries(m, key_ptrs, val_ptrs, count, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: return i32_map_entries(m, key_ptrs, val_ptrs, count, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: return i32_map_entries(m, key_ptrs, val_ptrs, count, spellElement_t, ops)
	case .MK_Effect: return i32_map_entries(m, key_ptrs, val_ptrs, count, Effect_t, ops)
	case .MK_EffectLocations: return i32_map_entries(m, key_ptrs, val_ptrs, count, EffectLocations_t, ops)
	case .MK_CalloutParticle: return i32_map_entries(m, key_ptrs, val_ptrs, count, CalloutParticle_t, ops)
	case .MK_ModelOffset: return i32_map_entries(m, key_ptrs, val_ptrs, count, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: return i32_map_entries(m, key_ptrs, val_ptrs, count, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: return i32_map_entries(m, key_ptrs, val_ptrs, count, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: return i32_map_entries(m, key_ptrs, val_ptrs, count, IconLookup_t, ops)
	case .MK_MonsterDataEntry: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: return i32_map_entries(m, key_ptrs, val_ptrs, count, MonsterAllies_t, ops)
	}
	return 0
}

@(export)
barony_dynamic_map_i32_for_each :: proc "c" (m: rawptr, value_kind: i32, cb: rawptr, userdata: rawptr) {
	context = runtime.default_context()
	f := transmute(MapForeachCb)(cb)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  i32_map_for_each(m, i32, f, userdata)
	case .MK_U32:  i32_map_for_each(m, u32, f, userdata)
	case .MK_Bool: i32_map_for_each(m, bool, f, userdata)
	case .MK_U64: i32_map_for_each(m, u64, f, userdata)
	case .MK_String:  i32_map_for_each(m, string, f, userdata)
	case .MK_DynArrayStr: i32_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_DynArrayS32: i32_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_StatueLimbArray: i32_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_StoreSlotsArray: i32_map_for_each(m, Raw_Dynamic_Array, f, userdata)
	case .MK_MonsterTrapIgnore: i32_map_for_each(m, MonsterTrapIgnoreEntities_t, f, userdata)
	case .MK_SetOfI32: i32_map_for_each(m, map[i32]struct{}, f, userdata)
	case .MK_I32Map: i32_map_for_each(m, map[[4]byte]i32, f, userdata)
	case .MK_U32Map: i32_map_for_each(m, map[[4]byte]u32, f, userdata)
	case .MK_U32MapEmitterHit: i32_map_for_each(m, map[[4]byte]ParticleEmitterHit_t, f, userdata)
	case .MK_EntityColliderData: i32_map_for_each(m, EntityColliderData_t, f, userdata)
	case .MK_SpellItem: i32_map_for_each(m, SpellItem_t, f, userdata)
	case .MK_Statue: i32_map_for_each(m, Statue_t, f, userdata)
	case .MK_I32MapModelOffset: i32_map_for_each(m, map[[4]byte]ModelOffset_t, f, userdata)
	case .MK_Lootbag: i32_map_for_each(m, Lootbag_t, f, userdata)
	case .MK_EnemyHPDetails: i32_map_for_each(m, EnemyHPDetails_t, f, userdata)
	case .MK_GlyphData: i32_map_for_each(m, GlyphData_t, f, userdata)
	case .MK_Statistic: i32_map_for_each(m, Statistic_t, f, userdata)
	case .MK_Ptr: i32_map_for_each(m, u64, f, userdata)
	case .MK_F64: i32_map_for_each(m, f64, f, userdata)
	case .MK_FormationInfo: i32_map_for_each(m, FormationInfo_t, f, userdata)
	case .MK_AnimatedTile: i32_map_for_each(m, AnimatedTile, f, userdata)
	case .MK_AdditionalOffset: i32_map_for_each(m, AdditionalOffset_t, f, userdata)
	case .MK_PlayerRaceHostility: i32_map_for_each(m, PlayerRaceHostility_t, f, userdata)
	case .MK_SpellElement: i32_map_for_each(m, spellElement_t, f, userdata)
	case .MK_Effect: i32_map_for_each(m, Effect_t, f, userdata)
	case .MK_EffectLocations: i32_map_for_each(m, EffectLocations_t, f, userdata)
	case .MK_CalloutParticle: i32_map_for_each(m, CalloutParticle_t, f, userdata)
	case .MK_ModelOffset: i32_map_for_each(m, ModelOffset_t, f, userdata)
	case .MK_EffectDefinitionEntry: i32_map_for_each(m, EffectDefinitionEntry_t, f, userdata)
	case .MK_ParticleTimerEffect: i32_map_for_each(m, ParticleTimerEffect_t, f, userdata)
	case .MK_IconLookup: i32_map_for_each(m, IconLookup_t, f, userdata)
	case .MK_MonsterDataEntry: i32_map_for_each(m, MonsterDataEntry_t, f, userdata)
	case .MK_MonsterAllies: i32_map_for_each(m, MonsterAllies_t, f, userdata)
	}
}

@(export)
barony_dynamic_map_i32_erase :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_erase(m, key, i32, ops)
	case .MK_U32:  return i32_map_erase(m, key, u32, ops)
	case .MK_Bool: return i32_map_erase(m, key, bool, ops)
	case .MK_U64: return i32_map_erase(m, key, u64, ops)
	case .MK_String:  return i32_map_erase(m, key, string, ops)
	case .MK_DynArrayStr: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return i32_map_erase(m, key, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: return i32_map_erase(m, key, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: return i32_map_erase(m, key, map[i32]struct{}, ops)
	case .MK_I32Map: return i32_map_erase(m, key, map[[4]byte]i32, ops)
	case .MK_U32Map: return i32_map_erase(m, key, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: return i32_map_erase(m, key, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: return i32_map_erase(m, key, EntityColliderData_t, ops)
	case .MK_SpellItem: return i32_map_erase(m, key, SpellItem_t, ops)
	case .MK_Statue: return i32_map_erase(m, key, Statue_t, ops)
	case .MK_I32MapModelOffset: return i32_map_erase(m, key, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: return i32_map_erase(m, key, Lootbag_t, ops)
	case .MK_EnemyHPDetails: return i32_map_erase(m, key, EnemyHPDetails_t, ops)
	case .MK_GlyphData: return i32_map_erase(m, key, GlyphData_t, ops)
	case .MK_Statistic: return i32_map_erase(m, key, Statistic_t, ops)
	case .MK_Ptr: return i32_map_erase(m, key, u64, ops)
	case .MK_F64: return i32_map_erase(m, key, f64, ops)
	case .MK_FormationInfo: return i32_map_erase(m, key, FormationInfo_t, ops)
	case .MK_AnimatedTile: return i32_map_erase(m, key, AnimatedTile, ops)
	case .MK_AdditionalOffset: return i32_map_erase(m, key, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: return i32_map_erase(m, key, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: return i32_map_erase(m, key, spellElement_t, ops)
	case .MK_Effect: return i32_map_erase(m, key, Effect_t, ops)
	case .MK_EffectLocations: return i32_map_erase(m, key, EffectLocations_t, ops)
	case .MK_CalloutParticle: return i32_map_erase(m, key, CalloutParticle_t, ops)
	case .MK_ModelOffset: return i32_map_erase(m, key, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: return i32_map_erase(m, key, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: return i32_map_erase(m, key, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: return i32_map_erase(m, key, IconLookup_t, ops)
	case .MK_MonsterDataEntry: return i32_map_erase(m, key, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: return i32_map_erase(m, key, MonsterAllies_t, ops)
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
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return str_map_find(m, key, out_key, out_key_len, out_val, i32, ops)
	case .MK_F32:  return str_map_find(m, key, out_key, out_key_len, out_val, f32, ops)
	case .MK_U32:  return str_map_find(m, key, out_key, out_key_len, out_val, u32, ops)
	case .MK_String:  return str_map_find(m, key, out_key, out_key_len, out_val, string, ops)
	case .MK_LightDef:  return str_map_find(m, key, out_key, out_key_len, out_val, LightDef, ops)
	case .MK_IconEntryTextMap:  return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryTextMap_t, ops)
	case .MK_IconEntryText:  return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryText_t, ops)
	case .MK_WorldIconEntry:  return str_map_find(m, key, out_key, out_key_len, out_val, WorldIconEntry_t, ops)
	case .MK_DiscoveryAnim:  return str_map_find(m, key, out_key, out_key_len, out_val, DiscoveryAnim_t, ops)
	case .MK_SpecialNPC:  return str_map_find(m, key, out_key, out_key_len, out_val, SpecialNPCEntry_t, ops)
	case .MK_ColliderDmg: return str_map_find(m, key, out_key, out_key_len, out_val, ColliderDmgProperties_t, ops)
	case .MK_ItemLoc: return str_map_find(m, key, out_key, out_key_len, out_val, ItemLocalization_t, ops)
	case .MK_Achievement: return str_map_find(m, key, out_key, out_key_len, out_val, Achievement_t, ops)
	case .MK_AchievementData: return str_map_find(m, key, out_key, out_key_len, out_val, AchievementData_t, ops)
	case .MK_IconEntry: return str_map_find(m, key, out_key, out_key_len, out_val, IconEntry_t, ops)
	case .MK_IconEntryCallout: return str_map_find(m, key, out_key, out_key_len, out_val, IconEntryCallout_t, ops)
	case .MK_Binding: return str_map_find(m, key, out_key, out_key_len, out_val, binding_t, ops)
	case .MK_Class: return str_map_find(m, key, out_key, out_key_len, out_val, Class_t, ops)
	case .MK_DynArrayStr: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return str_map_find(m, key, out_key, out_key_len, out_val, Raw_Dynamic_Array, ops)
	case .MK_StringPair: return str_map_find(m, key, out_key, out_key_len, out_val, DynamicStringPair_t, ops)
	case .MK_I32Map: return str_map_find(m, key, out_key, out_key_len, out_val, map[[4]byte]i32, ops)
	case .MK_ItemTooltip: return str_map_find(m, key, out_key, out_key_len, out_val, ItemTooltip_t, ops)
	case .MK_Entry: return str_map_find(m, key, out_key, out_key_len, out_val, Entry_t, ops)
	case .MK_DropDown: return str_map_find(m, key, out_key, out_key_len, out_val, DropDown_t, ops)
	case .MK_StrMapStr: return str_map_find(m, key, out_key, out_key_len, out_val, map[string]string, ops)
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
	#partial switch Value_Kind(value_kind) {
	case .MK_I32:  return i32_map_find(m, key, out_val, out_val_len, i32, ops)
	case .MK_U32:  return i32_map_find(m, key, out_val, out_val_len, u32, ops)
	case .MK_Bool: return i32_map_find(m, key, out_val, out_val_len, bool, ops)
	case .MK_U64: return i32_map_find(m, key, out_val, out_val_len, u64, ops)
	case .MK_String:  return i32_map_find(m, key, out_val, out_val_len, string, ops)
	case .MK_DynArrayStr: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case .MK_DynArrayS32: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case .MK_StatueLimbArray: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case .MK_StoreSlotsArray: return i32_map_find(m, key, out_val, out_val_len, Raw_Dynamic_Array, ops)
	case .MK_MonsterTrapIgnore: return i32_map_find(m, key, out_val, out_val_len, MonsterTrapIgnoreEntities_t, ops)
	case .MK_SetOfI32: return i32_map_find(m, key, out_val, out_val_len, map[i32]struct{}, ops)
	case .MK_I32Map: return i32_map_find(m, key, out_val, out_val_len, map[[4]byte]i32, ops)
	case .MK_U32Map: return i32_map_find(m, key, out_val, out_val_len, map[[4]byte]u32, ops)
	case .MK_U32MapEmitterHit: return i32_map_find(m, key, out_val, out_val_len, map[[4]byte]ParticleEmitterHit_t, ops)
	case .MK_EntityColliderData: return i32_map_find(m, key, out_val, out_val_len, EntityColliderData_t, ops)
	case .MK_SpellItem: return i32_map_find(m, key, out_val, out_val_len, SpellItem_t, ops)
	case .MK_Statue: return i32_map_find(m, key, out_val, out_val_len, Statue_t, ops)
	case .MK_I32MapModelOffset: return i32_map_find(m, key, out_val, out_val_len, map[[4]byte]ModelOffset_t, ops)
	case .MK_Lootbag: return i32_map_find(m, key, out_val, out_val_len, Lootbag_t, ops)
	case .MK_EnemyHPDetails: return i32_map_find(m, key, out_val, out_val_len, EnemyHPDetails_t, ops)
	case .MK_GlyphData: return i32_map_find(m, key, out_val, out_val_len, GlyphData_t, ops)
	case .MK_Statistic: return i32_map_find(m, key, out_val, out_val_len, Statistic_t, ops)
	case .MK_Ptr: return i32_map_find(m, key, out_val, out_val_len, u64, ops)
	case .MK_F64: return i32_map_find(m, key, out_val, out_val_len, f64, ops)
	case .MK_FormationInfo: return i32_map_find(m, key, out_val, out_val_len, FormationInfo_t, ops)
	case .MK_AnimatedTile: return i32_map_find(m, key, out_val, out_val_len, AnimatedTile, ops)
	case .MK_AdditionalOffset: return i32_map_find(m, key, out_val, out_val_len, AdditionalOffset_t, ops)
	case .MK_PlayerRaceHostility: return i32_map_find(m, key, out_val, out_val_len, PlayerRaceHostility_t, ops)
	case .MK_SpellElement: return i32_map_find(m, key, out_val, out_val_len, spellElement_t, ops)
	case .MK_Effect: return i32_map_find(m, key, out_val, out_val_len, Effect_t, ops)
	case .MK_EffectLocations: return i32_map_find(m, key, out_val, out_val_len, EffectLocations_t, ops)
	case .MK_CalloutParticle: return i32_map_find(m, key, out_val, out_val_len, CalloutParticle_t, ops)
	case .MK_ModelOffset: return i32_map_find(m, key, out_val, out_val_len, ModelOffset_t, ops)
	case .MK_EffectDefinitionEntry: return i32_map_find(m, key, out_val, out_val_len, EffectDefinitionEntry_t, ops)
	case .MK_ParticleTimerEffect: return i32_map_find(m, key, out_val, out_val_len, ParticleTimerEffect_t, ops)
	case .MK_IconLookup: return i32_map_find(m, key, out_val, out_val_len, IconLookup_t, ops)
	case .MK_MonsterDataEntry: return i32_map_find(m, key, out_val, out_val_len, MonsterDataEntry_t, ops)
	case .MK_MonsterAllies: return i32_map_find(m, key, out_val, out_val_len, MonsterAllies_t, ops)
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

// ---------------------------------------------------------------------------
// pointer-keyed maps: std::unordered_map<PtrType, V> -> map[rawptr]V.
//
// Keys are non-owning raw pointers (view_t*, SDL_GameController*, ...). Odin
// hashes the pointer address (map[rawptr]V). C++ passes the key as rawptr.
//
// The value is a POD Dither_t { value: i32, lastUpdateTick: u32 } (8 bytes).
// `put` is the only owned-path op and it writes the value by value (POD).
// ---------------------------------------------------------------------------

@(export)
barony_dynamic_map_ptr_init :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	(^[32]byte)(m)^ = 0
}

@(export)
barony_dynamic_map_ptr_put :: proc "c" (m: rawptr, key: rawptr, value: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither, .MK_ChunkDither:
		ptr_map_put(m, key, value, Dither_t, ops)
	}
}

ptr_map_put :: proc(m: rawptr, key: rawptr, value: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[rawptr]V)(m)
	if mm^ == nil {
		mm^ = make(map[rawptr]V)
	}
	k := key
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

@(export)
barony_dynamic_map_ptr_get :: proc "c" (m: rawptr, key: rawptr, out: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither, .MK_ChunkDither:
		return ptr_map_get(m, key, out, Dither_t, ops)
	}
	return false
}

ptr_map_get :: proc(m: rawptr, key: rawptr, out: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[rawptr]V)(m)
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

@(export)
barony_dynamic_map_ptr_len :: proc "c" (m: rawptr, value_kind: i32) -> i32 {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither, .MK_ChunkDither:
		return ptr_map_len(m, Dither_t)
	}
	return 0
}

ptr_map_len :: proc(m: rawptr, $V: typeid) -> i32 {
	mm := transmute(^map[rawptr]V)(m)
	if mm^ == nil {
		return 0
	}
	return i32(len(mm^))
}

@(export)
barony_dynamic_map_ptr_clear :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither, .MK_ChunkDither:
		ptr_map_clear(m, Dither_t, ops)
	}
}

ptr_map_clear :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[rawptr]V)(m)
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

@(export)
barony_dynamic_map_ptr_destroy :: proc "c" (m: rawptr, value_kind: i32) {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither, .MK_ChunkDither:
		ptr_map_destroy(m, Dither_t, ops)
	}
}

ptr_map_destroy :: proc(m: rawptr, $V: typeid, ops: Value_Ops) {
	mm := transmute(^map[rawptr]V)(m)
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

@(export)
barony_dynamic_map_ptr_entry :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> rawptr {
	context = runtime.default_context()
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither:
		return ptr_map_entry(m, key, Dither_t, 0)
	case .MK_ChunkDither:
		return ptr_map_entry(m, key, Dither_t, 10)
	}
	return nil
}

// ptr_map_entry: get-or-insert, returning the live slot. `default_value` sets
// the initial `value` field for a freshly-inserted slot so the two dithering
// defaults match their std::unordered_map origins:
//   Entity::Dither -> 0 (fade in)
//   Chunk::Dither  -> 10 (opaque immediately, fades out)
ptr_map_entry :: proc(m: rawptr, key: rawptr, $V: typeid, default_value: i32) -> rawptr {
	mm := transmute(^map[rawptr]V)(m)
	if mm^ == nil {
		mm^ = make(map[rawptr]V)
	}
	k := key
	_, vp, just_inserted, err := map_entry(mm, k)
	if err != nil {
		return nil
	}
	if just_inserted && default_value != 0 {
		(^Dither_t)(vp).value = default_value
	}
	return vp
}

@(export)
barony_dynamic_map_ptr_entries :: proc "c" (m: rawptr, key_ptrs: [^]rawptr, val_ptrs: rawptr, count: i32, value_kind: i32) -> i32 {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither:
		return ptr_map_entries(m, key_ptrs, val_ptrs, count, Dither_t, ops)
	}
	return 0
}

ptr_map_entries :: proc(m: rawptr, key_ptrs: [^]rawptr, val_ptrs: rawptr, count: i32, $V: typeid, ops: Value_Ops) -> i32 {
	mm := transmute(^map[rawptr]V)(m)
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

@(export)
barony_dynamic_map_ptr_erase :: proc "c" (m: rawptr, key: rawptr, value_kind: i32) -> bool {
	context = runtime.default_context()
	ops := value_ops_for(value_kind)
	#partial switch Value_Kind(value_kind) {
	case .MK_Dither:
		return ptr_map_erase(m, key, Dither_t, ops)
	}
	return false
}

ptr_map_erase :: proc(m: rawptr, key: rawptr, $V: typeid, ops: Value_Ops) -> bool {
	mm := transmute(^map[rawptr]V)(m)
	if mm^ == nil {
		return false
	}
	k := key
	if v, had := mm[k]; had {
		if ops.free != nil {
			ops.free(&v)
		}
		runtime.delete_key(mm, k)
		return true
	}
	return false
}
