// player.odin — Odin mirrors of player.hpp.
package main

import "containers"

// struct PlayerSettings_t (player.hpp:53, file scope) — 80 bytes.
// NOTE: distinct from the nested Player::PlayerSettings_t (16B, mirrored below
// as Player_Settings_T). This is the type of the global playerSettings[MAXPLAYERS].
PlayerSettings_T :: struct {
	player:                         i32,
	shootmode_crosshair:            i32,
	shootmode_crosshair_opacity:    i32,
	mousespeed:                     f64, // real_t
	mkb_world_tooltips_enabled:     bool,
	gamepad_facehotbar:             bool,
	hotbar_numkey_quick_add:        bool,
	hotbar_numkey_change_slot:      bool,
	reversemouse:                   bool,
	smoothmouse:                    bool,
	gamepad_rightx_sensitivity:     f64, // real_t
	gamepad_righty_sensitivity:     f64, // real_t
	gamepad_rightx_invert:          bool,
	gamepad_righty_invert:          bool,
	quick_turn_speed:               f32,
	quick_turn_speed_mkb:           f32,
	mouse_event_limit_mkb:          i32,
	spell_quickcast_mkb:            bool,
	spell_quickcast_controller:     bool,
	left_stick_deadzone:            i32, // Sint32
	right_stick_deadzone:           i32, // Sint32
}
#assert(size_of(PlayerSettings_T) == 80)

// ---------------------------------------------------------------------------
// Enums (4-byte i32 unless noted)
// ---------------------------------------------------------------------------

// enum SplitScreenTypes
Split_Screen_Types :: enum i32 {
	SPLITSCREEN_DEFAULT,
	SPLITSCREEN_VERTICAL,
}

// enum Player::PanelJustify_t
Panel_Justify_T :: enum i32 {
	PANEL_JUSTIFY_LEFT,
	PANEL_JUSTIFY_RIGHT,
}

// enum Player::GUI_t::GUIModules
GUI_Modules :: enum i32 {
	MODULE_NONE,
	MODULE_INVENTORY,
	MODULE_SHOP,
	MODULE_CHEST,
	MODULE_REMOVECURSE,
	MODULE_IDENTIFY,
	MODULE_TINKERING,
	MODULE_ALCHEMY,
	MODULE_FEATHER,
	MODULE_FOLLOWERMENU,
	MODULE_CHARACTERSHEET,
	MODULE_SKILLS_LIST,
	MODULE_BOOK_VIEW,
	MODULE_HOTBAR,
	MODULE_SPELLS,
	MODULE_STATUS_EFFECTS,
	MODULE_LOG,
	MODULE_MAP,
	MODULE_SIGN_VIEW,
	MODULE_ITEMEFFECTGUI,
	MODULE_PORTRAIT,
	MODULE_ASSISTSHRINE,
	MODULE_MAILBOX,
}

// enum Player::Inventory_t::GamepadDropdownTypes
Gamepad_Dropdown_Types :: enum i32 {
	GAMEPAD_DROPDOWN_DISABLE,
	GAMEPAD_DROPDOWN_FULL,
	GAMEPAD_DROPDOWN_COMPACT,
}

// enum Player::Inventory_t::ItemNotifyHoverStates
Item_Notify_Hover_States :: enum i32 {
	NOTIFY_ITEM_WAITING_TO_HOVER,
	NOTIFY_ITEM_HOVERED,
	NOTIFY_ITEM_REMOVE,
}

// enum Player::CharacterSheet_t::SheetDisplay
Sheet_Display :: enum i32 {
	CHARSHEET_DISPLAY_NORMAL,
	CHARSHEET_DISPLAY_COMPACT,
}

// enum Player::CharacterSheet_t::SheetElements
Sheet_Elements :: enum i32 {
	SHEET_UNSELECTED,
	SHEET_OPEN_LOG,
	SHEET_OPEN_MAP,
	SHEET_TIMER,
	SHEET_GOLD,
	SHEET_DUNGEON_FLOOR,
	SHEET_CHAR_CLASS,
	SHEET_CHAR_RACE_SEX,
	SHEET_SKILL_LIST,
	SHEET_STR,
	SHEET_DEX,
	SHEET_CON,
	SHEET_INT,
	SHEET_PER,
	SHEET_CHR,
	SHEET_ATK,
	SHEET_AC,
	SHEET_POW,
	SHEET_RES,
	SHEET_RGN,
	SHEET_RGN_MP,
	SHEET_WGT,
	SHEET_ENUM_END,
}

// enum Player::HUD_t::AnimateStates
Animate_States :: enum i32 {
	ANIMATE_NONE,
	ANIMATE_MOVING,
	ANIMATE_LEVELUP_RISING,
	ANIMATE_LEVELUP_FALLING,
}

// enum Player::HUD_t::AnimateFlashEffects_t
Animate_Flash_Effects_T :: enum i32 {
	FLASH_ON_DAMAGE,
	FLASH_ON_RECOVERY,
}

// enum Player::HUD_t::XPCycleInfo
XP_Cycle_Info :: enum i32 {
	CYCLE_NONE,
	CYCLE_LVL,
	CYCLE_XP,
}

// enum Player::HUD_t::CompactLayoutModes
Compact_Layout_Modes :: enum i32 {
	COMPACT_LAYOUT_INVENTORY,
	COMPACT_LAYOUT_CHARSHEET,
}

// enum Player::Ghost_t::GhostSpells_t
Ghost_Spells_T :: enum i32 {
	GHOST_SPELL_NONE,
	GHOST_SPELL_TELEPORT,
	GHOST_SPELL_BOLT,
	GHOST_SPELL_QUACK,
	GHOST_SPELL_POST_CASTING,
}

// enum Player::MessageZone_t::ChatAlignment_t
Chat_Alignment_T :: enum i32 {
	ALIGN_CENTER_BOTTOM,
	ALIGN_LEFT_BOTTOM,
	ALIGN_LEFT_TOP,
}

// enum Player::WorldUI_t::TooltipView
Tooltip_View :: enum i32 {
	TOOLTIP_VIEW_FREE,
	TOOLTIP_VIEW_LOCKED,
	TOOLTIP_VIEW_RESCAN,
}

// enum Player::WorldUI_t::WorldTooltipDialogue_t::DialogueType_t
Dialogue_Type_T :: enum i32 {
	DIALOGUE_NONE,
	DIALOGUE_NPC,
	DIALOGUE_GRAVE,
	DIALOGUE_SIGNPOST,
	DIALOGUE_FOLLOWER_CMD,
	DIALOGUE_BROADCAST,
	DIALOGUE_ATTACK,
}

// enum Player::PaperDoll_t::PaperDollSlotType
Paper_Doll_Slot_Type :: enum i32 {
	SLOT_GLASSES,
	SLOT_CLOAK,
	SLOT_AMULET,
	SLOT_RING,
	SLOT_OFFHAND,
	SLOT_HELM,
	SLOT_BREASTPLATE,
	SLOT_GLOVES,
	SLOT_BOOTS,
	SLOT_WEAPON,
	SLOT_MAX,
}

// enum Player::Hotbar_t::FaceMenuGroup
Face_Menu_Group :: enum i32 {
	GROUP_NONE,
	GROUP_LEFT,
	GROUP_MIDDLE,
	GROUP_RIGHT,
}

// ---------------------------------------------------------------------------
// GameController nested structs
// ---------------------------------------------------------------------------

// struct GameController::Haptic_t::HapticEffect — 16 bytes
Haptic_Effect :: struct {
	type:             u16, // SDL_HAPTIC_LEFTRIGHT
	length:           u32,
	large_magnitude:  u16,
	small_magnitude:  u16,
	left_right_balance: i32,
}
#assert(size_of(Haptic_Effect) == 16)

// class GameController::Haptic_t — 80 bytes
// (haptics member of GameController)
Haptic_T :: struct {
	haptic_effect_id:  i32,
	haptic_effect:     Haptic_Effect,
	haptic_tick:       u32,
	oscillator_tick:   u32,
	active_rumbles:    [dynamic]Uint32_Rumble_Pair, // vector<pair<Uint32,Rumble>>
	vibration_enabled: bool,
}
#assert(size_of(Haptic_T) == 80)

// struct GameController::Haptic_t::Rumble — 40 bytes
Rumble :: struct {
	start_tick:     u32,
	small_magnitude: u16,
	large_magnitude: u16,
	start_time:     u32,
	length:         u32,
	custom_effect:  f64,
	pattern:        i32, // RumblePattern enum
	entity_uid:     u32,
	is_playing:     bool,
}
#assert(size_of(Rumble) == 40)

// enum GameController::Haptic_t::RumblePattern
Rumble_Pattern :: enum i32 {
	RUMBLE_NORMAL,
	RUMBLE_BOULDER,
	RUMBLE_BOULDER_BOUNCE,
	RUMBLE_BOULDER_ROLLING,
	RUMBLE_DEATH,
	RUMBLE_TMP,
	RUMBLE_SPELL,
}

// enum GameController::DpadDirection
Dpad_Direction :: enum i32 {
	INVALID = -2,
	CENTERED = -1,
	DOWN,
	DOWNLEFT,
	LEFT,
	UPLEFT,
	UP,
	UPRIGHT,
	RIGHT,
	DOWNRIGHT,
}

// enum GameController::RadialSelection
Radial_Selection :: enum i32 {
	RADIAL_INVALID = -2,
	RADIAL_CENTERED = -1,
	RADIAL_MAX = 16,
}

// enum GameController::Binding_t::Bindtype_t
Bindtype_T :: enum i32 {
	INVALID,
	KEYBOARD,
	CONTROLLER_AXIS,
	CONTROLLER_BUTTON,
	MOUSE_BUTTON,
	JOYSTICK_AXIS,
	JOYSTICK_BUTTON,
	JOYSTICK_HAT,
	VIRTUAL_DPAD,
	RADIAL_SELECTION,
	NUM,
}

// struct GameController::Binding_t — 44 bytes
Binding_T :: struct {
	analog:                 f32,
	deadzone:               f32,
	binary:                 bool,
	consumed:               bool,
	button_held_ticks:      u32,
	button_held:            bool,
	binary_release:         bool,
	binary_release_consumed: bool,
	type:                   i32, // Bindtype_t enum
	pad_axis:               i32, // SDL_GameControllerAxis
	pad_button:             i32, // SDL_GameControllerButton
	pad_virtual_dpad:       i32, // DpadDirection enum
	pad_radial_selection:   i32, // RadialSelection enum
	pad_axis_negative:      bool,
}
#assert(size_of(Binding_T) == 44)

// class GameController — 1448 bytes
Game_Controller :: struct {
	sdl_device:          rawptr, // SDL_GameController*
	sdl_haptic:          rawptr, // SDL_Haptic*
	id:                  i32,
	name:                containers.DynamicString,
	haptics:             Haptic_T,
	buttons:             [21]Binding_T, // NUM_JOY_STATUS
	axis:                [6]Binding_T,  // NUM_JOY_AXIS_STATUS
	virtual_dpad:        Binding_T,
	radial_selection:    Binding_T,
	left_stick_deadzone_type:  i32, // DeadZoneType enum
	right_stick_deadzone_type: i32, // DeadZoneType enum
	old_float_right_x:   f64,
	old_float_right_y:   f64,
	old_axis_right_x:    i32,
	old_axis_right_y:    i32,
	x_force_max_forward_threshold:  f32,
	x_force_max_backward_threshold: f32,
	y_force_max_strafe_threshold:   f32,
}
#assert(size_of(Game_Controller) == 1448)

// struct Inputs::VirtualMouse — 96 bytes
Virtual_Mouse :: struct {
	xrel:                  i32,
	yrel:                  i32,
	ox:                    i32,
	oy:                    i32,
	x:                     i32,
	y:                     i32,
	floatxrel:             f64,
	floatyrel:             f64,
	floatx:                f64,
	floaty:                f64,
	floatox:               f64,
	floatoy:               f64,
	mouse_left_held_ticks: u32,
	mouse_left_held:       bool,
	mouse_right_held_ticks: u32,
	mouse_right_held:      bool,
	draw_cursor:           bool,
	moved:                 bool,
	last_movement_from_controller: bool,
	mouse_animation_percent: f64,
}
#assert(size_of(Virtual_Mouse) == 96)

// struct Inputs::UIStatus — 40 bytes
UI_Status :: struct {
	selected_item_from_hotbar: i32,
	selected_item_from_chest:  u32,
	selected_item:             ^Item,
	toggleclick:               bool,
	item_menu_open:            bool,
	item_menu_from_hotbar:     bool,
	item_menu_offset_detection_y: i32,
	item_menu_x:               i32,
	item_menu_y:               i32,
	item_menu_selected:        i32,
	item_menu_item:            u32,
}
#assert(size_of(UI_Status) == 40)

// class Inputs — 568 bytes
Inputs_Struct :: struct {
	player_controller_ids: [4]i32, // MAXPLAYERS
	player_using_keyboard_control: i32,
	vmouse:                [4]Virtual_Mouse, // MAXPLAYERS
	ui_status:             [4]UI_Status,     // MAXPLAYERS
}
#assert(size_of(Inputs_Struct) == 568)

// struct MonsterStringPair_t — 24 bytes
Monster_String_Pair_T :: struct {
	first:  i32, // Monster
	second: containers.DynamicString,
}
#assert(size_of(Monster_String_Pair_T) == 24)

// --- local pair mirrors (player.hpp raw DynamicArray members) ---
// vector<pair<Uint32,Rumble>> (Haptic_t::activeRumbles) — 48B
Uint32_Rumble_Pair :: struct {
	first:  u32,
	second: Rumble,
}
#assert(size_of(Uint32_Rumble_Pair) == 48)

// vector<pair<Uint32,FollowerBar_t>> (HUD_t::followerBars/playerBars) — 288B
Uint32_FollowerBar_Pair :: struct {
	first:  u32,
	second: Follower_Bar_T,
}
#assert(size_of(Uint32_FollowerBar_Pair) == 288)

// vector<pair<Entity*,real_t>> (WorldUI_t::tooltipsInRange) — 16B
EntityF64_Pair :: struct {
	first:  ^Entity,
	second: f64,
}
#assert(size_of(EntityF64_Pair) == 16)

// vector<pair<int,Uint32>> (PlayerMechanics_t::pendingDucks) — 8B
Int_U32_Pair :: struct {
	first:  i32,
	second: u32,
}
#assert(size_of(Int_U32_Pair) == 8)

// struct Player::GUIDropdown_t::DropDown_t — 88 bytes
Drop_Down_T :: struct {
	title:         containers.DynamicString,
	internal_name: containers.DynamicString,
	align_right:   bool,
	module:        i32,
	default_option: i32,
	options:       [dynamic]containers.DropdownOption_t, // DynamicArrayOption
}
#assert(size_of(Drop_Down_T) == 88)

// struct Player::GUIDropdown_t — 80 bytes
// (first member is a Player& reference -> rawptr)
GUI_Dropdown_T :: struct {
	player:                      rawptr, // Player&
	drop_down_x:                 i32,
	drop_down_y:                 i32,
	drop_down_option_selected:   i32,
	drop_down_item:              u32,
	b_open:                      bool,
	current_name:                containers.DynamicString,
	dropdown_block_click_frame:  rawptr, // Frame*
	dropdown_frame:              rawptr, // Frame*
	drop_down_toggle_click:      bool,
	dropdown_link_to_module:     i32,
	b_closed_this_tick:          bool,
}
#assert(size_of(GUI_Dropdown_T) == 80)

// class Player::GUI_t — 104 bytes
GUI_T :: struct {
	player:                rawptr, // Player&
	active_module:         i32, // GUIModules enum
	previous_module:       i32, // GUIModules enum
	hovering_button_module: i32, // GUIModules enum
	dropdown_menu:         GUI_Dropdown_T,
}
#assert(size_of(GUI_T) == 104)

// struct Player::Inventory_t::Cursor_t — 56 bytes
Inventory_Cursor_T :: struct {
	animate_x:          f64,
	animate_y:          f64,
	animate_setpoint_x: i32,
	animate_setpoint_y: i32,
	animate_start_x:    i32,
	animate_start_y:    i32,
	last_update_tick:   u32,
	cursor_to_slot_offset: i32,
	queued_module:      i32, // GUIModules enum
	queued_frame_to_warp_to: rawptr, // Frame*
}
#assert(size_of(Inventory_Cursor_T) == 56)

// struct Player::Inventory_t::SelectedItemAnimate_t — 16 bytes
Selected_Item_Animate_T :: struct {
	animate_x: f64,
	animate_y: f64,
}
#assert(size_of(Selected_Item_Animate_T) == 16)

// struct Player::Inventory_t::SpellPanel_t — 72 bytes
Spell_Panel_T :: struct {
	player:                rawptr, // Player&
	panel_justify:         i32, // PanelJustify_t enum
	animx:                 f64,
	scroll_percent:        f64,
	scroll_inertia:        f64,
	scroll_setpoint:       i32,
	scroll_animate_x:      f64,
	is_interactable:       bool,
	b_open:                bool,
	b_first_time_snap_cursor: bool,
	current_scroll_row:    i32,
	spell_filter_by_skill: i32,
	k_num_spells_to_display_vertical: i32,
}
#assert(size_of(Spell_Panel_T) == 72)

// struct Player::Inventory_t::ChestGUI_t — 80 bytes
Chest_GUI_T :: struct {
	player:                rawptr, // Player&
	panel_justify:         i32, // PanelJustify_t enum
	animx:                 f64,
	scroll_percent:        f64,
	scroll_inertia:        f64,
	scroll_setpoint:       i32,
	scroll_animate_x:      f64,
	is_interactable:       bool,
	b_open:                bool,
	b_first_time_snap_cursor: bool,
	current_scroll_row:    i32,
	void_chest:            bool,
	k_num_items_to_display_vertical: i32,
	selected_chest_slot_x: i32,
	selected_chest_slot_y: i32,
}
#assert(size_of(Chest_GUI_T) == 80)

// struct Player::Inventory_t::ItemTooltipDisplay_t — 192 bytes
Item_Tooltip_Display_T :: struct {
	type:                     u32,
	status:                   i32,
	beatitude:                i32,
	count:                    i32,
	appearance:               u32,
	identified:               bool,
	uid:                      u32,
	was_appraisal_target:     bool,
	playernum:                i32,
	player_lvl:               i32,
	player_exp:               i32,
	player_str:               i32,
	player_dex:               i32,
	player_con:               i32,
	player_int:               i32,
	player_per:               i32,
	player_chr:               i32,
	spell_cost:               i32,
	opacity_setpoint:         i32,
	opacity_animate:          f64,
	title_only_opacity_setpoint: i32,
	title_only_opacity_animate: f64,
	expand_animate:           f64,
	expand_setpoint:          i32,
	expand_current:           f64,
	expanded:                 bool,
	frame_tooltip_scroll_anim: f64,
	frame_tooltip_scroll_setpoint: f64,
	scrolled_to_max:          i32,
	frame_tooltip_scroll_prev_setpoint: f64,
	scrollable:               bool,
	tooltip_description_height: i32,
	tooltip_attribute_height: i32,
	tooltip_width:            i32,
	tooltip_height:           i32,
	displaying_short_form_tooltip: bool,
	displaying_title_only_tooltip: bool,
}
#assert(size_of(Item_Tooltip_Display_T) == 192)

// struct Player::Inventory_t::AppraisalBreakpoint_t — 12 bytes
Appraisal_Breakpoint_T :: struct {
	skill_lvl:         i32,
	gold_value_limit:  i32,
	fast_time_gold:    i32,
}
#assert(size_of(Appraisal_Breakpoint_T) == 12)

// class Player::Inventory_t::Appraisal_t — 128 bytes
Appraisal_T :: struct {
	player:                     rawptr, // Player&
	timer:                      i32,
	timermax:                   i32,
	current_item:               u32,
	appraisal_progression_items: map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	old_item:                   u32,
	manual_appraised_item:      u32,
	anim_appraisal:             f64,
	anim_start_tick:            u32,
	item_notify_updated_this_tick: u32,
	item_notify_anim_state:     i32,
	spell_learn_anim:           f64,
	items_to_notify:            map[[4]byte]Item_Notify_Hover_States, // DynamicMapI32T<ItemNotifyHoverStates> (i32-keyed)
}
#assert(size_of(Appraisal_T) == 128)

// class Player::Inventory_t — 1048 bytes
Inventory_T :: struct {
	sizex:                   i32,
	sizey:                   i32,
	starty:                  i32, // const int = 10
	player:                  rawptr, // Player&
	selected_slot_x:         i32,
	selected_slot_y:         i32,
	selected_spell_x:        i32,
	selected_spell_y:        i32,
	frame:                   rawptr, // Frame*
	tooltip_container_frame: rawptr, // Frame*
	tooltip_frame:           rawptr, // Frame*
	title_only_tooltip_frame: rawptr, // Frame*
	interact_frame:          rawptr, // Frame*
	interact_block_click_frame: rawptr, // Frame*
	tooltip_prompt_frame:    rawptr, // Frame*
	selected_item_cursor_frame: rawptr, // Frame*
	spell_frame:             rawptr, // Frame*
	chest_frame:             rawptr, // Frame*
	slot_frames:             map[[4]byte]rawptr, // DynamicMapI32T<Frame*> (i32-keyed, non-owning)
	spell_slot_frames:       map[[4]byte]rawptr, // DynamicMapI32T<Frame*> (i32-keyed, non-owning)
	chest_slot_frames:       map[[4]byte]rawptr, // DynamicMapI32T<Frame*> (i32-keyed, non-owning)
	b_compact_view:          bool,
	slide_out_percent:       f64,
	b_first_time_snap_cursor: bool,
	is_interactable:         bool,
	tooltip_delay_tick:      u32,
	anim_paper_doll_hide:    f64,
	misc_tooltip_opacity_setpoint: i32,
	misc_tooltip_opacity_animate: f64,
	misc_tooltip_deselected_tick: u32,
	misc_tooltip_frame:      rawptr, // Frame*
	inventory_panel_justify: i32, // PanelJustify_t enum
	paper_doll_panel_justify: i32, // PanelJustify_t enum
	use_item_dropdown_on_gamepad: i32, // GamepadDropdownTypes enum
	cursor:                  Inventory_Cursor_T,
	selected_item_animate:   Selected_Item_Animate_T,
	spell_panel:             Spell_Panel_T,
	chest_gui:               Chest_GUI_T,
	item_tooltip_display:    Item_Tooltip_Display_T,
	compendium_item_tooltip_display: Item_Tooltip_Display_T,
	default_inventory_sizex: i32,
	default_inventory_sizey: i32,
	appraisal:               Appraisal_T,
	b_new_inventory_layout:  bool,
}
#assert(size_of(Inventory_T) == 1048)

// struct Player::ShopGUI_t — 192 bytes
Shop_GUI_T :: struct {
	player:                 rawptr, // Player&
	shop_frame:             rawptr, // Frame*
	panel_justify:          i32, // PanelJustify_t enum
	buyback_view:           bool,
	animx:                  f64,
	is_interactable:        bool,
	b_open:                 bool,
	b_first_time_snap_cursor: bool,
	chat_ticks:             u32,
	chat_string_length:     i64, // size_t
	chat_str_full:          containers.DynamicString,
	item_price:             i32,
	item_unknown_prevent_purchase: bool,
	item_desc:              containers.DynamicString,
	item_requires_title_reflow: bool,
	player_current_gold:    i32,
	player_change_gold:     i32,
	anim_gold:              f64,
	anim_gold_start_ticks:  u32,
	anim_tooltip:           f64,
	anim_tooltip_ticks:     u32,
	anim_no_deal:           f64,
	anim_no_deal_ticks:     u32,
	last_tooltip_module:    i32,
	selected_shop_slot_x:   i32,
	selected_shop_slot_y:   i32,
	shop_slot_frames:       map[[4]byte]rawptr, // DynamicMapI32T<Frame*> (i32-keyed, non-owning)
}
#assert(size_of(Shop_GUI_T) == 192)

// class Player::BookGUI_t — 72 bytes
Book_GUI_T :: struct {
	player:                  rawptr, // Player&
	book_fade_in_animation_y: f64,
	book_frame:              rawptr, // Frame*
	offsetx:                 i32,
	offsety:                 i32,
	b_book_open:             bool,
	open_book_item:          ^Item,
	open_book_name:          containers.DynamicString,
	current_book_page:       i32,
}
#assert(size_of(Book_GUI_T) == 72)

// class Player::SignGUI_t — 80 bytes
Sign_GUI_T :: struct {
	player:                  rawptr, // Player&
	sign_fade_in_animation_y: f64,
	sign_anim_video:         f64,
	sign_world_coord_x:      f64,
	sign_world_coord_y:      f64,
	sign_frame:              rawptr, // Frame*
	b_sign_open:             bool,
	sign_name:               containers.DynamicString,
	current_sign_page:       i32,
	sign_uid:                u32,
}
#assert(size_of(Sign_GUI_T) == 80)

// class Player::CharacterSheet_t — 72 bytes
Character_Sheet_T :: struct {
	player:                  rawptr, // Player&
	panel_justify:           i32, // PanelJustify_t enum
	lock_right_sidebar:      bool,
	proficiencies_page:      i32,
	attributespage:          i32,
	show_game_timer_always:  bool,
	is_interactable:         bool,
	tooltip_opacity_setpoint: i32,
	tooltip_opacity_animate: f64,
	tooltip_deselected_tick: u32,
	sheet_display_type:      i32, // SheetDisplay enum
	sheet_frame:             rawptr, // Frame*
	selected_element:        i32, // SheetElements enum
	queued_element:          i32, // SheetElements enum
	cached_element_tooltip:  i32, // SheetElements enum
}
#assert(size_of(Character_Sheet_T) == 72)

// struct Player::SkillSheet_t::SkillEffect_t — 160 bytes
Skill_Effect_T :: struct {
	tag:                        containers.DynamicString,
	title:                      containers.DynamicString,
	title_short:                containers.DynamicString,
	raw_value:                  containers.DynamicString,
	value:                      containers.DynamicString,
	value_custom_width_offset:  i32,
	b_allow_auto_resize_value:  bool,
	b_allow_realtime_update:    bool,
	marquee:                    [4]f64, // real_t[4]
	marquee_ticks:              [4]u32,
	marquee_completed:          [4]bool,
	effect_updated_at_skill_level: i32,
	effect_updated_at_base_skill_level: i32,
	effect_updated_at_monster_type: i32,
	cached_width:               i32,
}
#assert(size_of(Skill_Effect_T) == 160)

// struct Player::SkillSheet_t::SkillEntry_t — 208 bytes
Skill_Entry_T :: struct {
	skill_name:               containers.DynamicString,
	skill_short_name:         containers.DynamicString,
	skill_id:                 i32,
	skill_icon_path:          containers.DynamicString,
	skill_icon_path_legend:   containers.DynamicString,
	skill_icon_path_32px:     containers.DynamicString,
	skill_icon_path_legend_32px: containers.DynamicString,
	stat_icon_path:           containers.DynamicString,
	description:              containers.DynamicString,
	legendary_description:    containers.DynamicString,
	skill_sfx:                i32,
	effect_start_offset_x:    i32,
	effect_background_offset_x: i32,
	effect_background_width:  i32,
	effects:                  [dynamic]Skill_Effect_T, // DynamicArrayT<SkillEffect_t>
}
#assert(size_of(Skill_Entry_T) == 208)

// struct Player::SkillSheet_t::SkillSheetData_t — 384 bytes
Skill_Sheet_Data_T :: struct {
	default_text_color:            u32,
	novice_text_color:             u32,
	expert_text_color:             u32,
	legend_text_color:             u32,
	skill_entries:                 [dynamic]Skill_Entry_T, // DynamicArrayT<SkillEntry_t>
	icon_bg_path_default:          containers.DynamicString,
	icon_bg_path_novice:           containers.DynamicString,
	icon_bg_path_expert:           containers.DynamicString,
	icon_bg_path_legend:           containers.DynamicString,
	icon_bg_selected_path_default: containers.DynamicString,
	icon_bg_selected_path_novice:  containers.DynamicString,
	icon_bg_selected_path_expert:  containers.DynamicString,
	icon_bg_selected_path_legend:  containers.DynamicString,
	highlight_skill_img:           containers.DynamicString,
	select_skill_img:              containers.DynamicString,
	highlight_skill_img_right:     containers.DynamicString,
	select_skill_img_right:        containers.DynamicString,
	potion_names_to_filter:        [dynamic]containers.DynamicString, // DynamicArrayStr
	leadership_ally_table_base:    map[[4]byte][dynamic]i32, // DynamicMapI32T<DynamicArrayS32> (i32-keyed)
	leadership_ally_table_legendary: map[[4]byte][dynamic]i32, // DynamicMapI32T<DynamicArrayS32> (i32-keyed)
	leadership_ally_table_special_recruitment: map[[4]byte][dynamic]Monster_String_Pair_T, // DynamicMapI32T<DynamicArrayT<MonsterStringPair_t>> (i32-keyed)
}
#assert(size_of(Skill_Sheet_Data_T) == 384)

// class Player::SkillSheet_t — 88 bytes
Skill_Sheet_T :: struct {
	player:                    rawptr, // Player&
	skill_frame:               rawptr, // Frame*
	selected_skill:            i32,
	highlighted_skill:         i32,
	skills_fade_in_animation_y: f64,
	b_skill_sheet_open:        bool,
	open_tick:                 u32,
	b_skill_sheet_entry_loaded: bool,
	scroll_percent:            f64,
	scroll_inertia:            f64,
	skill_slide_direction:     i32,
	skill_slide_amount:        f64,
	b_use_compact_skills_view: bool,
	b_slide_windows_only:      bool,
}
#assert(size_of(Skill_Sheet_T) == 88)

// struct Player::HUD_t::Cursor_t — 72 bytes
HUD_Cursor_T :: struct {
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
	last_update_tick:   u32,
	cursor_to_slot_offset: i32,
}
#assert(size_of(HUD_Cursor_T) == 72)

// struct Player::HUD_t::Bar_t — 88 bytes
Bar_T :: struct {
	animate_value:             f64,
	animate_value2:            f64,
	animate_previous_setpoint: f64,
	animate_setpoint:          i32,
	animate_ticks:             u32,
	animate_state:             i32, // AnimateStates enum
	xp_levelups:               u32,
	max_value:                 f64,
	fade_in:                   f64,
	fade_out:                  f64,
	width_multiplier:          f64,
	flash_ticks:               u32,
	flash_processed_on_tick:   u32,
	flash_anim_state:          i32,
	flash_type:                i32, // AnimateFlashEffects_t enum
}
#assert(size_of(Bar_T) == 88)

// struct Player::HUD_t::XPInfo_t — 32 bytes
XP_Info_T :: struct {
	cycle_status:          i32, // XPCycleInfo enum
	fade:                  f64,
	cycle_ticks:           u32,
	cycle_processed_on_tick: u32,
	fade_in:               bool,
}
#assert(size_of(XP_Info_T) == 32)

// struct Player::HUD_t::InteractPrompt_t — 24 bytes
Interact_Prompt_T :: struct {
	prompt_anim:          f64,
	active_ticks:         u32,
	processed_on_tick:    u32,
	cycle_anim:           f64,
}
#assert(size_of(Interact_Prompt_T) == 24)

// struct Player::HUD_t::FollowerBar_t — 280 bytes
Follower_Bar_T :: struct {
	hp_bar:               Bar_T,
	mp_bar:               Bar_T,
	animx:                f64,
	animy:                f64,
	expired:              bool,
	expired_ticks:        u32,
	anim_fade:            f64,
	anim_fade_scroll:     f64,
	anim_fade_scroll_dummy: f64,
	b_init:               bool,
	name:                 containers.DynamicString,
	custom_portrait_path: containers.DynamicString,
	level:                i32,
	model:                i32,
	monster_type:         i32,
	selected:             bool,
	dummy:                bool,
}
#assert(size_of(Follower_Bar_T) == 280)

// struct Player::HUD_t::FollowerDisplay_t — 64 bytes
Follower_Display_T :: struct {
	scroll_percent:      f64,
	scroll_inertia:      f64,
	scroll_setpoint:     i32,
	current_scroll_row:  i32,
	scroll_animate_x:    f64,
	last_uid_selected:   u32,
	anim_selected:       f64,
	scroll_ticks:        u32,
	is_interactable:     bool,
	b_compact:           bool,
	b_half_width_bars:   bool,
	b_cycle_next_disabled: bool,
	b_command_npc_disabled: bool,
	b_open_follower_menu_disabled: bool,
}
#assert(size_of(Follower_Display_T) == 64)

// class Player::HUD_t — 1040 bytes
HUD_T :: struct {
	player:                      rawptr, // Player&
	controller_frame:            rawptr, // Frame*
	hud_frame:                   rawptr, // Frame*
	xp_frame:                    rawptr, // Frame*
	levelup_frame:               rawptr, // Frame*
	skillup_frame:               rawptr, // Frame*
	hp_frame:                    rawptr, // Frame*
	mp_frame:                    rawptr, // Frame*
	minimap_frame:               rawptr, // Frame*
	game_timer_frame:            rawptr, // Frame*
	ally_status_frame:           rawptr, // Frame*
	minotaur_frame:              rawptr, // Frame*
	minotaur_shared_display:     rawptr, // Frame*
	minotaur_display:            rawptr, // Frame*
	map_prompt_frame:            rawptr, // Frame*
	ally_follower_frame:         rawptr, // Frame*
	ally_follower_title_frame:   rawptr, // Frame*
	ally_follower_glyph_frame:   rawptr, // Frame*
	ally_player_frame:           rawptr, // Frame*
	callout_prompt_frame:        rawptr, // Frame*
	voice_prompt_frame:          rawptr, // Frame*
	enemy_bar_frame:             rawptr, // Frame*
	enemy_bar_frame_hud:         rawptr, // Frame*
	action_prompts_frame:        rawptr, // Frame*
	world_tooltip_frame:         rawptr, // Frame*
	status_effect_focused_window: rawptr, // Frame*
	ui_nav_frame:                rawptr, // Frame*
	cursor_frame:                rawptr, // Frame*
	hud_damage_text_velocity_x:  f64,
	hud_damage_text_velocity_y:  f64,
	anim_hide_xp:                f64,
	anim_hide_action_prompts:    f64,
	weapon:                      ^Entity,
	arm:                         ^Entity,
	magic_left_hand:             ^Entity,
	magic_right_hand:            ^Entity,
	magic_rangefinder:           ^Entity,
	weapon_switch:               bool,
	shield_switch:               bool,
	throw_gimp_timer:            i32,
	pickaxe_gimp_timer:          i32,
	swap_weapon_gimp_timer:      i32,
	bow_gimp_timer:              i32,
	bow_fire:                    bool,
	bow_is_being_drawn:          bool,
	bow_start_drawing_tick:      u32,
	bow_draw_base_ticks:         u32,
	bow_drawing_sound_channel:   rawptr, // OPENAL_SOUND*
	bow_drawing_sound_playing:   bool, // ALboolean (1 byte)
	cursor:                      HUD_Cursor_T,
	xp_info:                     XP_Info_T,
	interact_prompt:             Interact_Prompt_T,
	xp_bar:                      Bar_T,
	hp_bar:                      Bar_T,
	mp_bar:                      Bar_T,
	enemy_bar:                   Bar_T,
	follower_display:            Follower_Display_T,
	follower_bars:               [dynamic]Uint32_FollowerBar_Pair, // vector<pair<Uint32,FollowerBar_t>>
	player_bars:                 [dynamic]Uint32_FollowerBar_Pair, // vector<pair<Uint32,FollowerBar_t>>
	compact_layout_mode:         i32, // CompactLayoutModes enum
	b_show_ui_navigation:        bool,
	status_fx_focused_window_active: bool,
	b_show_action_prompts:       bool,
	b_short_hpmp_for_action_bars: bool,
	b_open_callouts_menu_disabled: bool,
	xp_frame_width:              i32,
	xp_frame_start_y:            i32,
	xp_frame_height:             i32,
	hpmp_frame_width:            i32,
	hpmp_frame_start_x:          i32,
	hpmp_frame_start_y:          i32,
	hpmp_frame_height:           i32,
	enemybar_frame_width:        i32, // const int
	enemybar_bar_width:          i32, // const int
	enemybar_frame_start_y:      i32, // const int
	enemybar_frame_height:       i32, // const int
	anim_dead_prompt:            f64,
	anim_dead_prompt_display:    bool,
	offset_hud_above_hotbar_height: i32,
}
#assert(size_of(HUD_T) == 1040)

// class Player::Magic_t — 120 bytes
Magic_T :: struct {
	player:                       rawptr, // Player&
	selected_spell:               ^spell_t,
	quick_cast_spell:             ^spell_t,
	quick_cast_tome:              u32,
	selected_spell_alternate:     [5]^spell_t,
	selected_spell_last_appearance: i32,
	spell_list:                   list_t,
	b_has_unread_new_spell:       bool,
	no_mana_feedback_ticks:       u32,
	no_mana_processed_on_tick:    u32,
	spellbook_uid_from_hotbar_slot: u32,
	telekinesis_target:           u32,
}
#assert(size_of(Magic_T) == 120)

// struct Player::PlayerSettings_t — 16 bytes
Player_Settings_T :: struct {
	quick_turn_direction: i32,
	quick_turn_speed:     f64,
}
#assert(size_of(Player_Settings_T) == 16)

// struct Player::PlayerMovement_t — 40 bytes
Player_Movement_T :: struct {
	quick_turn_rotation:      f64,
	quick_turn_start_ticks:   u32,
	b_doing_quick_turn:       bool,
	player:                   rawptr, // Player&
	monster_emote_gimp_timer: i32,
	selected_entity_gimp_timer: i32,
	insectoid_levitating:     bool,
}
#assert(size_of(Player_Movement_T) == 40)

// class Player::Ghost_t — 96 bytes
Ghost_T :: struct {
	quick_turn_rotation:       f64,
	quick_turn_start_ticks:    u32,
	b_doing_quick_turn:        bool,
	casting_spell_animation:   i32, // GhostSpells_t enum
	casting_held_duration:     u32,
	player:                    rawptr, // Player&
	my:                        ^Entity,
	spawn_x:                   i32,
	spawn_y:                   i32,
	start_room_x:              i32,
	start_room_y:              i32,
	teleport_to_player:        i32,
	uid:                       u32,
	cooldown_push:             u32,
	cooldown_chill:            u32,
	cooldown_teleport:         u32,
	error_flash_push_ticks:    u32,
	error_flash_teleport_ticks: u32,
	error_flash_chill_ticks:   u32,
	push_points:               i32,
}
#assert(size_of(Ghost_T) == 96)

// class Player::MessageZone_t — 120 bytes
Message_Zone_T :: struct {
	font:                       rawptr, // TTF_Font*
	old_sdl_ticks:              u32,
	player:                     rawptr, // Player&
	notification_messages:      [dynamic]^Message, // DynamicArrayT<Message*> (non-owning)
	message_max_entries:        i32,
	chat_frame:                 rawptr, // Frame*
	anim_fade:                  f64,
	bottom_aligned_messages:    bool,
	use_big_font:               bool,
	message_alignment:          i32, // ChatAlignment_t enum
	actual_alignment:           i32, // ChatAlignment_t enum
	log_parent_frame:           rawptr, // Frame*
	log_window:                 rawptr, // Frame*
}
#assert(size_of(Message_Zone_T) == 120)

// struct Player::WorldUI_t::WorldTooltipItem_t — 48 bytes
World_Tooltip_Item_T :: struct {
	player:                    rawptr, // Player&
	type:                      u32,
	status:                    i32,
	beatitude:                 i32,
	count:                     i32,
	appearance:                u32,
	identified_item:           bool,
	has_appraise_capstone:     bool,
	item_world_tooltip_surface: rawptr, // SDL_Surface*
	item_frame:                rawptr, // Frame*
}
#assert(size_of(World_Tooltip_Item_T) == 48)

// struct Player::WorldUI_t::WorldTooltipDialogue_t::WorldDialogueSettings_t::Setting_t — 56 bytes
Setting_T :: struct {
	offset_z:             f64,
	text_delay:           i32,
	follow_entity:        bool,
	fade_dist:            f64,
	base_ticks_to_display: u32,
	extra_ticks_per_line: u32,
	max_width:            i32,
	padx:                 i32,
	pady:                 i32,
	pad_after_first_line: i32,
	scale_mod:            f64,
}
#assert(size_of(Setting_T) == 56)

// struct Player::WorldUI_t::WorldTooltipDialogue_t::Dialogue_t — 144 bytes
Dialogue_T :: struct {
	player:                   i32,
	parent:                   u32,
	x:                        f64,
	y:                        f64,
	z:                        f64,
	active:                   bool,
	draw:                     bool,
	init:                     bool,
	anim_z:                   f64,
	alpha:                    f64,
	draw_scale:               f64,
	spawn_tick:               u32,
	updated_this_tick:        u32,
	expiry_ticks:             u32,
	dialogue_field:           rawptr, // Field*
	dialogue_string_length:   i64, // size_t
	dialogue_str_full:        containers.DynamicString,
	dialogue_str_current:     containers.DynamicString,
	dialogue_type:            i32, // DialogueType_t enum
	dialogue_tooltip_surface: rawptr, // SDL_Surface*
}
#assert(size_of(Dialogue_T) == 144)

// struct Player::WorldUI_t::WorldTooltipDialogue_t — 184 bytes
World_Tooltip_Dialogue_T :: struct {
	player:           rawptr, // Player&
	player_dialogue:  Dialogue_T,
	shared_dialogues: map[[4]byte]Dialogue_T, // DynamicMapI32T<Dialogue_t> (i32-keyed)
}
#assert(size_of(World_Tooltip_Dialogue_T) == 184)

// class Player::WorldUI_t — 352 bytes
World_UI_T :: struct {
	player:                     rawptr, // Player&
	b_enabled:                  bool,
	world_tooltip_item:         World_Tooltip_Item_T,
	world_tooltip_dialogue:     World_Tooltip_Dialogue_T,
	tooltip_view:               i32, // TooltipView enum
	tooltips_in_range:          [dynamic]EntityF64_Pair, // vector<pair<Entity*,real_t>>
	modified_tooltip_draw_height: f64,
	player_last_yaw:            f64,
	player_last_pitch:          f64,
	gimp_display_timer:         i32,
	b_tooltip_in_view:          bool,
	uid_for_active_tooltip:     u32,
	interact_text:              containers.DynamicString,
}
#assert(size_of(World_UI_T) == 352)

// struct Player::PaperDoll_t::PaperDollSlot_t — 8 bytes
Paper_Doll_Slot_T :: struct {
	item:      u32,
	slot_type: i32, // PaperDollSlotType enum
}
#assert(size_of(Paper_Doll_Slot_T) == 8)

// class Player::PaperDoll_t — 168 bytes
Paper_Doll_T :: struct {
	player:                     rawptr, // Player&
	enabled:                    bool,
	doll_slots:                 [10]Paper_Doll_Slot_T,
	returning_items_to_inventory: [dynamic]u32, // DynamicArrayU32
	portrait_active_to_edit:    bool,
	portrait_rotation_inertia:  f64,
	portrait_rotation_percent:  f64,
	portrait_yaw:               f64,
}
#assert(size_of(Paper_Doll_T) == 168)

// struct hotbar_slot_t (interface.hpp) — 72 bytes
Hotbar_Slot_T :: struct {
	item:         u32,
	last_item:    Item,
	last_category: i32,
}
#assert(size_of(Hotbar_Slot_T) == 72)

// struct Player::Hotbar_t::Cursor_t — 40 bytes
Hotbar_Cursor_T :: struct {
	animate_x:          f64,
	animate_y:          f64,
	animate_setpoint_x: i32,
	animate_setpoint_y: i32,
	animate_start_x:    i32,
	animate_start_y:    i32,
	last_update_tick:   u32,
	cursor_to_slot_offset: i32,
}
#assert(size_of(Hotbar_Cursor_T) == 40)

// class Player::Hotbar_t — 4720 bytes
Hotbar_T :: struct {
	hotbar:                          [10]Hotbar_Slot_T,
	hotbar_alternate:                [5][10]Hotbar_Slot_T,
	player:                          rawptr, // Player&
	hotbar_slot_frames:              [10]rawptr, // Frame*[10]
	current_hotbar:                  i32,
	hotbar_shapeshift_init:          [5]bool,
	swap_hotbar_on_shapeshift:       i32,
	hotbar_has_focus:                bool,
	magic_boomerang_hotbar_slot:     i32,
	magic_duck_hotbar_slot:          i32,
	hotbar_tooltip_last_game_tick:   u32,
	hotbar_box:                      containers.SDL_Rect,
	hotbar_frame:                    rawptr, // Frame*
	selected_slot_animate_current_value: f64,
	is_interactable:                 bool,
	shootmode_cursor:                Hotbar_Cursor_T,
	use_hotbar_radial_menu:          bool,
	use_hotbar_face_menu:            bool,
	face_menu_invert_layout:         bool,
	face_menu_quick_cast_enabled:    bool,
	face_menu_quick_cast:            bool,
	face_menu_alternate_layout:      bool,
	face_menu_button_held:           i32, // FaceMenuGroup enum
	face_button_top_y_position:      i32,
	radial_hotbar_slots:             i32,
	radial_hotbar_progress:          i32,
	old_slot_frame_track_slot:       i32,
	anim_hide:                       f64,
	face_button_positions:           [10]containers.SDL_Rect,
}
#assert(size_of(Hotbar_T) == 4720)

// class Player::Minimap_t — 88 bytes
Minimap_T :: struct {
	player:                rawptr, // Player&
	big:                   bool,
	real_scale:            f64,
	scale:                 f64,
	scale_ang:             f64,
	animating:             bool,
	minimap_pos:           containers.SDL_Rect,
	map_parent_frame:      rawptr, // Frame*
	map_window:            rawptr, // Frame*
	b_scale_prompt_enabled: bool,
	b_expand_prompt_enabled: bool,
}
#assert(size_of(Minimap_T) == 88)

// class Player::CompendiumProgress_t — 168 bytes
Compendium_Progress_T :: struct {
	player:               rawptr, // Player&
	item_events:          map[string]map[[4]byte]i32, // DynamicMapStrI32Map (string->i32 map)
	floor_events:         map[[4]byte]map[string]map[[4]byte]i32, // DynamicMapI32T<DynamicMapStrI32Map> (i32-keyed)
	player_dist_accum:    f64,
	player_sneak_time:    u32,
	player_alive_time_moving: u32,
	player_alive_time_stopped: u32,
	player_alive_time_total: u32,
	player_game_time_total: u32,
	player_equip_slot_time: map[[4]byte]u32, // DynamicMapI32T<Uint32> (i32-keyed)
	ally_time_spent:      map[[4]byte]u32, // DynamicMapI32T<Uint32> (i32-keyed)
}
#assert(size_of(Compendium_Progress_T) == 168)

// class Player::PlayerMechanics_t — 512 bytes
Player_Mechanics_T :: struct {
	player:                       rawptr, // Player&
	item_degrade_rng:             map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	learned_spells:               map[i32]struct{}, // DynamicSetI32
	ducks_in_a_row:               [dynamic]containers.IntPair_t, // vector<pair<int,int>>
	pending_ducks:                [dynamic]Int_U32_Pair, // vector<pair<int,Uint32>>
	favorite_books_achievement:   map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	num_fishing_caught:           i32,
	sustained_spell_mp_used_sorcery: i32,
	sustained_spell_mp_used_mysticism: i32,
	sustained_spell_mp_used_thaumaturgy: i32,
	base_spell_mp_used_sorcery:   i32,
	base_spell_mp_used_mysticism: i32,
	base_spell_mp_used_thaumaturgy: i32,
	defend_ticks:                 u32,
	foci_holy_charge_time:        i32,
	foci_dark_charge_time:        i32,
	last_foci_held_type:          i32,
	escalating_rng_rolls:         map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	escalating_spell_rng_rolls:   map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	base_spell_level_up_procs:    map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	sustained_spell_id_counter:   map[[4]byte]f64, // DynamicMapI32T<real_t> (i32-keyed)
	enemy_raised_blocking_against: map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	enemy_raised_stealth_against: map[[4]byte]i32, // DynamicMapI32T<int> (i32-keyed)
	ensemble_playing:             i32,
	ensemble_require_recast:      bool,
	ensemble_taken_initial_mp:    bool,
	previously_levitating:        bool,
	donation_revealed_on_floor:   u32,
	donation_claimed:             bool,
	targets_compelled:            map[[4]byte]map[[4]byte]u32, // DynamicMapU32Map (i32->i32 map)
	targets_refuse_compel:        map[i32]struct{}, // DynamicSetI32
	gremlin_breakable_counter:    i32,
	ensemble_data_update:         u32,
}
#assert(size_of(Player_Mechanics_T) == 512)

// class Player — 9176 bytes
Player :: struct {
	local_host:              bool,
	cam:                     rawptr, // view_t*
	playernum:               i32,
	entity:                  ^Entity,
	player_last_x:           f64,
	player_last_y:           f64,
	b_splitscreen:           bool,
	split_screen_type:       i32, // SplitScreenTypes enum
	b_control_enabled:       bool,
	was_connected_to_game:   bool,
	gui:                     GUI_T,
	shootmode:               bool,
	inventory_mode:          i32,
	gui_mode:                i32,
	inventory_ui:            Inventory_T,
	shop_gui:                Shop_GUI_T,
	book_gui:                Book_GUI_T,
	sign_gui:                Sign_GUI_T,
	character_sheet:         Character_Sheet_T,
	skill_sheet:             Skill_Sheet_T,
	hud:                     HUD_T,
	magic:                   Magic_T,
	settings:                Player_Settings_T,
	movement:                Player_Movement_T,
	ghost:                   Ghost_T,
	message_zone:            Message_Zone_T,
	world_ui:                World_UI_T,
	paper_doll:              Paper_Doll_T,
	hotbar:                  Hotbar_T,
	minimap:                 Minimap_T,
	compendium_progress:     Compendium_Progress_T,
	mechanics:               Player_Mechanics_T,
}
#assert(size_of(Player) == 9176)
