// monster.odin - Odin mirrors of monster.hpp.
package main


// Monster enum is huge; referenced as i32 for layout (full mirror later).
Monster :: i32

// struct MonsterDataEntry_t::IconLookup_t - 32 bytes
// SpecialNPCEntry_T - 104B (SpecialNPCEntry_tMirror)
SpecialNPCEntry_T :: struct {
	internal_name: string,
	name:          string,
	shortname:     string,
	model_indexes: map[i32]struct{},
	base_model:    i32,
	unique_icon:   string,
}
#assert(size_of(SpecialNPCEntry_T) == 104)


MonsterData_IconLookup_t :: struct {
	key:      string,
	icon_path: string,
}
#assert(size_of(MonsterData_IconLookup_t) == 32)

// struct MonsterData_t::MonsterDataEntry_t - 200 bytes
MonsterDataEntry_t :: struct {
	monster_type:            i32,
	default_icon_path:       string,
	icon_sprites_and_paths:  map[[4]byte]MonsterData_IconLookup_t, // DynamicMapI32T<IconLookup_t> (i32-keyed)
	key_to_sprite_lookup:    map[string][dynamic]i32, // DynamicMapStrT<DynamicArrayS32> (string-keyed)
	model_indexes:           map[i32]struct{}, // DynamicSetI32
	player_model_indexes:    map[i32]struct{}, // DynamicSetI32
	default_short_display_name: string,
	special_npcs:            map[string]SpecialNPCEntry_T, // DynamicMapSpecialNPC (string-keyed)
}
#assert(size_of(MonsterDataEntry_t) == 200)

// struct MonsterData_t - holds the entries map (statics; the class itself is
// empty of per-instance data in the C++ struct) - represented as the entries map.
MonsterData_t :: struct {
	monster_data_entries: map[[4]byte]MonsterDataEntry_t, // DynamicMapI32T<MonsterDataEntry_t> (i32-keyed)
}

// class ShopkeeperPlayerHostility_t - 128 bytes
// { DynamicMapI32T<PlayerRaceHostility_t> playerHostility[MAXPLAYERS]; }
ShopkeeperPlayerHostility_t :: struct {
	player_hostility: [MAXPLAYERS]map[[4]byte]PlayerRaceHostility_t, // 4 x DynamicMapI32T (i32-keyed)
}
#assert(size_of(ShopkeeperPlayerHostility_t) == 128)

// struct ShopkeeperPlayerHostility_t::PlayerRaceHostility_t - 40 bytes
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

// struct MonsterAllies_t (MonsterAllyFormation_t::MonsterAllies_t) - 72 bytes
MonsterAllies_t :: struct {
	melee_units:   map[[4]byte]FormationInfo_t, // DynamicMapI32T<FormationInfo_t> (i32-keyed)
	ranged_units:  map[[4]byte]FormationInfo_t, // DynamicMapI32T<FormationInfo_t> (i32-keyed)
	updated_on_tick: u32,
}
#assert(size_of(MonsterAllies_t) == 72)

// struct FormationInfo_t - 20 bytes
FormationInfo_t :: struct {
	x:              i32,
	y:              i32,
	pathing_delay:  i32,
	try_extend_path: i32,
	init:           bool,
	expired:        bool,
}
#assert(size_of(FormationInfo_t) == 20)

// class MonsterAllyFormation_t - 72 bytes
// { DynamicMapI32T<MonsterAllies_t> units; DynamicArrayT<pair<int,int>> formationShape; }
MonsterAllyFormation_t :: struct {
	units:           map[[4]byte]MonsterAllies_t, // DynamicMapI32T<MonsterAllies_t> (i32-keyed)
	formation_shape: [dynamic]IntPair_T, // DynamicArrayT<pair<int,int>> (POD pair)
}
#assert(size_of(MonsterAllyFormation_t) == 72)

// class MimicGenerator - 592 bytes
MimicGenerator :: struct {
	mimic_rng:          Barony_RNG, // 528B
	mimic_floors:       map[i32]struct{}, // DynamicSetI32
	mimic_secret_floors: map[i32]struct{}, // DynamicSetI32
}
#assert(size_of(MimicGenerator) == 592)
