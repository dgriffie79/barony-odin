// input.odin -- Odin mirror of input.hpp.
package main

import "containers"

// Input::playerControlType_t
PlayerControlType_T :: enum i32 {
	PLAYER_CONTROLLED_BY_INVALID,
	PLAYER_CONTROLLED_BY_KEYBOARD,
	PLAYER_CONTROLLED_BY_CONTROLLER,
	PLAYER_CONTROLLED_BY_JOYSTICK,
	NUM,
}

// Input::ControllerType
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
	bindings:             map[string]containers.binding_t, // DynamicMapBinding = DynamicMapStrT<binding_tMirror> (string-keyed)
	kb_bindings:          map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	gamepad_bindings:     map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	joystick_bindings:    map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	kb_system_bindings:   map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	gamepad_system_bindings: map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	joystick_system_bindings: map[string]containers.DynamicString, // DynamicMapStr (string-keyed)
	disabled:             bool,
}
#assert(size_of(Input) == 240)
