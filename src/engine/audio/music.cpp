/*-------------------------------------------------------------------------------

	BARONY
	File: music.cpp
	Desc: Contains any music-specific code that would have gone into sound_game.cpp

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../../main.hpp"
#include "../../game.hpp"
#include "sound.hpp"
#include "../../entity.hpp"
#include "../../player.hpp"
#include "../../prng.hpp"
#include "../../files.hpp"
 #include "music_pc.hpp"

bool loadMusic()
{
#ifdef EDITOR
	return true;
#endif
	// Allocate the OpenAL music buffer arrays. OPENAL_CreateStreamSound just
	// records the file path; the actual decode/streaming happens at play time
	// (openal_oggopen in OPENAL_CreateChannel).
	if ( NUMMINESMUSIC > 0 ) { minesmusic = (OPENAL_BUFFER**)calloc(NUMMINESMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMSWAMPMUSIC > 0 ) { swampmusic = (OPENAL_BUFFER**)calloc(NUMSWAMPMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMLABYRINTHMUSIC > 0 ) { labyrinthmusic = (OPENAL_BUFFER**)calloc(NUMLABYRINTHMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMRUINSMUSIC > 0 ) { ruinsmusic = (OPENAL_BUFFER**)calloc(NUMRUINSMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMUNDERWORLDMUSIC > 0 ) { underworldmusic = (OPENAL_BUFFER**)calloc(NUMUNDERWORLDMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMHELLMUSIC > 0 ) { hellmusic = (OPENAL_BUFFER**)calloc(NUMHELLMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMMINOTAURMUSIC > 0 ) { minotaurmusic = (OPENAL_BUFFER**)calloc(NUMMINOTAURMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMCAVESMUSIC > 0 ) { cavesmusic = (OPENAL_BUFFER**)calloc(NUMCAVESMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMCITADELMUSIC > 0 ) { citadelmusic = (OPENAL_BUFFER**)calloc(NUMCITADELMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMINTROMUSIC > 0 ) { intromusic = (OPENAL_BUFFER**)calloc(NUMINTROMUSIC, sizeof(OPENAL_BUFFER*)); }
	if ( NUMFORTRESSMUSIC > 0 ) { fortressmusic = (OPENAL_BUFFER**)calloc(NUMFORTRESSMUSIC, sizeof(OPENAL_BUFFER*)); }

    bool introMusicChanged;
	physfsReloadMusic(introMusicChanged, true);
    return true;
}

void stopMusic()
{
#ifdef SOUND
    playMusic(nullptr, false, false, false);
#endif
}


