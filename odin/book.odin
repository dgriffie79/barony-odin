// book.odin — Odin mirrors of book.hpp.
package main


// struct Book_t — 72 bytes
Book_t :: struct {
	text:           string,
	default_name:   string,
	formatted_pages: [dynamic]string, // DynamicArrayStr (40B)
}
#assert(size_of(Book_t) == 72)

// struct BookParser_t — 40 bytes
// { const int versionJSON; DynamicMapStr tempBookData; }
BookParser_t :: struct {
	version_json:  i32, // const int
	temp_book_data: map[string]string, // DynamicMapStr (32B)
}
#assert(size_of(BookParser_t) == 40)


// ---------------------------------------------------------------------------
// Globals owned by Odin (C++ references via extern "C")
// ---------------------------------------------------------------------------
@(export)
allBooks : [dynamic]Book_t

@(export)
bookParser_t : BookParser_t

@(export)
numbooks : i32 = 0
