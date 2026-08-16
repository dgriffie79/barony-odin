// interface.odin — Odin mirror of interface/interface.hpp.
package main

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------

// enum DamageGib
DamageGib :: enum i32 {
	DMG_DEFAULT,
	DMG_WEAKER,
	DMG_WEAKEST,
	DMG_STRONGER,
	DMG_STRONGEST,
	DMG_FIRE,
	DMG_BLEED,
	DMG_POISON,
	DMG_HEAL,
	DMG_MISS,
	DMG_GUARD,
	DMG_TODO,
	DMG_DETECT_MONSTER,
}

// enum DamageGibDisplayType
DamageGibDisplayType :: enum i32 {
	DMG_GIB_NUMBER,
	DMG_GIB_MISS,
	DMG_GIB_SPRITE,
	DMG_GIB_GUARD,
	DMG_GIB_DPS_CHECK,
}

// enum EnemyHPDamageBarHandler::HPBarType
HPBarType :: enum i32 {
	BAR_TYPE_CREATURE,
	BAR_TYPE_FURNITURE,
}

// enum GUICurrentType
GUICurrentType :: enum i32 {
	GUI_TYPE_NONE,
	GUI_TYPE_ALCHEMY,
	GUI_TYPE_TINKERING,
	GUI_TYPE_SCRIBING,
	GUI_TYPE_ITEMFX,
	GUI_TYPE_ASSIST,
	GUI_TYPE_MAILBOX,
}

// enum GenericGUIMenu::TinkeringFilter
TinkeringFilter :: enum i32 {
	TINKER_FILTER_ALL,
	TINKER_FILTER_CRAFTABLE,
	TINKER_FILTER_SALVAGEABLE,
	TINKER_FILTER_REPAIRABLE,
}

// enum GenericGUIMenu::ScribingFilter
ScribingFilter :: enum i32 {
	SCRIBING_FILTER_CRAFTABLE,
	SCRIBING_FILTER_REPAIRABLE,
}

// enum GenericGUIMenu::TinkerGUI_t::TinkerActions_t
TinkerActions_T :: enum i32 {
	TINKER_ACTION_NONE,
	TINKER_ACTION_OK,
	TINKER_ACTION_OK_UPGRADE,
	TINKER_ACTION_INVALID_ITEM,
	TINKER_ACTION_INVALID_ROBOT_TO_SALVAGE,
	TINKER_ACTION_NO_MATERIALS,
	TINKER_ACTION_NO_MATERIALS_UPGRADE,
	TINKER_ACTION_NO_SKILL_LVL,
	TINKER_ACTION_NO_SKILL_LVL_UPGRADE,
	TINKER_ACTION_ITEM_FULLY_REPAIRED,
	TINKER_ACTION_ITEM_FULLY_UPGRADED,
	TINKER_ACTION_ROBOT_BROKEN,
	TINKER_ACTION_MUST_BE_UNEQUIPPED,
	TINKER_ACTION_ALREADY_USING_THIS_TINKERING_KIT,
	TINKER_ACTION_OK_UNIDENTIFIED_SALVAGE,
	TINKER_ACTION_NOT_IDENTIFIED_YET,
	TINKER_ACTION_KIT_NEEDS_REPAIRS,
}

// enum GenericGUIMenu::TinkerGUI_t::InvalidActionFeedback_t
InvalidActionFeedback_T :: enum i32 {
	INVALID_ACTION_NONE,
	INVALID_ACTION_SHAKE_PROMPT,
	INVALID_ACTION_SHAKE_METAL_SCRAP,
	INVALID_ACTION_SHAKE_MAGIC_SCRAP,
	INVALID_ACTION_SHAKE_ALL_SCRAP,
}

// enum GenericGUIMenu::ItemEffectGUI_t::CostEffectTypes
CostEffectTypes :: enum i32 {
	COST_EFFECT_NONE,
	COST_EFFECT_GOLD,
	COST_EFFECT_MANA,
	COST_EFFECT_MANA_RETURN_GOLD,
	COST_EFFECT_MANA_AND_GOLD,
}

// enum GenericGUIMenu::ItemEffectGUI_t::ItemEffectModes
ItemEffectModes :: enum i32 {
	ITEMFX_MODE_NONE,
	ITEMFX_MODE_SCROLL_REPAIR,
	ITEMFX_MODE_SCROLL_CHARGING,
	ITEMFX_MODE_SCROLL_IDENTIFY,
	ITEMFX_MODE_SCROLL_REMOVECURSE,
	ITEMFX_MODE_SPELL_IDENTIFY,
	ITEMFX_MODE_SPELL_REMOVECURSE,
	ITEMFX_MODE_SCROLL_ENCHANT_WEAPON,
	ITEMFX_MODE_SCROLL_ENCHANT_ARMOR,
	ITEMFX_MODE_ALTER_INSTRUMENT,
	ITEMFX_MODE_METALLURGY,
	ITEMFX_MODE_GEOMANCY,
	ITEMFX_MODE_FORGE_KEY,
	ITEMFX_MODE_FORGE_JEWEL,
	ITEMFX_MODE_ENHANCE_WEAPON,
	ITEMFX_MODE_RESHAPE_WEAPON,
	ITEMFX_MODE_ALTER_ARROW,
	ITEMFX_MODE_PUNCTURE_VOID,
	ITEMFX_MODE_ADORCISE_WEAPON,
	ITEMFX_MODE_RESTORE,
	ITEMFX_MODE_VANDALISE,
	ITEMFX_MODE_DESECRATE,
	ITEMFX_MODE_SANCTIFY,
	ITEMFX_MODE_SANCTIFY_WATER,
	ITEMFX_MODE_CLEANSE_FOOD,
	ITEMFX_MODE_ADORCISE_INSTRUMENT,
	ITEMFX_MODE_SCEPTER_CHARGE,
}

// enum GenericGUIMenu::ItemEffectGUI_t::ItemEffectActions_t
ItemEffectActions_T :: enum i32 {
	ITEMFX_ACTION_NONE,
	ITEMFX_ACTION_OK,
	ITEMFX_ACTION_INVALID_ITEM,
	ITEMFX_ACTION_ITEM_FULLY_REPAIRED,
	ITEMFX_ACTION_ITEM_FULLY_CHARGED,
	ITEMFX_ACTION_ITEM_IDENTIFIED,
	ITEMFX_ACTION_MUST_BE_UNEQUIPPED,
	ITEMFX_ACTION_NOT_IDENTIFIED_YET,
	ITEMFX_ACTION_CANT_AFFORD_GOLD,
	ITEMFX_ACTION_CANT_AFFORD_MANA,
	ITEMFX_ACTION_NOT_CURSED,
	ITEMFX_ACTION_UNVOIDABLE,
	ITEMFX_ACTION_AT_MAX_BLESSING,
	ITEMFX_ACTION_NEED_SKILL_LVLS,
	ITEMFX_ACTION_CANT_AFFORD_MANA_AND_GOLD,
}

// enum GenericGUIMenu::ItemEffectGUI_t::InvalidActionFeedback_t (2: none, shake)
ItemEffect_InvalidActionFeedback :: enum i32 {
	INVALID_ACTION_NONE,
	INVALID_ACTION_SHAKE_PROMPT,
}

// enum GenericGUIMenu::AssistShrineGUI_t::AssistShrineView_t
AssistShrineView_T :: enum i32 {
	ASSIST_SHRINE_VIEW_ITEMS,
	ASSIST_SHRINE_VIEW_CLASSES,
	ASSIST_SHRINE_VIEW_RACE,
}

// enum GenericGUIMenu::AssistShrineGUI_t::AssistNotification_t::NotificationTypes
AssistNotificationTypes :: enum i32 {
	NOTIF_DEFAULT,
	NOTIF_SEND_REQ,
	NOTIF_CHARACTER_CHANGE_OK,
	NOTIF_CLASS_RESET,
}

// enum GenericGUIMenu::AssistShrineGUI_t::AssistItemActions_t
AssistItemActions_T :: enum i32 {
	ASSIST_ITEM_NONE,
	ASSIST_ITEM_ACTIVATE,
	ASSIST_ITEM_DEACTIVATE,
	ASSIST_ITEM_CLAIMED,
	ASSIST_ITEM_NOTHING_TO_CLAIM,
	ASSIST_ITEM_FLAG_DISABLED,
	ASSIST_CLASS_OK,
	ASSIST_RACE_OK,
}

// enum GenericGUIMenu::AssistShrineGUI_t::InvalidActionFeedback_t (2)
Assist_InvalidActionFeedback :: enum i32 {
	INVALID_ACTION_NONE,
	INVALID_ACTION_SHAKE_PROMPT,
}

// enum GenericGUIMenu::MailboxGui_t::MailActions_t
MailActions_T :: enum i32 {
	MAIL_ACTION_NONE,
	MAIL_ACTION_OK,
	MAIL_ACTION_INVALID_ITEM,
	MAIL_ACTION_UNIDENTIFIED,
}

// enum GenericGUIMenu::FeatherGUI_t::FeatherActions_t
FeatherActions_T :: enum i32 {
	FEATHER_ACTION_NONE,
	FEATHER_ACTION_OK,
	FEATHER_ACTION_INVALID_ITEM,
	FEATHER_ACTION_NO_BLANK_SCROLL,
	FEATHER_ACTION_NO_BLANK_SCROLL_UNKNOWN_HIGHLIGHT,
	FEATHER_ACTION_FULLY_REPAIRED,
	FEATHER_ACTION_UNIDENTIFIED,
	FEATHER_ACTION_CANT_AFFORD,
	FEATHER_ACTION_MAY_SUCCEED,
	FEATHER_ACTION_OK_AND_DESTROY,
	FEATHER_ACTION_OK_UNKNOWN_SCROLL,
}

// enum GenericGUIMenu::FeatherGUI_t::InvalidActionFeedback_t (3: +no charge)
Feather_InvalidActionFeedback :: enum i32 {
	INVALID_ACTION_NONE,
	INVALID_ACTION_SHAKE_PROMPT,
	INVALID_ACTION_NO_CHARGE,
}

// enum GenericGUIMenu::FeatherGUI_t::SortTypes_t
SortTypes_T :: enum i32 {
	SORT_SCROLL_DEFAULT,
	SORT_SCROLL_DISCOVERED,
	SORT_SCROLL_UNKNOWN,
}

// enum GenericGUIMenu::AlchemyGUI_t::AlchemyActions_t
AlchemyActions_T :: enum i32 {
	ALCHEMY_ACTION_NONE,
	ALCHEMY_ACTION_OK,
	ALCHEMY_ACTION_INVALID_ITEM,
	ALCHEMY_ACTION_UNIDENTIFIED_POTION,
}

// enum GenericGUIMenu::AlchemyGUI_t::AlchemyView_t
AlchemyView_T :: enum i32 {
	ALCHEMY_VIEW_BREW,
	ALCHEMY_VIEW_RECIPES,
	ALCHEMY_VIEW_COOK,
	ALCHEMY_VIEW_RECIPES_COOK,
}

// enum CloseGUIShootmode
CloseGUIShootmode :: enum i32 {
	DONT_CHANGE_SHOOTMODE,
	CLOSEGUI_ENABLE_SHOOTMODE,
}

// enum CloseGUIIgnore
CloseGUIIgnore :: enum i32 {
	CLOSEGUI_CLOSE_ALL,
	CLOSEGUI_DONT_CLOSE_FOLLOWERGUI,
	CLOSEGUI_DONT_CLOSE_CHEST,
	CLOSEGUI_DONT_CLOSE_SHOP,
	CLOSEGUI_DONT_CLOSE_INVENTORY,
	CLOSEGUI_DONT_CLOSE_CALLOUTGUI,
}

// enum AttackHoverText_t::HoverTypes
HoverTypes :: enum i32 {
	ATK_HOVER_TYPE_DEFAULT,
	ATK_HOVER_TYPE_UNARMED,
	ATK_HOVER_TYPE_RANGED,
	ATK_HOVER_TYPE_THROWN,
	ATK_HOVER_TYPE_THROWN_POTION,
	ATK_HOVER_TYPE_THROWN_GEM,
	ATK_HOVER_TYPE_MELEE_WEAPON,
	ATK_HOVER_TYPE_WHIP,
	ATK_HOVER_TYPE_MAGICSTAFF,
	ATK_HOVER_TYPE_TOOL,
	ATK_HOVER_TYPE_PICKAXE,
	ATK_HOVER_TYPE_TOOL_TRAP,
	ATK_HOVER_TYPE_THROWN_MISC,
	ATK_HOVER_TYPE_RAPIER,
}

// enum MinimapPing::PingType : Uint8
PingType :: enum u8 {
	PING_DEFAULT,
	PING_DEATH_MARKER,
}

// enum FollowerRadialMenu::PanelDirections
PanelDirections :: enum i32 {
	NORTH,
	NORTHWEST,
	WEST,
	SOUTHWEST,
	SOUTH,
	SOUTHEAST,
	EAST,
	NORTHEAST,
	PANEL_DIRECTION_END,
}

// enum CalloutRadialMenu::CalloutCommand
CalloutCommand :: enum i32 {
	CALLOUT_CMD_LOOK,
	CALLOUT_CMD_HELP,
	CALLOUT_CMD_NEGATIVE,
	CALLOUT_CMD_SOUTHWEST,
	CALLOUT_CMD_SOUTH,
	CALLOUT_CMD_SOUTHEAST,
	CALLOUT_CMD_AFFIRMATIVE,
	CALLOUT_CMD_MOVE,
	CALLOUT_CMD_CANCEL,
	CALLOUT_CMD_SELECT,
	CALLOUT_CMD_END,
	CALLOUT_CMD_THANKS,
}

// enum CalloutRadialMenu::CalloutType
CalloutType :: enum i32 {
	CALLOUT_TYPE_NO_TARGET,
	CALLOUT_TYPE_NPC,
	CALLOUT_TYPE_PLAYER,
	CALLOUT_TYPE_BOULDER,
	CALLOUT_TYPE_TRAP,
	CALLOUT_TYPE_GENERIC_INTERACTABLE,
	CALLOUT_TYPE_CHEST,
	CALLOUT_TYPE_ITEM,
	CALLOUT_TYPE_SWITCH,
	CALLOUT_TYPE_SWITCH_ON,
	CALLOUT_TYPE_SWITCH_OFF,
	CALLOUT_TYPE_SHRINE,
	CALLOUT_TYPE_EXIT,
	CALLOUT_TYPE_SECRET_EXIT,
	CALLOUT_TYPE_SECRET_ENTRANCE,
	CALLOUT_TYPE_GOLD,
	CALLOUT_TYPE_FOUNTAIN,
	CALLOUT_TYPE_SINK,
	CALLOUT_TYPE_NPC_ENEMY,
	CALLOUT_TYPE_NPC_PLAYERALLY,
	CALLOUT_TYPE_TELEPORTER_LADDER_UP,
	CALLOUT_TYPE_TELEPORTER_LADDER_DOWN,
	CALLOUT_TYPE_TELEPORTER_PORTAL,
	CALLOUT_TYPE_BOMB_TRAP,
	CALLOUT_TYPE_COLLIDER_BREAKABLE,
	CALLOUT_TYPE_BELL,
	CALLOUT_TYPE_DAEDALUS,
	CALLOUT_TYPE_ASSIST_SHRINE,
	CALLOUT_TYPE_WALL_LOCK,
	CALLOUT_TYPE_WALL_LOCK_ON,
	CALLOUT_TYPE_WALL_LOCK_OFF,
	CALLOUT_TYPE_WALL_BUTTON_ON,
	CALLOUT_TYPE_WALL_BUTTON_OFF,
}

// enum CalloutRadialMenu::CalloutHelpFlags (bit flags)
CalloutHelpFlags :: enum i32 {
	CALLOUT_HELP_FOOD_HUNGRY     = 0b1,
	CALLOUT_HELP_BLOOD_HUNGRY    = 0b10,
	CALLOUT_HELP_FOOD_WEAK       = 0b100,
	CALLOUT_HELP_BLOOD_WEAK      = 0b1000,
	CALLOUT_HELP_FOOD_STARVING   = 0b10000,
	CALLOUT_HELP_BLOOD_STARVING  = 0b100000,
	CALLOUT_HELP_STEAM_CRITICAL  = 0b1000000,
	CALLOUT_HELP_HP_LOW          = 0b10000000,
	CALLOUT_HELP_HP_CRITICAL     = 0b100000000,
	CALLOUT_HELP_NEGATIVE_FX     = 0b1000000000,
}

// enum CalloutRadialMenu::SetCalloutTextTypes
SetCalloutTextTypes :: enum i32 {
	SET_CALLOUT_BANNER_TEXT,
	SET_CALLOUT_WORLD_TEXT,
	SET_CALLOUT_ICON_KEY,
}

// enum ItemContextMenuPrompts
ItemContextMenuPrompts :: enum i32 {
	PROMPT_EQUIP,
	PROMPT_UNEQUIP,
	PROMPT_SPELL_EQUIP,
	PROMPT_SPELL_QUICKCAST,
	PROMPT_SPELL_CHANGE_FOCUS,
	PROMPT_APPRAISE,
	PROMPT_DROPDOWN,
	PROMPT_INTERACT,
	PROMPT_INTERACT_SPELLBOOK_HOTBAR,
	PROMPT_EAT,
	PROMPT_CONSUME,
	PROMPT_CONSUME_ALTERNATE,
	PROMPT_INSPECT,
	PROMPT_INSPECT_ALTERNATE,
	PROMPT_SELL,
	PROMPT_BUY,
	PROMPT_STORE_CHEST,
	PROMPT_RETRIEVE_CHEST,
	PROMPT_RETRIEVE_CHEST_ALL,
	PROMPT_STORE_CHEST_ALL,
	PROMPT_DROP,
	PROMPT_TINKER,
	PROMPT_GRAB,
	PROMPT_UNEQUIP_FOR_DROP,
	PROMPT_CLEAR_HOTBAR_SLOT,
	PROMPT_COOK,
	PROMPT_SCEPTER_CHARGE,
}

// ---------------------------------------------------------------------------
// Standalone structs
// ---------------------------------------------------------------------------

// struct DamageIndicatorHandler_t::DamageIndicator_t — 72 bytes
DamageIndicator_T :: struct {
	player:                 i32, // default -1
	// pad 4
	x:                      f64,
	y:                      f64,
	alpha:                  f64,
	ticks:                  u32,
	animate_ticks:          u32,
	uid:                    u32,
	// pad 4
	size:                   i32,
	w:                      i32,
	h:                      i32,
	flash_ticks:            u32,
	flash_processed_on_tick: u32,
	flash_anim_state:       i32, // default -1
	hit_dealt_damage:       bool,
	expired:                bool,
}
#assert(size_of(DamageIndicator_T) == 72)

// struct DamageIndicatorHandler_t — 160 bytes
// { DynamicArray indicators[4]; } (vector<DamageIndicator_t>)
DamageIndicatorHandler_T :: struct {
	indicators: [4][dynamic]DamageIndicator_T, // DynamicArray x4 (40B each)
}
#assert(size_of(DamageIndicatorHandler_T) == 160)

// struct EnemyHPDamageBarHandler::BarAnimator_t — 120 bytes
EnemyHP_BarAnimator_T :: struct {
	foreground_value:   f64,
	background_value:   f64,
	previous_setpoint:  f64,
	setpoint:           i32, // Sint32
	animate_ticks:      u32,
	damage_taken:       i32, // Sint32
	// pad 4
	width_multiplier:   f64,
	max_value:          f64,
	current_opacity:    f64,
	fade_out:           f64,
	fade_in:            f64,
	skull_opacities:    [4]f64,
	damage_frame_opacity: f64,
}
#assert(size_of(EnemyHP_BarAnimator_T) == 120)

// struct EnemyHPDamageBarHandler::EnemyHPDetails — 280 bytes
EnemyHPDetails_T :: struct {
	bar_type:        HPBarType,
	animator:        EnemyHP_BarAnimator_T,
	enemy_name:      string, // DynamicString
	enemy_hp:        i32, // Sint32
	enemy_maxhp:     i32,
	enemy_oldhp:     i32,
	enemy_timer:     u32,
	enemy_uid:       u32,
	enemy_status_effects1: u32,
	enemy_status_effects2: u32,
	enemy_status_effects3: u32,
	enemy_status_effects4: u32,
	enemy_status_effects5: u32,
	enemy_status_effects_low_duration1: u32,
	enemy_status_effects_low_duration2: u32,
	enemy_status_effects_low_duration3: u32,
	enemy_status_effects_low_duration4: u32,
	enemy_status_effects_low_duration5: u32,
	low_priority_tick: bool,
	should_display:  bool,
	has_distance_check: bool,
	display_on_hud:  bool,
	expired:         bool,
	detect_monster_check_status: bool,
	// pad 2
	depletion_animation_percent: f64,
	world_x:         f64,
	world_y:         f64,
	world_z:         f64,
	screen_distance: f64,
	world_texture:   ^Temp_Texture, // TempTexture*
	world_surface_sprite: rawptr,   // SDL_Surface*
	world_surface_sprite_status_effects: rawptr, // SDL_Surface*
}
#assert(size_of(EnemyHPDetails_T) == 280)

// class EnemyHPDamageBarHandler — statics-only (no instance data; 0 bytes).
// The BarAnimator_t/EnemyHPDetails_t mirrors above carry the per-entity data.
EnemyHPDamageBarHandler :: struct {
}
#assert(size_of(EnemyHPDamageBarHandler) == 0)

// struct MinimapHighlight_t — 4 bytes
MinimapHighlight_T :: struct {
	ticks: u32,
}
#assert(size_of(MinimapHighlight_T) == 4)

// struct AttackHoverText_t — 48 bytes
AttackHoverText_T :: struct {
	hover_type:        HoverTypes,
	total_attack:      i32, // Sint32
	weapon_bonus:      i32,
	main_attribute_bonus: i32,
	secondary_attribute_bonus: i32,
	proficiency_bonus: i32,
	proficiency_variance: f64,
	attack_min_range:  i32,
	attack_max_range:  i32,
	equipment_and_effect_bonus: i32,
	proficiency:       i32,
}
#assert(size_of(AttackHoverText_T) == 48)

// class MinimapPing — 12 bytes
MinimapPing :: struct {
	tick_start:  u32,
	player:      u8,
	x:           u8,
	y:           u8,
	radius_ping: bool,
	ping_type:   PingType, // enum u8
}
#assert(size_of(MinimapPing) == 12)

// struct FollowerRadialMenu::PanelEntry — 80 bytes
FollowerRadialMenu_PanelEntry :: struct {
	x:                 i32,
	y:                 i32,
	// pad 4
	path:              string, // DynamicString
	path_locked:       string,
	path_hover:        string,
	path_locked_hover: string,
	icon_offsetx:      i32,
	icon_offsety:      i32,
}
#assert(size_of(FollowerRadialMenu_PanelEntry) == 80)

// class FollowerRadialMenu — 256 bytes
FollowerRadialMenu :: struct {
	follower_to_command: ^Entity,
	recent_entity:       ^Entity,
	entity_to_interact_with: ^Entity,
	menu_x:              i32,
	menu_y:              i32,
	option_selected:     i32,
	option_previous:     i32,
	select_move_to:      bool,
	// pad 3
	move_to_x:           i32,
	move_to_y:           i32,
	menu_toggle_click:   bool,
	hold_wheel:          bool,
	// pad 6
	interact_text:       [128]u8, // char[128]
	accessed_menu_from_party_sheet: bool,
	// pad 3
	party_sheet_mouse_x: i32,
	party_sheet_mouse_y: i32,
	sidebar_scroll_index: i32,
	max_monsters_to_draw: i32,
	gui_player:          i32,
	// pad 4
	follower_frame:      ^Frame,
	anim_title:          f64,
	anim_wheel:          f64,
	opened_this_tick:    u32,
	// pad 4
	anim_invalid_action: f64,
	anim_invalid_action_ticks: u32,
}
#assert(size_of(FollowerRadialMenu) == 256)

// struct CalloutRadialMenu::PanelEntry — 48 bytes
CalloutRadialMenu_PanelEntry :: struct {
	x:            i32,
	y:            i32,
	// pad 4
	path:         string, // DynamicString
	path_hover:   string,
	icon_offsetx: i32,
	icon_offsety: i32,
}
#assert(size_of(CalloutRadialMenu_PanelEntry) == 48)

// struct CalloutRadialMenu::CalloutParticle_t — 160 bytes
CalloutParticle_T :: struct {
	x:                    f64,
	y:                    f64,
	z:                    f64,
	entity_uid:           u32,
	ticks:                u32,
	lifetime:             u32,
	creation_tick:        u32,
	cmd:                  CalloutCommand,
	type:                 CalloutType,
	expired:              bool,
	lock_on_screen:       [4]bool,
	// pad 3
	player_color:         i32,
	tag_id:               i32,
	tag_small_id:         i32,
	animate_state:        i32,
	animate_state_init:   i32,
	// pad 4
	scale:                f64,
	animate_x:            f64,
	animate_scale_for_player_view: [4]f64,
	animate_bounce:       f64,
	animate_y:            f64,
	no_update:            bool,
	self_callout:         bool,
	do_message:           bool,
	// pad 1
	message_sent_tick:    u32,
	big:                  [4]bool,
}
#assert(size_of(CalloutParticle_T) == 160)

// struct CalloutRadialMenu — 272 bytes
CalloutRadialMenu :: struct {
	menu_x:            i32,
	menu_y:            i32,
	option_selected:   i32,
	select_move_to:    bool,
	// pad 3
	move_to_x:         i32,
	move_to_y:         i32,
	menu_toggle_click: bool,
	hold_wheel:        bool,
	// pad 6
	interact_text:     [128]u8, // char[128]
	max_monsters_to_draw: i32,
	gui_player:        i32,
	// pad 4
	callout_frame:     ^Frame,
	callout_ping_frame: ^Frame,
	b_open:            bool,
	// pad 3
	lock_on_entity_uid: u32,
	client_callout_help_flags: u32,
	// pad 4
	callouts:          map[[4]byte]CalloutParticle_T, // DynamicMapI32T<CalloutParticle_t> (32B)
	anim_title:        f64,
	anim_wheel:        f64,
	opened_this_tick:  u32,
	// pad 4
	anim_invalid_action: f64,
	anim_invalid_action_ticks: u32,
	updated_this_tick: u32,
}
#assert(size_of(CalloutRadialMenu) == 272)

// ---------------------------------------------------------------------------
// GenericGUIMenu + nested GUI structs
// ---------------------------------------------------------------------------

// struct GenericGUIMenu::AssistShrineGUI_t::AssistNotification_t — 72 bytes
AssistNotification_T :: struct {
	img:               string, // DynamicString
	title:             string,
	body:              string,
	lifetime:          u32,
	notification_type: AssistNotificationTypes,
	// pad 4
	animx:             f64,
	state:             i32,
}
#assert(size_of(AssistNotification_T) == 72)

// struct GenericGUIMenu::AssistShrineGUI_t — 656 bytes
AssistShrineGUI_T :: struct {
	parent_gui:              rawptr, // GenericGUIMenu& (reference)
	item_cloak:              Item,   // 56B
	item_mask:               Item,
	item_amulet:             Item,
	item_ring:               Item,
	current_view:            AssistShrineView_T,
	// pad 4
	class_slots:             map[[4]byte]i32, // DynamicMapI32T<int>
	race_slots:              [dynamic]i32,    // DynamicArrayT<int>
	selected_class:          i32,
	selected_race:           i32,
	selected_sex:            i32,
	selected_appearance:     i32,
	selected_disable_abilities: i32,
	saved_class:             i32,
	saved_race:              i32,
	saved_sex:               i32,
	saved_appearance:        i32,
	received_character_change_ok: bool,
	// pad 3
	animx:                   f64,
	anim_filter:             f64,
	anim_prompt:             f64,
	anim_tooltip:            f64,
	anim_assist_value_fade:  f64,
	anim_prompt_ticks:       u32,
	anim_tooltip_ticks:      u32,
	anim_class_race_tooltip_opacity: f64,
	anim_class_race_tooltip_ticks: u32,
	// pad 4
	claimed_items:           map[i32]struct{}, // DynamicSetI32
	is_interactable:         bool,
	b_open:                  bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	assist_shrine_frame:     ^Frame,
	shrine_uid:              u32,
	// pad 4
	assist_shrine_slot_frames: map[[4]byte]rawptr, // DynamicMapI32T<Frame*>
	selected_assist_shrine_slot_x: i32,
	selected_assist_shrine_slot_y: i32,
	current_scroll_row1:     i32,
	current_scroll_row2:     i32,
	scroll_percent1:         f64,
	scroll_inertia1:         f64,
	scroll_setpoint1:        i32,
	scroll_animate_x1:       f64,
	scroll_percent2:         f64,
	scroll_inertia2:         f64,
	scroll_setpoint2:        i32,
	scroll_animate_x2:       f64,
	// pad 4
	notifications:           [dynamic]Uint32_AssistNotif_Pair, // vector<pair<Uint32,AssistNotification_t>>
	item_action_type:        AssistItemActions_T,
	// pad 4
	anim_invalid_action:     f64,
	anim_invalid_action_ticks: u32,
	invalid_action_type:     Assist_InvalidActionFeedback,
	item_type:               i32,
}
#assert(size_of(AssistShrineGUI_T) == 656)

// struct GenericGUIMenu::TinkerGUI_t — 216 bytes
TinkerGUI_T :: struct {
	parent_gui:             rawptr, // GenericGUIMenu&
	tinker_frame:           ^Frame,
	animx:                  f64,
	is_interactable:        bool,
	b_open:                 bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	metal_scrap_price:      i32, // Sint32
	magic_scrap_price:      i32,
	item_desc:              string, // DynamicString
	item_type:              i32,
	item_requirement:       i32,
	item_action_type:       TinkerActions_T,
	item_requires_title_reflow: bool,
	// pad 3
	anim_drawer:            f64,
	anim_tooltip:           f64,
	anim_tooltip_ticks:     u32,
	anim_filter:            f64,
	anim_prompt:            f64,
	anim_prompt_ticks:      u32,
	anim_prompt_move_left:  bool,
	// pad 3
	anim_invalid_action:    f64,
	anim_invalid_action_ticks: u32,
	drawer_justify_inverted: bool,
	// pad 3
	invalid_action_type:    InvalidActionFeedback_T,
	player_current_metal_scrap: i32,
	player_current_magic_scrap: i32,
	player_change_metal_scrap: i32,
	player_change_magic_scrap: i32,
	anim_scrap:             f64,
	anim_scrap_start_ticks: u32,
	selected_tinker_slot_x: i32,
	selected_tinker_slot_y: i32,
	// pad 4
	tinker_slot_frames:     map[[4]byte]rawptr, // DynamicMapI32T<Frame*>
}
#assert(size_of(TinkerGUI_T) == 216)

// struct GenericGUIMenu::ItemEffectGUI_t — 160 bytes
ItemEffectGUI_T :: struct {
	parent_gui:             rawptr, // GenericGUIMenu&
	item_effect_frame:      ^Frame,
	animx:                  f64,
	is_interactable:        bool,
	b_open:                 bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	mode_has_cost_effect:   CostEffectTypes,
	cost_effect_gold_amount: i32,
	cost_effect_mp_amount:  i32,
	current_mode:           ItemEffectModes,
	item_desc:              string, // DynamicString
	item_type:              i32,
	item_requirement:       i32,
	confirm_action_steps:   i32,
	confirm_action_on_item_steps: Uint32IntPair_T, // 8B
	item_action_type:       ItemEffectActions_T,
	item_requires_title_reflow: bool,
	// pad 3
	anim_tooltip:           f64,
	anim_tooltip_ticks:     u32,
	anim_filter:            f64,
	anim_prompt:            f64,
	anim_prompt_ticks:      u32,
	anim_prompt_move_left:  bool,
	// pad 3
	anim_invalid_action:    f64,
	anim_invalid_action_ticks: u32,
	panel_justify_inverted: bool,
	// pad 3
	invalid_action_type:    ItemEffect_InvalidActionFeedback,
}
#assert(size_of(ItemEffectGUI_T) == 160)

// struct GenericGUIMenu::MailboxGui_t — 232 bytes
MailboxGui_T :: struct {
	parent_gui:             rawptr, // GenericGUIMenu&
	mail_receive_item:      Item,   // 56B
	item_action_type:       MailActions_T,
	// pad 4
	mail_frame:             ^Frame,
	animx:                  f64,
	anim_tooltip:           f64,
	anim_tooltip_ticks:     u32,
	anim_send_item1:        f64,
	anim_send_item1_start_x: i32,
	anim_send_item1_start_y: i32,
	anim_send_item1_dest_x: i32,
	anim_send_item1_dest_y: i32,
	send_item1_uid:         u32,
	anim_recv_item:         f64,
	anim_recv_item_start_x: i32,
	anim_recv_item_start_y: i32,
	anim_recv_item_dest_x:  i32,
	anim_recv_item_dest_y:  i32,
	recv_item_uid:          u32,
	is_interactable:        bool,
	b_open:                 bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	item_desc:              string, // DynamicString
	item_type:              i32,
	item_requires_title_reflow: bool,
	// pad 3
	selected_mail_slot_x:   i32,
	selected_mail_slot_y:   i32,
	// pad 4
	mail_slot_frames:       map[[4]byte]rawptr, // DynamicMapI32T<Frame*>
}
#assert(size_of(MailboxGui_T) == 232)

// struct GenericGUIMenu::FeatherGUI_t — 424 bytes
FeatherGUI_T :: struct {
	parent_gui:             rawptr, // GenericGUIMenu&
	feather_frame:          ^Frame,
	animx:                  f64,
	is_interactable:        bool,
	b_open:                 bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	item_desc:              string, // DynamicString
	item_type:              i32,
	item_action_type:       FeatherActions_T,
	item_requires_title_reflow: bool,
	// pad 3
	anim_drawer:            f64,
	anim_tooltip:           f64,
	anim_tooltip_ticks:     u32,
	anim_filter:            f64,
	anim_prompt:            f64,
	anim_prompt_ticks:      u32,
	anim_prompt_move_left:  bool,
	// pad 3
	anim_invalid_action:    f64,
	anim_invalid_action_ticks: u32,
	b_drawer_open:          bool,
	// pad 3
	inscribe_success_ticks: u32,
	inscribe_success_name:  string, // DynamicString
	drawer_justify_inverted: bool,
	// pad 7
	scroll_percent:         f64,
	scroll_inertia:         f64,
	scroll_setpoint:        i32,
	scroll_animate_x:       f64,
	current_scroll_row:     i32,
	charge_cost_min:        i32,
	charge_cost_max:        i32,
	current_hovering_inscription_label: string, // DynamicString
	current_feather_charge: i32, // Sint32
	change_feather_charge:  i32,
	anim_charge:            f64,
	anim_qty_change:        f64,
	anim_charge_start_ticks: u32,
	highlighted_slot:       i32,
	// pad 4
	label_discoveries:      map[string]DiscoveryAnim_T, // DynamicMapDiscoveryAnim
	invalid_action_type:    Feather_InvalidActionFeedback,
	// pad 4
	selected_feather_slot_x: i32,
	selected_feather_slot_y: i32,
	// pad 4
	feather_slot_frames:    map[[4]byte]rawptr, // DynamicMapI32T<Frame*>
	k_num_inscriptions_to_display_vertical: i32, // const
	sort_type:              SortTypes_T,
	scrolls:                map[string]ScrollEntry_T, // DynamicMapScrollEntry
	sorted_scrolls:         [dynamic]SortedScrollEntry_T, // DynamicArraySortedScrollEntry
	scroll_list_requires_sorting: bool,
}
#assert(size_of(FeatherGUI_T) == 424)

// ScrollEntry_T — 8B (pair<int,bool> mirror)
ScrollEntry_T :: struct {
	first:  i32,
	second: bool,
}
#assert(size_of(ScrollEntry_T) == 8)

// SortedScrollEntry_T — 24B (pair<string, pair<int,bool>> mirror)
SortedScrollEntry_T :: struct {
	first:  string,
	second: ScrollEntry_T,
}
#assert(size_of(SortedScrollEntry_T) == 24)

// DiscoveryAnim_T — 24B (DiscoveryAnim_tMirror)
DiscoveryAnim_T :: struct {
	start_ticks:      u32,
	processed_on_tick: u32,
	name:             string,
}
#assert(size_of(DiscoveryAnim_T) == 24)

// Uint32IntPair_T — 8B (pair<Uint32,int> mirror; e.g. confirmActionOnItemSteps)
Uint32IntPair_T :: struct {
	first:  u32,
	second: i32,
}
#assert(size_of(Uint32IntPair_T) == 8)



// struct GenericGUIMenu::AlchemyGUI_t::AlchemyRecipes_t — 152 bytes
AlchemyRecipes_T :: struct {
	alchemy:                    rawptr, // AlchemyGUI_t&
	justify_left:               bool,
	// pad 7
	animx:                      f64,
	scroll_percent:             f64,
	scroll_inertia:             f64,
	scroll_setpoint:            i32,
	scroll_animate_x:           f64,
	is_interactable:            bool,
	b_open:                     bool,
	b_first_time_snap_cursor:   bool,
	current_scroll_row:         i32,
	panel_justify_inverted:     bool,
	// pad 3
	k_num_recipes_to_display_vertical: i32, // const
	activate_recipe_index:      i32,
	recipe_list:                [dynamic]RecipeEntry_T, // DynamicArray (40B)
	stones:                     map[[4]byte]^Frame_Image_T, // DynamicMapI32T<Frame::image_t*>
}
#assert(size_of(AlchemyRecipes_T) == 152)

// struct GenericGUIMenu::AlchemyGUI_t::AlchemyRecipes_t::RecipeEntry_t — 184 bytes
RecipeEntry_T :: struct {
	result_item:       Item, // 56B
	dummy_potion1:     Item,
	dummy_potion2:     Item,
	x:                 i32,
	y:                 i32,
	base_potion_uid:   u32,
	secondary_potion_uid: u32,
}
#assert(size_of(RecipeEntry_T) == 184)

// struct GenericGUIMenu::AlchemyGUI_t::AlchNotification_t — 64 bytes
AlchNotification_T :: struct {
	img:   string, // DynamicString
	title: string,
	body:  string,
	animx: f64,
	state: i32,
}
#assert(size_of(AlchNotification_T) == 64)

// struct GenericGUIMenu::AlchemyGUI_t — 608 bytes
AlchemyGUI_T :: struct {
	parent_gui:              rawptr, // GenericGUIMenu&
	recipes_frame:           ^Frame,
	recipes:                 AlchemyRecipes_T,
	alchemy_result_potion:   Item, // 56B
	empty_bottle_count:      Item,
	torch_count:             Item,
	item_action_type:        AlchemyActions_T,
	// pad 4
	notifications:           [dynamic]Uint32_AlchNotif_Pair, // vector<pair<Uint32,AlchNotification_t>>
	current_view:            AlchemyView_T,
	has_tin_opener:          bool,
	// pad 3
	alch_frame:              ^Frame,
	animx:                   f64,
	anim_tooltip:            f64,
	anim_tooltip_ticks:      u32,
	anim_potion1:            f64,
	anim_potion1_start_x:    i32,
	anim_potion1_start_y:    i32,
	anim_potion1_dest_x:     i32,
	anim_potion1_dest_y:     i32,
	potion1_uid:             u32,
	anim_potion2:            f64,
	anim_potion2_start_x:    i32,
	anim_potion2_start_y:    i32,
	anim_potion2_dest_x:     i32,
	anim_potion2_dest_y:     i32,
	potion2_uid:             u32,
	anim_potion_result:      f64,
	anim_potion_result_start_x: i32,
	anim_potion_result_start_y: i32,
	anim_potion_result_dest_x: i32,
	anim_potion_result_dest_y: i32,
	potion_result_uid:       u32,
	anim_potion_result_count: i32,
	anim_random_potion_ticks: u32,
	anim_random_potion_updated_this_tick: u32,
	anim_random_potion_variation: i32,
	anim_recipe_auto_add_to_slot1_uid: u32,
	anim_recipe_auto_add_to_slot2_uid: u32,
	is_interactable:         bool,
	b_open:                  bool,
	b_first_time_snap_cursor: bool,
	// pad 1
	item_desc:               string, // DynamicString
	item_type:               i32,
	item_requires_title_reflow: bool,
	item_tooltip_for_recipe: bool,
	selected_alchemy_slot_x: i32,
	selected_alchemy_slot_y: i32,
	// pad 4
	alchemy_slot_frames:     map[[4]byte]rawptr, // DynamicMapI32T<Frame*>
}
#assert(size_of(AlchemyGUI_T) == 608)

// vector<pair<Uint32, AssistNotification_t>> element — 80B
Uint32_AssistNotif_Pair :: struct {
	first:  u32,
	second: AssistNotification_T,
}
#assert(size_of(Uint32_AssistNotif_Pair) == 80)

// vector<pair<Uint32, AlchNotification_t>> element — 72B
Uint32_AlchNotif_Pair :: struct {
	first:  u32,
	second: AlchNotification_T,
}
#assert(size_of(Uint32_AlchNotif_Pair) == 72)


// class GenericGUIMenu — 2592 bytes
GenericGUIMenu :: struct {
	gui_player:                i32,
	gui_type:                  GUICurrentType,
	gui_active:                bool,
	// pad 3
	base_potion:               ^Item,
	secondary_potion:          ^Item,
	alembic_item:              ^Item,
	alembic_entity_uid:        u32,
	experimenting_alchemy:     bool,
	// pad 3
	mailbox_entity_uid:        u32,
	// pad 4
	item_effect_scroll_item:   ^Item,
	item_effect_using_spell:   bool,
	item_effect_using_spellbook: bool,
	item_effect_item_type:     i32,
	item_effect_item_beatitude: i32,
	tinkering_kit_item:        ^Item,
	tinkering_total_items:     list_t, // 16B
	tinkering_total_last_craftable_node: ^node_t,
	tinkering_filter:          TinkeringFilter,
	tinkering_metal_scrap:     map[i32]struct{}, // DynamicSetI32
	tinkering_magic_scrap:     map[i32]struct{},
	tinkering_auto_salvage_kit_item: ^Item,
	tinkering_auto_salvage_this_item: ^Item,
	tinkering_sfx_last_ticks:  u32,
	tinkering_bulk_salvage:    bool,
	// pad 3
	tinkering_bulk_salvage_metal_scrap: i32, // Sint32
	tinkering_bulk_salvage_magic_scrap: i32,
	workstation_entity_uid:    u32,
	// pad 4
	scribing_tool_item:        ^Item,
	scribing_total_items:      list_t,
	scribing_total_last_craftable_node: ^node_t,
	scribing_blank_scroll_target: ^Item,
	scribing_filter:           ScribingFilter,
	transmute_item_target:     ^Item,
	transmute_item_scroll:     i32,
	scribing_last_usage_amount: i32,
	scribing_last_usage_display_timer: i32,
	tinker_gui:                TinkerGUI_T,   // 216B
	item_fx_gui:               ItemEffectGUI_T, // 160B
	assist_shrine_gui:         AssistShrineGUI_T, // 656B
	mailbox_gui:               MailboxGui_T,   // 232B
	feather_gui:               FeatherGUI_T,   // 424B
	alchemy_gui:               AlchemyGUI_T,   // 608B
}
#assert(size_of(GenericGUIMenu) == 2592)
