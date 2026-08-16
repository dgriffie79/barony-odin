// files.odin — Odin mirrors of files.hpp.
package main

import "containers"

// enum class FileMode : Uint8
FileMode :: enum u8 {
	INVALID,
	READ,
	WRITE,
}

// class File — 96 bytes
// { FileMode mode (1); std::string path (32, until string pass); FILE* fp (8);
//   DynamicArray data (40); size_t pos (8); }
File :: struct {
	mode: FileMode,
	// std::string path — 32B opaque (still std::string in C++; the string pass
	// converts it to DynamicString, which would shrink this to 16B — parity
	// assert will need updating then)
	path_opaque: [32]u8,
	fp:          rawptr, // FILE*
	data:        [dynamic]u8, // byte buffer (vector<uint8_t>)
	pos:         u64, // size_t
}
#assert(size_of(File) == 96)

// class FileIO — statics only, no per-instance data
