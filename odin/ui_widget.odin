// ui_widget.odin — Odin mirror of ui/Widget.hpp.
package main

// enum Widget::type_t
Widget_Type_T :: enum i32 {
	WIDGET_FRAME,
	WIDGET_BUTTON,
	WIDGET_FIELD,
	WIDGET_SLIDER,
}

// enum Widget::glyph_position_t
Widget_Glyph_Position_T :: enum i32 {
	CENTERED,
	CENTERED_RIGHT,
	CENTERED_LEFT,
	CENTERED_TOP,
	CENTERED_BOTTOM,
	BOTTOM_RIGHT,
	BOTTOM_LEFT,
	UPPER_RIGHT,
	UPPER_LEFT,
}

// enum Widget::SearchType
Widget_SearchType :: enum i32 {
	DEPTH_FIRST,
	BREADTH_FIRST,
}

// enum Widget::MenuConfirmTypes : int
Widget_MenuConfirmTypes :: enum i32 {
	MENU_CONFIRM_KEYBOARD  = 1,
	MENU_CONFIRM_CONTROLLER = 2,
}

// class Widget — 248 bytes
Widget :: struct {
	type:                   Widget_Type_T,          // type tag (4B)
	// pad 4
	parent:                 ^Widget,                // Widget*
	widgets:                [dynamic]^Widget,       // DynamicArrayT<Widget*> (40B)
	name:                   string,                 // DynamicString (16B)
	pressed:                bool,
	really_pressed:         bool,
	highlighted:            bool,
	selected:               bool,
	disabled:               bool,
	invisible:              bool,
	to_be_deleted:          bool,
	hide_glyphs:            bool,
	hide_keyboard_glyphs:   bool,   // default true
	hide_selectors:         bool,
	always_show_glyphs:     bool,
	menu_confirm_control_type: i32, // MenuConfirmTypes (KEYBOARD|CONTROLLER = 3)
	highlight_time:         u32,
	owner:                  i32,    // Sint32
	selector_offset:        SDL_Rect, // 16B
	buttons_offset:         SDL_Rect, // 16B
	glyph_position:         Widget_Glyph_Position_T, // default CENTERED_BOTTOM
	// pad 4
	tick_callback:          rawptr, // void (*)(Widget&)
	draw_callback:          rawptr, // void (*)(const Widget&, const SDL_Rect)
	user_data:              rawptr, // void*
	dont_search_ancestors:  bool,
	// pad 7
	widget_actions:         map[string]string, // DynamicMapStr (32B)
	widget_movements:       map[string]string, // DynamicMapStr (32B)
	widget_search_parent:   string, // DynamicString (16B)
}
#assert(size_of(Widget) == 248)
