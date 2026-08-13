//! @file input.hpp

#pragma once

#include "main.hpp"
#include "../odin/containers/dynamic_map.hpp"

#include <string>
#include <unordered_map>
#include <vector>

class GameController;

//! The Input class provides a way to bind physical keys to abstract names like "Move Forward",
//! collect the input data from the physical devices, and provide it back to you for any purpose.
class Input {
public:
	Input() = default;
	Input(const Input&) = delete;
	Input(Input&&) = delete;
	~Input() = default;

	Input& operator=(const Input&) = delete;
	Input& operator=(Input&&) = delete;

	//! one input for each player
	static Input inputs[MAXPLAYERS];
	int player = 0;

	//! set default bindings for all players
	static void defaultBindings();

	//! input mapping
	typedef binding_tMirror binding_t;
;

	//! useful way to get direct access to bindings
	auto& getBindings() const { return bindings; }

	//! useful way to get direct access to more bindings
	auto& getKeyboardBindings() { return kb_bindings; }
	auto& getSystemKeyboardBindings() { return kb_system_bindings; }
	auto& getGamepadBindings() { return gamepad_bindings; }
	auto& getSystemGamepadBindings() { return gamepad_system_bindings; }
	auto& getJoystickBindings() { return joystick_bindings; }
	auto& getSystemJoystickBindings() { return joystick_system_bindings; }
	void setKeyboardBindings(DynamicMapStr& toSet) { kb_bindings = toSet; }
	void setGamepadBindings(DynamicMapStr& toSet) { gamepad_bindings = toSet; }
	void setJoystickBindings(DynamicMapStr& toSet) { joystick_bindings = toSet; }

	//! controller type
	enum playerControlType_t {
		PLAYER_CONTROLLED_BY_INVALID,
		PLAYER_CONTROLLED_BY_KEYBOARD,
		PLAYER_CONTROLLED_BY_CONTROLLER,
		PLAYER_CONTROLLED_BY_JOYSTICK,
		NUM
	};
	playerControlType_t getPlayerControlType();

	//! disable all bindings temporarily
	void setDisabled(bool _disabled) { disabled = _disabled; }

	//! get the status of disabled variable
	auto isDisabled() const { return disabled; }

	//! gets the analog value of a particular input binding
	//! @param binding the binding to query
	//! @return the analog value (range = -1.f : +1.f)
	float analog(const char* binding) const;

	//! gets the binary value of a particular input binding
	//! @param binding the binding to query
	//! @return the bool value (false = not pressed, true = pressed)
	bool binary(const char* binding) const;

	//! gets the binary value of a particular input binding, if it's not been consumed
	//! releasing the input and retriggering it "unconsumes"
	//! @param binding the binding to query
	//! @return the bool value (false = not pressed, true = pressed)
	bool binaryToggle(const char* binding) const;

	//! consume an input action no matter what
	//! @param binding the binding to be consumed
	//! @return true if the toggle was consumed (ie the button was pressed)
	bool consumeBinary(const char* binding);

	//! consume an input action, if it is being pressed
	//! @param binding the binding to be consumed
	//! @return true if the toggle was consumed (ie the button was pressed)
	bool consumeBinaryToggle(const char* binding);

	//! gets the 'held' button value of a particular input binding, if it's not been consumed
	//! requires the button to be held for 'BUTTON_HELD_TICKS' to return true
	//! @param binding the binding to query
	//! @return the bool value (false = not pressed, true = pressed for greater than tick time)
	bool binaryHeldToggle(const char* binding) const;

	//! gets the input mapped to a particular input binding
	//! @param binding the binding to query
	//! @return the input mapped to the given binding
	const char* binding(const char* binding) const;

	//! bind the given action to the given input
	//! @param name the action to bind
	//! @param input the input to bind to the action
	void bind(const char* binding, const char* input);

	//! refresh bindings (eg after a new controller is detected)
	void refresh();

	//! updates the state of all current bindings from the physical devices
	void update();

	//! updates the state of release consumed variable on bindings (needs to happen exactly once per game tick)
	void updateReleaseConsumed();

	//! if true, Y axis for mouse/gamepads/joysticks is inverted
	bool inverted = false;

	//! return the binding_t struct for the input name
	binding_t input(const char* binding) const;
    
    enum class ControllerType {
        PlayStation,
        NintendoSwitch,
        Xbox,
        SteamDeck,
    };
    
    static ControllerType getControllerType(int index);
    static const char* getKeyboardGlyph(int index);
    static const char* getControllerGlyph(int index);
    ControllerType getControllerType() const;
    const char* getKeyboardGlyph() const;
    const char* getControllerGlyph() const;
    
	static std::string getGlyphPathForInput(const char* input, bool pressed = false, ControllerType type = ControllerType::Xbox);
	static std::string getGlyphPathForBinding(const binding_t& binding, bool pressed = false);
	std::string getGlyphPathForBinding(const char* binding, bool pressed = false) const;

	static float getJoystickRebindingDeadzone() { return rebinding_deadzone; }
	static float getAnalogToggleThreshold() { return analogToggleThreshold; }

	//! list of connected input devices
	static std::string lastInputOfAnyKind;
	static int waitingToBindControllerForPlayer;
	static DynamicMapI32T<SDL_GameController*> gameControllers;
	static DynamicMapI32T<SDL_Joystick*> joysticks;
    static std::unordered_map<SDL_Keycode, bool> keys;
	static bool mouseButtons[18];
	static const int MOUSE_WHEEL_UP;
	static const int MOUSE_WHEEL_DOWN;

	//! consume inputs related to the players face-hotbar if it is open
	void consumeBindingsSharedWithFaceHotbar();

	//! consume bindings that all use the same input as given binding
	void consumeBindingsSharedWithBinding(const char* binding);

	//! return true if binding conflicts with system binding (i.e left/right click, scroll wheel)
	bool bindingIsSharedWithKeyboardSystemBinding(const char* binding);
 
    //! get list of bindings for given input
    DynamicArrayStr getBindingsForInput(const char* input) const;
    
private:
	DynamicMapBinding bindings;

	//! bindings written by the config file
	DynamicMapStr kb_bindings;
	DynamicMapStr gamepad_bindings;
	DynamicMapStr joystick_bindings;

	//! default system bindings, likely not changeable
	DynamicMapStr kb_system_bindings;
	DynamicMapStr gamepad_system_bindings;
	DynamicMapStr joystick_system_bindings;

	bool disabled = false;

	//! converts the given input to a boolean value
	//! @return the converted value
	static bool binaryOf(binding_t& binding);

	//! converts the given input to a float value
	//! @return the converted value
	static float analogOf(binding_t& binding);

	//! mouse sensitivity
	static const float sensitivity;

	//! joystick deadzone
	static const float deadzone;

	//! joystick deadzone for rebinding
	static const float rebinding_deadzone;

	//! analog binding threshold
	static const float analogToggleThreshold;

	//! map of scancodes to input names
	static DynamicMapI32 keycodeNames;
	static SDL_Keycode getKeycodeFromName(const char* name);

	//! number of game ticks to consider a button 'held' for long-press actions
	static const Uint32 BUTTON_HELD_TICKS;

	//! number of game ticks to for analog button to repeatedly send
	static const Uint32 BUTTON_ANALOG_REPEAT_TICKS;
};
