package main

import shim "js:json_shim"
import "core:fmt"
import "core:os"

main :: proc() {
	// --- writer: pretty (4-space) ---
	w := shim.json_writer_create(false)
	shim.json_writer_begin_object(w)
	shim.json_writer_key(w, "a")
	shim.json_writer_int(w, 1)
	shim.json_writer_key(w, "arr")
	shim.json_writer_begin_array(w)
	shim.json_writer_int(w, 1)
	shim.json_writer_int(w, 2)
	shim.json_writer_int(w, 3)
	shim.json_writer_end_array(w)
	shim.json_writer_key(w, "nested")
	shim.json_writer_begin_object(w)
	shim.json_writer_key(w, "x")
	shim.json_writer_double(w, 1.5)
	shim.json_writer_end_object(w)
	shim.json_writer_end_object(w)
	fmt.printf("pretty:\n%s\n", string(shim.json_writer_get_string(w)))
	shim.json_writer_destroy(w)

	// --- writer: compact (2-space, single-line arrays) ---
	w2 := shim.json_writer_create(true)
	shim.json_writer_begin_object(w2)
	shim.json_writer_key(w2, "a")
	shim.json_writer_int(w2, 1)
	shim.json_writer_key(w2, "arr")
	shim.json_writer_begin_array(w2)
	shim.json_writer_int(w2, 1)
	shim.json_writer_int(w2, 2)
	shim.json_writer_int(w2, 3)
	shim.json_writer_end_array(w2)
	shim.json_writer_key(w2, "nested")
	shim.json_writer_begin_object(w2)
	shim.json_writer_key(w2, "x")
	shim.json_writer_double(w2, 1.5)
	shim.json_writer_end_object(w2)
	shim.json_writer_end_object(w2)
	fmt.printf("compact:\n%s\n", string(shim.json_writer_get_string(w2)))
	shim.json_writer_destroy(w2)

	// --- reader: parse + walk the test doc ---
	data, err := os.read_entire_file("tools/test_doc.json", context.allocator)
	if err != nil {
		fmt.eprintln("cannot read test_doc.json")
		return
	}
	r := shim.json_reader_parse(cstring(raw_data(data)))
	if r == nil {
		fmt.eprintln("parse failed")
		return
	}
	// walk: $.a, $.e, $.k (string with unicode), $.l[1], $.m.o.p
	shim.json_reader_begin_object(r) // root

	shim.json_reader_property_name(r, "a")
	i: i32
	shim.json_reader_value_int(r, &i)
	fmt.printf("$.a = %d\n", i)

	shim.json_reader_property_name(r, "e")
	d: f64
	shim.json_reader_value_double(r, &d)
	fmt.printf("$.e = %.17g\n", d)

	shim.json_reader_property_name(r, "k")
	s: cstring
	shim.json_reader_value_string(r, &s)
	fmt.printf("$.k = %q\n", string(s))

	shim.json_reader_property_name(r, "l")
	sz: u32
	shim.json_reader_begin_array(r, &sz)
	fmt.printf("$.l size = %d\n", sz)
	shim.json_reader_value_int(r, &i) // [0]
	shim.json_reader_value_double(r, &d) // [1]
	fmt.printf("$.l[1] = %.17g\n", d)
	shim.json_reader_end_array(r)

	shim.json_reader_property_name(r, "m")
	shim.json_reader_begin_object(r)
	shim.json_reader_property_name(r, "o")
	shim.json_reader_begin_object(r)
	shim.json_reader_property_name(r, "p")
	shim.json_reader_value_double(r, &d)
	fmt.printf("$.m.o.p = %.17g\n", d)
	shim.json_reader_end_object(r)
	shim.json_reader_end_object(r)

	shim.json_reader_end_object(r)
	shim.json_reader_destroy(r)
}
