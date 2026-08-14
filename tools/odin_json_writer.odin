package main

import "core:fmt"
import "core:strconv"
import "core:strings"

// format_f64 returns the shortest round-trip representation of v, guaranteed to
// contain '.' or 'e'/'E' so it re-parses as a JSON float (not an integer).
// This is the rapidjson-compatible float formatter (Ryu shortest + ".0" fixup).
format_f64 :: proc(v: f64, allocator := context.allocator) -> string {
	buf: [386]byte
	s := strconv.write_float(buf[:], v, 'g', -1, 64)
	if len(s) > 0 && s[0] == '+' {
		s = s[1:]
	}
	// Inf/NaN are non-numeric; leave as-is (game never serializes these).
	if s == "Inf" || s == "-Inf" || s == "NaN" {
		return strings.clone(s, allocator)
	}
	has_marker := false
	for c in s {
		if c == '.' || c == 'e' || c == 'E' {
			has_marker = true
			break
		}
	}
	if !has_marker {
		return strings.concatenate({s, ".0"}, allocator)
	}
	return strings.clone(s, allocator)
}

// Minimal streaming JSON writer (the eventual @(export) shim shape).
Json_Writer :: struct {
	sb: strings.Builder,
}

jw_init :: proc(w: ^Json_Writer) {
	strings.builder_init(&w.sb)
}

jw_begin_object :: proc(w: ^Json_Writer) {
	strings.write_byte(&w.sb, '{')
}

jw_end_object :: proc(w: ^Json_Writer) {
	strings.write_byte(&w.sb, '}')
}

jw_key :: proc(w: ^Json_Writer, key: string) {
	jw_write_string(w, key)
	strings.write_byte(&w.sb, ':')
}

jw_sep :: proc(w: ^Json_Writer) {
	strings.write_byte(&w.sb, ',')
}

jw_write_int :: proc(w: ^Json_Writer, v: i64) {
	strings.write_i64(&w.sb, v, 10)
}

jw_write_uint :: proc(w: ^Json_Writer, v: u64) {
	strings.write_u64(&w.sb, v, 10)
}

jw_write_double :: proc(w: ^Json_Writer, v: f64) {
	strings.write_string(&w.sb, format_f64(v))
}

jw_write_bool :: proc(w: ^Json_Writer, v: bool) {
	strings.write_string(&w.sb, "true" if v else "false")
}

jw_write_string :: proc(w: ^Json_Writer, v: string) {
	// minimal JSON escaping for the harness corpus
	strings.write_byte(&w.sb, '"')
	for c in v {
		switch c {
		case '"':  strings.write_string(&w.sb, "\\\"")
		case '\\': strings.write_string(&w.sb, "\\\\")
		case '\n': strings.write_string(&w.sb, "\\n")
		case '\t': strings.write_string(&w.sb, "\\t")
		case '\r': strings.write_string(&w.sb, "\\r")
		case:      strings.write_rune(&w.sb, c)
		}
	}
	strings.write_byte(&w.sb, '"')
}

main :: proc() {
	w: Json_Writer
	jw_init(&w)
	jw_begin_object(&w)

	jw_key(&w, "i_neg");  jw_write_int(&w, -1); jw_sep(&w)
	jw_key(&w, "i_zero"); jw_write_int(&w, 0); jw_sep(&w)
	jw_key(&w, "i_pos");  jw_write_int(&w, 123); jw_sep(&w)
	jw_key(&w, "u_zero"); jw_write_uint(&w, 0); jw_sep(&w)
	jw_key(&w, "u_pos");  jw_write_uint(&w, 123); jw_sep(&w)
	jw_key(&w, "u_max");  jw_write_uint(&w, 4294967295); jw_sep(&w)
	jw_key(&w, "d_one");  jw_write_double(&w, 1.0); jw_sep(&w)
	jw_key(&w, "d_half"); jw_write_double(&w, 1.5); jw_sep(&w)
	jw_key(&w, "d_zero"); jw_write_double(&w, 0.0); jw_sep(&w)
	jw_key(&w, "d_neg0"); jw_write_double(&w, -0.0); jw_sep(&w)
	jw_key(&w, "d_tenth"); jw_write_double(&w, 0.1); jw_sep(&w)
	jw_key(&w, "d_third"); jw_write_double(&w, 1.0 / 3.0); jw_sep(&w)
	jw_key(&w, "d_pi");    jw_write_double(&w, 3.141592653589793); jw_sep(&w)
	jw_key(&w, "d_e20");   jw_write_double(&w, 1e20); jw_sep(&w)
	jw_key(&w, "d_e-7");   jw_write_double(&w, 1e-7); jw_sep(&w)
	jw_key(&w, "d_big");   jw_write_double(&w, 1.7976931348623157e308); jw_sep(&w)
	jw_key(&w, "d_small"); jw_write_double(&w, 5e-324); jw_sep(&w)
	jw_key(&w, "f_tenth"); jw_write_double(&w, 0.10000000149011612); jw_sep(&w)
	jw_key(&w, "b_true");  jw_write_bool(&w, true); jw_sep(&w)
	jw_key(&w, "b_false"); jw_write_bool(&w, false); jw_sep(&w)
	jw_key(&w, "s_plain"); jw_write_string(&w, "hello"); jw_sep(&w)
	jw_key(&w, "s_esc");   jw_write_string(&w, "a\"b\\c\nd\te")

	jw_end_object(&w)
	fmt.println(strings.to_string(w.sb))
}
