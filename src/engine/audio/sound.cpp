/*-------------------------------------------------------------------------------

	BARONY
	File: sound.cpp
	Desc: various sound functions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../../main.hpp"
#include "../../files.hpp"
#include "../../game.hpp"
#include "sound.hpp"
#ifndef EDITOR
#include "../../player.hpp"
#endif

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

void setGlobalVolume(real_t master, real_t music, real_t gameplay, real_t ambient, real_t environment, real_t notification)
{
	master = std::min(std::max(0.0, master), 1.0);
	music = std::min(std::max(0.0, music / 4.0), 1.0); // music volume cut in half because the music is loud...
	gameplay = std::min(std::max(0.0, gameplay), 1.0);
	ambient = std::min(std::max(0.0, ambient), 1.0);
	environment = std::min(std::max(0.0, environment), 1.0);
	notification = std::min(std::max(0.0, notification), 1.0);

	OPENAL_ChannelGroup_SetVolume(music_group, master * music);
	OPENAL_ChannelGroup_SetVolume(sound_group, master * gameplay);
	OPENAL_ChannelGroup_SetVolume(soundAmbient_group, master * ambient);
	OPENAL_ChannelGroup_SetVolume(soundEnvironment_group, master * environment);
	OPENAL_ChannelGroup_SetVolume(music_notification_group, master * notification);
}
void setAudioDevice(const std::string& device)
{
	// OpenAL device selection is done at initOPENAL() time via alcOpenDevice(NULL).
	// Selecting a specific device at runtime is not implemented for OpenAL.
	(void)device;
}
void setRecordDevice(const std::string& device)
{
	// OpenAL recording device selection is not implemented.
	(void)device;
}


struct OPENAL_BUFFER {
	ALuint id;
	bool stream;
	char oggfile[64];
};
struct OPENAL_SOUND {
	ALuint id;
	OPENAL_CHANNELGROUP *group;
	float volume;
	OPENAL_BUFFER *buffer;
	bool active;
	char* oggdata;
	int oggdata_length;
	int ogg_seekoffset;
	OggVorbis_File oggStream;
	vorbis_info* vorbisInfo;
	vorbis_comment* vorbisComment;
	ALuint streambuff[4];
	bool loop;
	bool stream_active;
	int indice;
};

struct OPENAL_CHANNELGROUP {
	float volume;
	int num;
	int cap;
	OPENAL_SOUND **sounds;
};

SDL_mutex *openal_mutex;

static size_t openal_oggread(void* ptr, size_t size, size_t nmemb, void* datasource) {
	OPENAL_SOUND* self = (OPENAL_SOUND*)datasource;

	int bytes = size*nmemb;
	int remain = self->oggdata_length - self->ogg_seekoffset - bytes;
	if(remain < 0) bytes += remain;

	memcpy(ptr, self->oggdata + self->ogg_seekoffset, bytes);
	self->ogg_seekoffset += bytes;

	return bytes;
}

static int openal_oggseek(void* datasource, ogg_int64_t offset, int whence) {
	OPENAL_SOUND* self = (OPENAL_SOUND*)datasource;
	int seek_offset;

	switch(whence) {
	case SEEK_CUR:
		seek_offset = self->ogg_seekoffset + offset;
		break;
	case SEEK_END:
		seek_offset = self->oggdata_length + offset;
		break;
	case SEEK_SET:
		seek_offset = offset;
		break;
	/*default:
		exit(1);*/
	}
	if(seek_offset > self->oggdata_length) return -1;

	self->ogg_seekoffset = seek_offset;
	return 0;
}

static int openal_oggclose(void* datasource) {
	return 0;
}

static long int openal_oggtell(void* datasource) {
	OPENAL_SOUND* self = (OPENAL_SOUND*)datasource;
	return self->ogg_seekoffset;
}

static int openal_oggopen(OPENAL_SOUND *self, const char* oggfile) {
	File *f = openDataFile(oggfile, "rb");
	int err;

	ov_callbacks oggcb = {openal_oggread, openal_oggseek, openal_oggclose, openal_oggtell};

	if(!f) {
		return 0;
	}

	self->ogg_seekoffset = 0;
	self->oggdata_length = f->size();

	self->oggdata = (char*)malloc(self->oggdata_length);
	f->read(self->oggdata, sizeof(char), self->oggdata_length);
	FileIO::close(f);

	if(ov_open_callbacks(self, &self->oggStream, 0, 0, oggcb)) {
		printf("Issues with OGG callbacks\n");
		return 0;
	}

	self->vorbisInfo = ov_info(&self->oggStream, -1);
	self->vorbisComment = ov_comment(&self->oggStream, -1);

	alGenBuffers(4, self->streambuff);
	return 1;
}

static int openal_oggrelease(OPENAL_SOUND *self) {
	alSourceStop(self->id);
	ov_raw_seek(&self->oggStream, 0);
	int queued;
	alGetSourcei(self->id, AL_BUFFERS_QUEUED, &queued);
	while(queued--) {
		ALuint buffer;
		alSourceUnqueueBuffers(self->id, 1, &buffer);
	}
	alDeleteBuffers(4, self->streambuff);
	ov_clear(&self->oggStream);
	free(self->oggdata);
	return 1;
}

static int openal_streamread(OPENAL_SOUND *self, ALuint buffer) {
	#define OGGSIZE 65536
	char pcm[OGGSIZE];
	int size = 0;
	int section;
	int result;


	while (size < OGGSIZE) {
		result = ov_read(&self->oggStream, pcm+size, OGGSIZE -size, 0, 2, 1, &section);
		if(result==0 && self->loop)
			ov_raw_seek(&self->oggStream, 0);

		if(result>0)
			size += result;
		else
			break;
	}

	if(size==0) {
		return 0;
	}
	alBufferData(buffer, 
		(self->vorbisInfo->channels==1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16, 
		pcm, size, self->vorbisInfo->rate);

	return 1;

	#undef OGGSIZE
}

static int openal_streamupdate(OPENAL_SOUND* self) {
	int processed;
	int active = 1;

	alGetSourcei(self->id, AL_BUFFERS_PROCESSED, &processed);

	while(processed--) {
		ALuint buffer;

		alSourceUnqueueBuffers(self->id, 1, &buffer);

		active = openal_streamread(self, buffer);
		if(active)
			alSourceQueueBuffers(self->id, 1, &buffer);
	}
	self->stream_active = active;

	return active;
}

bool sfxUseDynamicAmbientVolume = true;
bool sfxUseDynamicEnvironmentVolume = true;

ALCcontext *openal_context = nullptr;
ALCdevice  *openal_device = nullptr;

//#define openal_maxchannels 100

OPENAL_BUFFER** sounds = nullptr;
OPENAL_BUFFER** minesmusic = NULL;
OPENAL_BUFFER** swampmusic = NULL;
OPENAL_BUFFER** labyrinthmusic = NULL;
OPENAL_BUFFER** ruinsmusic = NULL;
OPENAL_BUFFER** underworldmusic = NULL;
OPENAL_BUFFER** hellmusic = NULL;
OPENAL_BUFFER** intromusic = NULL;
OPENAL_BUFFER* intermissionmusic = NULL;
OPENAL_BUFFER* minetownmusic = NULL;
OPENAL_BUFFER* splashmusic = NULL;
OPENAL_BUFFER* librarymusic = NULL;
OPENAL_BUFFER* shopmusic = NULL;
OPENAL_BUFFER* storymusic = NULL;
OPENAL_BUFFER** minotaurmusic = NULL;
OPENAL_BUFFER* herxmusic = NULL;
OPENAL_BUFFER* templemusic = NULL;
OPENAL_BUFFER* endgamemusic = NULL;
OPENAL_BUFFER* devilmusic = NULL;
OPENAL_BUFFER* escapemusic = NULL;
OPENAL_BUFFER* sanctummusic = NULL;
OPENAL_BUFFER* introductionmusic = NULL;
OPENAL_BUFFER** cavesmusic = NULL;
OPENAL_BUFFER** citadelmusic = NULL;
OPENAL_BUFFER* gnomishminesmusic = NULL;
OPENAL_BUFFER* greatcastlemusic = NULL;
OPENAL_BUFFER* sokobanmusic = NULL;
OPENAL_BUFFER* caveslairmusic = NULL;
OPENAL_BUFFER* bramscastlemusic = NULL;
OPENAL_BUFFER* hamletmusic = NULL;
OPENAL_BUFFER** fortressmusic = NULL;
OPENAL_BUFFER* tutorialmusic = nullptr;
OPENAL_BUFFER* gameovermusic = nullptr;
OPENAL_BUFFER* introstorymusic = nullptr;

OPENAL_SOUND* music_channel = nullptr;
OPENAL_SOUND* music_channel2 = nullptr;
OPENAL_SOUND* music_resume = nullptr;

OPENAL_CHANNELGROUP *sound_group = NULL;
OPENAL_CHANNELGROUP *soundAmbient_group = NULL;
OPENAL_CHANNELGROUP *soundEnvironment_group = NULL;
OPENAL_CHANNELGROUP *music_group = NULL;
OPENAL_CHANNELGROUP *music_notification_group = NULL;

float fadein_increment = 0.002f;
float default_fadein_increment = 0.002f;
float fadeout_increment = 0.005f;
float default_fadeout_increment = 0.005f;

#define MAXSOUND 1024
OPENAL_SOUND openal_sounds[MAXSOUND];
int lower_freechannel = 0;
int upper_unfreechannel = 0;

SDL_Thread* openal_soundthread;
bool OpenALSoundON = true;

void OPENAL_RemoveChannelGroup(OPENAL_SOUND *channel, OPENAL_CHANNELGROUP *group);

static void private_OPENAL_Channel_Stop(OPENAL_SOUND* channel) {
	// stop and delete Sound (channel)
	channel->stream_active = false;
	alSourceStop(channel->id);
	if(channel->group)
		OPENAL_RemoveChannelGroup(channel, channel->group);
	if(channel->buffer->stream)
		openal_oggrelease(channel);
	alDeleteSources( 1, &channel->id );
	//free(channel);
	channel->active = false;
}


int OPENAL_ThreadFunction(void* data) {
	(void)data;
	while(OpenALSoundON) {
		SDL_LockMutex(openal_mutex);

		// Updates Stream channel
		for (int i=0; i<upper_unfreechannel; i++) {
			if(openal_sounds[i].active && openal_sounds[i].buffer->stream && openal_sounds[i].stream_active) {
				openal_streamupdate(&openal_sounds[i]);
			}
		}

		// check finished sound to free them, unless it's a streamed channel...
		for (int i=0; i<upper_unfreechannel; i++) {
			if(openal_sounds[i].active && !openal_sounds[i].buffer->stream) {
				ALint state = 0;
				alGetSourcei(openal_sounds[i].id, AL_SOURCE_STATE, &state);
				if(!(state==AL_PLAYING || state==AL_PAUSED || state==AL_INITIAL)) {
					private_OPENAL_Channel_Stop(&openal_sounds[i]);
					if (lower_freechannel > i)
						lower_freechannel = i;
				}
			}
		}
		while ((upper_unfreechannel > 0) && (!openal_sounds[upper_unfreechannel-1].active))
			--upper_unfreechannel;

		SDL_UnlockMutex(openal_mutex);
		
		SDL_Delay(100);
	}
	return 1;
}

int initOPENAL()
{
	static int initialized = 0;
	if(initialized)
		return 1;

	openal_device = alcOpenDevice(NULL); // preferred device
	if(!openal_device)
		return 0;

	openal_context = alcCreateContext(openal_device,NULL);
	if(!openal_context)
		return 0;

	alcMakeContextCurrent(openal_context);

	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alDopplerFactor(2.0f);

	// creates channels groups
	sound_group = (OPENAL_CHANNELGROUP*)malloc(sizeof(OPENAL_CHANNELGROUP));
	soundAmbient_group = (OPENAL_CHANNELGROUP*)malloc(sizeof(OPENAL_CHANNELGROUP));
	soundEnvironment_group = (OPENAL_CHANNELGROUP*)malloc(sizeof(OPENAL_CHANNELGROUP));
	music_group = (OPENAL_CHANNELGROUP*)malloc(sizeof(OPENAL_CHANNELGROUP));
	music_notification_group = (OPENAL_CHANNELGROUP*)malloc(sizeof(OPENAL_CHANNELGROUP));
	memset(sound_group, 0, sizeof(OPENAL_CHANNELGROUP));
	memset(soundAmbient_group, 0, sizeof(OPENAL_CHANNELGROUP));
	memset(soundEnvironment_group, 0, sizeof(OPENAL_CHANNELGROUP));
	memset(music_group, 0, sizeof(OPENAL_CHANNELGROUP));
	memset(music_notification_group, 0, sizeof(OPENAL_CHANNELGROUP));
	sound_group->volume = 1.0f;
	soundAmbient_group->volume = 1.0f;
	soundEnvironment_group->volume = 1.0f;
	music_group->volume = 1.0f;
	music_notification_group->volume = 1.0f;

	memset(openal_sounds, 0, sizeof(openal_sounds));
	lower_freechannel = 0;
	upper_unfreechannel = 0;

	OpenALSoundON = true;
	openal_mutex = SDL_CreateMutex();
	openal_soundthread = SDL_CreateThread(OPENAL_ThreadFunction, "openal", NULL);

	initialized = 1;


	return 1;
}

int closeOPENAL()
{
	if(OpenALSoundON) return 0;

	OpenALSoundON = false;
	int i = 0;
	SDL_WaitThread(openal_soundthread, &i);
	if(i!=1) {
		printlog("Warning, unable to stop Openal thread\n");
	}

	if(openal_mutex) {
		SDL_DestroyMutex(openal_mutex);
		openal_mutex = NULL;
	}

	// stop all remaining sound
	for (int i=0; i<upper_unfreechannel; i++) {
		if(openal_sounds[i].active && !openal_sounds[i].buffer->stream) {
			private_OPENAL_Channel_Stop(&openal_sounds[i]);
		}
	}

	alcMakeContextCurrent(NULL);
	alcDestroyContext(openal_context);
	openal_context = NULL;
	alcCloseDevice(openal_device);
	openal_device = NULL;
	initialized = 0;

	return 1;
}


static int get_firstfreechannel()
{
	int i = lower_freechannel;
	while ((i<MAXSOUND) && (openal_sounds[i].active))
		i++;
	if (i<MAXSOUND) {
		return i;
	}
	//no free channels, force free last one :(
	i = MAXSOUND-1;
	// TODO, check if it's a Stream one, then skip it if yes
	while((i>0) && (!openal_sounds[i].buffer->stream))
		--i;

	private_OPENAL_Channel_Stop(&openal_sounds[i]);

	return i;
}

void setGlobalVolume(real_t master, real_t music, real_t gameplay, real_t ambient, real_t environment) {
    master = std::min(std::max(0.0, master), 1.0);
    music = std::min(std::max(0.0, music / 4.0), 1.0); // music volume cut in half because the music is loud...
    gameplay = std::min(std::max(0.0, gameplay), 1.0);
    ambient = std::min(std::max(0.0, ambient), 1.0);
    environment = std::min(std::max(0.0, environment), 1.0);

	OPENAL_ChannelGroup_SetVolume(music_group, master * music);
	OPENAL_ChannelGroup_SetVolume(sound_group, master * gameplay);
	OPENAL_ChannelGroup_SetVolume(soundAmbient_group, master * ambient);
	OPENAL_ChannelGroup_SetVolume(soundEnvironment_group, master * environment);
	OPENAL_ChannelGroup_SetVolume(music_notification_group, master * gameplay);
}

void sound_update(int player, int index, int numplayers)
{
	if (no_sound)
	{
		return;
	}
	if (!openal_device)
	{
		return;
	}

	Vec3 position;

	auto& camera = cameras[index];
	if ( splitscreen )
	{
		camera = cameras[0];
	}

	position.x = -camera.y;
	position.y = -camera.z / 32;
	position.z = -camera.x;

	/*double cosroll = cos(0);
	double cosyaw = cos(camera.ang);
	double cospitch = cos(camera.vang);
	double sinroll = sin(0);
	double sinyaw = sin(camera.ang);
	double sinpitch = sin(camera.vang);

	double rx = sinroll*sinyaw - cosroll*sinpitch*cosyaw;
	double ry = sinroll*cosyaw + cosroll*sinpitch*sinyaw;
	double rz = cosroll*cospitch;*/

	float vector[6];
	vector[0] = 1 * sin(camera.ang);
	vector[1] = 0;
	vector[2] = 1 * cos(camera.ang);
	/*forward.x = rx;
	forward.y = ry;
	forward.z = rz;*/

	/*rx = sinroll*sinyaw - cosroll*cospitch*cosyaw;
	ry = sinroll*cosyaw + cosroll*cospitch*sinyaw;
	rz = cosroll*sinpitch;*/

	vector[3] = 0;
	vector[4] = 1;
	vector[5] = 0;
	/*up.x = rx;
	up.y = ry;
	up.z = rz;*/

	alListenerfv(AL_POSITION, (float*)&position);
	alListenerfv(AL_ORIENTATION, vector);
	//FMOD_System_Set3DListenerAttributes(fmod_system, 0, &position, 0, &forward, &up);

	//Fade in the currently playing music.
	if (player == 0) {
		if (music_channel)
		{
			ALint playing = 0;
			alGetSourcei( music_channel->id, AL_SOURCE_STATE, &playing );
			if (playing==AL_PLAYING)
			{
				float volume = music_channel->volume;

				if (volume < 1.0f)
				{
					volume += fadein_increment * 2;
					if (volume > 1.0f)
					{
						volume = 1.0f;
					}
					OPENAL_Channel_SetVolume(music_channel, volume);
				}
			}
		}
		//The following makes crossfading possible. Fade out the last playing music. //TODO: Support for saving music so that it can be resumed (for stuff interrupting like combat music).
		if (music_channel2)
		{
			ALint playing = 0;
			alGetSourcei( music_channel2->id, AL_SOURCE_STATE, &playing );
			if (playing)
			{
				float volume = music_channel2->volume;

				if (volume > 0.0f)
				{
					//volume -= 0.001f;
					//volume -= 0.005f;
					volume -= fadeout_increment * 2;
					if (volume < 0.0f)
					{
						volume = 0.0f;
					}
					OPENAL_Channel_SetVolume(music_channel2, volume);
				} else {
					/*OPENAL_Channel_Stop(music_channel2);
					music_channel2 = NULL;*/
					OPENAL_Channel_Pause(music_channel2);
				}
			}
		}
	}
}

void OPENAL_Channel_SetVolume(OPENAL_SOUND *channel, float f) {
	channel->volume = f;
	if(channel->group)
		f *= channel->group->volume;
	alSourcef(channel->id, AL_GAIN, f);
}

void OPENAL_ChannelGroup_Stop(OPENAL_CHANNELGROUP* group) {
	for (int i = 0; i< group->num; i++) {
		if (group->sounds[i])
			alSourceStop( group->sounds[i]->id );
	}
}

void OPENAL_ChannelGroup_SetVolume(OPENAL_CHANNELGROUP* group, float f) {
	group->volume = f;
	for (int i = 0; i< group->num; i++) {
		if (group->sounds[i])
			alSourcef( group->sounds[i]->id, AL_GAIN, f*group->sounds[i]->volume );
	}
}

void OPENAL_Channel_SetChannelGroup(OPENAL_SOUND *channel, OPENAL_CHANNELGROUP *group) {
	if(group->num==group->cap) {
		group->cap += 8;
		group->sounds = (OPENAL_SOUND**)realloc(group->sounds, group->cap*sizeof(OPENAL_SOUND*));
	}
	alSourcef(channel->id, AL_GAIN, channel->volume * group->volume);
	group->sounds[group->num++] = channel;
	channel->group = group;
}

void OPENAL_RemoveChannelGroup(OPENAL_SOUND *channel, OPENAL_CHANNELGROUP *group) {
	int i = 0;
	while ((i<group->num) && (channel!=group->sounds[i]))
		i++;
	if(i==group->num)
		return;
	memmove(group->sounds+i, group->sounds+i+1, sizeof(OPENAL_SOUND*)*(group->num-(i+1)));
	group->num--;
}

static size_t openal_file_oggread(void* ptr, size_t size, size_t nmemb, void* datasource) {
	File* file = (File*)datasource;
	return file->read(ptr, size, nmemb);
}

static int openal_file_oggseek(void* datasource, ogg_int64_t offset, int whence) {
	File* file = (File*)datasource;
	switch (whence) {
	case SEEK_CUR:
		return file->seek((ptrdiff_t)offset, File::SeekMode::ADD);
	case SEEK_END:
		return file->seek((ptrdiff_t)offset, File::SeekMode::SETEND);
	case SEEK_SET:
		return file->seek((ptrdiff_t)offset, File::SeekMode::SET);
	}
	return 0;
}

static int openal_file_oggclose(void* datasource) {
	return 0;
}

static long int openal_file_oggtell(void* datasource) {
	File* file = (File*)datasource;
	return file->tell();
}

int OPENAL_CreateSound(const char* name, bool b3D, OPENAL_BUFFER **buffer) {
	*buffer = (OPENAL_BUFFER*)malloc(sizeof(OPENAL_BUFFER));
	strncpy((*buffer)->oggfile, name, 64);	// for debugging purpose
	(*buffer)->stream = false;
	File *f = openDataFile(name, "rb");
	if(!f) {
		printlog("Error loading sound %s\n", name);
		return 0;
	}

	ov_callbacks oggcb = { openal_file_oggread, openal_file_oggseek, openal_file_oggclose, openal_file_oggtell };

	vorbis_info * pInfo;
	OggVorbis_File oggFile;
	ov_open_callbacks(f, &oggFile, NULL, 0, oggcb);
	pInfo = ov_info(&oggFile, -1);

	int channels = pInfo->channels;
	int freq = pInfo->rate;
	ov_pcm_seek(&oggFile, 0);
	size_t size = ov_pcm_total(&oggFile, -1) * 2 * (pInfo->channels+1);
	char* data = (char*)malloc(size+size/2);	// safe side
	char* ptr = data;
	int bytes = 0;
	size_t sz = 0;
	do {
		int bitStream;
		bytes = ov_read(&oggFile, ptr, size, 0, 2, 1, &bitStream);
		size-=bytes;
		ptr+=bytes;
		sz+=bytes;
	} while(bytes>0);
	char *data2 = data;
	if(b3D && channels==2) {
		// downmixing sound to mono, because 3D sounds NEEDS mono sound
		data2 = (char*)malloc(sz/2);
		int16_t *p1, *p2;
		p1 = (int16_t*)data2;
		p2 = (int16_t*)data;
		sz/=2;
		for(int i=0; i<sz/2; i++) {
			*(p1++) = (p2[0]+p2[1])/2;
			p2+=2;
		}
		channels = 1;
	}

	ov_clear(&oggFile);
	alGenBuffers(1, &(*buffer)->id);
	alBufferData((*buffer)->id, (channels==1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16, data2, sz, freq);
	if(data2!=data)
		free(data2);
	free(data);
	FileIO::close(f);
	return 1;
}

int OPENAL_CreateStreamSound(const char* name, OPENAL_BUFFER **buffer) {
	*buffer = (OPENAL_BUFFER*)malloc(sizeof(OPENAL_BUFFER));
	(*buffer)->stream = true;
	strcpy((*buffer)->oggfile, name);
	return 1;
}

OPENAL_SOUND* OPENAL_CreateChannel(OPENAL_BUFFER* buffer) {
	//OPENAL_SOUND *channel=(OPENAL_SOUND*)malloc(sizeof(OPENAL_SOUND));

	SDL_LockMutex(openal_mutex);

	int i = get_firstfreechannel();

	if(upper_unfreechannel < (i+1))
		upper_unfreechannel = i+1;
	lower_freechannel = i+1;

	OPENAL_SOUND *channel = &openal_sounds[i];
	alGenSources(1,&channel->id);
	channel->volume = 1.0f;
	channel->group = NULL;
	channel->active = true;
	channel->loop = false;
	channel->buffer = buffer;
	channel->stream_active = false;
	channel->indice = i;

	if(buffer->stream) {
		openal_oggopen(channel, buffer->oggfile);
	} else
		alSourcei(channel->id, AL_BUFFER, buffer->id);
	// default to 2D...
	alSourcei(channel->id,AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(channel->id, AL_POSITION, 0, 0, 0);

	SDL_UnlockMutex(openal_mutex);
	return channel;
}

void OPENAL_Channel_IsPlaying(void* channel, ALboolean *playing) {
	ALint state;
	alGetSourcei( ((OPENAL_SOUND*)channel)->id, AL_SOURCE_STATE, &state );
	(*playing) = (state == AL_PLAYING);
}

void OPENAL_Channel_Stop(void* chan) {
	SDL_LockMutex(openal_mutex);

	OPENAL_SOUND* channel = (OPENAL_SOUND*)chan;
	if(channel==NULL || !channel->active) {
		SDL_UnlockMutex(openal_mutex);
		return;
	}

	int i = channel->indice;
	private_OPENAL_Channel_Stop(channel);
	if (lower_freechannel > i)
		lower_freechannel = i;


	SDL_UnlockMutex(openal_mutex);
}

void OPENAL_Channel_Set3DAttributes(OPENAL_SOUND* channel, float x, float y, float z) {

	alSourcei(channel->id,AL_SOURCE_RELATIVE, AL_FALSE);
	alSource3f(channel->id, AL_POSITION, x, y, z);
	alSourcef(channel->id, AL_REFERENCE_DISTANCE, 1.f);	// hardcoding FMOD_System_Set3DSettings(fmod_system, 1.0, 2.0, 1.0);
	alSourcef(channel->id, AL_MAX_DISTANCE, 10.f);		// but this are simply OpenAL default (the 2.0f is used for Dopler only)
}

void OPENAL_Channel_Play(OPENAL_SOUND* channel) {
	SDL_LockMutex(openal_mutex);

	ALint state;
	alGetSourcei( channel->id, AL_SOURCE_STATE, &state );
	if(state != AL_PLAYING && state != AL_PAUSED) {
		if(channel->buffer->stream) {
			int processed;
			int num_buffers = 4;
			int i;
			ALuint trash[256];

			alGetSourcei(channel->id, AL_BUFFERS_PROCESSED, &processed);
			alSourceUnqueueBuffers(channel->id, processed, trash);

			for(i=0; i<4; i++) {
				if(!openal_streamread(channel, channel->streambuff[i])) {
					num_buffers = i;
					break;
				}
			}

			alSourceQueueBuffers(channel->id, num_buffers, channel->streambuff);
			channel->stream_active = true;
		}
	}
	alSourcePlay(channel->id);

	SDL_UnlockMutex(openal_mutex);
}

void OPENAL_Channel_Pause(OPENAL_SOUND* channel) {
	alSourcePause(channel->id);
}

void OPENAL_GetBuffer(OPENAL_SOUND* channel, OPENAL_BUFFER** buffer) {
	(*buffer) = channel->buffer;
}

void OPENAL_SetLoop(OPENAL_SOUND* channel, ALboolean looping) {
	channel->loop = looping;
	if(!channel->buffer->stream)
		alSourcei(channel->id, AL_LOOPING, looping);
}

void OPENAL_Channel_GetPosition(OPENAL_SOUND* channel, unsigned int *position) {
	alGetSourcei(channel->id, AL_BYTE_OFFSET, (GLint*)position);
}

void OPENAL_Sound_GetLength(OPENAL_BUFFER* buffer, unsigned int *length) {
	if(!buffer) return;
	alGetBufferi(buffer->id, AL_SIZE, (GLint*)length);
}

void OPENAL_Sound_Release(OPENAL_BUFFER* buffer) {
	if(!buffer) return;
	if(!buffer->stream)
		alDeleteBuffers( 1, &buffer->id );
	free(buffer);
}


bool physfsSearchMusicToUpdate_helper_findModifiedMusic(uint32_t numMusic, const char* filenameTemplate)
{
	for ( int c = 0; c < numMusic; c++ )
	{
		snprintf(tempstr, 1000, filenameTemplate, c);
		if ( PHYSFS_getRealDir(tempstr) != nullptr )
		{
			DynamicString musicDir = PHYSFS_getRealDir(tempstr);
			if ( musicDir.compare("./") != 0 )
			{
				printlog("[PhysFS]: Found modified music in music/ directory, reloading music files...");
				return true;
			}
		}
	}

	return false;
}

const std::vector<DynamicString> themeMusic = {
	"music/introduction.ogg",
	"music/intermission.ogg",
	"music/minetown.ogg",
	"music/splash.ogg",
	"music/library.ogg",
	"music/shop.ogg",
	"music/herxboss.ogg",
	"music/temple.ogg",
	"music/endgame.ogg",
	"music/escape.ogg",
	"music/devil.ogg",
	"music/sanctum.ogg",
	"music/gnomishmines.ogg",
	"music/greatcastle.ogg",
	"music/sokoban.ogg",
	"music/caveslair.ogg",
	"music/bramscastle.ogg",
	"music/hamlet.ogg",
	"music/tutorial.ogg",
	"sound/Death.ogg",
	"sound/ui/StoryMusicV3.ogg",
	"sound/ensemble/ensemble1_drumV1.ogg",
	"sound/ensemble/ensemble1_fluteV1.ogg",
	"sound/ensemble/ensemble1_hornV1.ogg",
	"sound/ensemble/ensemble1_luteV1.ogg",
	"sound/ensemble/ensemble1_lyreV1.ogg",
	"sound/ensemble/ensemble1_tamboV1.ogg",
	"sound/ensemble/ensemble1_BEB_tier1_V1.ogg",
	"sound/ensemble/ensemble1_BEB_tier2_V1.ogg",
	"sound/ensemble/ensemble1_drum_combatV1.ogg",
	"sound/ensemble/ensemble1_flute_combatV1.ogg",
	"sound/ensemble/ensemble1_horn_combatV1.ogg",
	"sound/ensemble/ensemble1_lute_combatV1.ogg",
	"sound/ensemble/ensemble1_lyre_combatV1.ogg",
	"sound/ensemble/ensemble1_tambo_combatV1.ogg",
	"sound/ensemble/ensemble1_BEB_tier1_combatV1.ogg",
	"sound/ensemble/ensemble1_BEB_tier2_combatV1.ogg",
	/*"sound/ensemble/Trans1/ensemble1_drum_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_flute_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_horn_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_lute_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_lyre_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_tambo_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_tambo_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans1/ensemble1_tambo_Trans1_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_drum_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_flute_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_horn_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_lute_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_lyre_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_tambo_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_tambo_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans2/ensemble1_tambo_Trans2_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_drum_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_flute_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_horn_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_lute_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_lyre_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_tambo_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_tambo_Trans3_120_4-4_V1.ogg",
	"sound/ensemble/Trans3/ensemble1_tambo_Trans3_120_4-4_V1.ogg",*/
	"sound/ensemble/CombatEnd1/ensemble1_drum_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_flute_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_horn_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_lute_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_lyre_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_tambo_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_BEB_tier1_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd1/ensemble1_BEB_tier2_combat_End1_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_drum_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_flute_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_horn_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_lute_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_lyre_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_tambo_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_BEB_tier1_combat_End2_90_7-8.ogg",
	"sound/ensemble/CombatEnd2/ensemble1_BEB_tier2_combat_End2_90_7-8.ogg",
	/*"sound/ensemble/CombatEnd3/ensemble1_drum_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_flute_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_horn_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_lute_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_lyre_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_tambo_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_tambo_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd3/ensemble1_tambo_combat_End3_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_drum_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_flute_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_horn_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_lute_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_lyre_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_tambo_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_tambo_combat_End4_90_7-8.ogg",
	"sound/ensemble/CombatEnd4/ensemble1_tambo_combat_End4_90_7-8.ogg",*/
	"sound/ensemble/Trans4/ensemble1_drum_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_flute_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_horn_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_lute_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_lyre_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_tambo_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_BEB_tier1_Trans_120_4-4.ogg",
	"sound/ensemble/Trans4/ensemble1_BEB_tier2_Trans_120_4-4.ogg"
};

bool physfsSearchMusicToUpdate()
{
	if ( no_sound )
	{
		return false;
	}
	// OpenAL music hot-reload: check the same numbered arrays + theme tracks.
#ifdef SOUND
	const char* themePaths[] = {
		"music/introduction.ogg", "music/intermission.ogg", "music/minetown.ogg",
		"music/splash.ogg", "music/library.ogg", "music/shop.ogg", "music/herxboss.ogg",
		"music/temple.ogg", "music/endgame.ogg", "music/escape.ogg", "music/devil.ogg",
		"music/sanctum.ogg", "music/gnomishmines.ogg", "music/greatcastle.ogg",
		"music/sokoban.ogg", "music/caveslair.ogg", "music/bramscastle.ogg",
		"music/hamlet.ogg", "music/tutorial.ogg", "sound/Death.ogg", "sound/ui/StoryMusicV3.ogg"
	};
	for ( int i = 0; i < 21; ++i )
	{
		if ( PHYSFS_getRealDir(themePaths[i]) != nullptr )
		{
			DynamicString musicDir = PHYSFS_getRealDir(themePaths[i]);
			if ( musicDir.compare("./") != 0 )
			{
				printlog("[PhysFS]: Found modified music in music/ directory, reloading music files...");
				return true;
			}
		}
	}
	if ( physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMMINESMUSIC, "music/mines%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMSWAMPMUSIC, "music/swamp%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMLABYRINTHMUSIC, "music/labyrinth%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMRUINSMUSIC, "music/ruins%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMUNDERWORLDMUSIC, "music/underworld%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMHELLMUSIC, "music/hell%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMMINOTAURMUSIC, "music/minotaur%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMCAVESMUSIC, "music/caves%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMCITADELMUSIC, "music/citadel%02d.ogg")
		|| physfsSearchMusicToUpdate_helper_findModifiedMusic(NUMFORTRESSMUSIC, "music/fortress%02d.ogg") )
	{
		return true;
	}
#endif // SOUND
	return false;
}


void physfsReloadMusic(bool &introMusicChanged, bool reloadAll) //TODO: This should probably return an error.
{
	if ( no_sound )
	{
		return;
	}
	// OpenAL music loading: OPENAL_CreateStreamSound records the file path;
	// decoding/streaming happens at play time (openal_oggopen). This mirrors
	// the FMOD path: load the theme tracks + the numbered per-level arrays.
	bool introChanged = false;

	// Helper to load a numbered music array from a PhysFS template path.
	auto reloadMusicArray = [](int numMusic, const char* filenameTemplate, OPENAL_BUFFER** musicArray, bool forceReload) {
		if ( !musicArray ) { return; }
		for ( int c = 0; c < numMusic; c++ )
		{
			char tempstr[1000];
			snprintf(tempstr, 1000, filenameTemplate, c);
			if ( PHYSFS_getRealDir(tempstr) != nullptr )
			{
				// Pass the *relative* PhysFS path (e.g. "music/mines00.ogg");
				// OPENAL_BUFFER::oggfile is only 64 chars and openal_oggopen
				// resolves it via openDataFile() at play time. Absolute paths
				// overflow the buffer.
				if ( musicArray[c] ) { OPENAL_Sound_Release(musicArray[c]); }
				OPENAL_CreateStreamSound(tempstr, &musicArray[c]);
			}
		}
	};

	// themeMusic: single non-array tracks, in the same order as the FMOD path's
	// switch() cases (introductionmusic ... introstorymusic).
	const char* themePaths[] = {
		"music/introduction.ogg", "music/intermission.ogg", "music/minetown.ogg",
		"music/splash.ogg", "music/library.ogg", "music/shop.ogg", "music/herxboss.ogg",
		"music/temple.ogg", "music/endgame.ogg", "music/escape.ogg", "music/devil.ogg",
		"music/sanctum.ogg", "music/gnomishmines.ogg", "music/greatcastle.ogg",
		"music/sokoban.ogg", "music/caveslair.ogg", "music/bramscastle.ogg",
		"music/hamlet.ogg", "music/tutorial.ogg", "sound/Death.ogg", "sound/ui/StoryMusicV3.ogg"
	};
	OPENAL_BUFFER** themeSlots[] = {
		&introductionmusic, &intermissionmusic, &minetownmusic, &splashmusic, &librarymusic,
		&shopmusic, &herxmusic, &templemusic, &endgamemusic, &escapemusic, &devilmusic,
		&sanctummusic, &gnomishminesmusic, &greatcastlemusic, &sokobanmusic, &caveslairmusic,
		&bramscastlemusic, &hamletmusic, &tutorialmusic, &gameovermusic, &introstorymusic
	};
	for ( int i = 0; i < 21; ++i )
	{
		const char* filename = themePaths[i];
		if ( PHYSFS_getRealDir(filename) != nullptr )
		{
			// Pass the relative PhysFS path (see reloadMusicArray comment).
			if ( *themeSlots[i] ) { OPENAL_Sound_Release(*themeSlots[i]); }
			OPENAL_CreateStreamSound(filename, themeSlots[i]);
			if ( i == 0 ) { introChanged = true; }
		}
	}

	// Numbered per-level music arrays.
	reloadMusicArray(NUMMINESMUSIC, "music/mines%02d.ogg", minesmusic, reloadAll);
	reloadMusicArray(NUMSWAMPMUSIC, "music/swamp%02d.ogg", swampmusic, reloadAll);
	reloadMusicArray(NUMLABYRINTHMUSIC, "music/labyrinth%02d.ogg", labyrinthmusic, reloadAll);
	reloadMusicArray(NUMRUINSMUSIC, "music/ruins%02d.ogg", ruinsmusic, reloadAll);
	reloadMusicArray(NUMUNDERWORLDMUSIC, "music/underworld%02d.ogg", underworldmusic, reloadAll);
	reloadMusicArray(NUMHELLMUSIC, "music/hell%02d.ogg", hellmusic, reloadAll);
	reloadMusicArray(NUMMINOTAURMUSIC, "music/minotaur%02d.ogg", minotaurmusic, reloadAll);
	reloadMusicArray(NUMCAVESMUSIC, "music/caves%02d.ogg", cavesmusic, reloadAll);
	reloadMusicArray(NUMCITADELMUSIC, "music/citadel%02d.ogg", citadelmusic, reloadAll);
	reloadMusicArray(NUMFORTRESSMUSIC, "music/fortress%02d.ogg", fortressmusic, reloadAll);

	// Intro tracks: intro.ogg + intro%02d.ogg
	for ( int c = 0; c < NUMINTROMUSIC; c++ )
	{
		char tempstr[1000];
		if ( c == 0 ) { strcpy(tempstr, "music/intro.ogg"); }
		else { snprintf(tempstr, 1000, "music/intro%02d.ogg", c); }
		if ( PHYSFS_getRealDir(tempstr) != nullptr )
		{
			// Pass the relative PhysFS path (see reloadMusicArray comment).
			if ( intromusic[c] ) { OPENAL_Sound_Release(intromusic[c]); }
			OPENAL_CreateStreamSound(tempstr, &intromusic[c]);
			introChanged = true;
		}
	}

	introMusicChanged = introChanged;
}

void gamemodsUnloadCustomThemeMusic()
{
#ifdef SOUND
	// free custom music slots, not used by official music assets.
	if ( gnomishminesmusic )
	{
		OPENAL_Sound_Release(gnomishminesmusic);
		gnomishminesmusic = nullptr;
	}
	if ( greatcastlemusic )
	{
		OPENAL_Sound_Release(greatcastlemusic);
		greatcastlemusic = nullptr;
	}
	if ( sokobanmusic )
	{
		OPENAL_Sound_Release(sokobanmusic);
		sokobanmusic = nullptr;
	}
	if ( caveslairmusic )
	{
		OPENAL_Sound_Release(caveslairmusic);
		caveslairmusic = nullptr;
	}
	if ( bramscastlemusic )
	{
		OPENAL_Sound_Release(bramscastlemusic);
		bramscastlemusic = nullptr;
	}
	if ( hamletmusic )
	{
		OPENAL_Sound_Release(hamletmusic);
		hamletmusic = nullptr;
	}
#endif // SOUND
}
