// globals_messages.odin — Odin ownership of globals from src/messages.hpp
package main

// extern "C" const int MESSAGE_LIST_SIZE_CAP;  (messages.hpp)
// C++ def (messages.cpp, deleted): const int MESSAGE_LIST_SIZE_CAP = 400;
@(export)
MESSAGE_LIST_SIZE_CAP : i32 = 400
