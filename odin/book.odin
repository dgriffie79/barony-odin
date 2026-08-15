// book.odin — Odin mirrors of book.hpp.
package main

import "containers"

// struct Book_t — 72 bytes
Book_t :: struct {
	text:           containers.DynamicString,
	default_name:   containers.DynamicString,
	formatted_pages: containers.Raw_Dynamic_Array, // DynamicArrayStr (40B)
}
#assert(size_of(Book_t) == 72)

// struct BookParser_t — 40 bytes
// { const int versionJSON; DynamicMapStr tempBookData; }
BookParser_t :: struct {
	version_json:  i32, // const int
	temp_book_data: containers.Raw_Map, // DynamicMapStr (32B)
}
#assert(size_of(BookParser_t) == 40)
