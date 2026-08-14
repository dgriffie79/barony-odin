package main
import shim "js:json_shim"
import "core:fmt"

main :: proc() {
	// build a DOM
	root := shim.json_node_create_object()
	shim.json_node_add_member(root, "version", shim.json_node_create_int(25))
	arr := shim.json_node_create_array()
	for i in 0..<3 {
		o := shim.json_node_create_object()
		shim.json_node_add_member(o, "class", shim.json_node_create_int(i32(i)))
		shim.json_node_add_member(o, "name", shim.json_node_create_string("npc"))
		shim.json_node_push_back(arr, o)
	}
	shim.json_node_add_member(root, "players", arr)
	shim.json_node_add_member(root, "enabled", shim.json_node_create_bool(true))
	shim.json_node_add_member(root, "ratio", shim.json_node_create_double(1.5))

	// serialize
	js := shim.json_node_serialize(root, false)
	fmt.println(string(js))

	// parse back via reader
	r := shim.json_reader_parse(js)
	if r == nil { fmt.eprintln("PARSE FAILED"); shim.json_string_free(js); shim.json_node_destroy(root); return }

	// DOM-access: traverse
	rroot := shim.json_node_root(r)
	v: i32
	shim.json_node_get_int(shim.json_node_get_member(rroot, "version"), &v)
	fmt.eprintf("version = %d\n", v)

	parr := shim.json_node_get_member(rroot, "players")
	sz := shim.json_node_array_size(parr)
	fmt.eprintf("players size = %d\n", sz)
	for i in 0..<sz {
		e := shim.json_node_element_at(parr, i)
		cls: i32
		shim.json_node_get_int(shim.json_node_get_member(e, "class"), &cls)
		fmt.eprintf("player[%d].class = %d\n", i, cls)
	}
	b: bool
	shim.json_node_get_bool(shim.json_node_get_member(rroot, "enabled"), &b)
	fmt.eprintf("enabled = %v\n", b)

	// duplicate keys: get_member returns first, member_count yields all
	dup := shim.json_node_create_object()
	shim.json_node_add_member(dup, "a", shim.json_node_create_int(1))
	shim.json_node_add_member(dup, "a", shim.json_node_create_int(2))
	shim.json_node_add_member(dup, "b", shim.json_node_create_int(3))
	first: i32
	shim.json_node_get_int(shim.json_node_get_member(dup, "a"), &first)
	fmt.eprintf("dup get_member(a) = %d (expect 1), count = %d (expect 3)\n", first, shim.json_node_member_count(dup))

	shim.json_string_free(js)
	shim.json_node_destroy(root)
	shim.json_node_destroy(dup)
	shim.json_reader_destroy(r)
}
