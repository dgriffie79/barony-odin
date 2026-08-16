// ui_field.odin — Odin mirror of ui/Field.hpp.
package main

// enum Field::justify_t (TOP,BOTTOM,LEFT,RIGHT,CENTER)
Field_Justify :: enum i32 {
	TOP,
	BOTTOM,
	LEFT,
	RIGHT,
	CENTER,
	JUSTIFY_TYPE_LENGTH,
}

// struct Field::result_t — 24 bytes
Field_Result_T :: struct {
	tooltip:       cstring, // const char*
	highlighted:   bool,
	highlight_time: u32,
	entered:       bool,
}
#assert(size_of(Field_Result_T) == 24)

// FieldCacheEntry_T — 24B (vector<pair<string,Text*>> cache element)
FieldCacheEntry_T :: struct {
	first:  string, // owned
	second: rawptr, // Text* (not owned)
}
#assert(size_of(FieldCacheEntry_T) == 24)

// class Field : public Widget — (248 base + own)
Field :: struct {
	using _: Widget, // 248B base

	dirty:           bool, // public member (rebuild text cache)
	// pad 7
	font:            string, // DynamicString (16B)
	guide:           string,
	tooltip:         string,
	text:            cstring, // char*
	textlen:         i64,     // size_t
	color:           u32,
	text_color:      u32,
	outline_color:   u32,
	background_color: u32,
	background_activated_color: u32,
	background_select_all_color: u32,
	size:            SDL_Rect, // 16B
	hjustify:        Field_Justify, // default LEFT
	vjustify:        Field_Justify, // default TOP
	editable:        bool,
	numbers_only:    bool,
	scroll:          bool,
	select_all:      bool,
	activated:       bool,
	// pad 3
	callback:        rawptr, // void (*)(Field&)
	ontop:           bool,
	// pad 7
	words_to_highlight: map[[4]byte]u32, // DynamicMapI32T<Uint32> (32B)
	lines_to_color:    map[[4]byte]u32,  // DynamicMapI32T<Uint32> (32B)
	padding_per_line:  i32,
	// pad 4
	individual_line_padding: map[[4]byte]i32, // DynamicMapI32T<int> (32B)
	cache:           [dynamic]FieldCacheEntry_T, // DynamicArrayT<FieldCacheEntry_t> (40B)
}
#assert(size_of(Field) == 536)
