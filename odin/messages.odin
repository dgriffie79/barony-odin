// messages.odin - Odin mirrors of messages.hpp.
package main

// struct Message - 16 bytes
// { string_t* text; int time_displayed; Sint16 alpha; }
Message :: struct {
	text:           ^string_t,
	time_displayed: i32,
	alpha:          i16,
}
#assert(size_of(Message) == 16)


// ---------------------------------------------------------------------------
// Globals owned by Odin (C++ references via extern "C")
// ---------------------------------------------------------------------------
@(export)
MESSAGE_LIST_SIZE_CAP : i32 = 400
