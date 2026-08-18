// engine_audio_sound.odin - Odin mirror of engine/audio/sound.hpp.
package main

// struct Vec3 - 12 bytes
Vec3 :: struct {
	x: f32,
	y: f32,
	z: f32,
}
#assert(size_of(Vec3) == 12)

// OPENAL_CHANNELGROUP / OPENAL_BUFFER / OPENAL_SOUND are opaque (forward
// declared); used only via rawptr in other mirrors (e.g. bowDrawingSoundChannel).

// ---------------------------------------------------------------------------
// Globals owned by Odin (flipped from C++ in engine/audio/sound.hpp)
// ---------------------------------------------------------------------------

@(export)
numsounds : u32 = 0

@(export)
openal_context : rawptr = nil  // ALCcontext* - OpenAL opaque type, no Odin mirror

@(export)
openal_device : rawptr = nil   // ALCdevice* - OpenAL opaque type, no Odin mirror

// openal_maxchannels: declaration-only in C++ header, no definition found.
// Leaving undeclared in Odin until the parent resolves.

@(export)
levelmusicplaying : bool = false

@(export)
currenttrack : i32 = -1

@(export)
shopmusicplaying : bool = false

@(export)
combatmusicplaying : bool = false

@(export)
minotaurmusicplaying : bool = false

@(export)
herxmusicplaying : bool = false

@(export)
devilmusicplaying : bool = false

@(export)
olddarkmap : bool = false

@(export)
sanctummusicplaying : bool = false

@(export)
sfxUseDynamicAmbientVolume : bool = true

@(export)
sfxUseDynamicEnvironmentVolume : bool = true

// OPENAL_BUFFER** globals - opaque pointer-to-pointer, no Odin mirror
@(export) sounds              : rawptr = nil
@(export) minesmusic          : rawptr = nil
@(export) swampmusic          : rawptr = nil
@(export) labyrinthmusic      : rawptr = nil
@(export) ruinsmusic          : rawptr = nil
@(export) underworldmusic     : rawptr = nil
@(export) hellmusic           : rawptr = nil
@(export) intromusic          : rawptr = nil

// OPENAL_BUFFER* single-pointer globals - opaque, no Odin mirror
@(export) intermissionmusic   : rawptr = nil
@(export) minetownmusic       : rawptr = nil
@(export) splashmusic         : rawptr = nil
@(export) librarymusic        : rawptr = nil
@(export) shopmusic           : rawptr = nil
@(export) storymusic          : rawptr = nil

@(export) minotaurmusic       : rawptr = nil  // OPENAL_BUFFER** (pointer-to-pointer)
@(export) herxmusic           : rawptr = nil
@(export) templemusic         : rawptr = nil

@(export) endgamemusic        : rawptr = nil
@(export) devilmusic          : rawptr = nil
@(export) escapemusic         : rawptr = nil
@(export) sanctummusic        : rawptr = nil
@(export) introductionmusic   : rawptr = nil

@(export) cavesmusic          : rawptr = nil  // OPENAL_BUFFER**
@(export) citadelmusic        : rawptr = nil  // OPENAL_BUFFER**
@(export) gnomishminesmusic   : rawptr = nil
@(export) greatcastlemusic    : rawptr = nil
@(export) sokobanmusic        : rawptr = nil
@(export) caveslairmusic      : rawptr = nil
@(export) bramscastlemusic    : rawptr = nil
@(export) hamletmusic         : rawptr = nil
@(export) fortressmusic       : rawptr = nil  // OPENAL_BUFFER**
@(export) tutorialmusic       : rawptr = nil
@(export) gameovermusic       : rawptr = nil
@(export) introstorymusic     : rawptr = nil

// OPENAL_SOUND* globals - opaque, no Odin mirror
@(export) music_channel       : rawptr = nil
@(export) music_channel2      : rawptr = nil
@(export) music_resume        : rawptr = nil

// OPENAL_CHANNELGROUP* globals - opaque, no Odin mirror
@(export) sound_group              : rawptr = nil
@(export) music_group              : rawptr = nil
@(export) soundAmbient_group       : rawptr = nil
@(export) soundEnvironment_group   : rawptr = nil
@(export) music_notification_group : rawptr = nil

// Fade increment globals (float in C++)
@(export) fadein_increment              : f32 = 0.002
@(export) default_fadein_increment      : f32 = 0.002
@(export) fadeout_increment             : f32 = 0.005
@(export) default_fadeout_increment     : f32 = 0.005
