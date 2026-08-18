// ui_button.odin - Odin mirror of ui/Button.hpp.
package main

// enum Button::style_t
Button_Style :: enum i32 {
	STYLE_NORMAL,
	STYLE_TOGGLE,
	STYLE_CHECKBOX,
	STYLE_RADIO,
	STYLE_DROPDOWN,
	STYLE_MAX,
}

// enum Button::justify_t
Button_Justify :: enum i32 {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	CENTER,
	JUSTIFY_TYPE_LENGTH,
}

// struct Button::result_t - 16 bytes
Button_Result_T :: struct {
	highlighted:   bool,
	pressed:       bool,
	clicked:       bool,
	highlight_time: u32,
	tooltip:       cstring, // const char*
}
#assert(size_of(Button_Result_T) == 16)

// class Button : public Widget - 464 bytes
Button :: struct {
	using _: Widget, // 248B base

	callback:                rawptr, // void (*)(Button&) (public, first own member)
	background:              string, // DynamicString (16B)
	background_highlighted:  string,
	background_activated:    string,
	text:                    string,
	font:                    string,
	icon:                    string,
	tooltip:                 string,
	border:                  i32,
	size:                    SDL_Rect, // 16B
	color:                   u32,
	icon_color:              u32,
	highlight_color:         u32,
	text_color:              u32,
	text_highlight_color:    u32,
	border_color:            u32,
	style:                   Button_Style,
	hjustify:                Button_Justify,
	vjustify:                Button_Justify,
	text_offset:             SDL_Rect, // 16B
	ontop:                   bool,
	// pad 3
	padding_per_text_line:   i32,
	scroll_parent_offset:    SDL_Rect, // 16B
}
#assert(size_of(Button) == 464)
