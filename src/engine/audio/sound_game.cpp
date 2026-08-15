/*-------------------------------------------------------------------------------

	BARONY
	File: sound_game.cpp
	Desc: Contains all the code that will cause the editor to crash and burn.
	Quick workaround because I don't want to separate the editor and game
	into two separate projects just because of sound.

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../../main.hpp"
#include "../../game.hpp"
#include "../../stat.hpp"
#include "sound.hpp"
#include "../../entity.hpp"
#include "../../net.hpp"
#include "../../player.hpp"
#include "../../ui/GameUI.hpp"
#include "../../ui/MainMenu.hpp"
#include "../../mod_tools.hpp"
#include "../../ui/Button.hpp"

/*-------------------------------------------------------------------------------

	playSoundPlayer

	has the given player play the specified global sound effect. Mostly
	used by the server to instruct clients to play a certain sound.

-------------------------------------------------------------------------------*/
OPENAL_CHANNELGROUP* getChannelGroupForSoundIndex(Uint32 snd)
{
	if ( snd == 155 || snd == 135 ) // water/lava
	{
		return soundEnvironment_group;
	}
	if ( snd == 149 || snd == 133 )
	{
		return soundAmbient_group;
	}
	return sound_group;
}

OPENAL_SOUND* playSoundPlayer(int player, Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}


	if ( player < 0 || player >= MAXPLAYERS )   //Perhaps this can be reprogrammed to remove MAXPLAYERS, and use a pointer to the player instead of an int?
	{
		return NULL;
	}
	if ( players[player]->isLocalPlayer() )
	{
		return playSound(snd, vol);
	}
	else if ( multiplayer == SERVER && vol > 0 )
	{
		if ( client_disconnected[player] || player <= 0 )
		{
			return NULL;
		}
		memcpy(net_packet->data, "SNDG", 4);
		SDLNet_Write16(snd, &net_packet->data[4]);
		net_packet->data[6] = vol;
		net_packet->address.host = net_clients[player - 1].host;
		net_packet->address.port = net_clients[player - 1].port;
		net_packet->len = 7;
		sendPacketSafe(net_sock, -1, net_packet, player - 1);
		return NULL;
	}

	return NULL;
}

/*-------------------------------------------------------------------------------

	playSoundNotificationPlayer

	Plays a global notification sound on the given player's channel.

-------------------------------------------------------------------------------*/

OPENAL_SOUND* playSoundNotificationPlayer(int player, Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}

	if ( player < 0 || player >= MAXPLAYERS )
	{
		return NULL;
	}
	if ( players[player]->isLocalPlayer() )
	{
		return playSoundNotification(snd, vol);
	}
	else if ( multiplayer == SERVER && vol > 0 )
	{
		if ( client_disconnected[player] || player <= 0 )
		{
			return NULL;
		}
		memcpy(net_packet->data, "SNDN", 4);
		SDLNet_Write16(snd, &net_packet->data[4]);
		net_packet->data[6] = vol;
		net_packet->address.host = net_clients[player - 1].host;
		net_packet->address.port = net_clients[player - 1].port;
		net_packet->len = 7;
		sendPacketSafe(net_sock, -1, net_packet, player - 1);
		return NULL;
	}

	return NULL;
}

/*-------------------------------------------------------------------------------

	playSoundNotification

	Plays a global notification sound (UI blips etc.), attached to the
	notification channel group so it isn't positional.

-------------------------------------------------------------------------------*/

OPENAL_SOUND* playSoundNotification(Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}
#ifndef SOUND
	return NULL;
#endif
	if ( !openal_context || snd < 0 || snd >= numsounds || !getChannelGroupForSoundIndex(snd) )
	{
		return NULL;
	}
	if ( sounds[snd] == NULL || vol == 0 )
	{
		return NULL;
	}
	OPENAL_SOUND* channel = OPENAL_CreateChannel(sounds[snd]);
	OPENAL_Channel_SetVolume(channel, vol / 255.f);
	OPENAL_Channel_SetChannelGroup(channel, music_notification_group);
	OPENAL_Channel_Play(channel);
	return channel;
}

/*-------------------------------------------------------------------------------

	playSoundPos

	plays a sound effect with the given volume at the given
	position; returns the channel that the sound is playing in

-------------------------------------------------------------------------------*/

OPENAL_SOUND* playSoundPos(real_t x, real_t y, Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}

#ifndef SOUND
	return NULL;
#endif

	OPENAL_SOUND* channel;
	int c;

	if (intro)
	{
		return NULL;
	}
	if (snd < 0 || snd >= numsounds)
	{
		return NULL;
	}
	if (sounds[snd] == NULL || vol == 0)
	{
		return NULL;
	}

	if (multiplayer == SERVER && vol > 0 )
	{
		for (c = 1; c < MAXPLAYERS; c++)
		{
			if ( client_disconnected[c] == true || players[c]->isLocalPlayer() )
			{
				continue;
			}
			memcpy(net_packet->data, "SNDP", 4);
			SDLNet_Write32(x, &net_packet->data[4]);
			SDLNet_Write32(y, &net_packet->data[8]);
			SDLNet_Write16(snd, &net_packet->data[12]);
			net_packet->data[14] = vol;
			net_packet->address.host = net_clients[c - 1].host;
			net_packet->address.port = net_clients[c - 1].port;
			net_packet->len = 15;
			sendPacketSafe(net_sock, -1, net_packet, c - 1);
		}
	}

	if (!openal_context)   //For the client.
	{
		return NULL;
	}

	channel = OPENAL_CreateChannel(sounds[snd]);
	OPENAL_Channel_SetVolume(channel, vol / 255.f);
	OPENAL_Channel_Set3DAttributes(channel, -y / 16.0, 0, -x / 16.0);
	OPENAL_Channel_SetChannelGroup(channel, getChannelGroupForSoundIndex(snd));
	OPENAL_Channel_Play(channel);

	return channel;
}

OPENAL_SOUND* playSoundPosLocal(real_t x, real_t y, Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}

#ifndef SOUND
	return NULL;
#endif

	OPENAL_SOUND* channel;

	if (intro)
	{
		return NULL;
	}
	if (snd < 0 || snd >= numsounds)
	{
		return NULL;
	}
	if (sounds[snd] == NULL || vol == 0)
	{
		return NULL;
	}

	if (!openal_context)   //For the client.
	{
		return NULL;
	}

	channel = OPENAL_CreateChannel(sounds[snd]);
	OPENAL_Channel_SetVolume(channel, vol / 255.f);
	OPENAL_Channel_Set3DAttributes(channel, -y / 16.0, 0, -x / 16.0);
	OPENAL_Channel_SetChannelGroup(channel, getChannelGroupForSoundIndex(snd));
	OPENAL_Channel_Play(channel);

	return channel;
}

/*-------------------------------------------------------------------------------

	playSoundEntity

	plays a sound effect with the given volume at the given entity's
	position; returns the channel that the sound is playing in

-------------------------------------------------------------------------------*/

OPENAL_SOUND* playSoundEntity(Entity* entity, Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}

	if ( entity == NULL )
	{
		return NULL;
	}
	return playSoundPos(entity->x, entity->y, snd, vol);
}

OPENAL_SOUND* playSoundEntityLocal(Entity* entity, Uint16 snd, Uint8 vol)
{
	if ( entity == NULL )
	{
		return NULL;
	}
	return playSoundPosLocal(entity->x, entity->y, snd, vol);
}

/*-------------------------------------------------------------------------------

	playSound

	plays a sound effect with the given volume and returns the channel that
	the sound is playing in

-------------------------------------------------------------------------------*/

OPENAL_SOUND* playSound(Uint16 snd, Uint8 vol)
{
	if (no_sound)
	{
		return NULL;
	}
#ifndef SOUND
	return NULL;
#endif
	if (!openal_context || snd < 0 || snd >= numsounds || !getChannelGroupForSoundIndex(snd) )
	{
		return NULL;
	}
	if (sounds[snd] == NULL || vol == 0)
	{
		return NULL;
	}
	OPENAL_SOUND* channel = OPENAL_CreateChannel(sounds[snd]);
	OPENAL_Channel_SetVolume(channel, vol / 255.f);

	OPENAL_Channel_SetChannelGroup(channel, getChannelGroupForSoundIndex(snd));

	OPENAL_Channel_Play(channel);

	return channel;
}

void playMusic(OPENAL_BUFFER* sound, bool loop, bool crossfade, bool resume)
{
	if (no_sound)
	{
		return;
	}
#ifndef SOUND
	return;
#endif
#ifndef MUSIC
	return;
#endif
	fadein_increment = default_fadein_increment;
	fadeout_increment = default_fadeout_increment;
	if (!openal_context || !sound)
	{
		printlog("Can't play music.\n");
		return;
	}
	if ( resume && music_channel2 )
	{
		OPENAL_BUFFER* lastmusic = NULL;
		OPENAL_GetBuffer(music_channel2, &lastmusic);
		if ( lastmusic == sound )
		{
			OPENAL_SOUND* tempmusic = music_channel;
			music_channel = music_channel2;
			music_channel2 = tempmusic;
		}
		else
		{
			OPENAL_Channel_Stop(music_channel2);
			music_channel2 = music_channel;
			music_channel = OPENAL_CreateChannel(sound);
		}
	}
	else
	{
		OPENAL_Channel_Stop(music_channel2);
		music_channel2 = music_channel;
		music_channel = OPENAL_CreateChannel(sound);
	}
	OPENAL_Channel_SetChannelGroup(music_channel, music_group);
	if (crossfade == true)
	{
		//Start at volume 0 to get louder.
		OPENAL_Channel_SetVolume(music_channel, 0.0f); //Start at 0 then pop up.
	}
	else
	{
		OPENAL_Channel_SetVolume(music_channel, 1.0f);
		OPENAL_Channel_Stop(music_channel2);
		music_channel2 = NULL;
	}
	if (loop == true)
	{
		OPENAL_SetLoop(music_channel, AL_TRUE);
	}
	OPENAL_Channel_Play(music_channel);
}

void handleLevelMusic()
{
	if (no_sound)
	{
		return;
	}
#ifndef SOUND
	return;
#endif
#ifndef MUSIC
	return;
#endif
	bool inshop = false;
	if (players[clientnum] && players[clientnum]->entity)
	{
		int x = (int)players[clientnum]->entity->x / 16;
		int y = (int)players[clientnum]->entity->y / 16;
		if ( x >= 0 && x < map.width && y >= 0 && y < map.height )
			if ( shoparea[y + x * map.height] )
			{
				inshop = true;
			}
	}

	bool devilaround = false;
	bool activeminotaur = false;
	bool herxaround = false;
	bool magisteraround = false;
	node_t* node;
	for ( node = map.creatures->first; node != NULL; node = node->next )
	{
		Entity* entity = (Entity*)node->element;
		if ( entity->sprite == 274 )   // herx head
		{
			herxaround = true;
			break;
		}
		else if ( entity->sprite == 304 )     // devil body
		{
			devilaround = true;
			break;
		}
		else if ( entity->sprite == 239 )     // minotaur head
		{
			activeminotaur = true;
			break;
		}
		else if ( entity->sprite == 646 || entity->sprite == 650 )     // magister body
		{
			magisteraround = true;
			break;
		}
	}

	ALboolean playing = true;
	OPENAL_Channel_IsPlaying(music_channel, &playing);

	if ( currenttrack == -1 )
	{
		currenttrack = local_rng.rand();
	}

	if ( (!levelmusicplaying || !playing || olddarkmap != darkmap) 
		&& (!combat || !strcmp(map.name, "Hell Boss")) 
		&& !inshop 
		&& (!activeminotaur || !strcmp(map.name, "Hell Boss")) && !herxaround && !devilaround && !magisteraround )
	{
		if ( !strncmp(map.name, "The Mines", 9) )     // the mines
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMMINESMUSIC - 1);
			}
			currenttrack = currenttrack % NUMMINESMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(minesmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "The Swamp", 9) )     // the swamp
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMSWAMPMUSIC - 1);
			}
			currenttrack = currenttrack % NUMSWAMPMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(swampmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "The Labyrinth", 13) )     // the labyrinth
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMLABYRINTHMUSIC - 1);
			}
			currenttrack = currenttrack % NUMLABYRINTHMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(labyrinthmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "The Ruins", 9) )     // the ruins
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMRUINSMUSIC - 1);
			}
			currenttrack = currenttrack % NUMRUINSMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(ruinsmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "Underworld", 10) )     // the underworld
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMUNDERWORLDMUSIC - 1);
			}
			currenttrack = currenttrack % NUMUNDERWORLDMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(underworldmusic[currenttrack], false, true, true);
		}
		else if ( !strcmp(map.name, "Minetown") )     // minetown
		{
			playMusic(minetownmusic, true, true, true);
		}
		else if ( !strcmp(map.name, "The Gnomish Mines") )
		{
			if ( gnomishminesmusic )
			{
				playMusic(gnomishminesmusic, true, true, true);
			}
			else
			{
				playMusic(minetownmusic, true, true, true);
			}
		}
		else if ( !strcmp(map.name, "The Haunted Castle") )
		{
			if ( greatcastlemusic )
			{
				playMusic(greatcastlemusic, true, true, true);
			}
			else
			{
				playMusic(intermissionmusic, true, true, true);
			}
		}
		else if ( !strcmp(map.name, "Sokoban") )
		{
			if ( sokobanmusic )
			{
				playMusic(sokobanmusic, true, true, true);
			}
			else
			{
				playMusic(intermissionmusic, true, true, true);
			}
		}
		else if ( !strcmp(map.name, "Cockatrice Lair") )
		{
			if ( caveslairmusic )
			{
				playMusic(caveslairmusic, true, true, true);
			}
			else
			{
				playMusic(cavesmusic[2], true, true, true);
			}
		}
		else if ( !strcmp(map.name, "Bram's Castle") )
		{
			if ( bramscastlemusic )
			{
				playMusic(bramscastlemusic, true, true, true);
			}
			else
			{
				playMusic(citadelmusic[2], true, true, true);
			}
		}
		else if ( !strcmp(map.name, "The Mystic Library") )     // mystic library
		{
			playMusic(librarymusic, true, true, true);
		}
		else if ( !strcmp(map.name, "The Minotaur Maze") )     // minotaur maze
		{
			playMusic(minotaurmusic[1], true, true, true);
		}
		else if ( !strcmp(map.name, "The Temple") )     // the temple
		{
			playMusic(templemusic, true, true, true);
		}
		else if ( !strcmp(map.name, "Hell Boss") )     // escape theme
		{
			playMusic(escapemusic, true, true, true);
		}
		else if ( !strncmp(map.name, "Hell", 4) )     // hell
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMHELLMUSIC - 1);
			}
			currenttrack = currenttrack % NUMHELLMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(hellmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "Caves", 5) )
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMCAVESMUSIC - 1);
			}
			currenttrack = currenttrack % NUMCAVESMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(cavesmusic[currenttrack], false, true, true);
		}
		else if ( !strncmp(map.name, "Citadel", 7) || !strncmp(map.name, "Sanctum", 7) )
		{
			if ( !playing )
			{
				currenttrack = 1 + local_rng.rand() % (NUMCITADELMUSIC - 1);
			}
			currenttrack = currenttrack % NUMCITADELMUSIC;
			if ( currenttrack == 0 )
			{
				currenttrack = 1;
			}
			playMusic(citadelmusic[currenttrack], false, true, true);
		}
		else if ( !strcmp(map.name, "Mages Guild") )
		{
			if ( hamletmusic )
			{
				playMusic(hamletmusic, true, true, true);
			}
			else
			{
				playMusic(minesmusic[4], true, true, true);
			}
		}
		else
		{
			playMusic(intermissionmusic, true, true, true);
		}
		olddarkmap = darkmap;
		levelmusicplaying = true;
		devilmusicplaying = false;
		herxmusicplaying = false;
		minotaurmusicplaying = false;
		combatmusicplaying = false;
		shopmusicplaying = false;
		fadein_increment = default_fadein_increment;
		fadeout_increment = default_fadeout_increment;
	}
	else if ( (!devilmusicplaying || !playing) && devilaround )
	{
		playMusic(devilmusic, true, true, true);
		levelmusicplaying = false;
		devilmusicplaying = true;
		herxmusicplaying = false;
		minotaurmusicplaying = false;
		combatmusicplaying = false;
		shopmusicplaying = false;
		fadein_increment = default_fadein_increment * 2;
		fadeout_increment = default_fadeout_increment * 2;
	}
	else if ( (!herxmusicplaying || !playing) && !devilaround && herxaround )
	{
		playMusic(herxmusic, true, true, true);
		levelmusicplaying = false;
		devilmusicplaying = false;
		herxmusicplaying = true;
		minotaurmusicplaying = false;
		combatmusicplaying = false;
		shopmusicplaying = false;
		fadein_increment = default_fadein_increment * 2;
		fadeout_increment = default_fadeout_increment * 2;
	}
	else if ( (!minotaurmusicplaying || !playing) && !herxaround && activeminotaur && strcmp(map.name, "Hell Boss") )
	{
		playMusic(minotaurmusic[0], true, true, true);
		levelmusicplaying = false;
		devilmusicplaying = false;
		herxmusicplaying = false;
		minotaurmusicplaying = true;
		combatmusicplaying = false;
		shopmusicplaying = false;
		fadein_increment = default_fadein_increment * 5;
		fadeout_increment = default_fadeout_increment * 5;
	}
	else if ( (!sanctummusicplaying || !playing) && magisteraround )
	{
		playMusic(sanctummusic, true, true, true);
		levelmusicplaying = false;
		devilmusicplaying = false;
		herxmusicplaying = false;
		minotaurmusicplaying = false;
		combatmusicplaying = false;
		sanctummusicplaying = true;
		shopmusicplaying = false;
		fadein_increment = default_fadein_increment * 2;
		fadeout_increment = default_fadeout_increment * 2;
	}
	else if ( (!combatmusicplaying || !playing) 
		&& !herxaround 
		&& !activeminotaur 
		&& combat 
		&& strcmp(map.name, "Hell Boss")
		&& strcmp(map.name, "Sanctum") )
	{
		if ( !strncmp(map.name, "The Swamp", 9) || !strncmp(map.name, "The Temple", 10) )   // the swamp
		{
			playMusic(swampmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "The Labyrinth", 13) || strstr(map.name, "Minotaur") )     // the labyrinth
		{
			playMusic(labyrinthmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "The Ruins", 9) )     // the ruins
		{
			playMusic(ruinsmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "Underworld", 10) )     // the underworld
		{
			playMusic(underworldmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "Hell", 4) )     // hell
		{
			playMusic(hellmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "Caves", 5) || !strcmp(map.name, "Cockatrice Lair") )
		{
			playMusic(cavesmusic[0], true, true, true);
		}
		else if ( !strncmp(map.name, "Citadel", 7) || !strcmp(map.name, "Bram's Castle") )
		{
			playMusic(citadelmusic[0], true, true, true);
		}
		else
		{
			playMusic(minesmusic[0], true, true, true);
		}
		levelmusicplaying = false;
		devilmusicplaying = false;
		herxmusicplaying = false;
		combatmusicplaying = true;
		shopmusicplaying = false;
		minotaurmusicplaying = false;
		fadein_increment = default_fadein_increment * 4;
		fadeout_increment = default_fadeout_increment;
	}
	else if ( (!shopmusicplaying || !playing) && !herxaround && !activeminotaur && !combat && inshop )
	{
		playMusic(shopmusic, true, true, true);
		levelmusicplaying = false;
		devilmusicplaying = false;
		herxmusicplaying = false;
		minotaurmusicplaying = false;
		combatmusicplaying = false;
		shopmusicplaying = true;
		fadein_increment = default_fadein_increment * 4;
		fadeout_increment = default_fadeout_increment;
	}
}


