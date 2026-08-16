// shader.odin — Odin mirrors of shader.hpp.
package main

// class Shader — 88 bytes
// { const char* name (8); [dynamic]u32 shaders (40); map[string]i32 uniforms (32);
//   u32 program (4); }
Shader :: struct {
	name:     cstring,
	shaders:  [dynamic]u32,  // DynamicArrayU32 (std::vector<Uint32>, 40B)
	uniforms: map[string]i32, // DynamicMapI32 (std::map<string,int32_t>, 32B, string-keyed)
	program:  u32,
}
#assert(size_of(Shader) == 88)
