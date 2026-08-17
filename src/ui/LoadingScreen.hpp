#pragma once

#include "../main.hpp"

extern "C" void createLoadingScreen(real_t progress);
extern "C" void createLevelLoadScreen(real_t progress);
extern "C" void updateLoadingScreen(real_t progress);
extern "C" void doLoadingScreen();
extern "C" void destroyLoadingScreen();

extern Uint32 loadingticks;