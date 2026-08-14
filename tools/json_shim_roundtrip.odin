package main

import shim "js:json_shim"
import "core:fmt"

// writer regression: write the structural cases that previously broke
// (array-of-objects missing commas), then parse back with the strict reader.
main :: proc() {
	w := shim.json_writer_create(false)
	shim.json_writer_begin_object(w)

	shim.json_writer_key(w, "players")
	shim.json_writer_begin_array(w)
	for i in 0..<3 {
		shim.json_writer_begin_object(w)
		shim.json_writer_key(w, "class")
		shim.json_writer_int(w, i32(i))
		shim.json_writer_key(w, "name")
		shim.json_writer_string(w, "npc")
		shim.json_writer_end_object(w)
	}
	shim.json_writer_end_array(w)

	shim.json_writer_key(w, "grid")
	shim.json_writer_begin_array(w)
	for i in 0..<2 {
		shim.json_writer_begin_array(w)
		for j in 0..<3 {
			shim.json_writer_int(w, i32(i*10+j))
		}
		shim.json_writer_end_array(w)
	}
	shim.json_writer_end_array(w)

	shim.json_writer_key(w, "empty_obj")
	shim.json_writer_begin_object(w)
	shim.json_writer_end_object(w)

	shim.json_writer_key(w, "empty_arr")
	shim.json_writer_begin_array(w)
	shim.json_writer_end_array(w)

	shim.json_writer_end_object(w)

	// copy the JSON out before destroying the writer
	json := string(shim.json_writer_get_string(w))
	fmt.println(json)

	// strict round-trip: parse it back
	r := shim.json_reader_parse(cstring(raw_data(json)))
	if r == nil {
		fmt.eprintln("ROUNDTRIP: PARSE FAILED")
		shim.json_writer_destroy(w)
		return
	}
	fmt.eprintln("ROUNDTRIP: OK")

	// verify the array-of-objects values via the cursor
	shim.json_reader_begin_object(r)
	shim.json_reader_property_name(r, "players")
	sz: u32
	shim.json_reader_begin_array(r, &sz)
	fmt.eprintf("players size = %d\n", sz)
	for i in 0..<3 {
		shim.json_reader_begin_object(r)
		shim.json_reader_property_name(r, "class")
		cls: i32
		shim.json_reader_value_int(r, &cls)
		fmt.eprintf("player[%d].class = %d\n", i, cls)
		shim.json_reader_end_object(r)
	}
	shim.json_reader_end_array(r)
	shim.json_reader_end_object(r)

	shim.json_reader_destroy(r)
	shim.json_writer_destroy(w)
}
