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
	size_t write(const void* src, size_t size, size_t count);

	// read data into the given buffer
	// @param buffer the buffer to read into
	// @param size the size of each data element in bytes
	// @param count the number of data elements to read
	// @return the number of bytes read
	size_t read(void* buffer, size_t size, size_t count);

	// get the size of the file
	// @return the size in bytes
	size_t size();

	// determine whether we have reached the end of the file or not
	// @return true if we are at the end of the file, otherwise false
	bool eof();

	// read a string from the file, culling newlines
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets2(char* buf, int size);

	// read a string from the file
	// @param buf the buffer to contain the read string
	// @param size the maximum size of the string to read
	// @return buf if successfully read a string, otherwise nullptr
	char* gets(char* buf, int size);

	// read an integer from the stream
	// @return the read integer or possibly 0 if we failed to read one
	int geti();

	// read 1 char from the stream
	// @return the read char or possibly '\0' if we failed to read one
	char getc();

	// write a formatted string to the file, printf style
	// @param fmt the string to format
	// @param ... variadic string format arguments
	// @return the number of characters written to the file
	int printf(const char* fmt, ...);
	int vprintf(const char* fmt, va_list args);

	// write the given string to the file
	// @param str the string to write
	// @return 0 on success, -1 on error
	int puts(const char* str);

	// write char to file
	// @param c the char to write
	// @return 0 on success, -1 on error
	int putc(char c);

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
	int seek(ptrdiff_t offset, SeekMode mode);

	// get the current offset into the stream
	// @return the offset into the stream, in bytes
	long int tell();

	// sets the file position back to the start of the file.
	void rewind();

	// file mode
	enum class FileMode : Uint8
	{
		INVALID,	// not valid
		READ,		// file set for read mode
		WRITE		// file set for write mode
	};

	// close the file, after this point no ops are valid
	void close();

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
	static File* open(const char* path, const char* mode);

	// close the given file
	// @param file the file to close
	static void close(File* file);
};

enum HolidayTheme {
    THEME_NONE,
    THEME_HALLOWEEN,
    THEME_XMAS,
    THEME_MAX
};
extern const char* holidayThemeDirs[HolidayTheme::THEME_MAX];
extern "C" HolidayTheme getCurrentHoliday(bool force = false);
extern "C" bool isCurrentHoliday(bool force = false);

#ifndef EDITOR
#include "interface/consolecommand.hpp"
extern CvarInt cvar_forceHoliday;
extern CvarBool cvar_disableHoliday;
#endif

extern char datadir[PATH_MAX]; //PATH_MAX as defined in main.hpp -- maybe define in Config.hpp?
extern char outputdir[PATH_MAX];
extern "C" void glLoadTexture(SDL_Surface* image, int texnum);
extern "C" SDL_Surface* loadImage(char const * const filename);
extern "C" voxel_t* loadVoxel(char* filename2);
extern "C" bool verifyMapHash(const char* filename, int hash, bool* fileExistsInTable = nullptr);
extern "C" int loadMap(const char* filename, map_t* destmap, list_t* entlist, list_t* creatureList, int *checkMapHash = nullptr);
extern "C" int loadConfig(char* filename);
extern "C" int loadDefaultConfig();
extern "C" int saveMap(const char* filename);
extern "C" char* readFile(char* filename);
extern "C" DynamicArrayStr directoryContents(const char* directory, bool includeSubdirectory, bool includeFiles, const char* base = datadir);
extern "C" File *openDataFile(const char *const filename, const char * const mode);
extern "C" DIR * openDataDir(const char *const);
extern "C" bool dataPathExists(const char *const, bool complete = true);
extern "C" bool completePath(char *dest, const char * const path, const char *base = datadir);
extern "C" void openLogFile();
extern "C" DynamicArrayStr getLinesFromDataFile(DynamicString filename);
extern "C" int loadMainMenuMap(bool blessedAdditionMaps, bool forceVictoryMap, int forcemap = -1);
extern "C" int physfsLoadMapFile(int levelToLoad, Uint32 seed, bool useRandSeed, int *checkMapHash = nullptr);
extern "C" DynamicArrayStr physfsGetFileNamesInDirectory(const char* dir);
extern "C" DynamicString physfsFormatMapName(char const * const levelfilename);
extern "C" bool physfsModelIndexUpdate(int &start, int &end);
extern "C" bool physfsSearchModelsToUpdate();
extern "C" bool physfsSearchSoundsToUpdate();
extern "C" void physfsReloadSounds(bool reloadAll);
extern "C" void physfsReloadBooks();
extern "C" bool physfsSearchBooksToUpdate();
extern "C" bool physfsSearchMusicToUpdate();
extern "C" void physfsReloadMusic(bool &introMusicChanged, bool reloadAll);
extern "C" void physfsReloadTiles(bool reloadAll);
extern "C" bool physfsSearchTilesToUpdate();
extern "C" void physfsReloadSprites(bool reloadAll);
extern "C" bool physfsSearchSpritesToUpdate();
extern "C" bool physfsIsMapLevelListModded();
extern "C" bool physfsSearchItemSpritesToUpdate();
extern "C" void physfsReloadItemSprites(bool reloadAll);
extern "C" bool physfsSearchMonsterLimbFilesToUpdate();
extern "C" void physfsReloadMonsterLimbFiles();
extern "C" void physfsReloadSystemImages();
extern "C" bool physfsSearchSystemImagesToUpdate();
extern "C" void gamemodsUnloadCustomThemeMusic();

enum MapParameterIndices : int
{
	LEVELPARAM_CHANCE_SECRET,
	LEVELPARAM_CHANCE_DARKNESS,
	LEVELPARAM_CHANCE_MINOTAUR,
	LEVELPARAM_DISABLE_NORMAL_EXIT
};
