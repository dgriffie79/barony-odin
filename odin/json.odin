// json.odin — Odin mirrors of json.hpp (types).
package main

// enum class EFileFormat — declared in json.hpp (underlying int)
EFileFormat :: enum i32 {
	Json,
	Json_Compact,
	Binary,
}

// class FileInterface — 32 bytes
// { EFileFormat format (4); bool reading (1); File* fp (8);
//   JsonWriterState* jsonWriter (8); JsonReaderState* jsonReader (8); }
FileInterface :: struct {
	format:      EFileFormat,
	reading:     bool,
	fp:          ^File,
	json_writer: rawptr, // JsonWriterState*
	json_reader: rawptr, // JsonReaderState*
}
#assert(size_of(FileInterface) == 32)

// JsonWriterState / JsonReaderState are opaque handles into json_shim (Odin);
// kept as rawptr here.
