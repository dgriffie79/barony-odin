// ui_gameui.odin — Odin mirror of ui/GameUI.hpp.
package main

// struct EnemyBarSettings_t — 64 bytes
// { DynamicMapF32 heightOffsets (32); DynamicMapF32 screenDistanceOffsets (32); }
EnemyBarSettings_T :: struct {
	height_offsets:          map[string]f32, // DynamicMapF32 (string-keyed)
	screen_distance_offsets: map[string]f32,
}
#assert(size_of(EnemyBarSettings_T) == 64)

// struct CustomColors_t — 76 bytes (19 x Uint32)
CustomColors_T :: struct {
	item_context_menu_heading_text:          u32,
	item_context_menu_option_text:           u32,
	item_context_menu_option_selected_text:  u32,
	item_context_menu_option_img:            u32,
	item_context_menu_option_selected_img:   u32,
	character_sheet_neutral:                 u32,
	character_sheet_light_neutral:           u32,
	character_sheet_lighter1_neutral:        u32,
	character_sheet_darker1_neutral:         u32,
	character_sheet_green:                   u32,
	character_sheet_red:                     u32,
	character_sheet_faint_text:              u32,
	character_sheet_off_white_text:          u32,
	character_sheet_heading_text:            u32,
	character_sheet_highlight_text:          u32,
	character_base_class_text:               u32,
	character_dlc1_class_text:               u32,
	character_dlc2_class_text:               u32,
	character_dlc3_class_text:               u32,
}
#assert(size_of(CustomColors_T) == 76)

// enum StatusEffectQueueEntry_t::NotificationStates_t
StatusEffect_NotificationStates :: enum i32 {
	STATE_1,
	STATE_2,
	STATE_3,
	STATE_4,
	STATE_END,
}

// enum StatusEffectQueueEntry_t::Dir_t
StatusEffect_Dir :: enum i32 {
	NONE,
	LEFT,
	UP,
	DOWN,
	RIGHT,
}

// struct StatusEffectQueueEntry_t — 160 bytes
StatusEffectQueueEntry_T :: struct {
	animate_x:                  f64,
	animate_y:                  f64,
	animate_w:                  f64,
	animate_h:                  f64,
	animate_setpoint_x:         i32,
	animate_setpoint_y:         i32,
	animate_setpoint_w:         i32,
	animate_setpoint_h:         i32,
	animate_start_x:            i32,
	animate_start_y:            i32,
	animate_start_w:            i32,
	animate_start_h:            i32,
	pos:                        SDL_Rect,
	notification_target_position: SDL_Rect,
	last_update_tick:           u32,
	effect:                     i32, // default -1
	custom_variable:            u32,
	low_duration:               bool,
	// pad 3
	notification_state:         StatusEffect_NotificationStates, // default STATE_1
	notification_state_init:    StatusEffect_NotificationStates,
	navigation:                 map[[4]byte]i64, // DynamicMapI32T<size_t> (32B)
	index:                      i64, // size_t
}
#assert(size_of(StatusEffectQueueEntry_T) == 160)

// struct StatusEffectQueue_t — 168 bytes
StatusEffectQueue_T :: struct {
	status_effect_frame:         ^Frame, // Frame*
	status_effect_tooltip_frame: ^Frame,
	player:                      i32, // default -1
	effect_queue:                [dynamic]StatusEffectQueueEntry_T, // 40B
	notification_queue:          [dynamic]StatusEffectQueueEntry_T,
	tooltip_opacity_setpoint:    f64, // default 100
	tooltip_opacity_animate:     f64, // default 1.0
	tooltip_deselected_tick:     u32,
	tooltip_showing_effect_id:   i32, // default -1
	tooltip_showing_effect_variable: i32, // default -1
	effects_per_row:             i32, // default 4
	focused_window_anim:         f64,
	requires_anim_update:        bool,
	b_compact_width:             bool,
	b_compact_height:            bool,
	effects_bounding_box:        SDL_Rect,
	selected_element:            i32, // default -1
}
#assert(size_of(StatusEffectQueue_T) == 168)

// struct StatusEffectQueue_t::EffectDefinitionEntry_t — 248 bytes
EffectDefinitionEntry_T :: struct {
	effect_id:                     i32, // default -1
	spell_id:                      i32, // default -1
	internal_name:                 string, // DynamicString
	name:                          string,
	desc:                          string,
	img_path:                      string,
	name_variations:               [dynamic]string, // DynamicArrayStr
	desc_variations:               [dynamic]string,
	use_spell_id_for_img_variations: [dynamic]i32, // DynamicArrayS32
	img_path_variations:           [dynamic]string,
	use_spell_id_for_img:          i32, // default -1
	never_display:                 bool,
	// pad 3
	sustained_spell_id:            i32, // default -1
	tooltip_width:                 i32, // default 200
}
#assert(size_of(EffectDefinitionEntry_T) == 248)

// struct SkillSheetFrames_t — 280 bytes
SkillSheetFrames_T :: struct {
	skills_frame:              ^Frame,
	entry_frame_left:          ^Frame,
	entry_frame_right:         ^Frame,
	skill_desc_frame:          ^Frame,
	skill_bg_imgs_frame:       ^Frame,
	scroll_area_outer_frame:   ^Frame,
	scroll_area:               ^Frame,
	entry_frames:              [16]^Frame, // NUMPROFICIENCIES
	effect_frames:             [10]^Frame,
	legend_frame:              ^Frame,
	legend_text_requires_reflow: bool,
	// pad 7
}
#assert(size_of(SkillSheetFrames_T) == 280)

// struct PlayerInventoryFrames_t — 152 bytes
PlayerInventoryFrames_T :: struct {
	inventory_bg_frame:         ^Frame,
	selected_slot_frame:        ^Frame,
	old_selected_slot_frame:    ^Frame,
	chest_frame_slots:          ^Frame,
	doll_slots_frame:           ^Frame,
	inv_slots_frame:            ^Frame,
	backpack_frame:             ^Frame,
	flourish_frame:             ^Frame,
	character_preview:          ^Frame,
	inventory_base_images_frame: ^Frame,
	backpack_slots_frame:       ^Frame,
	chest_bg_frame:             ^Frame,
	autosort_frame:             ^Frame,
	default_inv_img:            ^Frame_Image_T, // Frame::image_t*
	compact_inv_img:            ^Frame_Image_T,
	compact_char_img:           ^Frame_Image_T,
	old_selected_slot_item_img: ^Frame_Image_T,
	chest_base_img:             ^Frame_Image_T,
	spell_base_img:             ^Frame_Image_T,
}
#assert(size_of(PlayerInventoryFrames_T) == 152)

// struct MinotaurWarning_t — 152 bytes
MinotaurWarning_T :: struct {
	state:                      i32,
	state_init:                 i32,
	anim_fade:                  f64,
	anim_bg:                    f64,
	anim_flash:                 f64,
	anim_flash_increase:        bool,
	// pad 7
	animate_x:                  f64,
	animate_y:                  f64,
	animate_w:                  f64,
	animate_h:                  f64,
	animate_setpoint_x:         i32,
	animate_setpoint_y:         i32,
	animate_setpoint_w:         i32,
	animate_setpoint_h:         i32,
	animate_start_x:            i32,
	animate_start_y:            i32,
	animate_start_w:            i32,
	animate_start_h:            i32,
	anim_ticks:                 u32,
	pos:                        SDL_Rect,
	processed_on_tick:          u32,
	started:                    bool,
	initial_warning_completed:  bool,
	minotaur_uid:               u32,
	minotaur_spawned:           bool,
	minotaur_died:              bool,
	level_processed:            i32,
	secretlevel_processed:      bool,
}
#assert(size_of(MinotaurWarning_T) == 152)

// struct LevelUpAnimation_t — 40 bytes
LevelUpAnimation_T :: struct {
	lvl_ups: [dynamic]LevelUp_T, // DynamicArrayT<LevelUp_t> (40B)
}
#assert(size_of(LevelUpAnimation_T) == 40)

// struct LevelUpAnimation_t::LevelUp_t — 104 bytes
LevelUp_T :: struct {
	current_lvl:        i32, // default -1
	increase_lvl:       i32, // default -1
	ticks_active:       u32,
	processed_on_tick:  u32,
	ticks_to_live:      u32,
	stat_ups:           [dynamic]StatUp_T, // DynamicArrayT<StatUp_t> (40B)
	title_animate_pos:  SDL_Rect,
	anim_title_fade:    f64,
	fadeout:            f64,
	expired:            bool,
	title_finish_anim:  bool,
}
#assert(size_of(LevelUp_T) == 104)

// struct LevelUpAnimation_t::LevelUp_t::StatUp_t — 168 bytes
StatUp_T :: struct {
	which_stat:          i32, // default -1
	current_stat:        i32, // default -1
	increase_stat:       i32, // default -1
	// pad 4
	animate_x:           f64,
	animate_y:           f64,
	animate_w:           f64,
	animate_h:           f64,
	animate_setpoint_x:  i32,
	animate_setpoint_y:  i32,
	animate_setpoint_w:  i32,
	animate_setpoint_h:  i32,
	animate_start_x:     i32,
	animate_start_y:     i32,
	animate_start_w:     i32,
	animate_start_h:     i32,
	anim_angle:          f64,
	anim_current_stat:   f64,
	anim_increase_stat:  f64,
	base_x:              i32,
	base_y:              i32,
	ticks_active:        u32,
	processed_on_tick:   u32,
	pos:                 SDL_Rect,
	notification_target_position: SDL_Rect,
	init:                bool,
	// pad 3
	notification_state:  StatusEffect_NotificationStates,
	notification_state_init: StatusEffect_NotificationStates,
}
#assert(size_of(StatUp_T) == 168)

// struct SkillUpAnimation_t — 48 bytes
SkillUpAnimation_T :: struct {
	anim_frame_fade_in: f64, // default 1.0
	skill_ups:          [dynamic]SkillUp_T, // DynamicArrayT<SkillUp_t> (40B)
}
#assert(size_of(SkillUpAnimation_T) == 48)

// struct SkillUpAnimation_t::SkillUp_t — 200 bytes
SkillUp_T :: struct {
	which_skill:        i32, // default -1
	current_skill:      i32, // default -1
	increase_skill:     i32, // default -1
	spell_id:           i32,
	// pad 4
	animate_x:          f64,
	animate_y:          f64,
	animate_w:          f64,
	animate_h:          f64,
	animate_setpoint_x: i32,
	animate_setpoint_y: i32,
	animate_setpoint_w: i32,
	animate_setpoint_h: i32,
	animate_start_x:    i32,
	animate_start_y:    i32,
	animate_start_w:    i32,
	animate_start_h:    i32,
	anim_angle:         f64,
	anim_current_stat:  f64,
	anim_increase_stat: f64,
	anim_background:    f64,
	base_x:             i32,
	base_y:             i32,
	ticks_active:       u32,
	processed_on_tick:  u32,
	pre_delay_ticks:    u32, // default 5
	ticks_to_live:      u32,
	pos:                SDL_Rect,
	notification_target_position: SDL_Rect,
	init:               bool,
	// pad 3
	notification_state: StatusEffect_NotificationStates,
	notification_state_init: StatusEffect_NotificationStates,
	fadeout:            f64,
	expired:            bool,
	is_spell:           bool,
	// pad 6
}
#assert(size_of(SkillUp_T) == 200)
