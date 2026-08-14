package main

import "core:fmt"
import "core:strconv"
import "core:strings"

// rapidjson-compatible number classification.
//
// rapidjson sets a *bitmask* of every integer type the parsed value fits in:
//   IsInt    = integer && -2^31 <= n <= 2^31-1
//   IsUint   = integer &&  0     <= n <= 2^32-1
//   IsInt64  = integer && -2^63 <= n <= 2^63-1
//   IsUint64 = integer &&  0     <= n <= 2^64-1
//   IsDouble = token had '.' or 'e'/'E', OR integer overflowed u64/i64
//
// Returns the flags plus the typed value.

Int32_Max  :: i128(2147483647)
Int32_Min  :: i128(-2147483648)
Uint32_Max :: u128(4294967295)
Int64_Max  :: i128(9223372036854775807)
Int64_Min  :: i128(-9223372036854775808)
Uint64_Max :: u128(18446744073709551615)

// validate strict JSON number grammar: -? (0 | [1-9][0-9]*) (\.[0-9]+)? ([eE][+-]?[0-9]+)?
is_valid_json_number :: proc(tok: string) -> bool {
	i := 0
	if i < len(tok) && tok[i] == '-' { i += 1 }
	if i >= len(tok) { return false }
	// integer part
	if tok[i] == '0' {
		i += 1
		if i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { return false } // leading zero
	} else if tok[i] >= '1' && tok[i] <= '9' {
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	} else {
		return false
	}
	// fraction
	if i < len(tok) && tok[i] == '.' {
		i += 1
		if i >= len(tok) || tok[i] < '0' || tok[i] > '9' { return false }
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	}
	// exponent
	if i < len(tok) && (tok[i] == 'e' || tok[i] == 'E') {
		i += 1
		if i < len(tok) && (tok[i] == '+' || tok[i] == '-') { i += 1 }
		if i >= len(tok) || tok[i] < '0' || tok[i] > '9' { return false }
		for i < len(tok) && tok[i] >= '0' && tok[i] <= '9' { i += 1 }
	}
	return i == len(tok)
}

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

classify_number :: proc(tok: string) -> Number_Class {
	c := Number_Class{}

	has_dot := false
	has_exp := false
	for r in tok {
		if r == '.' { has_dot = true }
		if r == 'e' || r == 'E' { has_exp = true }
	}

	if has_dot || has_exp {
		// double path
		f, ok := strconv.parse_f64(tok)
		if !ok {
			// rapidjson rejects overflow (1e400) but accepts underflow (1e-400 -> 0)
			c.valid = false
			return c
		}
		// rapidjson rejects values that overflow to +Inf (1e400 -> parse error)
		if f > 1.7976931348623157e308 {
			c.valid = false
			return c
		}
		c.valid = true
		c.is_double = true
		c.f_val = f
		return c
	}

	// integer path
	neg := len(tok) > 0 && tok[0] == '-'
	s := tok
	if neg { s = s[1:] }

	mag: u128
	overflow := false
	for r in s {
		if r < '0' || r > '9' {
			c.valid = false
			return c
		}
		d := u128(r - '0')
		// detect overflow past u128 (astronomically unlikely)
		if mag > (u128(1) << 127) {
			overflow = true
			break
		}
		mag = mag * 10 + d
	}

	if overflow {
		// exceeds u64 range by far -> double
		f, ok := strconv.parse_f64(tok)
		if !ok { c.valid = false; return c }
		c.valid = true
		c.is_double = true
		c.f_val = f
		return c
	}

	if mag == 0 {
		// "0" and "-0" both classify as non-negative zero
		c.valid = true
		c.is_int = true
		c.is_uint = true
		c.is_int64 = true
		c.is_uint64 = true
		c.i_val = 0
		return c
	}

	if neg {
		if mag <= u128(1) << 63 {
			// fits int64
			n := -i128(mag)
			c.valid = true
			c.is_int = n >= Int32_Min
			c.is_int64 = true
			c.i_val = n
			return c
		} else {
			// overflow int64 -> double
			f, ok := strconv.parse_f64(tok)
			if !ok { c.valid = false; return c }
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
			// overflow uint64 -> double
			f, ok := strconv.parse_f64(tok)
			if !ok { c.valid = false; return c }
			c.valid = true
			c.is_double = true
			c.f_val = f
			return c
		}
	}
}

// classify a whole JSON scalar token (for the harness corpus)
classify_token :: proc(tok: string) -> (flags: string, value: string, valid: bool) {
	switch tok {
	case "true":  return "0 0 0 0 0 0 1 0 0 0 0", "Bool:1", true
	case "false": return "0 0 0 0 0 0 1 0 0 0 0", "Bool:0", true
	case "null":  return "0 0 0 0 0 0 0 0 1 0 0", "Null", true
	case "[]":    return "0 0 0 0 0 0 0 0 0 1 0", "Array", true
	case "{}":    return "0 0 0 0 0 0 0 0 0 0 1", "Object", true
	}
	if len(tok) >= 2 && tok[0] == '"' && tok[len(tok)-1] == '"' {
		inner := tok[1:len(tok)-1]
		return "0 0 0 0 0 0 0 1 0 0 0", strings.concatenate({"String:", inner}), true
	}

	// number
	if !is_valid_json_number(tok) {
		return "", "", false
	}
	c := classify_number(tok)
	if !c.valid {
		return "", "", false
	}
	b: strings.Builder
	strings.builder_init(&b)
	fmt.sbprintf(&b, "%d %d %d %d %d %d 0 0 0 0 0",
		int(c.is_int), int(c.is_uint), int(c.is_int64), int(c.is_uint64), int(c.is_double),
		int(c.is_int || c.is_uint || c.is_int64 || c.is_uint64 || c.is_double))
	flags = strings.to_string(b)

	// value (tab-separated, matching the oracle)
	parts: [dynamic]string
	if c.is_int { append(&parts, fmt.tprintf("Int:%d", c.i_val)) }
	if c.is_uint { append(&parts, fmt.tprintf("Uint:%d", u64(c.i_val))) }
	if c.is_int64 { append(&parts, fmt.tprintf("Int64:%d", c.i_val)) }
	if c.is_uint64 { append(&parts, fmt.tprintf("Uint64:%d", u64(c.i_val))) }
	if c.is_double { append(&parts, fmt.tprintf("Double:%.17g", c.f_val)) }
	value = strings.join(parts[:], "\t")
	return flags, value, true
}

main :: proc() {
	tokens := []string{
		"0", "1", "123", "-1", "-123", "-0",
		"2147483647", "-2147483648", "2147483648", "4294967295", "4294967296",
		"-2147483649", "9223372036854775807", "-9223372036854775808",
		"9223372036854775808", "18446744073709551615",
		"-9223372036854775809", "18446744073709551616",
		"1.0", "1.5", "0.0", "-0.0", "0.1", "123.0", "123.5",
		"1e20", "1e-7", "1e5", "1.5e3", "1E5", "1e+5", "1e05", "1e0",
		"3.141592653589793", "1e400", "1e-400",
		"true", "false", "null", "\"str\"", "[]", "{}",
		"+1", "01", ".5", "1.", "1e", "--1", "1,2", "nan", "Infinity",
	}

	for tok in tokens {
		flags, value, valid := classify_token(tok)
		if !valid {
			fmt.printf("%s\tPARSE_ERROR\n", tok)
			continue
		}
		fmt.printf("%s\t%s\t%s\n", tok, flags, value)
	}
}
