// magic.odin — Odin mirrors of magic/magic.hpp.
package main


// struct ParticleEmitterHit_t — 8 bytes
ParticleEmitterHit_t :: struct {
	tick: u32,
	hits: i32,
}
#assert(size_of(ParticleEmitterHit_t) == 8)

// struct Effect_t (ParticleTimerEffect_t::Effect_t) — 40 bytes
Effect_t :: struct {
	x:           f64, // real_t
	y:           f64,
	effect_type: i32, // EffectType enum (4B)
	yaw:         f64,
	sfx:         i32,
	first_effect: bool,
}
#assert(size_of(Effect_t) == 40)

// struct EffectLocations_t (ParticleTimerEffect_t::EffectLocations_t) — 40 bytes
EffectLocations_t :: struct {
	yaw_offset: f64,
	x_offset:   f64,
	seconds:    f64,
	dist:       f64,
	sfx:        i32,
}
#assert(size_of(EffectLocations_t) == 40)

// class ParticleTimerEffect_t — 32 bytes
// { DynamicMapI32T<Effect_t> effectMap; }
ParticleTimerEffect_t :: struct {
	effect_map: map[[4]byte]Effect_t, // DynamicMapI32T<Effect_t> (i32-keyed)
}
#assert(size_of(ParticleTimerEffect_t) == 32)

// struct spellElement_t — 168 bytes
spellElement_t :: struct {
	damage:                 i32,
	damage2:                i32,
	duration2:              i32,
	damage_mult:            f64, // real_t
	damage2_mult:           f64,
	channeled_mana_mult:    f64,
	duration_mult:          f64,
	duration2_mult:         f64,
	channeled_mana_duration: i32,
	duration:               i32,
	element_internal_name:  [64]u8,
	element_id:             i32,
	can_be_learned:         bool,
	channeled_mana:         i32,
	foci_spell:             bool,
	elements:               list_t,
	node:                   ^node_t,
}
#assert(size_of(spellElement_t) == 168)

// struct spell_t — 224 bytes
spell_t :: struct {
	id:                 i32,
	spell_internal_name: [64]u8,
	difficulty:         i32,
	sustain:            bool,
	magicstaff:         bool,
	spellbook:          bool,
	sustain_node:       ^node_t,
	magic_effects_node: ^node_t,
	hide_from_ui:       bool,
	rangefinder:        i32, // SpellRangefinderType enum
	caster:             u32,
	distance:           f64, // real_t
	distance_mult:      f64,
	skill_id:           i32,
	cast_time:          f64,
	cast_time_mult:     f64,
	mana:               i32,
	needs_data_freed:   i32,
	radius:             i32,
	radius_mult:        f64,
	drop_table:         i32,
	life_time:          i32,
	life_time_mult:     f64,
	sustain_effect_dissipate: i32,
	channel_duration:   i32,
	channel_effect_strength: i32,
	elements:           list_t,
}
#assert(size_of(spell_t) == 224)

// struct CastSpellProps_t — 56 bytes
CastSpellProps_t :: struct {
	caster_x:        f64,
	caster_y:        f64,
	target_x:        f64,
	target_y:        f64,
	target_uid:      u32,
	element_index:   i32,
	distance_offset: f64,
	wall_dir:        i32,
	optional_data:   u8,
	overcharge:      u8,
}
#assert(size_of(CastSpellProps_t) == 56)

// struct PrevData_t — 32 bytes
PrevData_t :: struct {
	r:       u8,
	g:       u8,
	b:       u8,
	a:       u8,
	rad_min: f64, // real_t
	rad_max: f64,
	size:    i32,
}
#assert(size_of(PrevData_t) == 32)
