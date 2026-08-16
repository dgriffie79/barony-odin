// interface_ui.odin — Odin mirror of interface/ui.hpp.
package main

// enum UIToastNotification::ActionFlags : Uint32
UIToast_ActionFlags :: enum u32 {
	UI_NOTIFICATION_DEFAULT   = 0,
	UI_NOTIFICATION_REMOVABLE = 1 << 0,
}

// enum class UIToastNotification::CardType : Uint32
UIToast_CardType :: enum u32 {
	UI_CARD_DEFAULT,
	UI_CARD_ACHIEVEMENT,
}

// enum class UIToastNotification::CardState : Uint32
UIToast_CardState :: enum u32 {
	UI_CARD_STATE_SHOW,
	UI_CARD_STATE_DOCK,
	UI_CARD_STATE_REMOVED,
}

// class UIToastNotification — 320 bytes (7 owned DynamicStrings + raw ptrs)
UIToastNotification :: struct {
	action_flags:                    u32, // ActionFlags
	card_type:                       UIToast_CardType,
	button_action:                   rawptr, // void (*)()
	show_height:                     i32,
	posx:                            i32,
	posy:                            i32,
	notification_image:              string, // DynamicString (16B)
	is_init:                         bool,
	// pad 3
	textx:                           i32,
	texty:                           i32,
	bodyx:                           i32,
	bodyy:                           i32,
	main_card_hide:                  bool,
	main_card_is_hidden:             bool,
	card_width:                      i32,
	animx:                           i32,
	anim_ticks:                      i32,
	anim_duration:                   i32,
	docked_card_hide:                bool,
	docked_card_is_hidden:           bool,
	docked_card_width:               i32,
	docked_animx:                    i32,
	docked_anim_ticks:               i32,
	docked_anim_duration:            i32,
	card_state:                      UIToast_CardState,
	temporary_card_hide:             bool,
	idle_disappear:                  bool,
	last_interacted_tick:            u32,
	card_update_display_main_text:   bool,
	card_update_display_secondary_text: bool,
	idle_ticks_to_hide:              u32,
	displayed_text:                  string, // DynamicString
	main_card_text:                  string,
	secondary_card_text:             string,
	header_card_text:                string,
	action_text:                     string,
	statistic_update_current:        i32,
	statistic_update_max:            i32,
	pending_statistic_update_current: i32,
	achievement_id:                  string, // DynamicString
	skip_drawing_card_this_tick:     bool,
	b_queued_for_undock:             bool,
	// pad 6
	frame:                           rawptr, // Frame*
	header_field:                    rawptr, // Field*
	main_field:                      rawptr, // Field*
	progress_field:                  rawptr, // Field*
	close_button:                    rawptr, // Button*
	action_button:                   rawptr, // Button*
	frame_image:                     rawptr, // Frame::image_t*
	progress_bar:                    rawptr, // Frame::image_t*
	progress_bar_background:         rawptr, // Frame::image_t*
}
#assert(size_of(UIToastNotification) == 320)

// class UIToastNotificationManager_t — 72 bytes
UIToastNotificationManager_T :: struct {
	undock_ticks:          u32,
	time_to_undock:        u32, // const
	last_undock_tick:      u32,
	b_is_init:             bool,
	// pad 3
	frame:                 ^Frame, // Frame*
	achievements_check:    bool,
	// pad 7
	all_notifications:     [dynamic]UIToastNotification, // DynamicArrayT (40B)
}
#assert(size_of(UIToastNotificationManager_T) == 72)
