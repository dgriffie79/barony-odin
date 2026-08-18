#+feature dynamic-literals

// draw.odin - Odin mirrors of draw.hpp.
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

// struct ClipResult - 24 bytes
Clip_Result :: struct {
	direction:     i32, // ClipResult::Direction enum
	is_behind:     bool,
	// 3B padding
	clipped_coords: vec4_t, // 16B
}
#assert(size_of(Clip_Result) == 24)

// union uif32 - 4 bytes
UIF32 :: struct #raw_union {
	f: f32,
	i: u32,
}
#assert(size_of(UIF32) == 4)

// class TempTexture - 12 bytes.
// References removed in C++ (draw.hpp): texid/w/h are now the real public
// members (were private _texid/_w/_h aliased by const refs; nothing wrote
// through the aliases). Odin mirror is just the 3 members.
Temp_Texture :: struct {
	texid: u32, // GLuint
	w:     i32,
	h:     i32,
}
#assert(size_of(Temp_Texture) == 12)

// struct Mesh - 144 bytes (140 data + pad to 8-alignment)
Mesh :: struct {
	data:          [3][dynamic]f32, // DynamicArrayT<float>[3] (native [dynamic]f32)
	vao:           u32,
	vbo:           [3]u32,
	num_vertices:  u32,
}
#assert(size_of(Mesh) == 144)

// struct framebuffer - 36 bytes
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

// typedef struct view_t - 320 bytes
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

// struct Chunk - 112 bytes
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

// struct Chunk::Dither - 8 bytes
Chunk_Dither :: struct {
	value:            i32, // default MAX = 10
	last_update_tick: u32,
}
#assert(size_of(Chunk_Dither) == 8)

// ============================================================================
// Globals owned by Odin
// ============================================================================
// C++ header (draw.hpp) declares these as `extern "C"` - Odin owns storage.
// C++ definitions deleted from draw.cpp, main.cpp, opengl.cpp.
// ============================================================================

@(export)
main_framebuffer : Framebuffer

@(export)
voxelShader : Shader
@(export)
voxelBrightShader : Shader
@(export)
voxelDitheredShader : Shader
@(export)
voxelBrightDitheredShader : Shader
@(export)
worldShader : Shader
@(export)
worldDitheredShader : Shader
@(export)
worldDarkShader : Shader
@(export)
skyShader : Shader
@(export)
spriteShader : Shader
@(export)
spriteDitheredShader : Shader
@(export)
spriteBrightShader : Shader
@(export)
spriteUIShader : Shader

// Mesh spriteMesh - initialized with quad data (opengl.cpp original)
@(export)
spriteMesh : Mesh = {
	data = {
		[dynamic]f32{-0.5, -0.5, 0, 0.5, -0.5, 0, 0.5, 0.5, 0, -0.5, -0.5, 0, 0.5, 0.5, 0, -0.5, 0.5, 0},
		[dynamic]f32{0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0},
		[dynamic]f32{},
	},
}

// Mesh skyMesh - initialized with sky-plane data (opengl.cpp original).
// CLIPFAR=4000, sky_size=64000, sky_htex_size=1000, sky_ltex_size=2000.
@(export)
skyMesh : Mesh = {
	data = {
		[dynamic]f32{
			-64000, 65, -64000,  64000, 65, -64000,  64000, 65,  64000,
			-64000, 65, -64000,  64000, 65,  64000, -64000, 65,  64000,
			-64000, 64, -64000,  64000, 64, -64000,  64000, 64,  64000,
			-64000, 64, -64000,  64000, 64,  64000, -64000, 64,  64000,
		},
		[dynamic]f32{
			0, 0,  1000, 0,  1000, 1000,  0, 0,  1000, 1000,  0, 1000,
			0, 0,  2000, 0,  2000, 2000,  0, 0,  2000, 2000,  0, 2000,
		},
		[dynamic]f32{
			1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
			1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
			1, 1, 1, 0.5,  1, 1, 1, 0.5,  1, 1, 1, 0.5,
			1, 1, 1, 0.5,  1, 1, 1, 0.5,  1, 1, 1, 0.5,
		},
	},
}

// TempTexture* lightmapTexture[MAXPLAYERS + 1] - pointer array.
// C++ fills entries with `new TempTexture()` at runtime.
@(export)
lightmapTexture : [5]^Temp_Texture = {}

@(export)
ditherDisabledTime : u32 = 0

@(export)
cameras : [MAXPLAYERS]View_T

@(export)
menucam : View_T

@(export)
hdrEnabled : bool = true

// ---------------------------------------------------------------------------
// Game-only globals (C++ side: #ifndef EDITOR in draw.hpp).
// Odin always exports them; editor C++ code does not reference them.
// ---------------------------------------------------------------------------

@(export)
cvar_hdrBrightness : CvarVector4
@(export)
cvar_fogDistance : CvarFloat
@(export)
cvar_fogColor : CvarVector4
@(export)
cvar_hdrExposure : CvarFloat
@(export)
cvar_hdrGamma : CvarFloat
@(export)
cvar_hdrAdjustment : CvarFloat
@(export)
cvar_hdrLimitHigh : CvarFloat
@(export)
cvar_hdrLimitLow : CvarFloat

// ---------------------------------------------------------------------------
// Cvar ctor ports (init_cvars / init_cvars_bool) live in
// interface_consolecommand.odin next to the Cvar type mirrors.

// const globals - values preserved from C++ (opengl.cpp)
@(export)
defaultBrightness : Vector4 = {1, 1, 1, 1}
@(export)
defaultGamma : f32 = 0.75
@(export)
defaultExposure : f32 = 0.5
@(export)
defaultAdjustmentRate : f32 = 0.1
@(export)
defaultLimitHigh : f32 = 4.0
@(export)
defaultLimitLow : f32 = 0.1
