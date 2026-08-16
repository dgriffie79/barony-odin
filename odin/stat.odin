// stat.odin — Odin mirrors of stat.hpp.
package main

import "containers"


// constants (stat.hpp)
NUMEFFECTS       :: 160
NUMSTATS         :: 6
NUMPROFICIENCIES :: 16
ITEM_SLOT_NUM    :: 112 // chain: HELM=0 + 16*7

// enums (stat.hpp)
// typedef enum { MALE=0, FEMALE=1 } sex_t;
sex_t :: enum i32 {
	MALE = 0,
	FEMALE = 1,
}

// enum KilledBy { UNKNOWN, MONSTER, ITEM, ... } — declared in stat.hpp
KilledBy :: enum i32 {
	UNKNOWN,
	MONSTER,
	ITEM,
	ALLY_BETRAYAL,
	ATTEMPTED_ROBBERY,
	TRESPASSING,
	TRAP_ARROW,
	TRAP_BEAR,
	TRAP_SPIKE,
	TRAP_MAGIC,
	TRAP_BOMB,
	BOULDER,
	LAVA,
	WATER,
}


// struct Lootbag_t — 48 bytes
// { int spawn_x, spawn_y; bool spawnedOnGround, looted; DynamicArrayT<Item> items; }
Lootbag_t :: struct {
	spawn_x:         i32,
	spawn_y:         i32,
	spawned_on_ground: bool,
	looted:            bool,
	items:           [dynamic]Item, // DynamicArrayT<Item>
}

#assert(size_of(Lootbag_t) == 56) // 12B scalars + pad + 40B array

// struct MonsterRangedAccuracy — 24 bytes
// { Uint32 lastTarget; real_t accuracy; Uint32 lastTick; }
Monster_Ranged_Accuracy :: struct {
	last_target: u32,
	accuracy:    f64, // real_t
	last_tick:   u32,
}

#assert(size_of(Monster_Ranged_Accuracy) == 24)

// class Stat — 2856 bytes (field order from libclang dump of stat.hpp)
// The 20 former reference members (sneaking, playerRace, ...) are now getters
// in C++; in Odin they are accessor procs reading MISC_FLAGS[] (below).
Stat :: struct {
	proficiencies:       [16]i32, // Sint32 PROFICIENCIES[NUMPROFICIENCIES]
	effects:             [160]u8, // Uint8 EFFECTS[NUMEFFECTS]
	type:                Monster,
	sex:                 sex_t,
	stat_appearance:     u32,
	name:                [128]u8, // char name[128]
	poison_killer:       u32,
	obituary:            [128]u8,
	killer:              KilledBy,
	killer_uid:          u32,
	killer_monster:      Monster,
	killer_item:         ItemType,
	killer_name:         containers.DynamicString, // 16B
	hp:                  i32, // HP
	maxhp:               i32,
	oldhp:               i32,
	mp:                  i32,
	maxmp:               i32,
	str:                 i32,
	dex:                 i32,
	con:                 i32,
	int_:                i32,
	per:                 i32,
	chr:                 i32,
	exp:                 i32,
	lvl:                 i32,
	gold:                i32,
	hunger:              i32,
	random_str:          i32,
	random_dex:          i32,
	random_con:          i32,
	random_int:          i32,
	random_per:          i32,
	random_chr:          i32,
	random_maxhp:        i32,
	random_hp:           i32,
	random_maxmp:        i32,
	random_mp:           i32,
	random_lvl:          i32,
	random_gold:         i32,
	misc_flags:          [32]i32,
	player_lvl_stat_bonus: [6]i32,
	player_lvl_stat_timer: [12]i32,
	effects_accretion_time: [160]u32,
	effects_timers:      [160]i32,
	defending:           bool,
	parrying:            u32,
	leader_uid:          u32,
	followers:           list_t,
	inventory:           list_t,
	helmet:              ^Item,
	breastplate:         ^Item,
	gloves:              ^Item,
	shoes:               ^Item,
	shield:              ^Item,
	weapon:              ^Item,
	cloak:               ^Item,
	amulet:              ^Item,
	ring:                ^Item,
	mask:                ^Item,
	monster_sound:       rawptr, // void* (former FMOD/OpenAL channel)
	monster_idlevar:     i32,
	attributes:          map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	player_lootbags:     map[[4]byte]Lootbag_t, // DynamicMapI32T<Lootbag_t> (i32-keyed)
	void_chest_inventory: list_t,
	magic_effects:       list_t,
	editor_items:        [112]i32, // Sint32 EDITOR_ITEMS[ITEM_SLOT_NUM] (112)
	monster_ranged_accuracy: Monster_Ranged_Accuracy,
	item_last_degrades_tick: map[[4]byte]u32, // DynamicMapI32T<Uint32> (i32-keyed)
}

#assert(size_of(Stat) == 2856)

// ---------------------------------------------------------------------------
// Accessor procs for the former reference members (stat.hpp getters).
// C++: inline int& sneaking() { return MISC_FLAGS[1]; } etc.
// ---------------------------------------------------------------------------
stat_sneaking              :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[1] }
stat_ally_item_pickup      :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[2] }
stat_ally_class            :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[3] }
stat_player_race           :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[4] }
stat_player_polymorph_storage :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[5] }
stat_player_summon_lvlhp   :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[6] }
stat_player_summon_strdexconint :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[7] }
stat_player_summon_perchr  :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[8] }
stat_player_summon2_lvlhp  :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[9] }
stat_player_summon2_strdexconint :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[10] }
stat_player_summon2_perchr :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[11] }
stat_monster_is_charmed    :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[12] }
stat_player_shapeshift_storage :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[13] }
stat_monster_tinkering_status :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[14] }
stat_monster_mimic_locked_by :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[14] } // coincident alias (same slot)
stat_monster_demon_has_been_exorcised :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[15] }
stat_bleed_inflicted_by    :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[17] }
stat_burning_inflicted_by  :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[18] }
stat_monster_no_drop_items :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[19] }
stat_monster_force_allegiance :: proc(s: ^Stat) -> ^i32 { return &s.misc_flags[20] }
