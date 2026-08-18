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
	data:       string, // DynamicString (16B) - ABI-identical to Odin string
}
#assert(size_of(CvarString) == 56)

// CvarVector4 -- 56 bytes
CvarVector4 :: struct {
	using base: ConsoleCommand, // ConsoleCommand base (40B)
	data:       Vector4, // 16B
}
#assert(size_of(CvarVector4) == 56)

// ---------------------------------------------------------------------------
// Cvar ctor ports (consolecommand.hpp:84-136)
// ---------------------------------------------------------------------------
// The C++ Cvar* ctors set name/type/func=cvar_setter/data_ptr=&data/data,
// then call register_console_entry. Odin zero-init skips all that, which
// breaks *cvar_fogDistance etc. in the 3D render path (uploadUniforms) and
// *framesEatMouse in handleEvents. These port the ctors for every Odin-owned
// cvar. Called from run_barony() before barony_main. Game-only: the editor
// static lib builds only editor_sources (no consolecommand.cpp), so it does
// not provide cvar_setter/register_console_entry.

when !#config(EDITOR, false) {
init_cvars :: proc() {
	// CvarFloat: name, type, func, data_ptr, data, register
	cvar_fogDistance.name = "/fog_distance"
	cvar_fogDistance.type = .CvarFloat
	cvar_fogDistance.func = rawptr(cvar_setter)
	cvar_fogDistance.data_ptr = &cvar_fogDistance.data
	cvar_fogDistance.data = 0.0
	register_console_entry(rawptr(&cvar_fogDistance))

	cvar_hdrExposure.name = "/hdr_exposure"
	cvar_hdrExposure.type = .CvarFloat
	cvar_hdrExposure.func = rawptr(cvar_setter)
	cvar_hdrExposure.data_ptr = &cvar_hdrExposure.data
	cvar_hdrExposure.data = 0.5
	register_console_entry(rawptr(&cvar_hdrExposure))

	cvar_hdrGamma.name = "/hdr_gamma"
	cvar_hdrGamma.type = .CvarFloat
	cvar_hdrGamma.func = rawptr(cvar_setter)
	cvar_hdrGamma.data_ptr = &cvar_hdrGamma.data
	cvar_hdrGamma.data = 0.75
	register_console_entry(rawptr(&cvar_hdrGamma))

	cvar_hdrAdjustment.name = "/hdr_adjust_rate"
	cvar_hdrAdjustment.type = .CvarFloat
	cvar_hdrAdjustment.func = rawptr(cvar_setter)
	cvar_hdrAdjustment.data_ptr = &cvar_hdrAdjustment.data
	cvar_hdrAdjustment.data = 0.1
	register_console_entry(rawptr(&cvar_hdrAdjustment))

	cvar_hdrLimitHigh.name = "/hdr_limit_high"
	cvar_hdrLimitHigh.type = .CvarFloat
	cvar_hdrLimitHigh.func = rawptr(cvar_setter)
	cvar_hdrLimitHigh.data_ptr = &cvar_hdrLimitHigh.data
	cvar_hdrLimitHigh.data = 4.0
	register_console_entry(rawptr(&cvar_hdrLimitHigh))

	cvar_hdrLimitLow.name = "/hdr_limit_low"
	cvar_hdrLimitLow.type = .CvarFloat
	cvar_hdrLimitLow.func = rawptr(cvar_setter)
	cvar_hdrLimitLow.data_ptr = &cvar_hdrLimitLow.data
	cvar_hdrLimitLow.data = 0.1
	register_console_entry(rawptr(&cvar_hdrLimitLow))

	// CvarVector4: name, type, func, data_ptr, data (Vector4), register
	cvar_hdrBrightness.name = "/hdr_brightness"
	cvar_hdrBrightness.type = .CvarVector4
	cvar_hdrBrightness.func = rawptr(cvar_setter)
	cvar_hdrBrightness.data_ptr = &cvar_hdrBrightness.data
	cvar_hdrBrightness.data = {1, 1, 1, 1}
	register_console_entry(rawptr(&cvar_hdrBrightness))

	cvar_fogColor.name = "/fog_color"
	cvar_fogColor.type = .CvarVector4
	cvar_fogColor.func = rawptr(cvar_setter)
	cvar_fogColor.data_ptr = &cvar_fogColor.data
	cvar_fogColor.data = {0, 0, 0, 0}
	register_console_entry(rawptr(&cvar_fogColor))
}

init_cvars_bool :: proc() {
	shareMinimap.name = "/shareminimap"
	shareMinimap.type = .CvarBool
	shareMinimap.func = rawptr(cvar_setter)
	shareMinimap.data_ptr = &shareMinimap.data
	shareMinimap.data = true
	register_console_entry(rawptr(&shareMinimap))

	framesEatMouse.name = "/gui_eat_mouseclicks"
	framesEatMouse.type = .CvarBool
	framesEatMouse.func = rawptr(cvar_setter)
	framesEatMouse.data_ptr = &framesEatMouse.data
	framesEatMouse.data = true
	register_console_entry(rawptr(&framesEatMouse))
}
} // when !#config(EDITOR, false)
