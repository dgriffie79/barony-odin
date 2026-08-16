// book.odin — Odin mirrors of book.hpp.
package main

import "containers"

// struct Book_t — 72 bytes
Book_t :: struct {
	text:           containers.DynamicString,
	default_name:   containers.DynamicString,
	formatted_pages: [dynamic]containers.DynamicString, // DynamicArrayStr (40B)
}
#assert(size_of(Book_t) == 72)

// struct BookParser_t — 40 bytes
// { const int versionJSON; DynamicMapStr tempBookData; }
BookParser_t :: struct {
	version_json:  i32, // const int
	temp_book_data: map[string]containers.DynamicString, // DynamicMapStr (32B)
}
#assert(size_of(BookParser_t) == 40)
