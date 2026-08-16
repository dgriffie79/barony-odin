// draw.odin — Odin mirrors of draw.hpp.
package main


// enum class ClipResult::Direction
Clip_Result_Direction :: enum i32 {
	Invalid,
	Left,
	Right,
	Top,
	Bottom,
	Front,
	Behind,
}

// struct ClipResult — 24 bytes
Clip_Result :: struct {
	direction:     i32, // ClipResult::Direction enum
	is_behind:     bool,
	// 3B padding
	clipped_coords: vec4_t, // 16B
}
#assert(size_of(Clip_Result) == 24)

// union uif32 — 4 bytes
UIF32 :: struct #raw_union {
	f: f32,
	i: u32,
}
#assert(size_of(UIF32) == 4)

// class TempTexture — 12 bytes.
// References removed in C++ (draw.hpp): texid/w/h are now the real public
// members (were private _texid/_w/_h aliased by const refs; nothing wrote
// through the aliases). Odin mirror is just the 3 members.
Temp_Texture :: struct {
	texid: u32, // GLuint
	w:     i32,
	h:     i32,
}
#assert(size_of(Temp_Texture) == 12)

// struct Mesh — 144 bytes (140 data + pad to 8-alignment)
Mesh :: struct {
	data:          [3][dynamic]f32, // DynamicArrayT<float>[3] (native [dynamic]f32)
	vao:           u32,
	vbo:           [3]u32,
	num_vertices:  u32,
}
#assert(size_of(Mesh) == 144)

// struct framebuffer — 36 bytes
Framebuffer :: struct {
	fbo:        u32,
	fbo_color:  u32,
	fbo_depth:  u32,
	xsize:      u32,
	ysize:      u32,
	pbos:       [2]u32, // NUM_PBOS = 2
	pbo_index:  u32,
	mapped:     bool,
}
#assert(size_of(Framebuffer) == 36)

// typedef struct view_t — 320 bytes
View_T :: struct {
	x:                        f64, // real_t
	y:                        f64,
	z:                        f64,
	ang:                      f64,
	vang:                     f64,
	winx:                     i32, // Sint32
	winy:                     i32,
	winw:                     i32,
	winh:                     i32,
	global_light_modifier:    f64, // real_t
	global_light_modifier_entities: f64,
	global_light_modifier_active: i32, // LightModifierValues enum
	fb:                       [1]Framebuffer,
	vismap:                   ^bool, // bool*
	luminance:                f32,
	drawn_frames:             u32,
	projview:                 mat4x4_t,
	proj:                     mat4x4_t,
	proj_hud:                 mat4x4_t,
}
#assert(size_of(View_T) == 320)

// struct Chunk — 112 bytes
Chunk :: struct {
	vao:            u32,
	vbo_positions:  u32,
	vbo_texcoords:  u32,
	vbo_colors:     u32,
	indices:        i32,
	x:              i32,
	y:              i32,
	w:              i32,
	h:              i32,
	tiles:          [dynamic]i32, // DynamicArrayS32
	dithering:      map[rawptr]Chunk_Dither,      // DynamicMapPtrT<ChunkDither_t> (ptr-keyed)
}
#assert(size_of(Chunk) == 112)

// struct Chunk::Dither — 8 bytes
Chunk_Dither :: struct {
	value:            i32, // default MAX = 10
	last_update_tick: u32,
}
#assert(size_of(Chunk_Dither) == 8)
