/*-------------------------------------------------------------------------------

	BARONY
	File: init_audio.cpp
	Desc: init, load, exit audio engine stuff.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../../main.hpp"
#include "../../files.hpp"
#include "../../ui/LoadingScreen.hpp"
#include "sound.hpp"

#ifndef EDITOR
#include "../../ui/MainMenu.hpp"
#endif


bool initSoundEngine()
{
	if (!no_sound)
	{
		initOPENAL();
	}

#ifndef EDITOR
	// saves your ears getting blasted if the game starts without window focus.
	setGlobalVolume(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
#endif

	return !no_sound; //No double negatives pls
}

int loadSoundResources(real_t base_load_percent, real_t top_load_percent)
{
	File* fp;
	Uint32 c;
	char name[128];

	if ( !PHYSFS_getRealDir("sound/sounds.txt") )
	{
		printlog("error: could not find file: %s", "sound/sounds.txt");
		return 10;
	}

	// load sound effects
	std::string soundsDirectory = PHYSFS_getRealDir("sound/sounds.txt");
	soundsDirectory.append(PHYSFS_getDirSeparator()).append("sound/sounds.txt");
	printlog("loading sounds...\n");
	fp = openDataFile(soundsDirectory.c_str(), "rb");
	for ( numsounds = 0; !fp->eof(); ++numsounds )
	{
		while ( fp->getc() != '\n' )
		{
			if ( fp->eof() )
			{
				break;
			}
		}
	}
	FileIO::close(fp);
	if ( numsounds == 0 )
	{
		printlog("failed to identify any sounds in sounds.txt\n");
		return 10;
	}
	sounds = (OPENAL_BUFFER**) malloc(sizeof(OPENAL_BUFFER*)*numsounds);
	for (c = 0, fp = openDataFile(soundsDirectory.c_str(), "rb"); fp->gets2(name, 128); ++c)
	{
		//TODO: Might need to malloc the sounds[c]->sound
		OPENAL_CreateSound(name, true, &sounds[c]);
		//TODO: set sound volume? Or otherwise handle sound volume.
		updateLoadingScreen(base_load_percent + (top_load_percent * c) / numsounds);
	}
	FileIO::close(fp);
	//FMOD_System_Set3DSettings(fmod_system, 1.0, 2.0, 1.0); // This on is hardcoded, I've been lazy here'

	return 0;
}

void freeSoundResources()
{
	uint32_t c;
	// free sounds
}

void exitSoundEngine()
{
}
