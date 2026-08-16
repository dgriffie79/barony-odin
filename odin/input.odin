// input.odin -- Odin mirror of input.hpp.
package main


// Input::playerControlType_t
PlayerControlType_T :: enum i32 {
	PLAYER_CONTROLLED_BY_INVALID,
	PLAYER_CONTROLLED_BY_KEYBOARD,
	PLAYER_CONTROLLED_BY_CONTROLLER,
	PLAYER_CONTROLLED_BY_JOYSTICK,
	NUM,
}

// Input::ControllerType
// InputBinding_T — 104B (binding_tMirror: input string + pad/joystick/mouse fields)
InputBinding_T :: struct {
	input:               string,
	analog:              f32,
	binary:              bool,
	consumed:            bool,
	held_ticks:          u32,
	type:                i32, // bindtype_t enum
	keycode:             i64, // SDL_Keycode
	pad_index:           i32,
	pad:                 rawptr, // SDL_GameController* (non-owning)
	pad_axis:            i32,
	pad_button:          i32,
	pad_axis_negative:   bool,
	joystick:            rawptr, // SDL_Joystick* (non-owning)
	joystick_axis:       i32,
	joystick_axis_negative: bool,
	joystick_button:     i32,
	joystick_hat:        i32,
	joystick_hat_state:  u8,
	mouse_button:        i32,
}
#assert(size_of(InputBinding_T) == 104)


ControllerType :: enum i32 {
	PlayStation,
	NintendoSwitch,
	Xbox,
	SteamDeck,
}

// class Input — 240 bytes
// { int player; bool inverted; DynamicMapBinding bindings (32B);
//   DynamicMapStr kb_bindings, gamepad_bindings, joystick_bindings (3×32);
//   DynamicMapStr kb_system_bindings, gamepad_system_bindings, joystick_system_bindings (3×32);
//   bool disabled; }
Input :: struct {
	player:               i32,
	inverted:             bool,
	bindings:             map[string]InputBinding_T, // DynamicMapBinding = DynamicMapStrT<binding_tMirror> (string-keyed)
	kb_bindings:          map[string]string, // DynamicMapStr (string-keyed)
	gamepad_bindings:     map[string]string, // DynamicMapStr (string-keyed)
	joystick_bindings:    map[string]string, // DynamicMapStr (string-keyed)
	kb_system_bindings:   map[string]string, // DynamicMapStr (string-keyed)
	gamepad_system_bindings: map[string]string, // DynamicMapStr (string-keyed)
	joystick_system_bindings: map[string]string, // DynamicMapStr (string-keyed)
	disabled:             bool,
}
#assert(size_of(Input) == 240)
