// light.odin — Odin mirrors of light.hpp.
package main

import "containers"

// typedef struct light_t — 32 bytes
// { Sint32 x,y; Sint32 radius; vec4_t* tiles; int index; node_t* node; }
light_t :: struct {
	x:      i32,
	y:      i32,
	radius: i32,
	tiles:  ^vec4_t,
	index:  i32,
	node:   ^node_t,
}

#assert(size_of(light_t) == 40)

// struct LightDef — 28 bytes
// { int radius; float r,g,b,a; float falloff_exp; bool shadows; }
Light_Def :: struct {
	radius:      i32,
	r:           f32,
	g:           f32,
	b:           f32,
	a:           f32,
	falloff_exp: f32,
	shadows:     bool,
}

#assert(size_of(Light_Def) == 28)

// struct KV { const char* first; LightDef second; } — 40 bytes
Light_KV :: struct {
	first:  cstring,
	second: Light_Def,
}

#assert(size_of(Light_KV) == 40)

// struct Iterator { KV kv; bool valid; } — 48 bytes
Light_Iterator :: struct {
	kv:    Light_KV,
	valid: bool,
}

#assert(size_of(Light_Iterator) == 48)

// DynamicMapLightDef — a string-keyed map of LightDef. C++ wraps DynamicMapRaw
// (32B) with per-key LightDef values. Mirrors as map[string]Light_Def (native).
// Layout note: the C++ class has one DynamicMapRaw member (32B) + no other data.
DynamicMapLightDef :: struct {
	raw: map[string]Light_Def, // DynamicMapLightDef (map<string,LightDef>, native)
}

#assert(size_of(DynamicMapLightDef) == 32)
