// monster.odin — Odin mirrors of monster.hpp.
package main

import "containers"

// Monster enum is huge; referenced as i32 for layout (full mirror later).
Monster :: i32

// struct MonsterDataEntry_t::IconLookup_t — 32 bytes
MonsterData_IconLookup_t :: struct {
	key:      containers.DynamicString,
	icon_path: containers.DynamicString,
}
#assert(size_of(MonsterData_IconLookup_t) == 32)

// struct MonsterData_t::MonsterDataEntry_t — 200 bytes
MonsterDataEntry_t :: struct {
	monster_type:            i32,
	default_icon_path:       containers.DynamicString,
	icon_sprites_and_paths:  containers.Raw_Map, // DynamicMapI32T<IconLookup_t> (32B)
	key_to_sprite_lookup:    containers.Raw_Map, // DynamicMapStrT<DynamicArrayS32> (32B)
	model_indexes:           containers.Raw_Map, // DynamicSetI32 (32B)
	player_model_indexes:    containers.Raw_Map, // DynamicSetI32 (32B)
	default_short_display_name: containers.DynamicString,
	special_npcs:            containers.Raw_Map, // DynamicMapSpecialNPC (32B)
}
#assert(size_of(MonsterDataEntry_t) == 200)

// struct MonsterData_t — holds the entries map (statics; the class itself is
// empty of per-instance data in the C++ struct) — represented as the entries map.
MonsterData_t :: struct {
	monster_data_entries: containers.Raw_Map, // DynamicMapI32T<MonsterDataEntry_t>
}

// class ShopkeeperPlayerHostility_t — 128 bytes
// { DynamicMapI32T<PlayerRaceHostility_t> playerHostility[4]; }
ShopkeeperPlayerHostility_t :: struct {
	player_hostility: [4]containers.Raw_Map, // 4 x DynamicMapI32T (32B each)
}
#assert(size_of(ShopkeeperPlayerHostility_t) == 128)

// struct ShopkeeperPlayerHostility_t::PlayerRaceHostility_t — 40 bytes
PlayerRaceHostility_t :: struct {
	num_aggressions:  i32,
	num_kills:        i32,
	num_accessories:  i32,
	player_race:      Monster,
	sex:              sex_t,
	equipment:        u8,
	type:             u32,
	wanted_level:     Wanted_Level, // enum WantedLevel
	player:           i32,
	b_requires_net_update: bool,
}
#assert(size_of(PlayerRaceHostility_t) == 40)

// enum WantedLevel (monster.hpp, nested in ShopkeeperPlayerHostility_t)
Wanted_Level :: enum i32 {
	NO_WANTED_LEVEL,
	WANTED_MURDER,
	WANTED_THEFT,
	WANTED_FOR_KILL,
}

// struct MonsterAllies_t (MonsterAllyFormation_t::MonsterAllies_t) — 72 bytes
MonsterAllies_t :: struct {
	melee_units:   containers.Raw_Map, // DynamicMapI32T<FormationInfo_t> (32B)
	ranged_units:  containers.Raw_Map, // DynamicMapI32T<FormationInfo_t> (32B)
	updated_on_tick: u32,
}
#assert(size_of(MonsterAllies_t) == 72)

// struct FormationInfo_t — 20 bytes
FormationInfo_t :: struct {
	x:              i32,
	y:              i32,
	pathing_delay:  i32,
	try_extend_path: i32,
	init:           bool,
	expired:        bool,
}
#assert(size_of(FormationInfo_t) == 20)

// class MonsterAllyFormation_t — 72 bytes
// { DynamicMapI32T<MonsterAllies_t> units; DynamicArrayT<pair<int,int>> formationShape; }
MonsterAllyFormation_t :: struct {
	units:           containers.Raw_Map, // DynamicMapI32T<MonsterAllies_t> (32B)
	formation_shape: containers.Raw_Dynamic_Array, // DynamicArrayT<pair<int,int>> (40B)
}
#assert(size_of(MonsterAllyFormation_t) == 72)

// class MimicGenerator — 592 bytes
MimicGenerator :: struct {
	mimic_rng:          Barony_RNG, // 528B
	mimic_floors:       containers.Raw_Map, // DynamicSetI32 (32B)
	mimic_secret_floors: containers.Raw_Map, // DynamicSetI32 (32B)
}
#assert(size_of(MimicGenerator) == 592)
