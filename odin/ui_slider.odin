// ui_slider.odin - Odin mirror of ui/Slider.hpp.
package main

// enum Slider::orientation_t
Slider_Orientation :: enum i32 {
	SLIDER_HORIZONTAL,
	SLIDER_VERTICAL,
}

// struct Slider::result_t - 16 bytes
Slider_Result_T :: struct {
	highlighted:   bool,
	clicked:       bool,
	highlight_time: u32,
	tooltip:       cstring, // const char*
}
#assert(size_of(Slider_Result_T) == 16)

// class Slider : public Widget - 408 bytes
Slider :: struct {
	using _: Widget, // 248B base

	callback:              rawptr, // void (*)(Slider&) (public, first own member)
	orientation:           Slider_Orientation,
	value:                 f32,
	max_value:             f32,
	min_value:             f32,
	value_speed:           f32,
	border:                i32,
	activated:             bool,
	// pad 3
	handle_size:           SDL_Rect, // 16B
	rail_size:             SDL_Rect, // 16B
	tooltip:               string,   // DynamicString (16B)
	color:                 u32,
	highlight_color:       u32,
	move_start_time:       u32,
	last_move_time:        u32,
	handle_image_activated: string, // 16B
	handle_image:          string,
	rail_image:            string,
	ontop:                 bool,
	// pad 7
}
#assert(size_of(Slider) == 408)
