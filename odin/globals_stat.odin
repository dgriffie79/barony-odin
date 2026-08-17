// globals_stat.odin — Odin ownership of the extern globals of stat.hpp.
package main

// extern "C" Stat* stats[MAXPLAYERS];  (stat.hpp:507)
// C++ defs deleted from stat.cpp:25 (game) + stat_editor.cpp:20 (editor).
// stats is Stat*[MAXPLAYERS]; in this build MAXPLAYERS is 4 (odin/main.odin:65),
// so the Odin owner is an array of 4 pointers to Stat.
@(export)
stats : [4]^Stat
