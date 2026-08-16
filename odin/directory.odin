// directory.odin -- Odin mirror of Directory.hpp.
package main


// class Directory — 48 bytes { DynamicArrayStr list (40B); const char* path (8B) }
Directory :: struct {
	list: [dynamic]string, // DynamicArrayStr (40B)
	path: cstring,                       // const char*
}
#assert(size_of(Directory) == 48)
