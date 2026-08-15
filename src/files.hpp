/*-------------------------------------------------------------------------------

	BARONY
	File: files.hpp
	Desc: prototypes for file.cpp, all file access should be mediated
		  through this interface

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/
#pragma once
#include "../odin/containers/dynamic_string.hpp"

#include <list>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>
#include <cstdint>
#include <dirent.h>

#include "main.hpp"

//This class provides a common platform-independent interface for file accesses.
//It is a single concrete class (no virtual dispatch) - the PC implementation.
//Don't create a File directly, use FileIO::open to get one...
class File {
	friend class FileIO;

public:
	// write data to the file
	// @param src where the data is coming from
	// @param size the size of each data element in bytes
	// @param count the number of data elements to write
	// @return the number of bytes written
	size_t write(const void* src, size_t size, size_t count)
	{
		if (mode != FileMode::WRITE || nullptr == src)
		{
			return 0U;
		}
		const size_t writeSize = size * count;
		if (pos + writeSize > (size_t)data.len) {
		    barony_dynamic_array_resize(&data, 1, (int32_t)(pos + writeSize));
		}
		if (writeSize) {
		    memmove((uint8_t*)data.data + pos + writeSize, (uint8_t*)data.data + pos, (size_t)data.len - pos - writeSize);
		    memcpy((uint8_t*)data.data + pos, src, writeSize);
		}
		pos += writeSize;
		return writeSize / size;
	}

	// read data into the given buffer
	// @param buffer the buffer to read into
	// @param size the size of each data element in bytes
	// @param count the number of data elements to read
	// @return the number of bytes read
	size_t read(void* buffer, size_t size, size_t count)
	{
		if (mode != FileMode::READ || nullptr == buffer)
		{
			return 0U;
		}
		size_t readSize = 0U;
		size_t end = std::min(this->size(), pos + size * count);
		uint8_t* buf = (uint8_t*)buffer;
		for (size_t c = pos; c < end; ++c) {
			*buf = ((uint8_t*)data.data)[c]; ++buf;
			++readSize;
		}
		pos += readSize;
		return readSize / size;
	}

	// get the size of the file
	// @return the size in bytes
	size_t size()
	{
		return (size_t)data.len;
	}

	// determine whether we have reached the end of the file or not
	// @return true if we are at the end of the file, otherwise false
	bool eof()
	{
		return pos >= size();
	}

	// read a string from the file, culling newlines
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets2(char* buf, int size)
	{
		auto result = gets(buf, size);
		for (int c = 0; c < size; ++c)
		{
			if (buf[c] == '\n' || buf[c] == '\r')
			{
				buf[c] = '\0';
				return result;
			}
		}
		return result;
	}

	// read a string from the file
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets(char* buf, int size)
	{
		char* result = buf;
	    if (!buf) {
		    return nullptr;
	    }
		for (int c = 0; c < size - 1; ++c) {
			size_t bytesRead = read(buf, sizeof(char), 1);
			if (bytesRead > 0U) {
				if (*buf == '\0' || *buf == '\n') {
					buf += bytesRead;
					break;
				}
				buf += bytesRead;
			} else {
				*buf = '\0';
				if (c == 0) {
					return nullptr;
				} else {
					return result;
				}
			}
		}
		*(buf) = '\0';
		return result;
	}

	// read an integer from the stream
	// @return the read integer or possibly 0 if we failed to read one
	int geti()
	{
		char field[64];
		gets(field, 64);
		long result = strtol(field, nullptr, 10);
		return (int)result;
	}

	// read 1 char from the stream
	// @return the read char or possibly '\0' if we failed to read one
	char getc()
	{
		char result = '\0';
		if (read(&result, sizeof(char), 1) != 1)
		{
			return '\0';
		}
		return result;
	}

	// write a formatted string to the file, printf style
	// @param fmt the string to format
	// @param ... variadic string format arguments
	// @return the number of characters written to the file
	int printf(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		char buf[1024];
		int result = vsnprintf(buf, 1024, fmt, args);
		buf[1023] = '\0';

		write(buf, sizeof(char), result);

		va_end(args);

		return result;
	}

	// write the given string to the file
	// @param str the string to write
	// @return 0 on success, -1 on error
	int puts(const char* str)
	{
		size_t size = strlen(str);
		return write(str, sizeof(char), size) == size ? 0 : -1;
	}

	// write char to file
	// @param c the char to write
	// @return 0 on success, -1 on error
	int putc(char c)
	{
		return write(&c, sizeof(char), 1) == 1 ? 0 : -1;
	}

	// seek mode associated with seek()
	enum class SeekMode : Uint8
	{
		SET,		// set the offset into the stream
		ADD,		// change the offset based on the current offset
		SETEND		// set the offset into the stream, starting from the end of the file
	};

	// moves the current offset in the file stream without reading or writing anything
	// @param offset how much to move the stream, or where
	// @param mode The seek mode
	// @return 0 on success, non-zero on error
	int seek(ptrdiff_t offset, SeekMode mode)
	{
		switch (mode) {
		case SeekMode::SET: pos = offset; break;
		case SeekMode::ADD: pos += offset; break;
		case SeekMode::SETEND: pos = size() + offset; break;
		}
		if (eof()) {
			return -1;
		} else {
			return 0;
		}
	}

	// get the current offset into the stream
	// @return the offset into the stream, in bytes
	long int tell()
	{
		return (long int)pos;
	}

	// sets the file position back to the start of the file.
	void rewind()
	{
		seek(0, File::SeekMode::SET);
	}

	// file mode
	enum class FileMode : Uint8
	{
		INVALID,	// not valid
		READ,		// file set for read mode
		WRITE		// file set for write mode
	};

	// close the file, after this point no ops are valid
	void close()
	{
	    assert(fp);
	    if (mode == FileMode::WRITE) {
	        size_t c = 0u;
	        size_t end = size();
		    for (; c < end;) {
		        size_t result = fwrite((uint8_t*)data.data, sizeof(uint8_t), end - c, fp);
		        if (!result) {
		            // failed to write, try to write just a chunk
		            constexpr size_t chunk_size = 1024;
		            size_t chunk = std::min(end - c, chunk_size);
		            printlog("[FILES] failed to write %llu bytes to '%s', trying %llu bytes instead", end - c, path.c_str(), chunk);
		            result = fwrite((uint8_t*)data.data, sizeof(uint8_t), chunk, fp);
		            assert(result);
		        }
		        c += result;
		    }
	        assert(c == end);
	    }
		int result = fclose(fp);
		assert(result == 0);
	}

private:
	File(FILE* fp, FileMode mode, const char* path) :
		mode(mode),
		path(path),
		fp(fp)
	{
	    assert(fp);
	    if (mode == FileMode::READ) {
		    (void)fseek(fp, 0, SEEK_END);
		    size_t end = ftell(fp);
		    (void)fseek(fp, 0, SEEK_SET);
		    barony_dynamic_array_resize(&data, 1, (int32_t)end);
		    size_t c = 0;
		    for (; c < end;) {
		        size_t result = fread((uint8_t*)data.data, sizeof(uint8_t), end - c, fp);
		        if (!result) {
		            // failed to read, try to read just a chunk
		            constexpr size_t chunk_size = 1024;
		            size_t chunk = std::min(end - c, chunk_size);
		            printlog("[FILES] failed to read %llu bytes from '%s', trying %llu bytes instead", end - c, path, chunk);
		            result = fread((uint8_t*)data.data, sizeof(uint8_t), chunk, fp);
		            assert(result);
		        }
		        c += result;
		    }
	        assert(c == end);
		}
	}

	~File()
	{
	}

	FileMode mode = FileMode::INVALID;
	std::string path;
	FILE* fp = nullptr;
	DynamicArray data{};  // byte buffer (vector<uint8_t>)
	size_t pos = 0u;
};


//Don't create a File directly, use this to get one...
class FileIO {
private:
	FileIO() {}
	~FileIO() {}

public:
	// open a new file
	// @param path complete path to the file to open
	// @param mode access mode (see fopen in stdio)
	static File* open(const char* path, const char* mode)
	{
		if (!path || !mode)
		{
			return nullptr;
		}

		File::FileMode fileMode;
		switch (mode[0])
		{
		case 'r': fileMode = File::FileMode::READ; break;
		case 'w': fileMode = File::FileMode::WRITE; break;
		default: fileMode = File::FileMode::INVALID; break;
		}

        // note: on PC, files are ALWAYS opened in binary mode
		FILE* fp;
		switch (fileMode) {
		default: assert(0 && "invalid file open mode");
		case File::FileMode::READ: fp = fopen(path, "rb"); break;
		case File::FileMode::WRITE: fp = fopen(path, "wb"); break;
		}
		if (fp) {
			return new File(fp, fileMode, path);
		} else {
			return nullptr;
		}
	}

	// close the given file
	// @param file the file to close
	static void close(File* file)
	{
		if (!file)
		{
			return;
		}
		file->close();
		delete file;
	}
};

enum HolidayTheme {
    THEME_NONE,
    THEME_HALLOWEEN,
    THEME_XMAS,
    THEME_MAX
};
extern const char* holidayThemeDirs[HolidayTheme::THEME_MAX];
HolidayTheme getCurrentHoliday(bool force = false);
bool isCurrentHoliday(bool force = false);

#ifndef EDITOR
#include "interface/consolecommand.hpp"
extern CvarInt cvar_forceHoliday;
extern CvarBool cvar_disableHoliday;
#endif

extern char datadir[PATH_MAX]; //PATH_MAX as defined in main.hpp -- maybe define in Config.hpp?
extern char outputdir[PATH_MAX];
void glLoadTexture(SDL_Surface* image, int texnum);
SDL_Surface* loadImage(char const * const filename);
voxel_t* loadVoxel(char* filename2);
bool verifyMapHash(const char* filename, int hash, bool* fileExistsInTable = nullptr);
int loadMap(const char* filename, map_t* destmap, list_t* entlist, list_t* creatureList, int *checkMapHash = nullptr);
int loadConfig(char* filename);
int loadDefaultConfig();
int saveMap(const char* filename);
char* readFile(char* filename);
DynamicArrayStr directoryContents(const char* directory, bool includeSubdirectory, bool includeFiles, const char* base = datadir);
File *openDataFile(const char *const filename, const char * const mode);
DIR * openDataDir(const char *const);
bool dataPathExists(const char *const, bool complete = true);
bool completePath(char *dest, const char * const path, const char *base = datadir);
void openLogFile();
DynamicArrayStr getLinesFromDataFile(DynamicString filename);
int loadMainMenuMap(bool blessedAdditionMaps, bool forceVictoryMap, int forcemap = -1);
int physfsLoadMapFile(int levelToLoad, Uint32 seed, bool useRandSeed, int *checkMapHash = nullptr);
DynamicArrayStr physfsGetFileNamesInDirectory(const char* dir);
DynamicString physfsFormatMapName(char const * const levelfilename);
bool physfsModelIndexUpdate(int &start, int &end);
bool physfsSearchModelsToUpdate();
bool physfsSearchSoundsToUpdate();
void physfsReloadSounds(bool reloadAll);
void physfsReloadBooks();
bool physfsSearchBooksToUpdate();
bool physfsSearchMusicToUpdate();
void physfsReloadMusic(bool &introMusicChanged, bool reloadAll);
void physfsReloadTiles(bool reloadAll);
bool physfsSearchTilesToUpdate();
void physfsReloadSprites(bool reloadAll);
bool physfsSearchSpritesToUpdate();
bool physfsIsMapLevelListModded();
bool physfsSearchItemSpritesToUpdate();
void physfsReloadItemSprites(bool reloadAll);
bool physfsSearchMonsterLimbFilesToUpdate();
void physfsReloadMonsterLimbFiles();
void physfsReloadSystemImages();
bool physfsSearchSystemImagesToUpdate();
void gamemodsUnloadCustomThemeMusic();

enum MapParameterIndices : int
{
	LEVELPARAM_CHANCE_SECRET,
	LEVELPARAM_CHANCE_DARKNESS,
	LEVELPARAM_CHANCE_MINOTAUR,
	LEVELPARAM_DISABLE_NORMAL_EXIT
};
