// mod_tools.odin — Odin mirror of mod_tools.hpp (data-carrying top-level structs).
//
// NOTE: mod_tools.hpp is the "port-with-file" giant. Most of its deeply-nested
// map-value structs already have shim twins in the containers package
// (Statue_t, EntityColliderData_t, ItemTooltip_t, spellItem_t, Entry_t,
// World_t, Codex_t, Monster_t, Event_t, EventVal_t, ModelOffset_t, etc.) used
// by the C++ map shims — those are NOT re-mirrored here. The manager classes
// (CustomHelpers, MonsterStatCustomManager, GameplayCustomManager,
// GameModeManager_t, Compendium_t, ItemTooltips_t, StatueManager_t, ...) are
// statics-only and need no instance mirror. This file mirrors the top-level
// instance-data structs that cross the C++/Odin boundary.
package main

// struct LibCURL_t — 16 bytes
LibCURL_T :: struct {
	b_init:  bool,
	// pad 7
	handle:  rawptr, // CURL*
}
#assert(size_of(LibCURL_T) == 16)

// struct EquipmentModelOffsets_t::ModelOffset_t::AdditionalOffset_t — 48 bytes
ModelOffset_AdditionalOffset_T :: struct {
	focalx: f64,
	focaly: f64,
	focalz: f64,
	scalex: f64,
	scaley: f64,
	scalez: f64,
}
#assert(size_of(ModelOffset_AdditionalOffset_T) == 48)

// struct EquipmentModelOffsets_t::ModelOffset_t — 160 bytes
ModelOffset_T :: struct {
	focalx:         f64,
	focaly:         f64,
	focalz:         f64,
	scalex:         f64,
	scaley:         f64,
	scalez:         f64,
	rotation:       f64,
	pitch:          f64,
	x:              f64,
	y:              f64,
	z:              f64,
	limbs_index:    i32,
	oversized_mask: bool,
	expand_to_fit_mask: bool,
	// pad 6 (align maps)
	adjust_to_oversize_mask: map[[4]byte]ModelOffset_AdditionalOffset_T, // DynamicMapI32T<AdditionalOffset_t>
	adjust_to_expanded_helm: map[[4]byte]ModelOffset_AdditionalOffset_T,
}
#assert(size_of(ModelOffset_T) == 160)

// struct EquipmentModelOffsets_t — 64 bytes (2 maps)
EquipmentModelOffsets_T :: struct {
	monster_models_map:     map[[4]byte]map[[4]byte]ModelOffset_T, // DynamicMapI32T<DynamicMapI32T<ModelOffset_t>>
	misc_items_base_offsets: map[[4]byte]ModelOffset_T,            // DynamicMapI32T<ModelOffset_t>
}
#assert(size_of(EquipmentModelOffsets_T) == 64)

// struct TreasureRoomGenerator — 688 bytes
TreasureRoomGenerator :: struct {
	treasure_rng:          Barony_RNG, // 528B
	treasure_floors:       map[i32]struct{}, // DynamicSetI32 (32B)
	treasure_secret_floors: map[i32]struct{},
	orb_floors:            map[[4]byte]string, // DynamicMapI32Str (32B)
	station_floors:        map[[4]byte]string,
	station_secret_floors: map[[4]byte]string,
}
#assert(size_of(TreasureRoomGenerator) == 688)
