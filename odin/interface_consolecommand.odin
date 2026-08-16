// interface_consolecommand.odin -- Odin mirror of interface/consolecommand.hpp.
package main

// ---------------------------------------------------------------------------
// Enums
// ---------------------------------------------------------------------------
ConsoleEntryType :: enum i32 {
	Command,
	CvarBool,
	CvarInt,
	CvarFloat,
	CvarString,
	CvarVector4,
}

// ---------------------------------------------------------------------------
// Structs (mirror interface/consolecommand.hpp)
// ---------------------------------------------------------------------------

// 4-float POD used by Vector4 cvars. 16 bytes
Vector4 :: struct {
	x: f32,
	y: f32,
	z: f32,
	w: f32,
}
#assert(size_of(Vector4) == 16)

// ConsoleCommand -- 40 bytes
ConsoleCommand :: struct {
	name:       cstring,            // const char*
	desc:       cstring,            // const char*
	type:       ConsoleEntryType,   // ConsoleEntryType (i32 underlying)
	func:       rawptr,             // ccmd_function void (*)(int, const char**)
	data_ptr:   rawptr,             // void*
}
#assert(size_of(ConsoleCommand) == 40)

// CvarBool -- 48 bytes (ConsoleCommand + bool data)
CvarBool :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       bool,
}
#assert(size_of(CvarBool) == 48)

// CvarInt -- 48 bytes
CvarInt :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       i32,
}
#assert(size_of(CvarInt) == 48)

// CvarFloat -- 48 bytes
CvarFloat :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       f32,
}
#assert(size_of(CvarFloat) == 48)

// CvarString -- 56 bytes (ConsoleCommand + DynamicString)
CvarString :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       string, // DynamicString (16B) — ABI-identical to Odin string
}
#assert(size_of(CvarString) == 56)

// CvarVector4 -- 56 bytes
CvarVector4 :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       Vector4, // 16B
}
#assert(size_of(CvarVector4) == 56)
