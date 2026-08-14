// json_shim — rapidjson-compatible JSON writer + reader + DOM access/builder,
// exposed to C++ via @(export) procs. Replaces rapidjson in FileInterface
// (json.cpp) and the ad-hoc DOM parser/builder sites.
//
// Number semantics match rapidjson exactly (range-based Is* classification,
// strict grammar, overflow→double, 1e400 rejection). Duplicate object keys are
// accepted; lookup returns the FIRST match, iteration yields ALL members in
// insertion order (same as rapidjson). Verified against rapidjson in tools/.
package json_shim

import "base:runtime"
import "core:strconv"
import "core:strings"

// ---------------------------------------------------------------------------
// Number classification (rapidjson-compatible)
// ---------------------------------------------------------------------------

Int32_Max  :: i128(2147483647)
Int32_Min  :: i128(-2147483648)
Uint32_Max :: u128(4294967295)
Int64_Max  :: i128(9223372036854775807)
Int64_Min  :: i128(-9223372036854775808)
Uint64_Max :: u128(18446744073709551615)

Number_Class :: struct {
	is_double: bool,
	is_int:    bool,
	is_uint:   bool,
	is_int64:  bool,
	is_uint64: bool,
	i_val:     i128,
	f_val:     f64,
	valid:     bool,
}

is_valid_json_number :: proc(tok: string) -> bool {
	i := 0
	if i < len(tok) && tok[i] == '-' { i += 1 }
	if i >= len(tok) { return false }
	if tok[i] == '0' {
		i += 1
		if i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { return false }
	} else if tok[i] >= '1' && tok[i] <= '9' {
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	} else {
		return false
	}
	if i < len(tok) && tok[i] == '.' {
		i += 1
		if i >= len(tok) || tok[i] < '0' || tok[i] > '9' { return false }
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	}
	if i < len(tok) && (tok[i] == 'e' || tok[i] == 'E') {
		i += 1
		if i < len(tok) && (tok[i] == '+' || tok[i] == '-') { i += 1 }
		if i >= len(tok) || tok[i] < '0' || tok[i] > '9' { return false }
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	}
	return i == len(tok)
}

classify_number :: proc(tok: string) -> Number_Class {
	c := Number_Class{}
	has_dot := false
	has_exp := false
	for r in tok {
		if r == '.' { has_dot = true }
		if r == 'e' || r == 'E' { has_exp = true }
	}
	if has_dot || has_exp {
		f, ok := strconv.parse_f64(tok)
		if !ok || f > 1.7976931348623157e308 { return c }
		c.valid = true
		c.is_double = true
		c.f_val = f
		return c
	}

	neg := len(tok) > 0 && tok[0] == '-'
	s := tok
	if neg { s = s[1:] }
	mag: u128
	for r in s {
		d := u128(r - '0')
		if mag > (u128(1) << 127) {
			f, ok := strconv.parse_f64(tok)
			if !ok { return c }
			c.valid = true
			c.is_double = true
			c.f_val = f
			return c
		}
		mag = mag * 10 + d
	}
	if mag == 0 {
		c.valid = true
		c.is_int = true
		c.is_uint = true
		c.is_int64 = true
		c.is_uint64 = true
		return c
	}
	if neg {
		if mag <= u128(1) << 63 {
			n := -i128(mag)
			c.valid = true
			c.is_int = n >= Int32_Min
			c.is_int64 = true
			c.i_val = n
			return c
		} else {
			f, ok := strconv.parse_f64(tok)
			if !ok { return c }
			c.valid = true
			c.is_double = true
			c.f_val = f
			return c
		}
	} else {
		if mag <= Uint64_Max {
			c.valid = true
			c.is_int = mag <= u128(Int32_Max)
			c.is_uint = mag <= Uint32_Max
			c.is_int64 = mag <= u128(Int64_Max)
			c.is_uint64 = true
			c.i_val = i128(mag)
			return c
		} else {
			f, ok := strconv.parse_f64(tok)
			if !ok { return c }
			c.valid = true
			c.is_double = true
			c.f_val = f
			return c
		}
	}
}

// shortest round-trip float formatter; guarantees '.' or 'e'/'E' present so it
// re-parses as a JSON float (not integer).
format_f64 :: proc(v: f64, allocator := context.allocator) -> string {
	buf: [386]byte
	s := strconv.write_float(buf[:], v, 'g', -1, 64)
	if len(s) > 0 && s[0] == '+' { s = s[1:] }
	if s == "Inf" || s == "-Inf" || s == "NaN" {
		return strings.clone(s, allocator)
	}
	has_marker := false
	for c in s {
		if c == '.' || c == 'e' || c == 'E' { has_marker = true; break }
	}
	if !has_marker {
		return strings.concatenate({s, ".0"}, allocator)
	}
	return strings.clone(s, allocator)
}

// ---------------------------------------------------------------------------
// Writer (streaming, rapidjson PrettyWriter-compatible)
// ---------------------------------------------------------------------------

Json_Writer :: struct {
	sb:                 strings.Builder,
	indent:             string,
	single_line_arrays: bool,
	depth:              int,
	first:              [dynamic]bool,
	is_array:           [dynamic]bool,
}

writer_init :: proc(w: ^Json_Writer, compact: bool) {
	strings.builder_init(&w.sb)
	if compact {
		w.indent = "  "
		w.single_line_arrays = true
	} else {
		w.indent = "    "
		w.single_line_arrays = false
	}
}

write_indent :: proc(w: ^Json_Writer, depth: int) {
	for _ in 0..<depth {
		strings.write_string(&w.sb, w.indent)
	}
}

write_json_string :: proc(sb: ^strings.Builder, s: string) {
	strings.write_byte(sb, '"')
	for c in s {
		switch c {
		case '"':  strings.write_string(sb, "\\\"")
		case '\\': strings.write_string(sb, "\\\\")
		case '\n': strings.write_string(sb, "\\n")
		case '\t': strings.write_string(sb, "\\t")
		case '\r': strings.write_string(sb, "\\r")
		case '\b': strings.write_string(sb, "\\b")
		case '\f': strings.write_string(sb, "\\f")
		case:      strings.write_rune(sb, c)
		}
	}
	strings.write_byte(sb, '"')
}

// write the prefix for an array element (comma + newline + indent)
array_element_prefix :: proc(w: ^Json_Writer) {
	if w.single_line_arrays {
		if w.first[w.depth-1] {
			w.first[w.depth-1] = false
		} else {
			strings.write_string(&w.sb, ", ")
		}
	} else {
		if w.first[w.depth-1] {
			strings.write_byte(&w.sb, '\n')
			write_indent(w, w.depth)
			w.first[w.depth-1] = false
		} else {
			strings.write_string(&w.sb, ",\n")
			write_indent(w, w.depth)
		}
	}
}

writer_begin_object :: proc(w: ^Json_Writer) {
	if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_byte(&w.sb, '{')
	w.depth += 1
	append(&w.first, true)
	append(&w.is_array, false)
}

writer_end_object :: proc(w: ^Json_Writer) {
	w.depth -= 1
	if !w.first[w.depth] {
		strings.write_byte(&w.sb, '\n')
		write_indent(w, w.depth)
	}
	strings.write_byte(&w.sb, '}')
	pop(&w.first)
	pop(&w.is_array)
}

writer_begin_array :: proc(w: ^Json_Writer) {
	if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_byte(&w.sb, '[')
	w.depth += 1
	append(&w.first, true)
	append(&w.is_array, true)
}

writer_end_array :: proc(w: ^Json_Writer) {
	w.depth -= 1
	if !w.single_line_arrays && !w.first[w.depth] {
		strings.write_byte(&w.sb, '\n')
		write_indent(w, w.depth)
	}
	strings.write_byte(&w.sb, ']')
	pop(&w.first)
	pop(&w.is_array)
}

writer_key :: proc(w: ^Json_Writer, key: string) {
	if w.first[w.depth-1] {
		strings.write_byte(&w.sb, '\n')
		write_indent(w, w.depth)
		w.first[w.depth-1] = false
	} else {
		strings.write_string(&w.sb, ",\n")
		write_indent(w, w.depth)
	}
	write_json_string(&w.sb, key)
	strings.write_string(&w.sb, ": ")
}

writer_uint :: proc(w: ^Json_Writer, v: u32) {
	if w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_u64(&w.sb, u64(v), 10)
}

writer_int :: proc(w: ^Json_Writer, v: i32) {
	if w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_i64(&w.sb, i64(v), 10)
}

writer_double :: proc(w: ^Json_Writer, v: f64) {
	if w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_string(&w.sb, format_f64(v))
}

writer_bool :: proc(w: ^Json_Writer, v: bool) {
	if w.is_array[w.depth-1] { array_element_prefix(w) }
	strings.write_string(&w.sb, "true" if v else "false")
}

writer_string :: proc(w: ^Json_Writer, s: string) {
	if w.is_array[w.depth-1] { array_element_prefix(w) }
	write_json_string(&w.sb, s)
}

// --- @(export) writer wrappers ---

@(export)
json_writer_create :: proc "c" (compact: bool) -> rawptr {
	context = runtime.default_context()
	w := new(Json_Writer)
	writer_init(w, compact)
	return w
}

@(export)
json_writer_destroy :: proc "c" (w: rawptr) {
	context = runtime.default_context()
	ww := cast(^Json_Writer)w
	delete(ww.first)
	delete(ww.is_array)
	strings.builder_destroy(&ww.sb)
	free(ww)
}

@(export)
json_writer_begin_object :: proc "c" (w: rawptr) -> bool {
	context = runtime.default_context()
	writer_begin_object(cast(^Json_Writer)w)
	return true
}

@(export)
json_writer_end_object :: proc "c" (w: rawptr) {
	context = runtime.default_context()
	writer_end_object(cast(^Json_Writer)w)
}

@(export)
json_writer_begin_array :: proc "c" (w: rawptr) -> bool {
	context = runtime.default_context()
	writer_begin_array(cast(^Json_Writer)w)
	return true
}

@(export)
json_writer_end_array :: proc "c" (w: rawptr) {
	context = runtime.default_context()
	writer_end_array(cast(^Json_Writer)w)
}

@(export)
json_writer_key :: proc "c" (w: rawptr, key: cstring) {
	context = runtime.default_context()
	writer_key(cast(^Json_Writer)w, string(key))
}

@(export)
json_writer_uint :: proc "c" (w: rawptr, v: u32) -> bool {
	context = runtime.default_context()
	writer_uint(cast(^Json_Writer)w, v)
	return true
}

@(export)
json_writer_int :: proc "c" (w: rawptr, v: i32) -> bool {
	context = runtime.default_context()
	writer_int(cast(^Json_Writer)w, v)
	return true
}

@(export)
json_writer_double :: proc "c" (w: rawptr, v: f64) -> bool {
	context = runtime.default_context()
	writer_double(cast(^Json_Writer)w, v)
	return true
}

@(export)
json_writer_bool :: proc "c" (w: rawptr, v: bool) -> bool {
	context = runtime.default_context()
	writer_bool(cast(^Json_Writer)w, v)
	return true
}

@(export)
json_writer_string :: proc "c" (w: rawptr, s: cstring) -> bool {
	context = runtime.default_context()
	writer_string(cast(^Json_Writer)w, string(s))
	return true
}

@(export)
json_writer_get_string :: proc "c" (w: rawptr) -> cstring {
	context = runtime.default_context()
	ww := cast(^Json_Writer)w
	strings.write_byte(&ww.sb, 0)
	s := strings.to_string(ww.sb)
	return cstring(raw_data(s))
}

// ---------------------------------------------------------------------------
// DOM node types (heap-allocated; stable pointers across the C ABI)
// ---------------------------------------------------------------------------

Node_Kind :: enum { Null, Bool, Number, String, Array, Object }

Member :: struct {
	name:  string,
	value: ^Node,
}

Node :: struct {
	kind:    Node_Kind,
	num:     Number_Class,
	b:       bool,
	str:     string, // decoded, NUL-terminated (data[len]==0)
	arr:     [dynamic]^Node,
	members: [dynamic]Member,
}

// ---------------------------------------------------------------------------
// Parser (recursive descent → heap-allocated Node tree)
// ---------------------------------------------------------------------------

Parser :: struct {
	data: string,
	pos:  int,
}

skip_ws :: proc(p: ^Parser) {
	for p.pos < len(p.data) {
		switch p.data[p.pos] {
		case ' ', '\t', '\n', '\r': p.pos += 1
		case: return
		}
	}
}

parse_hex4 :: proc(s: string) -> (rune, bool) {
	if len(s) != 4 { return 0, false }
	v: rune
	for c in s {
		v <<= 4
		switch c {
		case '0'..='9': v |= rune(c - '0')
		case 'a'..='f': v |= rune(c - 'a' + 10)
		case 'A'..='F': v |= rune(c - 'A' + 10)
		case: return 0, false
		}
	}
	return v, true
}

// returns a string whose backing data is NUL-terminated (data[len]==0)
nul_terminated :: proc(s: string, allocator := context.allocator) -> string {
	buf := make([]byte, len(s) + 1, allocator)
	copy(buf, s)
	buf[len(s)] = 0
	return string(buf[:len(s)])
}

parse_string :: proc(p: ^Parser) -> (string, bool) {
	p.pos += 1
	sb: strings.Builder
	strings.builder_init(&sb)
	for p.pos < len(p.data) {
		c := p.data[p.pos]
		if c == '"' {
			p.pos += 1
			return nul_terminated(strings.to_string(sb)), true
		}
		if c == '\\' {
			p.pos += 1
			if p.pos >= len(p.data) { return "", false }
			e := p.data[p.pos]
			p.pos += 1
			switch e {
			case '"':  strings.write_byte(&sb, '"')
			case '\\': strings.write_byte(&sb, '\\')
			case '/':  strings.write_byte(&sb, '/')
			case 'b':  strings.write_byte(&sb, '\b')
			case 'f':  strings.write_byte(&sb, '\f')
			case 'n':  strings.write_byte(&sb, '\n')
			case 'r':  strings.write_byte(&sb, '\r')
			case 't':  strings.write_byte(&sb, '\t')
			case 'u':
				if p.pos + 4 > len(p.data) { return "", false }
				cp, ok := parse_hex4(p.data[p.pos:p.pos+4])
				if !ok { return "", false }
				p.pos += 4
				if cp >= 0xD800 && cp <= 0xDBFF {
					if p.pos + 2 > len(p.data) || p.data[p.pos] != '\\' || p.data[p.pos+1] != 'u' {
						return "", false
					}
					p.pos += 2
					if p.pos + 4 > len(p.data) { return "", false }
					cp2, ok2 := parse_hex4(p.data[p.pos:p.pos+4])
					if !ok2 { return "", false }
					p.pos += 4
					if cp2 < 0xDC00 || cp2 > 0xDFFF { return "", false }
					cp = 0x10000 + ((cp - 0xD800) << 10) + (cp2 - 0xDC00)
				} else if cp >= 0xDC00 && cp <= 0xDFFF {
					return "", false
				}
				strings.write_rune(&sb, cp)
			case: return "", false
			}
		} else if c < 0x20 {
			return "", false
		} else {
			strings.write_byte(&sb, c)
			p.pos += 1
		}
	}
	return "", false
}

parse_number_node :: proc(p: ^Parser) -> (^Node, bool) {
	start := p.pos
	for p.pos < len(p.data) {
		c := p.data[p.pos]
		if (c >= '0' && c <= '9') || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E' {
			p.pos += 1
		} else {
			break
		}
	}
	tok := p.data[start:p.pos]
	if !is_valid_json_number(tok) { return nil, false }
	c := classify_number(tok)
	if !c.valid { return nil, false }
	n := new(Node)
	n.kind = .Number
	n.num = c
	return n, true
}

parse_object :: proc(p: ^Parser) -> (^Node, bool) {
	p.pos += 1
	n := new(Node)
	n.kind = .Object
	skip_ws(p)
	if p.pos < len(p.data) && p.data[p.pos] == '}' {
		p.pos += 1
		return n, true
	}
	for {
		skip_ws(p)
		if p.pos >= len(p.data) || p.data[p.pos] != '"' { return nil, false }
		key, key_ok := parse_string(p)
		if !key_ok { return nil, false }
		skip_ws(p)
		if p.pos >= len(p.data) || p.data[p.pos] != ':' { return nil, false }
		p.pos += 1
		val, val_ok := parse_value(p)
		if !val_ok { return nil, false }
		append(&n.members, Member{name = key, value = val})
		skip_ws(p)
		if p.pos < len(p.data) && p.data[p.pos] == ',' {
			p.pos += 1
			continue
		}
		if p.pos < len(p.data) && p.data[p.pos] == '}' {
			p.pos += 1
			return n, true
		}
		return nil, false
	}
}

parse_array :: proc(p: ^Parser) -> (^Node, bool) {
	p.pos += 1
	n := new(Node)
	n.kind = .Array
	skip_ws(p)
	if p.pos < len(p.data) && p.data[p.pos] == ']' {
		p.pos += 1
		return n, true
	}
	for {
		val, ok := parse_value(p)
		if !ok { return nil, false }
		append(&n.arr, val)
		skip_ws(p)
		if p.pos < len(p.data) && p.data[p.pos] == ',' {
			p.pos += 1
			continue
		}
		if p.pos < len(p.data) && p.data[p.pos] == ']' {
			p.pos += 1
			return n, true
		}
		return nil, false
	}
}

parse_literal :: proc(p: ^Parser, lit: string, kind: Node_Kind, b: bool) -> (^Node, bool) {
	if p.pos + len(lit) <= len(p.data) && p.data[p.pos:p.pos+len(lit)] == lit {
		p.pos += len(lit)
		n := new(Node)
		n.kind = kind
		n.b = b
		return n, true
	}
	return nil, false
}

parse_value :: proc(p: ^Parser) -> (^Node, bool) {
	skip_ws(p)
	if p.pos >= len(p.data) { return nil, false }
	switch p.data[p.pos] {
	case '{': return parse_object(p)
	case '[': return parse_array(p)
	case '"':
		s, ok := parse_string(p)
		if !ok { return nil, false }
		n := new(Node)
		n.kind = .String
		n.str = s
		return n, true
	case 't': return parse_literal(p, "true", .Bool, true)
	case 'f': return parse_literal(p, "false", .Bool, false)
	case 'n': return parse_literal(p, "null", .Null, false)
	case: return parse_number_node(p)
	}
}

parse_document :: proc(data: string) -> (^Node, bool) {
	p := Parser{data = data, pos = 0}
	n, ok := parse_value(&p)
	if !ok { return nil, false }
	skip_ws(&p)
	if p.pos != len(p.data) { return nil, false }
	return n, true
}

destroy_node :: proc(n: ^Node) {
	switch n.kind {
	case .Null, .Bool, .Number:
	case .String: delete(n.str)
	case .Array:
		for e in n.arr { destroy_node(e); free(e) }
		delete(n.arr)
	case .Object:
		for m in n.members {
			delete(m.name)
			destroy_node(m.value)
			free(m.value)
		}
		delete(n.members)
	}
}

// ---------------------------------------------------------------------------
// Reader (DOM + lockstep cursor, rapidjson-compatible)
// ---------------------------------------------------------------------------

Frame :: struct {
	node:  ^Node,
	index: int, // array index (or -1)
}

Json_Reader :: struct {
	root:     ^Node,
	stack:    [dynamic]Frame,
	prop:     string,
	has_prop: bool,
}

// resolve the "current value" the same way rapidjson's GetCurrentValue() does
get_current :: proc(r: ^Json_Reader) -> ^Node {
	if len(r.stack) == 0 {
		return r.root
	}
	frame := &r.stack[len(r.stack)-1]
	if frame.node.kind == .Array {
		if frame.index >= 0 && frame.index < len(frame.node.arr) {
			n := frame.node.arr[frame.index]
			frame.index += 1
			return n
		}
		return nil
	}
	if r.has_prop {
		name := r.prop
		r.has_prop = false
		for m in frame.node.members {
			if m.name == name {
				return m.value
			}
		}
		return nil
	}
	return nil
}

@(export)
json_reader_parse :: proc "c" (data: cstring) -> rawptr {
	context = runtime.default_context()
	r := new(Json_Reader)
	root, ok := parse_document(string(data))
	if !ok {
		free(r)
		return nil
	}
	r.root = root
	return r
}

@(export)
json_reader_destroy :: proc "c" (r: rawptr) {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	if rr.root != nil {
		destroy_node(rr.root)
		free(rr.root)
	}
	delete(rr.stack)
	free(rr)
}

@(export)
json_reader_begin_object :: proc "c" (r: rawptr) -> bool {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	cur := get_current(rr)
	if cur == nil || cur.kind != .Object { return false }
	append(&rr.stack, Frame{node = cur, index = -1})
	return true
}

@(export)
json_reader_end_object :: proc "c" (r: rawptr) {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	if len(rr.stack) > 0 { pop(&rr.stack) }
}

@(export)
json_reader_begin_array :: proc "c" (r: rawptr, size: ^u32) -> bool {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	cur := get_current(rr)
	if cur == nil || cur.kind != .Array { return false }
	append(&rr.stack, Frame{node = cur, index = 0})
	size^ = u32(len(cur.arr))
	return true
}

@(export)
json_reader_end_array :: proc "c" (r: rawptr) {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	if len(rr.stack) > 0 { pop(&rr.stack) }
}

@(export)
json_reader_property_name :: proc "c" (r: rawptr, name: cstring) {
	context = runtime.default_context()
	rr := cast(^Json_Reader)r
	rr.prop = string(name)
	rr.has_prop = true
}

@(export)
json_reader_value_uint :: proc "c" (r: rawptr, out: ^u32) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .Number || !cur.num.is_uint { return false }
	out^ = u32(cur.num.i_val)
	return true
}

@(export)
json_reader_value_int :: proc "c" (r: rawptr, out: ^i32) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .Number || !cur.num.is_int { return false }
	out^ = i32(cur.num.i_val)
	return true
}

@(export)
json_reader_value_float :: proc "c" (r: rawptr, out: ^f32) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .Number || !cur.num.is_double { return false }
	f := cur.num.f_val
	if f < -3.4028234e38 || f > 3.4028234e38 { return false }
	out^ = f32(f)
	return true
}

@(export)
json_reader_value_double :: proc "c" (r: rawptr, out: ^f64) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .Number || !cur.num.is_double { return false }
	out^ = cur.num.f_val
	return true
}

@(export)
json_reader_value_bool :: proc "c" (r: rawptr, out: ^bool) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .Bool { return false }
	out^ = cur.b
	return true
}

@(export)
json_reader_value_string :: proc "c" (r: rawptr, out: ^cstring) -> bool {
	context = runtime.default_context()
	cur := get_current(cast(^Json_Reader)r)
	if cur == nil || cur.kind != .String { return false }
	out^ = cstring(raw_data(cur.str))
	return true
}

// ---------------------------------------------------------------------------
// DOM access (read side) — for the ad-hoc parser sites
// ---------------------------------------------------------------------------

@(export)
json_node_root :: proc "c" (r: rawptr) -> rawptr {
	context = runtime.default_context()
	return (cast(^Json_Reader)r).root
}

@(export)
json_node_get_member :: proc "c" (node: rawptr, key: cstring) -> rawptr {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Object { return nil }
	for m in n.members {
		if m.name == string(key) { return m.value }
	}
	return nil
}

@(export)
json_node_has_member :: proc "c" (node: rawptr, key: cstring) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Object { return false }
	for m in n.members {
		if m.name == string(key) { return true }
	}
	return false
}

@(export)
json_node_member_count :: proc "c" (node: rawptr) -> u32 {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Object { return 0 }
	return u32(len(n.members))
}

@(export)
json_node_member_name_at :: proc "c" (node: rawptr, i: u32) -> cstring {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Object || int(i) >= len(n.members) { return nil }
	return cstring(raw_data(n.members[i].name))
}

@(export)
json_node_member_value_at :: proc "c" (node: rawptr, i: u32) -> rawptr {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Object || int(i) >= len(n.members) { return nil }
	return n.members[i].value
}

@(export)
json_node_array_size :: proc "c" (node: rawptr) -> u32 {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Array { return 0 }
	return u32(len(n.arr))
}

@(export)
json_node_element_at :: proc "c" (node: rawptr, i: u32) -> rawptr {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Array || int(i) >= len(n.arr) { return nil }
	return n.arr[i]
}

@(export)
json_node_is_object :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .Object
}

@(export)
json_node_is_array :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .Array
}

@(export)
json_node_is_string :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .String
}

@(export)
json_node_is_bool :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .Bool
}

@(export)
json_node_is_null :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .Null
}

@(export)
json_node_is_number :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	return (cast(^Node)node).kind == .Number
}

@(export)
json_node_is_int :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	return n.kind == .Number && n.num.is_int
}

@(export)
json_node_is_uint :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	return n.kind == .Number && n.num.is_uint
}

@(export)
json_node_is_int64 :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	return n.kind == .Number && n.num.is_int64
}

@(export)
json_node_is_uint64 :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	return n.kind == .Number && n.num.is_uint64
}

@(export)
json_node_is_double :: proc "c" (node: rawptr) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	return n.kind == .Number && n.num.is_double
}

@(export)
json_node_get_int :: proc "c" (node: rawptr, out: ^i32) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number || !n.num.is_int { return false }
	out^ = i32(n.num.i_val)
	return true
}

@(export)
json_node_get_uint :: proc "c" (node: rawptr, out: ^u32) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number || !n.num.is_uint { return false }
	out^ = u32(n.num.i_val)
	return true
}

@(export)
json_node_get_int64 :: proc "c" (node: rawptr, out: ^i64) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number || !n.num.is_int64 { return false }
	out^ = i64(n.num.i_val)
	return true
}

@(export)
json_node_get_uint64 :: proc "c" (node: rawptr, out: ^u64) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number || !n.num.is_uint64 { return false }
	out^ = u64(n.num.i_val)
	return true
}

@(export)
json_node_get_double :: proc "c" (node: rawptr, out: ^f64) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number { return false }
	if n.num.is_double {
		out^ = n.num.f_val
	} else {
		out^ = f64(n.num.i_val)
	}
	return true
}

@(export)
json_node_get_float :: proc "c" (node: rawptr, out: ^f32) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Number { return false }
	if n.num.is_double {
		out^ = f32(n.num.f_val)
	} else {
		out^ = f32(n.num.i_val)
	}
	return true
}

@(export)
json_node_get_bool :: proc "c" (node: rawptr, out: ^bool) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .Bool { return false }
	out^ = n.b
	return true
}

@(export)
json_node_get_string :: proc "c" (node: rawptr, out: ^cstring) -> bool {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind != .String { return false }
	out^ = cstring(raw_data(n.str))
	return true
}

// ---------------------------------------------------------------------------
// DOM builder (write side) — for the ad-hoc write sites
// ---------------------------------------------------------------------------

@(export)
json_node_create_object :: proc "c" () -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Object
	return n
}

@(export)
json_node_create_array :: proc "c" () -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Array
	return n
}

@(export)
json_node_create_int :: proc "c" (v: i32) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Number
	n.num = Number_Class{is_int = true, is_int64 = true, i_val = i128(v), valid = true}
	return n
}

@(export)
json_node_create_uint :: proc "c" (v: u32) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Number
	n.num = Number_Class{is_uint = true, is_uint64 = true, i_val = i128(v), valid = true}
	return n
}

@(export)
json_node_create_int64 :: proc "c" (v: i64) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Number
	n.num = Number_Class{is_int64 = true, i_val = i128(v), valid = true}
	return n
}

@(export)
json_node_create_uint64 :: proc "c" (v: u64) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Number
	n.num = Number_Class{is_uint64 = true, i_val = i128(v), valid = true}
	return n
}

@(export)
json_node_create_double :: proc "c" (v: f64) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Number
	n.num = Number_Class{is_double = true, f_val = v, valid = true}
	return n
}

@(export)
json_node_create_bool :: proc "c" (v: bool) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Bool
	n.b = v
	return n
}

@(export)
json_node_create_string :: proc "c" (s: cstring) -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .String
	n.str = nul_terminated(string(s))
	return n
}

@(export)
json_node_create_null :: proc "c" () -> rawptr {
	context = runtime.default_context()
	n := new(Node)
	n.kind = .Null
	return n
}

@(export)
json_node_add_member :: proc "c" (obj: rawptr, key: cstring, value: rawptr) {
	context = runtime.default_context()
	n := cast(^Node)obj
	append(&n.members, Member{name = nul_terminated(string(key)), value = cast(^Node)value})
}

@(export)
json_node_push_back :: proc "c" (arr: rawptr, value: rawptr) {
	context = runtime.default_context()
	n := cast(^Node)arr
	append(&n.arr, cast(^Node)value)
}

@(export)
json_node_destroy :: proc "c" (node: rawptr) {
	context = runtime.default_context()
	n := cast(^Node)node
	destroy_node(n)
	free(n)
}

@(export)
json_node_erase_member :: proc "c" (obj: rawptr, key: cstring) {
	context = runtime.default_context()
	n := cast(^Node)obj
	if n.kind != .Object { return }
	for i := 0; i < len(n.members); i += 1 {
		if n.members[i].name == string(key) {
			delete(n.members[i].name)
			destroy_node(n.members[i].value)
			free(n.members[i].value)
			for j := i; j < len(n.members)-1; j += 1 {
				n.members[j] = n.members[j+1]
			}
			pop(&n.members)
			return
		}
	}
}

@(export)
json_node_clear_array :: proc "c" (arr: rawptr) {
	context = runtime.default_context()
	n := cast(^Node)arr
	if n.kind != .Array { return }
	for e in n.arr {
		destroy_node(e)
		free(e)
	}
	clear(&n.arr)
}

@(export)
json_node_remove_all_members :: proc "c" (obj: rawptr) {
	context = runtime.default_context()
	n := cast(^Node)obj
	if n.kind != .Object { return }
	for m in n.members {
		delete(m.name)
		destroy_node(m.value)
		free(m.value)
	}
	clear(&n.members)
}

@(export)
json_node_parse_document :: proc "c" (data: cstring) -> rawptr {
	context = runtime.default_context()
	n, ok := parse_document(string(data))
	if !ok { return nil }
	return n
}

@(export)
json_node_set_string :: proc "c" (node: rawptr, s: cstring) {
	context = runtime.default_context()
	n := cast(^Node)node
	if n.kind == .String { delete(n.str) }
	n.kind = .String
	n.str = nul_terminated(string(s))
}

// ---------------------------------------------------------------------------
// Serializer (walk a Node tree → JSON string)
// ---------------------------------------------------------------------------

serialize_node :: proc(w: ^Json_Writer, n: ^Node) {
	switch n.kind {
	case .Null:
		if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
		strings.write_string(&w.sb, "null")
	case .Bool:
		if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
		strings.write_string(&w.sb, "true" if n.b else "false")
	case .Number:
		if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
		if n.num.is_double {
			strings.write_string(&w.sb, format_f64(n.num.f_val))
		} else if n.num.i_val >= 0 {
			strings.write_u64(&w.sb, u64(n.num.i_val), 10)
		} else {
			strings.write_i64(&w.sb, i64(n.num.i_val), 10)
		}
	case .String:
		if w.depth > 0 && w.is_array[w.depth-1] { array_element_prefix(w) }
		write_json_string(&w.sb, n.str)
	case .Array:
		writer_begin_array(w)
		for e in n.arr {
			serialize_node(w, e)
		}
		writer_end_array(w)
	case .Object:
		writer_begin_object(w)
		for m in n.members {
			writer_key(w, m.name)
			serialize_node(w, m.value)
		}
		writer_end_object(w)
	}
}

// clone a string to a NUL-terminated cstring (heap-allocated)
clone_to_cstring :: proc(s: string, allocator := context.allocator) -> cstring {
	buf := make([]byte, len(s) + 1, allocator)
	copy(buf, s)
	buf[len(s)] = 0
	return cstring(raw_data(buf))
}

@(export)
json_node_serialize :: proc "c" (node: rawptr, compact: bool) -> cstring {
	context = runtime.default_context()
	w := new(Json_Writer)
	writer_init(w, compact)
	serialize_node(w, cast(^Node)node)
	s := strings.to_string(w.sb)
	c := clone_to_cstring(s)
	strings.builder_destroy(&w.sb)
	delete(w.first)
	delete(w.is_array)
	free(w)
	return c
}

@(export)
json_string_free :: proc "c" (s: cstring) {
	context = runtime.default_context()
	free(rawptr(s))
}
