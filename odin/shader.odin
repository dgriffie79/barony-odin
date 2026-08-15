// shader.odin — Odin mirrors of shader.hpp.
package main

import "containers"

// class Shader — 88 bytes
// { const char* name (8); DynamicArrayU32 shaders (40); DynamicMapI32 uniforms (32);
//   uint32_t program (4); }
Shader :: struct {
	name:     cstring,
	shaders:  containers.Raw_Dynamic_Array, // DynamicArrayU32 (40B)
	uniforms: containers.Raw_Map,           // DynamicMapI32 (32B)
	program:  u32,
}
#assert(size_of(Shader) == 88)
