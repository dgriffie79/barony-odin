/*-------------------------------------------------------------------------------

	BARONY
	File: init.hpp
	Desc: prototypes for init.cpp, various setup/teardown functions

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/
#pragma once

extern "C" int initApp(char const * const title, int fullscreen);
extern "C" int deinitApp();
extern "C" bool initVideo();
extern "C" bool changeVideoMode(int new_xres = 0, int new_yres = 0);
extern "C" bool resizeWindow(int new_xres = 0, int new_yres = 0);
extern "C" void generatePolyModels(int start, int end, bool forceCacheRebuild);
extern "C" void generateVBOs(int start, int end);
extern "C" void reloadModels(int start, int end);
extern "C" void generateTileTextures();
extern "C" void destroyTileTextures();
extern "C" void bindTextureAtlas(int index);
extern "C" bool mountBaseDataFolders();
extern "C" bool remountBaseDataFolders();
