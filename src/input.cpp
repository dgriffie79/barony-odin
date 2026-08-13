#include "main.hpp"
#include "input.hpp"
#ifdef EDITOR
#ifndef TICKS_PER_SECOND
#define TICKS_PER_SECOND 50
#endif
#else
#include "player.hpp"
#endif
#include "mod_tools.hpp"
#include "ui/MainMenu.hpp"

#include <algorithm>

Input Input::inputs[MAXPLAYERS];

const float Input::sensitivity = 1.f;
const float Input::deadzone = 0.2f;
const float Input::rebinding_deadzone = 0.5f;
const float Input::analogToggleThreshold = .5f;
const Uint32 Input::BUTTON_HELD_TICKS = TICKS_PER_SECOND / 4;
const Uint32 Input::BUTTON_ANALOG_REPEAT_TICKS = TICKS_PER_SECOND / 4;
DynamicMapI32 Input::keycodeNames;
DynamicMapI32T<SDL_GameController*> Input::gameControllers;
DynamicMapI32T<SDL_Joystick*> Input::joysticks;
std::unordered_map<SDL_Keycode, bool> Input::keys;
bool Input::mouseButtons[18] = { false };
const int Input::MOUSE_WHEEL_UP = 16;
const int Input::MOUSE_WHEEL_DOWN = 17;
std::string Input::lastInputOfAnyKind;
int Input::waitingToBindControllerForPlayer = 0;

void Input::defaultBindings() {
	for (int i = 0; i < MAXPLAYERS; ++i) {
		inputs[i].player = i;
		inputs[i].kb_system_bindings.clear();
		inputs[i].gamepad_system_bindings.clear();
		inputs[i].joystick_system_bindings.clear();
	}

	// these bindings should probably not be accessible to the player to change.
	for (int c = 0; c < MAXPLAYERS; ++c) {
		// NOTE disabled on public release!!!
#ifdef NINTENDO_DEBUG
		inputs[c].gamepad_system_bindings["ConsoleCommand1"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str();
		inputs[c].gamepad_system_bindings["ConsoleCommand2"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str();
		inputs[c].gamepad_system_bindings["ConsoleCommand3"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonBack")).c_str();
#endif

		inputs[c].gamepad_system_bindings["Console Command"] = "/";
		inputs[c].gamepad_system_bindings["MenuUp"] = (std::string("Pad") + std::to_string(c) + std::string("DpadY-")).c_str();
		inputs[c].gamepad_system_bindings["MenuLeft"] = (std::string("Pad") + std::to_string(c) + std::string("DpadX-")).c_str();
		inputs[c].gamepad_system_bindings["MenuRight"] = (std::string("Pad") + std::to_string(c) + std::string("DpadX+")).c_str();
		inputs[c].gamepad_system_bindings["MenuDown"] = (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str();
		inputs[c].gamepad_system_bindings["MenuConfirm"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonA")).c_str();
		inputs[c].gamepad_system_bindings["MenuCancel"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
		inputs[c].gamepad_system_bindings["MenuListCancel"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
		inputs[c].gamepad_system_bindings["MenuAlt1"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str();
		inputs[c].gamepad_system_bindings["MenuAlt2"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str();
		inputs[c].gamepad_system_bindings["MenuStart"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonStart")).c_str();
		inputs[c].gamepad_system_bindings["MenuSelect"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonBack")).c_str();
		inputs[c].gamepad_system_bindings["MenuPageLeft"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str();
		inputs[c].gamepad_system_bindings["MenuPageRight"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str();
		inputs[c].gamepad_system_bindings["MenuPageLeftAlt"] = (std::string("Pad") + std::to_string(c) + std::string("LeftTrigger")).c_str();
		inputs[c].gamepad_system_bindings["MenuPageRightAlt"] = (std::string("Pad") + std::to_string(c) + std::string("RightTrigger")).c_str();
		inputs[c].gamepad_system_bindings["AltMenuUp"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftY-")).c_str();
		inputs[c].gamepad_system_bindings["AltMenuLeft"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftX-")).c_str();
		inputs[c].gamepad_system_bindings["AltMenuRight"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftX+")).c_str();
		inputs[c].gamepad_system_bindings["AltMenuDown"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftY+")).c_str();
		inputs[c].gamepad_system_bindings["MenuScrollUp"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str();
		inputs[c].gamepad_system_bindings["MenuScrollLeft"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str();
		inputs[c].gamepad_system_bindings["MenuScrollRight"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str();
		inputs[c].gamepad_system_bindings["MenuScrollDown"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str();

        /*
		inputs[c].gamepad_system_bindings["HotbarFacebarLeft"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonX")).c_str();
		inputs[c].gamepad_system_bindings["HotbarFacebarUp"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str();
		inputs[c].gamepad_system_bindings["HotbarFacebarRight"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
        */
		inputs[c].gamepad_system_bindings["HotbarFacebarModifierLeft"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str();
		inputs[c].gamepad_system_bindings["HotbarFacebarModifierRight"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str();
		//inputs[c].gamepad_system_bindings["HotbarFacebarCancel"] = (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str();

		//inputs[c].bind("HotbarInventoryClearSlot", (std::string("Pad") + std::to_string(c) + std::string("ButtonY")).c_str()));

		inputs[c].gamepad_system_bindings["InventoryMoveUp"] = (std::string("Pad") + std::to_string(c) + std::string("DpadY-")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveLeft"] = (std::string("Pad") + std::to_string(c) + std::string("DpadX-")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveRight"] = (std::string("Pad") + std::to_string(c) + std::string("DpadX+")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveDown"] = (std::string("Pad") + std::to_string(c) + std::string("DpadY+")).c_str();

		inputs[c].gamepad_system_bindings["InventoryMoveUpAnalog"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveLeftAnalog"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveRightAnalog"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str();
		inputs[c].gamepad_system_bindings["InventoryMoveDownAnalog"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str();

		inputs[c].gamepad_system_bindings["InventoryCharacterRotateLeft"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str();
		inputs[c].gamepad_system_bindings["InventoryCharacterRotateRight"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str();

		inputs[c].gamepad_system_bindings["InventoryTooltipPromptAppraise"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftStick")).c_str();
		inputs[c].gamepad_system_bindings["Expand Inventory Tooltip"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightStick")).c_str();

		inputs[c].gamepad_system_bindings["UINavLeftBumper"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str();
		inputs[c].gamepad_system_bindings["UINavRightBumper"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str();
		inputs[c].gamepad_system_bindings["UINavLeftTrigger"] = (std::string("Pad") + std::to_string(c) + std::string("LeftTrigger")).c_str();
		inputs[c].gamepad_system_bindings["UINavRightTrigger"] = (std::string("Pad") + std::to_string(c) + std::string("RightTrigger")).c_str();

		inputs[c].gamepad_system_bindings["Move Forward"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftY-")).c_str();
		inputs[c].gamepad_system_bindings["Move Left"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftX-")).c_str();
		inputs[c].gamepad_system_bindings["Move Backward"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftY+")).c_str();
		inputs[c].gamepad_system_bindings["Move Right"] = (std::string("Pad") + std::to_string(c) + std::string("StickLeftX+")).c_str();
		inputs[c].gamepad_system_bindings["Turn Left"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str();
		inputs[c].gamepad_system_bindings["Turn Right"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str();
		inputs[c].gamepad_system_bindings["Look Up"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str();
		inputs[c].gamepad_system_bindings["Look Down"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str();

		inputs[c].gamepad_system_bindings["PaperDollContextMenu"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftStick")).c_str();
		inputs[c].gamepad_system_bindings["LogHome"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftStick")).c_str();
		inputs[c].gamepad_system_bindings["LogEnd"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightStick")).c_str();
		inputs[c].gamepad_system_bindings["LogPageDown"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightBumper")).c_str();
		inputs[c].gamepad_system_bindings["LogPageUp"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonLeftBumper")).c_str();
		inputs[c].gamepad_system_bindings["LogScrollDown"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str();
		inputs[c].gamepad_system_bindings["LogClose"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
		inputs[c].gamepad_system_bindings["LogScrollUp"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str();
		inputs[c].gamepad_system_bindings["MinimapPing"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonA")).c_str();
		inputs[c].gamepad_system_bindings["MinimapClose"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
		inputs[c].gamepad_system_bindings["MinimapRight"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX+")).c_str();
		inputs[c].gamepad_system_bindings["MinimapLeft"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightX-")).c_str();
		inputs[c].gamepad_system_bindings["MinimapDown"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY+")).c_str();
		inputs[c].gamepad_system_bindings["MinimapUp"] = (std::string("Pad") + std::to_string(c) + std::string("StickRightY-")).c_str();
		inputs[c].gamepad_system_bindings["ResetPortraitRotation"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonRightStick")).c_str();

		inputs[c].gamepad_system_bindings["GamepadLoginA"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonA")).c_str();
		inputs[c].gamepad_system_bindings["GamepadLoginB"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonB")).c_str();
		inputs[c].gamepad_system_bindings["GamepadLoginStart"] = (std::string("Pad") + std::to_string(c) + std::string("ButtonStart")).c_str();

		inputs[c].kb_system_bindings["GamepadScreenshot"] = "F6";
		inputs[c].kb_system_bindings["MenuMouseWheelUp"] = "MouseWheelUp";
		inputs[c].kb_system_bindings["MenuMouseWheelDown"] = "MouseWheelDown";
		inputs[c].kb_system_bindings["MenuMouseWheelUpAlt"] = "MouseWheelUp";
		inputs[c].kb_system_bindings["MenuMouseWheelDownAlt"] = "MouseWheelDown";
		inputs[c].kb_system_bindings["InventoryCharacterRotateLeftMouse"] = "MouseWheelUp";
		inputs[c].kb_system_bindings["InventoryCharacterRotateRightMouse"] = "MouseWheelDown";
		inputs[c].kb_system_bindings["MenuLeftClick"] = "Mouse1";
		inputs[c].kb_system_bindings["MenuMiddleClick"] = "Mouse2";
		inputs[c].kb_system_bindings["MenuRightClick"] = "Mouse3";
		inputs[c].kb_system_bindings["InspectWithMouse"] = "Mouse1";
		inputs[c].kb_system_bindings["MinimapPing"] = "Mouse1";
		inputs[c].kb_system_bindings["ResetPortraitRotation"] = "Mouse2";

		inputs[c].kb_system_bindings["KeyboardLogin"] = "Space";

		inputs[c].kb_system_bindings["LogHome"] = "Home";
		inputs[c].kb_system_bindings["LogEnd"] = "End";
		inputs[c].kb_system_bindings["LogPageDown"] = "PageDown";
		inputs[c].kb_system_bindings["LogPageUp"] = "PageUp";
		inputs[c].kb_system_bindings["LogScrollDown"] = "MouseWheelDown";
		inputs[c].kb_system_bindings["LogScrollUp"] = "MouseWheelUp";

		inputs[c].kb_system_bindings["MenuUp"] = "Up";
		inputs[c].kb_system_bindings["MenuLeft"] = "Left";
		inputs[c].kb_system_bindings["MenuRight"] = "Right";
		inputs[c].kb_system_bindings["MenuDown"] = "Down";
		inputs[c].kb_system_bindings["MenuConfirm"] = "Space";
		inputs[c].kb_system_bindings["MenuCancel"] = "Escape";
		inputs[c].kb_system_bindings["MenuListCancel"] = "Escape";
		//inputs[c].kb_system_bindings["MenuAlt1"] = "Left Shift";
		//inputs[c].kb_system_bindings["MenuAlt2"] = "Left Ctrl";
		inputs[c].kb_system_bindings["MenuStart"] = "Return";
		inputs[c].kb_system_bindings["MenuSelect"] = "Backspace";
		inputs[c].kb_system_bindings["MenuPageLeft"] = "[";
		inputs[c].kb_system_bindings["MenuPageRight"] = "]";
		inputs[c].kb_system_bindings["Console Command"] = "/";
	}
}

float Input::analog(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].analog(binding);
	}
    if (disabled) { return 0.f; }
	if ( !bindings.contains(binding) ) { return 0.f; }
	binding_tMirror _b;
	bindings.get(binding, _b);
	return _b.analog;
}

bool Input::binary(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].binary(binding);
	}
    if (disabled) { return false; }
	if ( !bindings.contains(binding) ) {
		return false;
	} else {
		binding_tMirror _b;
		bindings.get(binding, _b);
		return _b.binary;
	}
	//return b != bindings.end() ? _b.binary : false;
}

bool Input::binaryToggle(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].binaryToggle(binding);
	}
    if (disabled) { return false; }
	if ( !bindings.contains(binding) ) {
		return false;
	} else {
		binding_tMirror _b;
		bindings.get(binding, _b);
		return _b.binary && !_b.consumed;
	}
	//return b != bindings.end() ? _b.binary && !_b.consumed : false;
}

bool Input::consumeBinary(const char* binding) {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].consumeBinary(binding);
	}
	if ( bindings.contains(binding) && !bindings[binding].consumed ) {
		bindings[binding].consumed = true;
		return disabled == false;
	} else {
		return false;
	}
}

bool Input::consumeBinaryToggle(const char* binding) {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].consumeBinaryToggle(binding);
	}
	if ( bindings.contains(binding) && bindings[binding].binary && !bindings[binding].consumed ) {
		bindings[binding].consumed = true;
		return disabled == false;
	} else {
		return false;
	}
}

bool Input::binaryHeldToggle(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].binaryHeldToggle(binding);
	}
    if (disabled) { return false; }
	if ( !bindings.contains(binding) ) { return false; }
	binding_tMirror _b;
	bindings.get(binding, _b);
	return (_b.binary && !_b.consumed && (ticks - _b.heldTicks) > BUTTON_HELD_TICKS);
}

const char* Input::binding(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].binding(binding);
	}
	if ( !bindings.contains(binding) ) { return ""; }
	binding_tMirror _b;
	bindings.get(binding, _b);
	return _b.input.c_str();
}

void Input::refresh() {
	bindings.clear();
	defaultBindings();
#ifndef EDITOR
	DynamicMapStr::Entry kbEntries[128];
	int32_t kbCount = kb_system_bindings.entryList(kbEntries, 128);
	for ( int32_t bi = 0; bi < kbCount; ++bi )
	{
		bind(kbEntries[bi].key, kbEntries[bi].value.c_str());
	}
	if ( getPlayerControlType() == playerControlType_t::PLAYER_CONTROLLED_BY_KEYBOARD )
	{
		printlog("keyboard bindings for player %d", player);
	    DynamicMapStr::Entry kbUserEntries[128];
	    int32_t kbUserCount = getKeyboardBindings().entryList(kbUserEntries, 128);
	    for ( int32_t bi = 0; bi < kbUserCount; ++bi )
	    {
		    bind(kbUserEntries[bi].key, kbUserEntries[bi].value.c_str());
	    }
	}
	if ( getPlayerControlType() == playerControlType_t::PLAYER_CONTROLLED_BY_CONTROLLER )
	{
		printlog("controller bindings for player %d", player);
		DynamicMapStr::Entry gamepadEntries[128];
		int32_t gamepadCount = gamepad_system_bindings.entryList(gamepadEntries, 128);
		for ( int32_t bi = 0; bi < gamepadCount; ++bi )
		{
			bind(gamepadEntries[bi].key, gamepadEntries[bi].value.c_str());
		}

		DynamicString prefix;
		prefix.append("Pad");
		prefix.append(std::to_string(player));
		DynamicMapStr::Entry gamepadUserEntries[128];
		int32_t gamepadUserCount = getGamepadBindings().entryList(gamepadUserEntries, 128);
		for ( int32_t bi = 0; bi < gamepadUserCount; ++bi ) {
			if ( strcmp(gamepadUserEntries[bi].value.c_str(), MainMenu::hiddenBinding) == 0 )
			{
				if ( bindings.contains(gamepadUserEntries[bi].key) && bindings[gamepadUserEntries[bi].key].isBindingUsingGamepad() )
				{
					// hidden binding, don't override existing bind by the defaults.
					continue;
				}
			}

			bind(gamepadUserEntries[bi].key, (prefix + gamepadUserEntries[bi].value.c_str()).c_str());
			if ( strcmp(gamepadUserEntries[bi].key, "Voice Chat") == 0 ) // special binding can be on gamepad or keyboard
			{
				if ( bindings[gamepadUserEntries[bi].key].type == binding_t::INVALID )
				{
					// try bind to keyboard as fallback
					bind(gamepadUserEntries[bi].key, gamepadUserEntries[bi].value.c_str());
				}
			}
		}
	}
	if ( getPlayerControlType() == playerControlType_t::PLAYER_CONTROLLED_BY_JOYSTICK )
	{
		printlog("joystick bindings for player %d", player);
		DynamicMapStr::Entry joystickEntries[128];
		int32_t joystickCount = joystick_system_bindings.entryList(joystickEntries, 128);
		for ( int32_t bi = 0; bi < joystickCount; ++bi )
		{
			bind(joystickEntries[bi].key, joystickEntries[bi].value.c_str());
		}

		DynamicString prefix;
		prefix.append("Joy");
		prefix.append(std::to_string(player));
		DynamicMapStr::Entry joystickUserEntries[128];
		int32_t joystickUserCount = getJoystickBindings().entryList(joystickUserEntries, 128);
		for ( int32_t bi = 0; bi < joystickUserCount; ++bi ) {
		    bind(joystickUserEntries[bi].key, (prefix + joystickUserEntries[bi].value.c_str()).c_str());
		}
	}
#endif // !EDITOR
}

Input::binding_t Input::input(const char* binding) const {
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].input(binding);
	}
	if ( !bindings.contains(binding) ) { return Input::binding_t(); }
	binding_tMirror _b;
	bindings.get(binding, _b);
	return _b;
}

Input::ControllerType Input::getControllerType() const {
    return getControllerType(player);
}

#ifndef EDITOR
static ConsoleVariable<int> cvar_forceGlyphs("/forceglyphs", -1, "Force use of specific controller glyphs");
#endif

Input::ControllerType Input::getControllerType(int index) {
#if defined(EDITOR)
    return ControllerType::Xbox;
#else
    if (*cvar_forceGlyphs >= 0) {
        return (ControllerType)*cvar_forceGlyphs;
    } else {
        // SDL lets us differentiate controller types
        const int device = ::inputs.getControllerID(index);
        auto type = SDL_GameControllerTypeForIndex(device);
        switch(type) {
        default:
        case SDL_CONTROLLER_TYPE_UNKNOWN: return ControllerType::Xbox;
        case SDL_CONTROLLER_TYPE_XBOX360: return ControllerType::Xbox;
        case SDL_CONTROLLER_TYPE_XBOXONE: return ControllerType::Xbox;
        case SDL_CONTROLLER_TYPE_PS3: return ControllerType::PlayStation;
        case SDL_CONTROLLER_TYPE_PS4: return ControllerType::PlayStation;
        case SDL_CONTROLLER_TYPE_PS5: return ControllerType::PlayStation;
        case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO: return ControllerType::NintendoSwitch;
        //case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT: return ControllerType::NintendoSwitch;
        //case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT: return ControllerType::NintendoSwitch;
        //case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR: return ControllerType::NintendoSwitch;
        }
    }
#endif
}

const char* Input::getKeyboardGlyph(int index) {
    return "*#images/ui/Glyphs/G_Control_KBM_01.png";
}

const char* Input::getKeyboardGlyph() const {
    return "*#images/ui/Glyphs/G_Control_KBM_01.png";
}

const char* Input::getControllerGlyph(int index) {
    switch (getControllerType(index)) {
    default:
    case ControllerType::Xbox:
        return "*#images/ui/Glyphs/G_Control_Xbox_02.png";
    case ControllerType::NintendoSwitch:
        return "*#images/ui/Glyphs/G_Control_Switch_01.png";
    case ControllerType::PlayStation:
        return "*#images/ui/Glyphs/G_Control_PS5_01.png";
    }
}

const char* Input::getControllerGlyph() const {
    return getControllerGlyph(player);
}

std::string Input::getGlyphPathForInput(const char* input, bool pressed, ControllerType type)
{
#ifndef EDITOR
    if (*cvar_hideGlyphs) {
        return "";
    }
#endif
    if (!input || !input[0]) {
        return "";
    }
    
    // base path
    const DynamicString rootPath = "images/ui/Glyphs/";
    
    if (type == ControllerType::PlayStation) {
        static const std::unordered_map<std::string, std::pair<std::string, std::string>> mappings = {
            {"Mouse1", {"Mouse/Mouse_LClick_Pressed_00.png", "Mouse/Mouse_LClick_Unpressed_00.png"}},
            {"Mouse2", {"Mouse/Mouse_MClick_Pressed_00.png", "Mouse/Mouse_MClick_Unpressed_00.png"}},
            {"Mouse3", {"Mouse/Mouse_RClick_Pressed_00.png", "Mouse/Mouse_RClick_Unpressed_00.png"}},
			{"Mouse4", {"Mouse/Mouse_M4_Pressed_00.png", "Mouse/Mouse_M4_Unpressed_00.png"}},
			{"Mouse5", {"Mouse/Mouse_M5_Pressed_00.png", "Mouse/Mouse_M5_Unpressed_00.png"}},
            {"MouseWheelDown", {"Mouse/Mouse_MWheelDown_Pressed_00.png", "Mouse/Mouse_MWheelDown_Unpressed_00.png"}},
            {"MouseWheelUp", {"Mouse/Mouse_MWheelUp_Pressed_00.png", "Mouse/Mouse_MWheelUp_Unpressed_00.png"}},
            {"ButtonA", {"G_PS_X00.png", "G_PS_X_Press00.png"}},
            {"ButtonB", {"G_PS_O00.png", "G_PS_O_Press00.png"}},
            {"ButtonX", {"G_PS_Box00.png", "G_PS_Box_Press00.png"}},
            {"ButtonY", {"G_PS_Tri00.png", "G_PS_Tri_Press00.png"}},
            {"ButtonLeftBumper", {"Button_PS_L1_00.png", "Button_PS_L1_Press_00.png"}},
            {"ButtonRightBumper", {"Button_PS_R1_00.png", "Button_PS_R1_Press_00.png"}},
            {"ButtonLeftStick", {"Stick_PS_L_00.png", "Stick_PS_L_Pressed_00.png"}},
            {"ButtonRightStick", {"Stick_PS_R_00.png", "Stick_PS_R_Pressed_00.png"}},
            {"ButtonStart", {"Button_OptC.png", "Button_OptC_Press00.png"}},
            {"ButtonBack", {"Button_Touchpad_PS5_00B.png", "Button_Touchpad_PS5_00C.png"}},
            {"StickLeftX-", {"Stick_PS_L_Left_00.png", "Stick_PS_L_Left_Pressed_00.png"}},
            {"StickLeftX+", {"Stick_PS_L_Right_00.png", "Stick_PS_L_Right_Pressed_00.png"}},
            {"StickLeftY-", {"Stick_PS_L_Up_00.png", "Stick_PS_L_Up_Pressed_00.png"}},
            {"StickLeftY+", {"Stick_PS_L_Down_00.png", "Stick_PS_L_Down_Pressed_00.png"}},
            {"StickRightX-", {"Stick_PS_R_Left_00.png", "Stick_PS_R_Left_Pressed_00.png"}},
            {"StickRightX+", {"Stick_PS_R_Right_00.png", "Stick_PS_R_Right_Pressed_00.png"}},
            {"StickRightY-", {"Stick_PS_R_Up_00.png", "Stick_PS_R_Up_Pressed_00.png"}},
            {"StickRightY+", {"Stick_PS_R_Down_00.png", "Stick_PS_R_Down_Pressed_00.png"}},
            {"LeftTrigger", {"Button_PS_L2_00.png", "Button_PS_L2_Press_00.png"}},
            {"RightTrigger", {"Button_PS_R2_00.png", "Button_PS_R2_Press_00.png"}},
            {"DpadX-", {"G_Direct_Left_Press00.png", "G_Direct_00.png"}},
            {"DpadX+", {"G_Direct_Right_Press00.png", "G_Direct_00.png"}},
            {"DpadY-", {"G_Direct_Up_Press00.png", "G_Direct_00.png"}},
            {"DpadY+", {"G_Direct_Down_Press00.png", "G_Direct_00.png"}},
        };
        
        // look for glyph in table
        auto find = mappings.find(input);
        if (find != mappings.end()) {
            auto& glyphs = find->second;
            return pressed ? rootPath + glyphs.second : rootPath + glyphs.first;
        }
    }
    else if (type == ControllerType::NintendoSwitch) {
        static const std::unordered_map<std::string, std::pair<std::string, std::string>> mappings = {
            {"Mouse1", {"Mouse/Mouse_LClick_Pressed_00.png", "Mouse/Mouse_LClick_Unpressed_00.png"}},
            {"Mouse2", {"Mouse/Mouse_MClick_Pressed_00.png", "Mouse/Mouse_MClick_Unpressed_00.png"}},
            {"Mouse3", {"Mouse/Mouse_RClick_Pressed_00.png", "Mouse/Mouse_RClick_Unpressed_00.png"}},
			{"Mouse4", {"Mouse/Mouse_M4_Pressed_00.png", "Mouse/Mouse_M4_Unpressed_00.png"}},
			{"Mouse5", {"Mouse/Mouse_M5_Pressed_00.png", "Mouse/Mouse_M5_Unpressed_00.png"}},
            {"MouseWheelDown", {"Mouse/Mouse_MWheelDown_Pressed_00.png", "Mouse/Mouse_MWheelDown_Unpressed_00.png"}},
            {"MouseWheelUp", {"Mouse/Mouse_MWheelUp_Pressed_00.png", "Mouse/Mouse_MWheelUp_Unpressed_00.png"}},
            {"ButtonA", {"Button_Xbox_DarkA_00.png", "Button_Xbox_DarkA_Press_00.png"}},
            {"ButtonB", {"Button_Xbox_DarkB_00.png", "Button_Xbox_DarkB_Press_00.png"}},
            {"ButtonX", {"Button_Xbox_DarkX_00.png", "Button_Xbox_DarkX_Press_00.png"}},
            {"ButtonY", {"Button_Xbox_DarkY_00.png", "Button_Xbox_DarkY_Press_00.png"}},
            {"ButtonLeftBumper", {"G_Switch_L00.png", "G_Switch_L_Press00.png"}},
            {"ButtonRightBumper", {"G_Switch_R00.png", "G_Switch_R_Press00.png"}},
            {"ButtonLeftStick", {"Stick_Switch_L_00.png", "Stick_Switch_L_Pressed_00.png"}},
            {"ButtonRightStick", {"Stick_Switch_R_00.png", "Stick_Switch_R_Pressed_00.png"}},
            {"ButtonStart", {"PlusMed00.png", "PlusMed_Press00.png"}},
            {"ButtonBack", {"MinusMed00.png", "MinusMed_Press00.png"}},
            {"StickLeftX-", {"Stick_Switch_L_Left_00.png", "Stick_Switch_L_Left_Pressed_00.png"}},
            {"StickLeftX+", {"Stick_Switch_L_Right_00.png", "Stick_Switch_L_Right_Pressed_00.png"}},
            {"StickLeftY-", {"Stick_Switch_L_Up_00.png", "Stick_Switch_L_Up_Pressed_00.png"}},
            {"StickLeftY+", {"Stick_Switch_L_Down_00.png", "Stick_Switch_L_Down_Pressed_00.png"}},
            {"StickRightX-", {"Stick_Switch_R_Left_00.png", "Stick_Switch_R_Left_Pressed_00.png"}},
            {"StickRightX+", {"Stick_Switch_R_Right_00.png", "Stick_Switch_R_Right_Pressed_00.png"}},
            {"StickRightY-", {"Stick_Switch_R_Up_00.png", "Stick_Switch_R_Up_Pressed_00.png"}},
            {"StickRightY+", {"Stick_Switch_R_Down_00.png", "Stick_Switch_R_Down_Pressed_00.png"}},
            {"LeftTrigger", {"G_Switch_ZL00.png", "G_Switch_ZL_Press00.png"}},
            {"RightTrigger", {"G_Switch_ZR00.png", "G_Switch_ZR_Press00.png"}},
            {"DpadX-", {"G_Switch_Direct_Left_Press00.png", "G_Switch_Direct_00.png"}},
            {"DpadX+", {"G_Switch_Direct_Right_Press00.png", "G_Switch_Direct_00.png"}},
            {"DpadY-", {"G_Switch_Direct_Up_Press00.png", "G_Switch_Direct_00.png"}},
            {"DpadY+", {"G_Switch_Direct_Down_Press00.png", "G_Switch_Direct_00.png"}},
        };
        
        // look for glyph in table
        auto find = mappings.find(input);
        if (find != mappings.end()) {
            auto& glyphs = find->second;
            return pressed ? rootPath + glyphs.second : rootPath + glyphs.first;
        }
    }
    else if (type == ControllerType::Xbox) {
        static const std::unordered_map<std::string, std::pair<std::string, std::string>> mappings =
        {
            {"Mouse1", {"Mouse/Mouse_LClick_Pressed_00.png", "Mouse/Mouse_LClick_Unpressed_00.png"}},
            {"Mouse2", {"Mouse/Mouse_MClick_Pressed_00.png", "Mouse/Mouse_MClick_Unpressed_00.png"}},
            {"Mouse3", {"Mouse/Mouse_RClick_Pressed_00.png", "Mouse/Mouse_RClick_Unpressed_00.png"}},
			{"Mouse4", {"Mouse/Mouse_M4_Pressed_00.png", "Mouse/Mouse_M4_Unpressed_00.png"}},
			{"Mouse5", {"Mouse/Mouse_M5_Pressed_00.png", "Mouse/Mouse_M5_Unpressed_00.png"}},
            {"MouseWheelDown", {"Mouse/Mouse_MWheelDown_Pressed_00.png", "Mouse/Mouse_MWheelDown_Unpressed_00.png"}},
            {"MouseWheelUp", {"Mouse/Mouse_MWheelUp_Pressed_00.png", "Mouse/Mouse_MWheelUp_Unpressed_00.png"}},
            {"ButtonA", {"Button_Xbox_DarkA_00.png", "Button_Xbox_DarkA_Press_00.png"}},
            {"ButtonB", {"Button_Xbox_DarkB_00.png", "Button_Xbox_DarkB_Press_00.png"}},
            {"ButtonX", {"Button_Xbox_DarkX_00.png", "Button_Xbox_DarkX_Press_00.png"}},
            {"ButtonY", {"Button_Xbox_DarkY_00.png", "Button_Xbox_DarkY_Press_00.png"}},
            {"ButtonLeftBumper", {"Button_Xbox_LB_00.png", "Button_Xbox_LB_Press_00.png"}},
            {"ButtonRightBumper", {"Button_Xbox_RB_00.png", "Button_Xbox_RB_Press_00.png"}},
            {"ButtonLeftStick", {"Stick_Xbox_L_00.png", "Stick_Xbox_L_Pressed_00.png"}},
            {"ButtonRightStick", {"Stick_Xbox_R_00.png", "Stick_Xbox_R_Pressed_00.png"}},
            {"ButtonStart", {"Button_Xbox_Menu_00.png", "Button_Xbox_Menu_Press_00.png"}},
            {"ButtonBack", {"Button_Xbox_View_00.png", "Button_Xbox_View_Press_00.png"}},
            {"StickLeftX-", {"Stick_Xbox_L_Left_00.png", "Stick_Xbox_L_Left_Pressed_00.png"}},
            {"StickLeftX+", {"Stick_Xbox_L_Right_00.png", "Stick_Xbox_L_Right_Pressed_00.png"}},
            {"StickLeftY-", {"Stick_Xbox_L_Up_00.png", "Stick_Xbox_L_Up_Pressed_00.png"}},
            {"StickLeftY+", {"Stick_Xbox_L_Down_00.png", "Stick_Xbox_L_Down_Pressed_00.png"}},
            {"StickRightX-", {"Stick_Xbox_R_Left_00.png", "Stick_Xbox_R_Left_Pressed_00.png"}},
            {"StickRightX+", {"Stick_Xbox_R_Right_00.png", "Stick_Xbox_R_Right_Pressed_00.png"}},
            {"StickRightY-", {"Stick_Xbox_R_Up_00.png", "Stick_Xbox_R_Up_Pressed_00.png"}},
            {"StickRightY+", {"Stick_Xbox_R_Down_00.png", "Stick_Xbox_R_Down_Pressed_00.png"}},
            {"LeftTrigger", {"Button_Xbox_LT_00.png", "Button_Xbox_LT_Press_00.png"}},
            {"RightTrigger", {"Button_Xbox_RT_00.png", "Button_Xbox_RT_Press_00.png"}},
            {"DpadX-", {"G_Direct_Left_Press00.png", "G_Direct_00.png"}},
            {"DpadX+", {"G_Direct_Right_Press00.png", "G_Direct_00.png"}},
            {"DpadY-", {"G_Direct_Up_Press00.png", "G_Direct_00.png"}},
            {"DpadY+", {"G_Direct_Down_Press00.png", "G_Direct_00.png"}},
        };
        
        // look for glyph in table
        auto find = mappings.find(input);
        if (find != mappings.end()) {
            auto& glyphs = find->second;
            return pressed ? rootPath + glyphs.second : rootPath + glyphs.first;
        }
    }
    else if (type == ControllerType::SteamDeck) {
        static const std::unordered_map<std::string, std::pair<std::string, std::string>> mappings =
        {
            {"Mouse1", {"Mouse/Mouse_LClick_Pressed_00.png", "Mouse/Mouse_LClick_Unpressed_00.png"}},
            {"Mouse2", {"Mouse/Mouse_MClick_Pressed_00.png", "Mouse/Mouse_MClick_Unpressed_00.png"}},
            {"Mouse3", {"Mouse/Mouse_RClick_Pressed_00.png", "Mouse/Mouse_RClick_Unpressed_00.png"}},
			{"Mouse4", {"Mouse/Mouse_M4_Pressed_00.png", "Mouse/Mouse_M4_Unpressed_00.png"}},
			{"Mouse5", {"Mouse/Mouse_M5_Pressed_00.png", "Mouse/Mouse_M5_Unpressed_00.png"}},
            {"MouseWheelDown", {"Mouse/Mouse_MWheelDown_Pressed_00.png", "Mouse/Mouse_MWheelDown_Unpressed_00.png"}},
            {"MouseWheelUp", {"Mouse/Mouse_MWheelUp_Pressed_00.png", "Mouse/Mouse_MWheelUp_Unpressed_00.png"}},
            {"ButtonA", {"Button_Xbox_DarkA_00.png", "Button_Xbox_DarkA_Press_00.png"}},
            {"ButtonB", {"Button_Xbox_DarkB_00.png", "Button_Xbox_DarkB_Press_00.png"}},
            {"ButtonX", {"Button_Xbox_DarkX_00.png", "Button_Xbox_DarkX_Press_00.png"}},
            {"ButtonY", {"Button_Xbox_DarkY_00.png", "Button_Xbox_DarkY_Press_00.png"}},
            {"ButtonLeftBumper", {"Button_PS_L1_00.png", "Button_PS_L1_Press_00.png"}},
            {"ButtonRightBumper", {"Button_PS_R1_00.png", "Button_PS_R1_Press_00.png"}},
            {"ButtonLeftStick", {"Stick_Xbox_L_00.png", "Stick_Xbox_L_Pressed_00.png"}},
            {"ButtonRightStick", {"Stick_Xbox_R_00.png", "Stick_Xbox_R_Pressed_00.png"}},
            {"ButtonStart", {"Button_Xbox_Menu_00.png", "Button_Xbox_Menu_Press_00.png"}},
            {"ButtonBack", {"Button_Xbox_View_00.png", "Button_Xbox_View_Press_00.png"}},
            {"StickLeftX-", {"Stick_Xbox_L_Left_00.png", "Stick_Xbox_L_Left_Pressed_00.png"}},
            {"StickLeftX+", {"Stick_Xbox_L_Right_00.png", "Stick_Xbox_L_Right_Pressed_00.png"}},
            {"StickLeftY-", {"Stick_Xbox_L_Up_00.png", "Stick_Xbox_L_Up_Pressed_00.png"}},
            {"StickLeftY+", {"Stick_Xbox_L_Down_00.png", "Stick_Xbox_L_Down_Pressed_00.png"}},
            {"StickRightX-", {"Stick_Xbox_R_Left_00.png", "Stick_Xbox_R_Left_Pressed_00.png"}},
            {"StickRightX+", {"Stick_Xbox_R_Right_00.png", "Stick_Xbox_R_Right_Pressed_00.png"}},
            {"StickRightY-", {"Stick_Xbox_R_Up_00.png", "Stick_Xbox_R_Up_Pressed_00.png"}},
            {"StickRightY+", {"Stick_Xbox_R_Down_00.png", "Stick_Xbox_R_Down_Pressed_00.png"}},
            {"LeftTrigger", {"Button_PS_L2_00.png", "Button_PS_L2_Press_00.png"}},
            {"RightTrigger", {"Button_PS_R2_00.png", "Button_PS_R2_Press_00.png"}},
            {"DpadX-", {"G_Direct_Left_Press00.png", "G_Direct_00.png"}},
            {"DpadX+", {"G_Direct_Right_Press00.png", "G_Direct_00.png"}},
            {"DpadY-", {"G_Direct_Up_Press00.png", "G_Direct_00.png"}},
            {"DpadY+", {"G_Direct_Down_Press00.png", "G_Direct_00.png"}},
        };
        
        // look for glyph in table
        auto find = mappings.find(input);
        if (find != mappings.end()) {
            auto& glyphs = find->second;
            return pressed ? rootPath + glyphs.second : rootPath + glyphs.first;
        }
    }

	// if the above lookups don't work, it's probably a keyboard glyph
	auto keycode = getKeycodeFromName(input);
	return GlyphHelper.getGlyphPath(keycode, pressed);
}

std::string Input::getGlyphPathForBinding(const char* binding, bool pressed) const
{
#ifndef EDITOR
	if (*cvar_hideGlyphs) {
		return "";
	}
#endif
	return getGlyphPathForBinding(input(binding), pressed);
}

std::string Input::getGlyphPathForBinding(const binding_t& binding, bool pressed)
{
	const auto prefix = binding.input.substr(0, 3);
	if (prefix == "Pad" || prefix == "Joy") {
        const int index = binding.input[3] - '0';
		return getGlyphPathForInput(binding.input.c_str() + 4, pressed, getControllerType(index));
	} else {
		return getGlyphPathForInput(binding.input.c_str(), pressed, ControllerType::Xbox);
	}
}

void Input::bind(const char* binding, const char* input) {
	binding_tMirror& _b = bindings[binding];
	_b.input.assign(input);
	if (input == nullptr) {
		_b.type = binding_t::INVALID;
		return;
	}

	size_t len = strlen(input);
	if (len >= 3 && strncmp(input, "Pad", 3) == 0) {
		// game controller

		char* type = nullptr;
		Uint32 index = (Uint32)strtol((const char*)(input + 3), &type, 10);
		bool foundControllerForPlayer = false;
		SDL_GameController* pad = nullptr;
#ifndef EDITOR
		if ( auto controller = ::inputs.getController(player) )
		{
			foundControllerForPlayer = true;
			pad = controller->getControllerDevice();
		}
#endif
		if ( foundControllerForPlayer ) {
			_b.pad = pad;
			_b.padIndex = index;
			if (strncmp(type, "Button", 6) == 0) {
				if (strcmp((const char*)(type + 6), "A") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_A;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "B") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_B;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "X") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_X;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Y") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_Y;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Back") == 0) {
					_b.padButton = getControllerType() == ControllerType::PlayStation ?
                        SDL_CONTROLLER_BUTTON_TOUCHPAD : SDL_CONTROLLER_BUTTON_BACK;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "Start") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_START;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "LeftStick") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_LEFTSTICK;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "RightStick") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "LeftBumper") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 6), "RightBumper") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else {
					_b.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "StickLeft", 9) == 0) {
				if (strcmp((const char*)(type + 9), "X-") == 0) {
					_b.padAxisNegative = true;
					_b.padAxis = SDL_CONTROLLER_AXIS_LEFTX;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "X+") == 0) {
					_b.padAxisNegative = false;
					_b.padAxis = SDL_CONTROLLER_AXIS_LEFTX;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "Y-") == 0) {
					_b.padAxisNegative = true;
					_b.padAxis = SDL_CONTROLLER_AXIS_LEFTY;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 9), "Y+") == 0) {
					_b.padAxisNegative = false;
					_b.padAxis = SDL_CONTROLLER_AXIS_LEFTY;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else {
					_b.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "StickRight", 10) == 0) {
				if (strcmp((const char*)(type + 10), "X-") == 0) {
					_b.padAxisNegative = true;
					_b.padAxis = SDL_CONTROLLER_AXIS_RIGHTX;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "X+") == 0) {
					_b.padAxisNegative = false;
					_b.padAxis = SDL_CONTROLLER_AXIS_RIGHTX;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "Y-") == 0) {
					_b.padAxisNegative = true;
					_b.padAxis = SDL_CONTROLLER_AXIS_RIGHTY;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else if (strcmp((const char*)(type + 10), "Y+") == 0) {
					_b.padAxisNegative = false;
					_b.padAxis = SDL_CONTROLLER_AXIS_RIGHTY;
					_b.type = binding_t::CONTROLLER_AXIS;
					return;
				} else {
					_b.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "Dpad", 4) == 0) {
				if (strcmp((const char*)(type + 4), "X-") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "X+") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "Y-") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_DPAD_UP;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else if (strcmp((const char*)(type + 4), "Y+") == 0) {
					_b.padButton = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
					_b.type = binding_t::CONTROLLER_BUTTON;
					return;
				} else {
					_b.type = binding_t::INVALID;
					return;
				}
			} else if (strncmp(type, "LeftTrigger", 11) == 0) {
				_b.padAxisNegative = false;
				_b.padAxis = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
				_b.type = binding_t::CONTROLLER_AXIS;
				return;
			} else if (strncmp(type, "RightTrigger", 12) == 0) {
				_b.padAxisNegative = false;
				_b.padAxis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
				_b.type = binding_t::CONTROLLER_AXIS;
				return;
			} else {
				_b.type = binding_t::INVALID;
				return;
			}
		} else {
			_b.type = binding_t::INVALID;
			return;
		}
	} else if (len >= 3 && strncmp(input, "Joy", 3) == 0) {
		// joystick

		char* type = nullptr;
		Uint32 index = (Uint32)strtol((const char*)(input + 3), &type, 10);
		auto& list = joysticks;
		auto find = list.find(index);
		if (find != list.end()) {
			SDL_Joystick* joystick = (*find).second;
			_b.joystick = joystick;
			if (strncmp(type, "Button", 6) == 0) {
				_b.type = binding_t::JOYSTICK_BUTTON;
				_b.joystickButton = (Uint32)strtol((const char*)(type + 6), nullptr, 10);
				return;
			} else if (strncmp(type, "Axis-", 5) == 0) {
				_b.type = binding_t::JOYSTICK_AXIS;
				_b.joystickAxisNegative = true;
				_b.joystickAxis = (Uint32)strtol((const char*)(type + 5), nullptr, 10);
				return;
			} else if (strncmp(type, "Axis+", 5) == 0) {
				_b.type = binding_t::JOYSTICK_AXIS;
				_b.joystickAxisNegative = false;
				_b.joystickAxis = (Uint32)strtol((const char*)(type + 5), nullptr, 10);
				return;
			} else if (strncmp(type, "Hat", 3) == 0) {
				_b.type = binding_t::JOYSTICK_HAT;
				_b.joystickHat = (Uint32)strtol((const char*)(type + 3), nullptr, 10);
				if (type[3]) {
					if (strncmp((const char*)(type + 4), "LeftUp", 6) == 0) {
						_b.joystickHatState = SDL_HAT_LEFTUP;
						return;
					} else if (strncmp((const char*)(type + 4), "Up", 2) == 0) {
						_b.joystickHatState = SDL_HAT_UP;
						return;
					} else if (strncmp((const char*)(type + 4), "RightUp", 7) == 0) {
						_b.joystickHatState = SDL_HAT_RIGHTUP;
						return;
					} else if (strncmp((const char*)(type + 4), "Right", 5) == 0) {
						_b.joystickHatState = SDL_HAT_RIGHT;
						return;
					} else if (strncmp((const char*)(type + 4), "RightDown", 9) == 0) {
						_b.joystickHatState = SDL_HAT_RIGHTDOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "Down", 4) == 0) {
						_b.joystickHatState = SDL_HAT_DOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "LeftDown", 8) == 0) {
						_b.joystickHatState = SDL_HAT_LEFTDOWN;
						return;
					} else if (strncmp((const char*)(type + 4), "Left", 4) == 0) {
						_b.joystickHatState = SDL_HAT_LEFT;
						return;
					} else if (strncmp((const char*)(type + 4), "Centered", 8) == 0) {
						_b.joystickHatState = SDL_HAT_CENTERED;
						return;
					} else {
						_b.type = binding_t::INVALID;
						return;
					}
				}
			} else {
				_b.type = binding_t::INVALID;
				return;
			}
		}

		return;
	} else if (len >= 5 && strncmp(input, "Mouse", 5) == 0) {
		// mouse
		_b.type = binding_t::MOUSE_BUTTON;
		if ( (strncmp((const char*)(input + 5), "WheelUp", 7) == 0) )
		{
			_b.mouseButton = MOUSE_WHEEL_UP;
			return;
		}
		else if ( (strncmp((const char*)(input + 5), "WheelDown", 9) == 0) )
		{
			_b.mouseButton = MOUSE_WHEEL_DOWN;
			return;
		}
		Uint32 index = (Uint32)strtol((const char*)(input + 5), nullptr, 10);
		int result = std::min(index, 15U);
		_b.mouseButton = result;
		return;
	} else {
		// keyboard
		_b.type = binding_t::KEYBOARD;
		_b.keycode = getKeycodeFromName(input);
		return;
	}
}

void Input::update() {
	std::vector<const char*> _keys;
	bindings.keys(_keys);
	for ( const char* _k : _keys ) {
		auto& binding = bindings[_k];
		//const float oldAnalog = binding.analog;
		binding.analog = analogOf(binding);
		const bool oldBinary = binding.binary;
		binding.binary = binaryOf(binding);
		if (oldBinary != binding.binary) {
			if (binding.binary) {
				if (binding.heldTicks == 0) {
					binding.heldTicks = ticks; // start the held detection counter
				}
			} else {
			    binding.consumed = false;
				binding.heldTicks = 0; // button not pressed, reset the held counter
			}
		}
	}
}

bool Input::binaryOf(binding_t& binding) {
	if (binding.type == binding_t::CONTROLLER_AXIS ||
		binding.type == binding_t::CONTROLLER_BUTTON) {
		SDL_GameController* pad = (SDL_GameController*)binding.pad;
		if (binding.type == binding_t::CONTROLLER_BUTTON) {
			return SDL_GameControllerGetButton(pad, (SDL_GameControllerButton)binding.padButton) == 1;
		} else {
			if (binding.padAxisNegative) {
				return SDL_GameControllerGetAxis(pad, (SDL_GameControllerAxis)binding.padAxis) < -16384;
			} else {
				return SDL_GameControllerGetAxis(pad, (SDL_GameControllerAxis)binding.padAxis) > 16384;
			}
		}
	} else if (
		binding.type == binding_t::JOYSTICK_AXIS ||
		binding.type == binding_t::JOYSTICK_BUTTON ||
		binding.type == binding_t::JOYSTICK_HAT) {
		SDL_Joystick* joystick = (SDL_Joystick*)binding.joystick;
		if (binding.type == binding_t::JOYSTICK_BUTTON) {
			return SDL_JoystickGetButton(joystick, binding.joystickButton) == 1;
		} else if (binding.type == binding_t::JOYSTICK_AXIS) {
			if (binding.joystickAxisNegative) {
				return SDL_JoystickGetAxis(joystick, binding.joystickAxis) < -16384;
			} else {
				return SDL_JoystickGetAxis(joystick, binding.joystickAxis) > 16384;
			}
		} else {
			return SDL_JoystickGetHat(joystick, binding.joystickHat) == binding.joystickHatState;
		}
	} else if (binding.type == binding_t::MOUSE_BUTTON) {
		return mouseButtons[binding.mouseButton];
	} else if (binding.type == binding_t::KEYBOARD) {
		SDL_Keycode key = binding.keycode;
		if (key != SDLK_UNKNOWN) {
			return keys[(int)key];
		}
	}

	return false;
}

float Input::analogOf(binding_t& binding) {
	if (binding.type == binding_t::CONTROLLER_AXIS ||
		binding.type == binding_t::CONTROLLER_BUTTON) {
		SDL_GameController* pad = (SDL_GameController*)binding.pad;
		if (binding.type == binding_t::CONTROLLER_BUTTON) {
			return SDL_GameControllerGetButton(pad, (SDL_GameControllerButton)binding.padButton) ? 1.f : 0.f;
		} else {
			if (binding.padAxisNegative) {
				float result = std::min(SDL_GameControllerGetAxis(pad, (SDL_GameControllerAxis)binding.padAxis) / 32768.f, 0.f) * -1.f;
				return (fabs(result) > deadzone) ? result : 0.f;
			} else {
				float result = std::max(SDL_GameControllerGetAxis(pad, (SDL_GameControllerAxis)binding.padAxis) / 32767.f, 0.f);
				return (fabs(result) > deadzone) ? result : 0.f;
			}
		}
	} else if (
		binding.type == binding_t::JOYSTICK_AXIS ||
		binding.type == binding_t::JOYSTICK_BUTTON ||
		binding.type == binding_t::JOYSTICK_HAT) {
		SDL_Joystick* joystick = (SDL_Joystick*)binding.joystick;
		if (binding.type == binding_t::JOYSTICK_BUTTON) {
			return SDL_JoystickGetButton(joystick, binding.joystickButton) ? 1.f : 0.f;
		} else if (binding.type == binding_t::JOYSTICK_AXIS) {
			if (binding.joystickAxisNegative) {
				float result = std::min(SDL_JoystickGetAxis(joystick, binding.joystickAxis) / 32768.f, 0.f) * -1.f;
				return (fabs(result) > deadzone) ? result : 0.f;
			} else {
				float result = std::max(SDL_JoystickGetAxis(joystick, binding.joystickAxis) / 32767.f, 0.f);
				return (fabs(result) > deadzone) ? result : 0.f;
			}
		} else {
			return SDL_JoystickGetHat(joystick, binding.joystickHat) == binding.joystickHatState ? 1.f : 0.f;
		}
	} else if (binding.type == binding_t::MOUSE_BUTTON) {
		return mouseButtons[binding.mouseButton] ? 1.f : 0.f;
	} else if (binding.type == binding_t::KEYBOARD) {
		SDL_Keycode key = binding.keycode;
		if (key != SDLK_UNKNOWN) {
			return keys[(int)key] ? 1.f : 0.f;
		}
	}

	return 0.f;
}

SDL_Keycode Input::getKeycodeFromName(const char* name) {
	auto search = keycodeNames.find(name);
	if (search == keycodeNames.end()) {
		SDL_Keycode keycode = SDL_GetKeyFromName(name);
		if (keycode != SDLK_UNKNOWN) {
            keycodeNames[name] = (int32_t)keycode;
		}
		return keycode;
	} else {
		return search->second;
	}
}

Input::playerControlType_t Input::getPlayerControlType()
{
	if (multiplayer != SINGLE && player != 0) {
		return inputs[0].getPlayerControlType();
	}
#ifndef EDITOR
	if ( ::inputs.hasController(player) )
	{
		return Input::PLAYER_CONTROLLED_BY_CONTROLLER;
	}
	if ( ::inputs.bPlayerUsingKeyboardControl(player) )
	{
		return Input::PLAYER_CONTROLLED_BY_KEYBOARD;
	}
#endif // !EDITOR
	return Input::PLAYER_CONTROLLED_BY_INVALID;
}

DynamicArrayStr Input::getBindingsForInput(const char* input) const {
    DynamicArrayStr result;
    std::vector<const char*> _keys;
    bindings.keys(_keys);
    for ( const char* _k : _keys ) {
        binding_tMirror _b;
        bindings.get(_k, _b);
        const bool isController =
            _b.type == binding_t::bindtype_t::CONTROLLER_AXIS ||
            _b.type == binding_t::bindtype_t::CONTROLLER_BUTTON;
        if (isController) {
            if (_b.input.substr(4) == input) {
                result.push_back(_k);
            }
        } else {
            if (_b.input == input) {
                result.push_back(_k);
            }
        }
    }
    return result;
}

bool Input::bindingIsSharedWithKeyboardSystemBinding(const char* binding)
{
	if ( multiplayer != SINGLE && player != 0 ) {
		return inputs[0].bindingIsSharedWithKeyboardSystemBinding(binding);
	}
#ifndef EDITOR
	if ( disabled )
	{
		return false;
	}

	const std::pair<std::string, binding_t> checkBinding =
		std::make_pair(binding, input(binding));
	if ( !checkBinding.second.isBindingUsingKeyboard() )
	{
		return false;
	}

	if ( checkBinding.second.type == Input::binding_t::bindtype_t::KEYBOARD )
	{
		switch ( checkBinding.second.keycode )
		{
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_SLASH:
				return true;
			default:
				break;
		}
	}

	if ( checkBinding.second.input.find("Mouse") != std::string::npos )
	{
		return true;
	}
#endif
	return false;
}

void Input::consumeBindingsSharedWithBinding(const char* binding)
{
	if (multiplayer != SINGLE && player != 0) {
		inputs[0].consumeBindingsSharedWithBinding(binding);
		return;
	}
#ifndef EDITOR
	if ( disabled )
	{
		return;
	}
	const std::pair<std::string, binding_t> checkBinding =
		std::make_pair(binding, input(binding));
	std::vector<const char*> _keys;
	bindings.keys(_keys);
	for ( const char* _k : _keys )
	{
		auto& _b = bindings[_k];
		if ( !_b.binary )
		{
			continue; // don't pre-consume non-pressed buttons
		}
		if ( _b.consumed )
		{
			continue; // no need to consume again
		}
		if ( _b.type == checkBinding.second.type )
		{
			if ( _k == checkBinding.first )
			{
				continue; // skip the hotbar bindings
			}
			if ( _b.type == binding_t::CONTROLLER_AXIS ||
				_b.type == binding_t::CONTROLLER_BUTTON )
			{
				if ( _b.type == binding_t::CONTROLLER_BUTTON )
				{
					if ( _b.padButton == checkBinding.second.padButton )
					{
						_b.consumed = true;
					}
				}
				else
				{
					if ( _b.padAxis == checkBinding.second.padAxis )
					{
						_b.consumed = true;
					}
				}
			}
			else if (
				_b.type == binding_t::JOYSTICK_AXIS ||
				_b.type == binding_t::JOYSTICK_BUTTON ||
				_b.type == binding_t::JOYSTICK_HAT )
			{
				if ( _b.type == binding_t::JOYSTICK_BUTTON )
				{
					if ( _b.joystickButton == checkBinding.second.joystickButton )
					{
						_b.consumed = true;
					}
				}
				else if ( _b.type == binding_t::JOYSTICK_AXIS )
				{
					if ( _b.joystickAxis == checkBinding.second.joystickAxis )
					{
						_b.consumed = true;
					}
				}
				else
				{
					if ( _b.joystickHat == checkBinding.second.joystickHat )
					{
						_b.consumed = true;
					}
				}
			}
			else if ( _b.type == binding_t::MOUSE_BUTTON )
			{
				if ( _b.mouseButton == checkBinding.second.mouseButton )
				{
					_b.consumed = true;
				}
			}
			else if ( _b.type == binding_t::KEYBOARD )
			{
				if ( _b.keycode == checkBinding.second.keycode )
				{
					_b.consumed = true;
				}
			}
		}
	}
#endif
}

void Input::consumeBindingsSharedWithFaceHotbar()
{
	if (multiplayer != SINGLE && player != 0) {
		inputs[0].consumeBindingsSharedWithFaceHotbar();
		return;
	}
#ifndef EDITOR
	if ( disabled )
	{
		return;
	}
	if ( players[player]->hotbar.useHotbarFaceMenu )
	{
		if ( players[player]->hotbar.faceMenuButtonHeld != Player::Hotbar_t::FaceMenuGroup::GROUP_NONE )
		{
			const std::map<std::string, binding_tMirror> faceMenuBindings =
			{
				std::make_pair("Hotbar Right", input("Hotbar Right")),
				std::make_pair("Hotbar Left", input("Hotbar Left")),
				std::make_pair("Hotbar Up / Select", input("Hotbar Up / Select")),
				std::make_pair("Hotbar Down / Cancel", input("Hotbar Down / Cancel")),
				std::make_pair("HotbarFacebarModifierLeft", input("HotbarFacebarModifierLeft")),
				std::make_pair("HotbarFacebarModifierRight", input("HotbarFacebarModifierRight"))
			};
			std::vector<const char*> _keys2;
			bindings.keys(_keys2);
			for ( const char* _k2 : _keys2 )
			{
				auto& _b = bindings[_k2];
				if ( !_b.binary )
				{
					continue; // don't pre-consume non-pressed buttons
				}
				if ( _b.consumed )
				{
					continue; // no need to consume again
				}
				for ( auto& faceMenuBinding : faceMenuBindings )
				{
					if ( _b.type == faceMenuBinding.second.type )
					{
						if ( _k2 == faceMenuBinding.first )
						{
							continue; // skip the hotbar bindings
						}
						if ( _b.type == binding_t::CONTROLLER_AXIS ||
							_b.type == binding_t::CONTROLLER_BUTTON ) 
						{
							if ( _b.type == binding_t::CONTROLLER_BUTTON )
							{
								if ( _b.padButton == faceMenuBinding.second.padButton )
								{
									_b.consumed = true;
								}
							}
							else 
							{
								if ( _b.padAxis == faceMenuBinding.second.padAxis )
								{
									_b.consumed = true;
								}
							}
						}
						else if (
							_b.type == binding_t::JOYSTICK_AXIS ||
							_b.type == binding_t::JOYSTICK_BUTTON ||
							_b.type == binding_t::JOYSTICK_HAT ) 
						{
							if ( _b.type == binding_t::JOYSTICK_BUTTON ) 
							{
								if ( _b.joystickButton == faceMenuBinding.second.joystickButton )
								{
									_b.consumed = true;
								}
							}
							else if ( _b.type == binding_t::JOYSTICK_AXIS ) 
							{
								if ( _b.joystickAxis == faceMenuBinding.second.joystickAxis )
								{
									_b.consumed = true;
								}
							}
							else 
							{
								if ( _b.joystickHat == faceMenuBinding.second.joystickHat )
								{
									_b.consumed = true;
								}
							}
						}
						else if ( _b.type == binding_t::MOUSE_BUTTON ) 
						{
							if ( _b.mouseButton == faceMenuBinding.second.mouseButton )
							{
								_b.consumed = true;
							}
						}
						else if ( _b.type == binding_t::KEYBOARD ) 
						{
							if ( _b.keycode == faceMenuBinding.second.keycode )
							{
								_b.consumed = true;
							}
						}
					}
				}
			}
		}
	}
#endif
}
