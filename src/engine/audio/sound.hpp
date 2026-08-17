/*-------------------------------------------------------------------------------

	BARONY
	File: sound.hpp
	Desc: Defines sound related stuff.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include <stdio.h>

// Odin-shim-backed dynamic array (replaces std::vector in shared structs)
#include "../../../odin/containers/dynamic_array.hpp"
#ifdef USE_OPENAL
#ifdef APPLE
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#endif
#include "../../interface/consolecommand.hpp"

extern Uint32 numsounds;
extern "C" bool initSoundEngine(); //If it fails to initialize the sound engine, it'll just disable audio.
extern "C" void exitSoundEngine();
extern "C" int loadSoundResources(real_t base_load_percent, real_t top_load_percent);
extern "C" void freeSoundResources();
// all parameters should be in ranges of [0.0 - 1.0]
extern "C" void setGlobalVolume(real_t master, real_t music, real_t gameplay, real_t ambient, real_t environment, real_t notification);
extern "C" void setAudioDevice(const std::string& device);
extern "C" void setRecordDevice(const std::string& device);
extern "C" bool loadMusic();


#define SOUND
#define MUSIC

extern ALCcontext *openal_context;
extern ALCdevice  *openal_device;

extern int openal_maxchannels;

extern bool levelmusicplaying;

extern int currenttrack; // track index for handleLevelMusic (defined in defines.cpp)

extern bool shopmusicplaying;
extern bool combatmusicplaying;
extern bool minotaurmusicplaying;
extern bool herxmusicplaying;
extern bool devilmusicplaying;
extern bool olddarkmap;
extern bool sanctummusicplaying;
extern bool sfxUseDynamicAmbientVolume, sfxUseDynamicEnvironmentVolume;

struct OPENAL_CHANNELGROUP;

struct OPENAL_BUFFER;
struct OPENAL_SOUND;

struct Vec3 {
	float x,y,z;
};

extern OPENAL_BUFFER** sounds;
extern OPENAL_BUFFER** minesmusic;
#define NUMMINESMUSIC 5
extern OPENAL_BUFFER** swampmusic;
#define NUMSWAMPMUSIC 4
extern OPENAL_BUFFER** labyrinthmusic;
#define NUMLABYRINTHMUSIC 3
extern OPENAL_BUFFER** ruinsmusic;
#define NUMRUINSMUSIC 3
extern OPENAL_BUFFER** underworldmusic;
#define NUMUNDERWORLDMUSIC 3
extern OPENAL_BUFFER** hellmusic;
#define NUMHELLMUSIC 3
extern OPENAL_BUFFER** intromusic, *intermissionmusic, *minetownmusic, *splashmusic, *librarymusic, *shopmusic, *storymusic;
extern OPENAL_BUFFER** minotaurmusic, *herxmusic, *templemusic;
extern OPENAL_BUFFER* endgamemusic, *escapemusic, *devilmusic, *sanctummusic, *tutorialmusic, *introstorymusic, *gameovermusic;
extern OPENAL_BUFFER* introductionmusic;
#define NUMMINOTAURMUSIC 2
extern OPENAL_BUFFER** cavesmusic;
extern OPENAL_BUFFER** citadelmusic;
extern OPENAL_BUFFER* gnomishminesmusic;
extern OPENAL_BUFFER* greatcastlemusic;
extern OPENAL_BUFFER* sokobanmusic;
extern OPENAL_BUFFER* caveslairmusic;
extern OPENAL_BUFFER* bramscastlemusic;
extern OPENAL_BUFFER* hamletmusic;
extern OPENAL_BUFFER** fortressmusic;
#define NUMCAVESMUSIC 3
#define NUMCITADELMUSIC 3
#define NUMINTROMUSIC 3
#define NUMFORTRESSMUSIC 2
//TODO: Automatically scan the music folder for a mines subdirectory and use all the music for the mines or something like that. I'd prefer something neat like for that loading music for a level, anyway. And I can just reuse the code I had for ORR.

extern OPENAL_SOUND* music_channel, *music_channel2, *music_resume; //TODO: List of music, play first one, fade out all the others? Eh, maybe some other day. //music_resume is the music to resume after, say, combat or shops. //TODO: Clear music_resume every biome change. Or otherwise validate it for that level set.
extern OPENAL_CHANNELGROUP *sound_group, *music_group;
extern OPENAL_CHANNELGROUP *soundAmbient_group, *soundEnvironment_group, *music_notification_group;

extern "C" int initOPENAL();
extern "C" int closeOPENAL();

extern "C" void sound_update(int player, int index, int numplayers);

extern "C" OPENAL_SOUND* playSoundPlayer(int player, Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundPos(real_t x, real_t y, Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundPosLocal(real_t x, real_t y, Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundEntity(Entity* entity, Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundEntityLocal(Entity* entity, Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSound(Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundVelocity(); //TODO: Write.
extern "C" OPENAL_SOUND* playSoundNotification(Uint16 snd, Uint8 vol);
extern "C" OPENAL_SOUND* playSoundNotificationPlayer(int player, Uint16 snd, Uint8 vol);

extern "C" void stopMusic();
extern "C" void playMusic(OPENAL_BUFFER* sound, bool loop, bool crossfade, bool resume); //Automatically crossfades. NOTE: Resets fadein and fadeout increments to the defaults every time it is called. You'll have to change the fadein and fadeout increments AFTER calling this function.

extern "C" void handleLevelMusic(); //Manages and updates the level music.

extern "C" int OPENAL_CreateSound(const char* name, bool b3D, OPENAL_BUFFER **buffer);
extern "C" int OPENAL_CreateStreamSound(const char* name, OPENAL_BUFFER **buffer);

extern "C" void OPENAL_ChannelGroup_Stop(OPENAL_CHANNELGROUP* group);
extern "C" void OPENAL_ChannelGroup_SetVolume(OPENAL_CHANNELGROUP* group, float f);
extern "C" void OPENAL_Channel_SetChannelGroup(OPENAL_SOUND *channel, OPENAL_CHANNELGROUP *group);
extern "C" void OPENAL_Channel_SetVolume(OPENAL_SOUND *channel, float f);
extern "C" void OPENAL_Channel_Stop(void* channel);
extern "C" void OPENAL_Channel_Pause(OPENAL_SOUND* channel);
extern "C" void OPENAL_Channel_IsPlaying(void* channel, ALboolean *playing);
extern "C" OPENAL_SOUND* OPENAL_CreateChannel(OPENAL_BUFFER* buffer);
extern "C" void OPENAL_Channel_Set3DAttributes(OPENAL_SOUND* channel, float x, float y, float z);
extern "C" void OPENAL_Channel_Play(OPENAL_SOUND* channel);
extern "C" void OPENAL_GetBuffer(OPENAL_SOUND* channel, OPENAL_BUFFER** buffer);
extern "C" void OPENAL_SetLoop(OPENAL_SOUND* channel, ALboolean looping);
extern "C" void OPENAL_Channel_GetPosition(OPENAL_SOUND* channel, unsigned int *position);
extern "C" void OPENAL_Sound_GetLength(OPENAL_BUFFER* buffer, unsigned int *length);
extern "C" void OPENAL_Sound_Release(OPENAL_BUFFER* buffer);

extern float fadein_increment, fadeout_increment, default_fadein_increment, default_fadeout_increment;
