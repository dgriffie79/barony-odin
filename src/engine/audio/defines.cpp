/*-------------------------------------------------------------------------------

	BARONY
	File: defines.cpp
	Desc: defines extern'd sound variables and stuff. This should really all be part of a class.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../../main.hpp"
#include "sound.hpp"

Uint32 numsounds = 0;

// Shared music state (used by both FMOD and OpenAL paths, and the editor).
bool levelmusicplaying = false;
bool shopmusicplaying = false;
bool combatmusicplaying = false;
bool minotaurmusicplaying = false;
bool herxmusicplaying = false;
bool devilmusicplaying = false;
bool olddarkmap = false;
bool sanctummusicplaying = false;
int currenttrack = -1;

