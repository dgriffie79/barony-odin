package main

import "core:fmt"
import json "core:encoding/json"

main :: proc() {
	vals := []f64{1.0, 1.5, 0.0, 0.1, 1.0 / 3.0, 1e20, 1e-7, 0.10000000149011612}
	for v in vals {
		out, err := json.unparse(json.Value(v), {pretty = false})
		fmt.printf("v=%.17g  unparse=%s  err=%v\n", v, out, err)
	}
}
