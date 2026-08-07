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
#endif
