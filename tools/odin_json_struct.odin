package main

import "core:fmt"
import "core:os"
import "core:strconv"
import "core:strings"

// ---------------------------------------------------------------------------
// Number classification (rapidjson-compatible) — copied from the proven
// odin_json_reader.odin; will be consolidated into the real package later.
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

// strict JSON number grammar: -? (0 | [1-9][0-9]*) (\.[0-9]+)? ([eE][+-]?[0-9]+)?
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
		if !ok || f > 1.7976931348623157e308 {
			return c
		}
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
			// far overflow -> double
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

// ---------------------------------------------------------------------------
// DOM + parser
// ---------------------------------------------------------------------------

Node_Kind :: enum { Null, Bool, Number, String, Array, Object }

Member :: struct {
	name:  string,
	value: Node,
}

Node :: struct {
	kind:    Node_Kind,
	num:     Number_Class,
	b:       bool,
	str:     string,
	arr:     [dynamic]Node,
	members: [dynamic]Member,
}

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

// parse a string literal (assumes p.data[p.pos] == '"'); returns the decoded string
parse_string :: proc(p: ^Parser) -> (string, bool) {
	p.pos += 1
	sb: strings.Builder
	strings.builder_init(&sb)
	for p.pos < len(p.data) {
		c := p.data[p.pos]
		if c == '"' {
			p.pos += 1
			return strings.to_string(sb), true
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
					// high surrogate: require \uXXXX low surrogate
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
					return "", false // lone low surrogate
				}
				strings.write_rune(&sb, cp)
			case: return "", false
			}
		} else if c < 0x20 {
			return "", false // unescaped control char
		} else {
			strings.write_byte(&sb, c)
			p.pos += 1
		}
	}
	return "", false // unterminated
}

parse_number_node :: proc(p: ^Parser) -> (Node, bool) {
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
	if !is_valid_json_number(tok) { return {}, false }
	c := classify_number(tok)
	if !c.valid { return {}, false }
	return Node{kind = .Number, num = c}, true
}

parse_object :: proc(p: ^Parser) -> (Node, bool) {
	p.pos += 1 // consume '{'
	n := Node{kind = .Object}
	skip_ws(p)
	if p.pos < len(p.data) && p.data[p.pos] == '}' {
		p.pos += 1
		return n, true
	}
	for {
		skip_ws(p)
		if p.pos >= len(p.data) || p.data[p.pos] != '"' { return {}, false }
		key, key_ok := parse_string(p)
		if !key_ok { return {}, false }
		skip_ws(p)
		if p.pos >= len(p.data) || p.data[p.pos] != ':' { return {}, false }
		p.pos += 1
		val, val_ok := parse_value(p)
		if !val_ok { return {}, false }
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
		return {}, false
	}
}

parse_array :: proc(p: ^Parser) -> (Node, bool) {
	p.pos += 1 // consume '['
	n := Node{kind = .Array}
	skip_ws(p)
	if p.pos < len(p.data) && p.data[p.pos] == ']' {
		p.pos += 1
		return n, true
	}
	for {
		val, ok := parse_value(p)
		if !ok { return {}, false }
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
		return {}, false
	}
}

parse_literal :: proc(p: ^Parser, lit: string, n: Node) -> (Node, bool) {
	if p.pos + len(lit) <= len(p.data) && p.data[p.pos:p.pos+len(lit)] == lit {
		p.pos += len(lit)
		return n, true
	}
	return {}, false
}

parse_value :: proc(p: ^Parser) -> (Node, bool) {
	skip_ws(p)
	if p.pos >= len(p.data) { return {}, false }
	switch p.data[p.pos] {
	case '{': return parse_object(p)
	case '[': return parse_array(p)
	case '"': 
		s, ok := parse_string(p)
		if !ok { return {}, false }
		return Node{kind = .String, str = s}, true
	case 't': return parse_literal(p, "true", Node{kind = .Bool, b = true})
	case 'f': return parse_literal(p, "false", Node{kind = .Bool, b = false})
	case 'n': return parse_literal(p, "null", Node{kind = .Null})
	case: return parse_number_node(p)
	}
}

parse_document :: proc(data: string) -> (Node, bool) {
	p := Parser{data = data, pos = 0}
	n, ok := parse_value(&p)
	if !ok { return {}, false }
	skip_ws(&p)
	if p.pos != len(p.data) { return {}, false }
	return n, true
}

// ---------------------------------------------------------------------------
// depth-first dump (must match rapidjson_struct_walk.cpp byte-for-byte)
// ---------------------------------------------------------------------------

dump_node :: proc(n: ^Node, path: string) {
	switch n.kind {
	case .Null:
		fmt.printf("%s\tnull\n", path)
	case .Bool:
		fmt.printf("%s\tbool\t%v\n", path, n.b)
	case .String:
		fmt.printf("%s\tstring\t%s\n", path, n.str)
	case .Number:
		c := n.num
		fmt.printf("%s\tnumber\t%d %d %d %d %d", path,
			int(c.is_int), int(c.is_uint), int(c.is_int64), int(c.is_uint64), int(c.is_double))
		if c.is_int { fmt.printf("\tInt:%d", c.i_val) }
		if c.is_uint { fmt.printf("\tUint:%d", u64(c.i_val)) }
		if c.is_int64 { fmt.printf("\tInt64:%d", c.i_val) }
		if c.is_uint64 { fmt.printf("\tUint64:%d", u64(c.i_val)) }
		if c.is_double { fmt.printf("\tDouble:%.17g", c.f_val) }
		fmt.println()
	case .Array:
		for i in 0..<len(n.arr) {
			dump_node(&n.arr[i], fmt.tprintf("%s[%d]", path, i))
		}
	case .Object:
		for i in 0..<len(n.members) {
			dump_node(&n.members[i].value, fmt.tprintf("%s.%s", path, n.members[i].name))
		}
	}
}

main :: proc() {
	data, err := os.read_entire_file("tools/test_doc.json", context.allocator)
	if err != nil {
		fmt.eprintln("cannot read tools/test_doc.json")
		return
	}
	root, ok2 := parse_document(string(data))
	if !ok2 {
		fmt.eprintln("PARSE ERROR")
		return
	}
	dump_node(&root, "$")
}
