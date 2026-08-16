// engine_audio_sound.odin — Odin mirror of engine/audio/sound.hpp.
package main

// struct Vec3 — 12 bytes
Vec3 :: struct {
	x: f32,
	y: f32,
	z: f32,
}
#assert(size_of(Vec3) == 12)

// OPENAL_CHANNELGROUP / OPENAL_BUFFER / OPENAL_SOUND are opaque (forward
// declared); used only via rawptr in other mirrors (e.g. bowDrawingSoundChannel).
