// prng.odin — Odin port of prng.cpp / prng.hpp
//
// Barony's pseudo-random number generator: an RC4 (ARC4) stream cipher used
// as a deterministic byte source. Byte-exact fidelity is REQUIRED — net_rng
// must stay synchronized across all clients and map generation seeds must
// reproduce. Do not "improve" the algorithm.
//
// Class -> struct + procs (Odin has no classes/methods). Each proc takes
// `rng: ^Barony_RNG` as its first parameter (the "this").
package main

import "core:math"
import "core:mem"
import "core:time"

Barony_RNG :: struct {
	seeded:     bool,
	seed:       [256]u8,
	seed_size:  u8,
	buf:        [256]u8,
	i1:         u8,
	i2:         u8,
	bytes_read: int, // C++ size_t
}

// The 5 RNG globals (C++: extern BaronyRNG local_rng; net_rng; map_rng;
// map_server_rng; map_sequence_rng)
local_rng:        Barony_RNG
net_rng:          Barony_RNG
map_rng:          Barony_RNG
map_server_rng:   Barony_RNG
map_sequence_rng: Barony_RNG

// debug marker (C++: static uint8_t marker[256] in the header — one per TU;
// Odin: one shared global)
marker: [256]u8

@(private)
swap_byte :: proc(a: ^u8, b: ^u8) {
	t := a^
	a^ = b^
	b^ = t
}

// C++: seedImpl(const void* key, size_t size) — the KSA (key scheduling).
// `bytes[i % size]` means a 1-byte seed repeats that byte 256 times.
rng_seed_impl :: proc(rng: ^Barony_RNG, key: []u8) {
	assert(key != nil && len(key) > 0)
	for i in 0..<256 {
		rng.buf[i] = u8(i)
	}

	b := u8(0)
	for i in 0..<256 {
		b = b + rng.buf[i] + key[i % len(key)]
		swap_byte(&rng.buf[i], &rng.buf[b])
	}

	// memcpy(seed, key, size) — only copies len(key) bytes
	for i in 0..<len(key) {
		rng.seed[i] = key[i]
	}
	rng.seed_size = u8(len(key)) // C++ uint8_t — truncates if len(key) > 255

	rng.i1 = 0
	rng.i2 = 0
	rng.bytes_read = 0
	rng.seeded = true
}

// C++: seedBytes(const void* key, size_t size)
rng_seed_bytes :: proc(rng: ^Barony_RNG, key: []u8) {
	rng_seed_impl(rng, key)
}

// C++: seedTime() — seed with a 32-bit unix time value
rng_seed_time :: proc(rng: ^Barony_RNG) {
	t := u32(time.time_to_unix(time.now()))
	t_bytes: [4]u8 = transmute([4]u8)t
	rng_seed_impl(rng, t_bytes[:])
}

// C++: getSeed(void* out, size_t size) const — copy the seed out, return its
// size, or -1 if not seeded / buffer too small.
rng_get_seed :: proc(rng: ^Barony_RNG, out: []u8) -> int {
	if !rng.seeded || len(out) < int(rng.seed_size) {
		assert(false, "wtf are you doin")
		return -1
	}
	for i in 0..<int(rng.seed_size) {
		out[i] = rng.seed[i]
	}
	return int(rng.seed_size)
}

// C++: getBytes(void* data_, size_t size) — the PRGA. Auto-seeds by time if
// not seeded. Byte-exact.
rng_get_bytes :: proc(rng: ^Barony_RNG, data: []u8) {
	if !rng.seeded {
		// printlog("rng not seeded, seeding by unix time")
		rng_seed_time(rng)
	}
	for i in 0..<len(data) {
		rng.i1 = u8((int(rng.i1) + 1) & 255)
		rng.i2 = u8((int(rng.i2) + int(rng.buf[rng.i1])) & 255)
		swap_byte(&rng.buf[rng.i1], &rng.buf[rng.i2])
		data[i] = rng.buf[(int(rng.buf[rng.i1]) + int(rng.buf[rng.i2])) & 255]
		rng.bytes_read += 1
	}
}

// C++: uint8_t getU8()
rng_get_u8 :: proc(rng: ^Barony_RNG) -> u8 {
	result: u8
	bytes := ([^]u8)(&result)[:1]
	rng_get_bytes(rng, bytes)
	return result
}

// C++: uint16_t getU16()
rng_get_u16 :: proc(rng: ^Barony_RNG) -> u16 {
	result: u16
	bytes := ([^]u8)(&result)[:2]
	rng_get_bytes(rng, bytes)
	return result
}

// C++: uint32_t getU32()
rng_get_u32 :: proc(rng: ^Barony_RNG) -> u32 {
	result: u32
	bytes := ([^]u8)(&result)[:4]
	rng_get_bytes(rng, bytes)
	return result
}

// C++: uint64_t getU64()
rng_get_u64 :: proc(rng: ^Barony_RNG) -> u64 {
	result: u64
	bytes := ([^]u8)(&result)[:8]
	rng_get_bytes(rng, bytes)
	return result
}

// C++: int8_t getI8()
rng_get_i8 :: proc(rng: ^Barony_RNG) -> i8 {
	return i8(rng_get_u8(rng))
}

// C++: int16_t getI16()
rng_get_i16 :: proc(rng: ^Barony_RNG) -> i16 {
	return i16(rng_get_u16(rng))
}

// C++: int32_t getI32()
rng_get_i32 :: proc(rng: ^Barony_RNG) -> i32 {
	return i32(rng_get_u32(rng))
}

// C++: int64_t getI64()
rng_get_i64 :: proc(rng: ^Barony_RNG) -> i64 {
	return i64(rng_get_u64(rng))
}

// C++: float getF32() — NOTE: draws only 4 bytes (u32), divides by 2^32
rng_get_f32 :: proc(rng: ^Barony_RNG) -> f32 {
	u := rng_get_u32(rng)
	div :: u64(1) << 32
	return f32(f64(u) / f64(div))
}

// C++: double getF64() — NOTE: draws only 4 bytes (u32), divides by 2^32
rng_get_f64 :: proc(rng: ^Barony_RNG) -> f64 {
	u := rng_get_u32(rng)
	div :: u64(1) << 32
	return f64(u) / f64(div)
}

// C++: int rand() — draw 4 bytes, & 0x7fffffff
rng_rand :: proc(rng: ^Barony_RNG) -> int {
	i := rng_get_u32(rng)
	return int(i & 0x7fffffff)
}

// C++: int uniform(int a, int b) — inclusive range, a or b may be larger.
// choice = getF64() * (max-min+1), truncated to int (implicit double->int).
rng_uniform :: proc(rng: ^Barony_RNG, a, b: int) -> int {
	if a == b {
		return a
	}
	min_v := min(a, b)
	max_v := max(a, b)
	diff := (max_v - min_v) + 1
	choice := int(rng_get_f64(rng) * f64(diff))
	return min_v + choice
}

// C++: int discrete(const unsigned int* chances, int size) — weighted pick.
// Asserts + returns 0 on empty/zero-chance; returns the FIRST index whose
// cumulative weight exceeds the choice.
rng_discrete :: proc(rng: ^Barony_RNG, chances: []u32) -> int {
	if len(chances) <= 0 {
		assert(false, "BaronyRNG::discrete() list is less-or-equal than 0")
		return 0
	}
	total: u32
	for c in chances {
		total += c
	}
	if total == 0 {
		assert(false, "BaronyRNG::discrete() chances of picking anything are 0")
		return 0
	}
	choice := u32(rng_get_f64(rng) * f64(total))
	for c in 0..<len(chances) {
		if chances[c] > choice {
			return c
		}
		choice -= chances[c]
	}
	assert(false, "BaronyRNG::discrete() nothing was picked. this should never happen")
	return 0
}

// C++: int normal(int mean, int deviation) — Box-Muller.
rng_normal :: proc(rng: ^Barony_RNG, mean, deviation: int) -> int {
	m := f64(mean)
	d := f64(deviation)
	f1 := rng_get_f64(rng)
	f2 := rng_get_f64(rng)
	norm := math.cos_f64(2.0 * math.PI * f2) * math.sqrt_f64(-2.0 * math.ln_f64(f1))
	return int(math.round_f64(norm * d + m))
}

// C++: size_t bytesRead() const
rng_bytes_read :: proc(rng: ^Barony_RNG) -> int {
	return rng.bytes_read
}

// C++: bool isSeeded() const
rng_is_seeded :: proc(rng: ^Barony_RNG) -> bool {
	return rng.seeded
}

// C++: void setMarker() const (debug)
rng_set_marker :: proc(rng: ^Barony_RNG) {
	when ODIN_DEBUG {
		mem.copy(&marker[0], &rng.buf[0], len(marker))
	}
}

// C++: void checkMarker() const (debug)
rng_check_marker :: proc(rng: ^Barony_RNG) {
	when ODIN_DEBUG {
		if mem.compare(marker[:], rng.buf[:]) == 0 {
			// printlog("reached marker")
		}
	}
}

// C++: void testSeedHealth() const — prints % bits set + the bit string
rng_test_seed_health :: proc(rng: ^Barony_RNG) {
	seed_str := make([dynamic]u8)
	defer delete(seed_str)
	sum := f64(0)
	for c in 0..<256 {
		for b in 0..<8 {
			if (rng.buf[c] & (1 << u8(b))) != 0 {
				sum += 1.0
				append(&seed_str, '1')
			} else {
				append(&seed_str, '0')
			}
		}
	}
	sum /= 2048.0
	// printlog("rng seed bits are %.2f%% on", sum * 100.0)
	// printlog("seed: %s", string(seed_str[:]))
}

// The C++ prng.cpp also defines a set of ConsoleCommand test_* handlers
// (/test_rng_seed, /test_rng_seed_health, /test_rng_u8, /test_rng_i8,
// /test_rng_f32, /test_rng_f64, /test_rng_uniform, /test_rng_discrete,
// /test_rng_normal) guarded by #ifndef EDITOR. Those are debug/test tools
// that call messagePlayer(); they are not ported (the console command system
// itself is not ported yet).
