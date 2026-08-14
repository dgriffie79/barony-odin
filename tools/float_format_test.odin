package main

import "core:fmt"
import "core:strconv"

main :: proc() {
	vals := []f64{
		1.0,
		1.5,
		0.0,
		-0.0,
		0.1,
		1.0 / 3.0,
		3.141592653589793,
		1e20,
		1e-7,
		1.7976931348623157e308,
		5e-324,
		0.10000000149011612, // float 0.1 promoted to double
	}
	buf: [512]byte
	for v in vals {
		f := strconv.write_float(buf[:], v, 'f', 16, 64)
		g := strconv.write_float(buf[:], v, 'g', -1, 64)
		e := strconv.write_float(buf[:], v, 'e', -1, 64)
		fmt.printf("v=%.17g\n  'f'16 = %s\n  'g'-1 = %s\n  'e'-1 = %s\n  %%v    = %v\n", v, f, g, e, v)
	}
}
