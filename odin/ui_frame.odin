// ui_frame.odin - Odin mirror of ui/Frame.hpp.
package main

// enum Frame::border_style_t
Frame_BorderStyle :: enum i32 {
	BORDER_FLAT,
	BORDER_BEVEL_HIGH,
	BORDER_BEVEL_LOW,
	BORDER_MAX,
}

// enum Frame::justify_t
Frame_Justify :: enum i32 {
	LEFT,
	RIGHT,
	CENTER,
	JUSTIFY_TYPE_LENGTH,
}

// enum Frame::FrameSearchType
Frame_SearchType :: enum i32 {
	FRAME_SEARCH_DEPTH_FIRST,
	FRAME_SEARCH_BREADTH_FIRST,
}

// struct Frame::image_t - 80 bytes
Frame_Image_T :: struct {
	name:           string, // DynamicString (16B)
	path:           string,
	color:          u32,
	outline_color:  u32,
	pos:            SDL_Rect, // 16B
	section:        SDL_Rect,
	tiled:          bool,
	disabled:       bool,
	ontop:          bool,
	outline:        bool,
	no_blit_parent: bool,
}
#assert(size_of(Frame_Image_T) == 80)

// struct Frame::entry_t - 144 bytes
Frame_Entry_T :: struct {
	parent:         rawptr, // Frame& (reference -> rawptr)
	name:           string,
	text:           string,
	tooltip:        string,
	image:          string,
	color:          u32,
	data:           rawptr, // void*
	clickable:      bool,
	pressed:        bool,
	highlighted:    bool,
	leftright_control: bool,
	leftright_allow_nonclickable: bool,
	updown_allow_nonclickable: bool,
	movement_nonclickable: bool,
	navigable:      bool,
	highlight_time: u32,
	suicide:        bool,
	// pad 3
	click:          rawptr, // void (*)(entry_t&)
	ctrl_click:     rawptr,
	highlight:      rawptr,
	highlighting:   rawptr,
	selected:       rawptr,
}
#assert(size_of(Frame_Entry_T) == 144)

// struct Frame::result_t - 24 bytes
Frame_Result_T :: struct {
	usable:         bool,
	highlight_time: u32,
	tooltip:        cstring, // const char*
	removed:        bool,
}
#assert(size_of(Frame_Result_T) == 24)

// class Frame : public Widget - 808 bytes
Frame :: struct {
	// Widget base (248B) - inlined field-for-field
	using _: Widget,

	ticks:                       u32,
	font:                        string, // DynamicString (16B)
	border:                      i32,
	size:                        SDL_Rect, // 16B
	actual_size:                 SDL_Rect,
	border_style:                Frame_BorderStyle, // default BORDER_BEVEL_HIGH
	color:                       u32,
	selected_entry_color:        u32,
	activated_entry_color:       u32,
	border_color:                u32,
	slider_color:                u32,
	tooltip:                     cstring, // const char*
	hollow:                      bool,
	dragging_h_slider:           bool,
	dragging_v_slider:           bool,
	old_slider_x:                i32,
	old_slider_y:                i32,
	drop_down:                   bool,
	drop_down_clicked:           u32,
	selection:                   i32,
	activation:                  ^Frame_Entry_T, // entry_t*
	allow_scroll_binds:          bool,
	allow_scrolling:             bool,
	scrollbars:                  bool,
	activated:                   bool,
	list_offset:                 SDL_Rect,
	opacity:                     f64, // real_t (default 100.0)
	inherit_parent_frame_opacity: bool,
	justify:                     Frame_Justify,
	clickable:                   bool,
	// pad 7
	scroll_x:                    f64,
	scroll_y:                    f64,
	scroll_velocity_x:           f64,
	scroll_velocity_y:           f64,
	scroll_acceleration_x:       f64,
	scroll_acceleration_y:       f64,
	dont_tick_children:          bool,
	entry_size:                  i32,
	scroll_with_left_controls:   bool,
	b_blit_children_to_texture:  bool,
	b_blit_dirty:                bool,
	b_blit_to_parent:            bool,
	b_list_menu_list_cancel_override: bool,
	scroll_parent_offset:        SDL_Rect,
	allow_scroll_parent:         bool,
	// pad 7
	frames:                      [dynamic]^Frame,       // DynamicArrayT<Frame*> (40B)
	buttons:                     [dynamic]^Button,      // DynamicArrayT<Button*> (40B)
	fields:                      [dynamic]^Field,       // DynamicArrayT<Field*> (40B)
	images:                      [dynamic]^Frame_Image_T, // DynamicArrayT<image_t*> (40B)
	sliders:                     [dynamic]^Slider,      // DynamicArrayT<Slider*> (40B)
	list:                        [dynamic]^Frame_Entry_T, // DynamicArrayT<entry_t*> (40B)
	sync_scroll_targets:         [dynamic]string,       // DynamicArrayStr (40B)
	blit_surface:                rawptr, // SDL_Surface*
	blit_texture:                ^Temp_Texture, // TempTexture*
}
#assert(size_of(Frame) == 808)
