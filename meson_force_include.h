// Forced include for the Meson build (added via -FI on MSVC).
// Resolves Windows.h macro collisions with rapidjson and std::min/std::max
// without touching Barony's source files.

#ifdef _WIN32
// rapidjson's document.h temporarily undefines GetObject during its own parse
// but restores it at the end, so call sites like lights.GetObject() get
// expanded to GetObjectA. Undefine it globally (it's only a Win32 GDI macro
// that Barony never uses).
#ifdef GetObject
#undef GetObject
#endif
// SDL_MAIN_HANDLED: stop SDL_main.h from doing `#define main SDL_main`.
// The Odin driver provides the real process entry and calls barony_main
// (the renamed C++ main) directly, so SDL must not hijack main(). Must be
// defined before any SDL header is included (this file is force-included
// via -FI on MSVC, before every TU).
#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#endif
