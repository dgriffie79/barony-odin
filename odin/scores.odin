// scores.odin — Odin mirrors of scores.hpp.
package main


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
	items:            [dynamic]score_item_t, // vector<item_t> (POD)
}
#assert(size_of(score_lootbag_t) == 56)

// --- pair mirrors for the serialized stat_t arrays (scores.hpp) ---
// vector<pair<DynamicString, Uint32>> (player_equipment) — 24B
Score_StringU32_Pair :: struct {
	first:  string,
	second: u32,
}
#assert(size_of(Score_StringU32_Pair) == 24)

// vector<pair<DynamicString, item_t>> (npc_equipment) — 48B
Score_StringItem_Pair :: struct {
	first:  string,
	second: score_item_t,
}
#assert(size_of(Score_StringItem_Pair) == 48)

// vector<pair<Uint32, lootbag_t>> (player_lootbags) — 64B
Score_U32Lootbag_Pair :: struct {
	first:  u32,
	second: score_lootbag_t,
}
#assert(size_of(Score_U32Lootbag_Pair) == 64)

// struct stat_t (scores.hpp serialization mirror) — 528 bytes
score_stat_t :: struct {
	name:                  string,
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
	proficiencies:         [dynamic]i32, // DynamicArrayS32
	effects:               [dynamic]i32, // DynamicArrayS32
	effects_timers:        [dynamic]i32, // DynamicArrayS32
	effects_accretion_time: [dynamic]i32, // DynamicArrayS32
	misc_flags:            [dynamic]i32, // DynamicArrayS32
	attributes:            [dynamic]DynamicStringPair_T, // vector<pair<DynamicString,DynamicString>>
	player_equipment:      [dynamic]Score_StringU32_Pair, // vector<pair<DynamicString,Uint32>>
	npc_equipment:         [dynamic]Score_StringItem_Pair, // vector<pair<DynamicString,item_t>>
	inventory:             [dynamic]score_item_t, // vector<item_t> (POD)
	void_chest_inventory:  [dynamic]score_item_t, // vector<item_t> (POD)
	player_lootbags:       [dynamic]Score_U32Lootbag_Pair, // vector<pair<Uint32,lootbag_t>>
}
#assert(size_of(score_stat_t) == 528)

// --- SaveGameInfo::Player::PlayerRaceHostility_t (scores.hpp) — 36B.
// NOTE: distinct from monster.hpp ShopkeeperPlayerHostility_t::PlayerRaceHostility_t
// (40B, has b_requires_net_update). This one is 9 plain ints.
Score_PlayerRaceHostility_t :: struct {
	num_aggressions: i32,
	num_kills:       i32,
	num_accessories: i32,
	player_race:     i32, // int (NOTHING)
	sex:             i32, // int (sex_t::MALE)
	equipment:       i32,
	type:            i32,
	wanted_level:    i32,
	player:          i32,
}
#assert(size_of(Score_PlayerRaceHostility_t) == 36)

// --- pair mirrors for SaveGameInfo::Player pair-vectors (scores.hpp) ---
// DynamicStringPair_T — 32B (std::pair<DynamicString,DynamicString>)
DynamicStringPair_T :: struct {
	first:  string,
	second: string,
}
#assert(size_of(DynamicStringPair_T) == 32)


// vector<pair<int, PlayerRaceHostility_t>> (shopkeeperHostility) — 44B
Score_IntHostility_Pair :: struct {
	first:  i32,
	second: Score_PlayerRaceHostility_t,
}
#assert(size_of(Score_IntHostility_Pair) == 40)

// vector<pair<DynamicString, DynamicArrayS32>> (compendium_item_events) — 56B
Score_StringS32Array_Pair :: struct {
	first:  string,
	second: [dynamic]i32,
}
#assert(size_of(Score_StringS32Array_Pair) == 56)

// vector<pair<int,int>> (itemDegradeRNG/escalatingRngRolls/escalatingSpellRngRolls/
// appraisal_item_progress/sustainedSpellIDCounter/ducksInARow/favoriteBooksAchievement) — 8B
Score_IntInt_Pair :: struct {
	first:  i32,
	second: i32,
}
#assert(size_of(Score_IntInt_Pair) == 8)

// recipe_t = pair<int, pair<int,int>> — 12B (vector<recipe_t> known_recipes)
Score_Recipe_T :: struct {
	first:  i32,
	second: IntPair_T, // pair<int,int>
}
#assert(size_of(Score_Recipe_T) == 12)

// struct SaveGameInfo::Player (scores.hpp) — 1816 bytes.
// Mirror of the nested hiscore save record. NOTE: distinct from the game
// class Player (player.odin, 9176B). The containers shim twin (HiscorePlayer_t) is the
// shim-side layout twin for element free/copy dispatch.
SaveGameInfo_Player_T :: struct {
	char_class:                  u32,
	race:                        u32,
	kills:                       [dynamic]i32, // DynamicArrayS32
	conduct_penniless:           bool,
	conduct_foodless:            bool,
	conduct_vegetarian:          bool,
	conduct_illiterate:          bool,
	additional_conducts:         [32]i32, // NUM_CONDUCT_CHALLENGES
	game_statistics:             [64]i32, // NUM_GAMEPLAY_STATISTICS
	hotbar:                      [10]u32, // NUM_HOTBAR_SLOTS
	hotbar_alternate:            [5][10]u32, // NUM_HOTBAR_ALTERNATES x NUM_HOTBAR_SLOTS
	selected_spell:              u32,
	selected_spell_alternate:    [5]u32, // NUM_HOTBAR_ALTERNATES
	selected_spell_last_appearance: i32,
	spells:                      [dynamic]u32, // DynamicArrayU32
	known_recipes:               [dynamic]Score_Recipe_T, // vector<recipe_t> = pair<int,pair<int,int>> (12B)
	known_scrolls:               [dynamic]i32, // DynamicArrayS32
	shopkeeper_hostility:        [dynamic]Score_IntHostility_Pair, // vector<pair<int,PlayerRaceHostility_t>>
	compendium_item_events:      [dynamic]Score_StringS32Array_Pair, // vector<pair<DynamicString,DynamicArrayS32>>
	item_degrade_rng:            [dynamic]Score_IntInt_Pair, // vector<pair<int,int>>
	escalating_rng_rolls:        [dynamic]Score_IntInt_Pair,
	escalating_spell_rng_rolls:  [dynamic]Score_IntInt_Pair,
	appraisal_item_progress:     [dynamic]Score_IntInt_Pair,
	learned_spells:              [dynamic]i32, // DynamicArrayS32
	sustained_spell_id_counter:  [dynamic]Score_IntInt_Pair,
	ducks_in_a_row:              [dynamic]Score_IntInt_Pair,
	favorite_books_achievement:  [dynamic]Score_IntInt_Pair,
	sustained_spell_mp_used_sorcery: i32,
	sustained_spell_mp_used_mysticism: i32,
	sustained_spell_mp_used_thaumaturgy: i32,
	base_spell_mp_used_sorcery:  i32,
	base_spell_mp_used_mysticism: i32,
	base_spell_mp_used_thaumaturgy: i32,
	stats:                       score_stat_t, // stat_t
	followers:                   [dynamic]score_stat_t, // DynamicArrayT<stat_t>
}
#assert(size_of(SaveGameInfo_Player_T) == 1816)

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
	magic_cookie:         string,
	game_version:         i32,
	timestamp:            string,
	hash:                 u32,
	gamename:             string,
	gamekey:              u32,
	lobbykey:             u32,
	mapseed:              u32,
	gametimer:            u32,
	svflags:              u32,
	customseed:           u32,
	customseed_string:    string,
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
	players_connected:    [dynamic]i32, // DynamicArrayS32
	players:              [dynamic]SaveGameInfo_Player_T, // DynamicArrayT<SaveGameInfo::Player> (hiscore save record, NOT game Player)
	map_messages:         [dynamic]DynamicStringPair_T, // DynamicArrayStringPair
	additional_data:      [dynamic]DynamicStringPair_T, // DynamicArrayStringPair
}
#assert(size_of(SaveGameInfo) == 312)
