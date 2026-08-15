// scores.odin — Odin mirrors of scores.hpp.
package main

import "containers"

// enum EFileFormat (json.hpp)
EFileFormat :: enum i32 {
	Json,
	Json_Compact,
	Binary,
}

// struct item_t (scores.hpp serialization mirror) — 32 bytes
score_item_t :: struct {
	type:       u32,
	status:     u32,
	appearance: u32,
	beatitude:  i32,
	count:      i32,
	identified: bool,
	x:          i32,
	y:          i32,
}
#assert(size_of(score_item_t) == 32)

// struct lootbag_t (scores.hpp) — 56 bytes
score_lootbag_t :: struct {
	spawn_x:          i32,
	spawn_y:          i32,
	spawned_on_ground: bool,
	looted:           bool,
	items:            containers.Raw_Dynamic_Array, // DynamicArray (40B)
}
#assert(size_of(score_lootbag_t) == 56)

// struct stat_t (scores.hpp serialization mirror) — 528 bytes
score_stat_t :: struct {
	name:                  containers.DynamicString,
	type:                  u32,
	sex:                   u32,
	statscore_appearance:  u32,
	hp:                    i32, // HP
	maxhp:                 i32,
	mp:                    i32,
	maxmp:                 i32,
	str:                   i32,
	dex:                   i32,
	con:                   i32,
	int_:                  i32,
	per:                   i32,
	chr:                   i32,
	exp:                   i32,
	lvl:                   i32,
	gold:                  i32,
	hunger:                i32,
	proficiencies:         containers.Raw_Dynamic_Array, // DynamicArrayS32 (40B)
	effects:               containers.Raw_Dynamic_Array,
	effects_timers:        containers.Raw_Dynamic_Array,
	effects_accretion_time: containers.Raw_Dynamic_Array,
	misc_flags:            containers.Raw_Dynamic_Array,
	attributes:            containers.Raw_Dynamic_Array,
	player_equipment:      containers.Raw_Dynamic_Array,
	npc_equipment:         containers.Raw_Dynamic_Array,
	inventory:             containers.Raw_Dynamic_Array,
	void_chest_inventory:  containers.Raw_Dynamic_Array,
	player_lootbags:       containers.Raw_Dynamic_Array,
}
#assert(size_of(score_stat_t) == 528)

// struct score_t — 632 bytes
score_t :: struct {
	kills:                 [53]i32, // Sint32 kills[53]
	stats:                 ^Stat,
	classnum:              i32,
	dungeonlevel:          i32,
	victory:               i32,
	totalscore:            i32,
	completion_time:       u32,
	conduct_penniless:     bool,
	conduct_foodless:      bool,
	conduct_vegetarian:    bool,
	conduct_illiterate:    bool,
	conduct_game_challenges: [32]i32,
	game_statistics:       [64]i32,
}
#assert(size_of(score_t) == 632)

// struct SaveGameInfo — 312 bytes
SaveGameInfo :: struct {
	magic_cookie:         containers.DynamicString,
	game_version:         i32,
	timestamp:            containers.DynamicString,
	hash:                 u32,
	gamename:             containers.DynamicString,
	gamekey:              u32,
	lobbykey:             u32,
	mapseed:              u32,
	gametimer:            u32,
	svflags:              u32,
	customseed:           u32,
	customseed_string:    containers.DynamicString,
	player_num:           i32,
	multiplayer_type:     i32,
	dungeon_lvl:          i32,
	level_track:          i32,
	hiscore_loadstatus:   i32,
	hiscore_totalscore:   i32,
	hiscore_rank:         i32,
	hiscore_victory:      i32,
	hiscore_killed_by:    i32,
	hiscore_killed_monster: i32,
	hiscore_killed_item:  i32,
	hiscore_dummy_loading: bool,
	players_connected:    containers.Raw_Dynamic_Array, // DynamicArrayS32 (40B)
	players:              containers.Raw_Dynamic_Array, // DynamicArrayT<Player> (40B)
	map_messages:         containers.Raw_Dynamic_Array, // DynamicArrayStringPair (40B)
	additional_data:      containers.Raw_Dynamic_Array, // DynamicArrayStringPair (40B)
}
#assert(size_of(SaveGameInfo) == 312)
