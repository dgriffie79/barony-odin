// prng.odin - Odin port of prng.cpp / prng.hpp
//
// Barony's pseudo-random number generator: an RC4 (ARC4) stream cipher used
// as a deterministic byte source. Byte-exact fidelity is REQUIRED - net_rng
// must stay synchronized across all clients and map generation seeds must
// reproduce. Do not "improve" the algorithm.
//
// The C++ `BaronyRNG` methods forward to these @(export) proc "c" entry
// points (see src/prng.cpp / prng.hpp). This is the single implementation -
// no Odin wrapper layer, no shim-to-self.
package main

import "core:math"
import "core:mem"
import "core:time"
import "base:runtime"

Barony_RNG :: struct {
	seeded:     bool,
	seed:       [256]u8,
	seed_size:  u8,
	buf:        [256]u8,
	i1:         u8,
	i2:         u8,
	bytes_read: int, // C++ size_t
}

// C++: bool seeded; uint8_t seed[256]; uint8_t seed_size; uint8_t buf[256];
//      uint8_t i1,i2; size_t bytes_read;
#assert(size_of(Barony_RNG) == 528)

// The 5 RNG globals (C++: extern BaronyRNG local_rng; net_rng; map_rng;
// map_server_rng; map_sequence_rng)
local_rng:        Barony_RNG
net_rng:          Barony_RNG
map_rng:          Barony_RNG
map_server_rng:   Barony_RNG
map_sequence_rng: Barony_RNG

// debug marker (C++: static uint8_t marker[256] in the header - one per TU;
// Odin: one shared global)
marker: [256]u8

@(private)
swap_byte :: proc(a: ^u8, b: ^u8) {
	t := a^
	a^ = b^
	b^ = t
}

// ---------------------------------------------------------------------------
// @(export) proc "c" - the C++ boundary AND the implementation.
// C++ BaronyRNG methods forward to these flat names.
// ---------------------------------------------------------------------------

// C++: seedImpl(const void* key, size_t size) - the KSA (key scheduling).
// `bytes[i % size]` means a 1-byte seed repeats that byte 256 times.
@(export)
BaronyRNG_seedImpl :: proc "c" (self: ^Barony_RNG, key: rawptr, size: uint) {
	context = runtime.default_context()
	key_bytes := ([^]u8)(key)[:size]
	assert(key != nil && len(key_bytes) > 0)
	for i in 0..<256 {
		self.buf[i] = u8(i)
	}

	b := u8(0)
	for i in 0..<256 {
		b = b + self.buf[i] + key_bytes[i % len(key_bytes)]
		swap_byte(&self.buf[i], &self.buf[b])
	}

	// memcpy(seed, key, size) - only copies len(key) bytes
	for i in 0..<len(key_bytes) {
		self.seed[i] = key_bytes[i]
	}
	self.seed_size = u8(len(key_bytes)) // C++ uint8_t - truncates if > 255

	self.i1 = 0
	self.i2 = 0
	self.bytes_read = 0
	self.seeded = true
}

// C++: seedBytes(const void* key, size_t size)
@(export)
BaronyRNG_seedBytes :: proc "c" (self: ^Barony_RNG, key: rawptr, size: uint) {
	context = runtime.default_context()
	BaronyRNG_seedImpl(self, key, size)
}

// C++: seedTime() - seed with a 32-bit unix time value
@(export)
BaronyRNG_seedTime :: proc "c" (self: ^Barony_RNG) {
	context = runtime.default_context()
	t := u32(time.time_to_unix(time.now()))
	BaronyRNG_seedImpl(self, &t, size_of(t))
}

// C++: getSeed(void* out, size_t size) const - copy the seed out, return its
// size, or -1 if not seeded / buffer too small.
@(export)
BaronyRNG_getSeed :: proc "c" (self: ^Barony_RNG, out: rawptr, size: uint) -> i32 {
	context = runtime.default_context()
	out_bytes := ([^]u8)(out)[:size]
	if !self.seeded || size < uint(self.seed_size) {
		assert(false, "wtf are you doin")
		return -1
	}
	for i in 0..<int(self.seed_size) {
		out_bytes[i] = self.seed[i]
	}
	return i32(self.seed_size)
}

// C++: getBytes(void* data_, size_t size) - the PRGA. Auto-seeds by time if
// not seeded. Byte-exact.
@(export)
BaronyRNG_getBytes :: proc "c" (self: ^Barony_RNG, data: rawptr, size: uint) {
	context = runtime.default_context()
	data_bytes := ([^]u8)(data)[:size]
	if !self.seeded {
		// printlog("rng not seeded, seeding by unix time")
		BaronyRNG_seedTime(self)
	}
	for i in 0..<len(data_bytes) {
		self.i1 = u8((int(self.i1) + 1) & 255)
		self.i2 = u8((int(self.i2) + int(self.buf[self.i1])) & 255)
		swap_byte(&self.buf[self.i1], &self.buf[self.i2])
		data_bytes[i] = self.buf[(int(self.buf[self.i1]) + int(self.buf[self.i2])) & 255]
		self.bytes_read += 1
	}
}

// C++: uint8_t getU8()
@(export)
BaronyRNG_getU8 :: proc "c" (self: ^Barony_RNG) -> u8 {
	context = runtime.default_context()
	result: u8
	BaronyRNG_getBytes(self, &result, size_of(result))
	return result
}

// C++: uint16_t getU16()
@(export)
BaronyRNG_getU16 :: proc "c" (self: ^Barony_RNG) -> u16 {
	context = runtime.default_context()
	result: u16
	BaronyRNG_getBytes(self, &result, size_of(result))
	return result
}

// C++: uint32_t getU32()
@(export)
BaronyRNG_getU32 :: proc "c" (self: ^Barony_RNG) -> u32 {
	context = runtime.default_context()
	result: u32
	BaronyRNG_getBytes(self, &result, size_of(result))
	return result
}

// C++: uint64_t getU64()
@(export)
BaronyRNG_getU64 :: proc "c" (self: ^Barony_RNG) -> u64 {
	context = runtime.default_context()
	result: u64
	BaronyRNG_getBytes(self, &result, size_of(result))
	return result
}

// C++: int8_t getI8()
@(export)
BaronyRNG_getI8 :: proc "c" (self: ^Barony_RNG) -> i8 {
	context = runtime.default_context()
	return i8(BaronyRNG_getU8(self))
}

// C++: int16_t getI16()
@(export)
BaronyRNG_getI16 :: proc "c" (self: ^Barony_RNG) -> i16 {
	context = runtime.default_context()
	return i16(BaronyRNG_getU16(self))
}

// C++: int32_t getI32()
@(export)
BaronyRNG_getI32 :: proc "c" (self: ^Barony_RNG) -> i32 {
	context = runtime.default_context()
	return i32(BaronyRNG_getU32(self))
}

// C++: int64_t getI64()
@(export)
BaronyRNG_getI64 :: proc "c" (self: ^Barony_RNG) -> i64 {
	context = runtime.default_context()
	return i64(BaronyRNG_getU64(self))
}

// C++: float getF32() - NOTE: draws only 4 bytes (u32), divides by 2^32
@(export)
BaronyRNG_getF32 :: proc "c" (self: ^Barony_RNG) -> f32 {
	context = runtime.default_context()
	u := BaronyRNG_getU32(self)
	div :: u64(1) << 32
	return f32(f64(u) / f64(div))
}

// C++: double getF64() - NOTE: draws only 4 bytes (u32), divides by 2^32
@(export)
BaronyRNG_getF64 :: proc "c" (self: ^Barony_RNG) -> f64 {
	context = runtime.default_context()
	u := BaronyRNG_getU32(self)
	div :: u64(1) << 32
	return f64(u) / f64(div)
}

// C++: int rand() - draw 4 bytes, & 0x7fffffff
@(export)
BaronyRNG_rand :: proc "c" (self: ^Barony_RNG) -> i32 {
	context = runtime.default_context()
	i := BaronyRNG_getU32(self)
	return i32(i & 0x7fffffff)
}

// C++: int uniform(int a, int b) - inclusive range, a or b may be larger.
// choice = getF64() * (max-min+1), truncated to int (implicit double->int).
@(export)
BaronyRNG_uniform :: proc "c" (self: ^Barony_RNG, a: i32, b: i32) -> i32 {
	context = runtime.default_context()
	if a == b {
		return a
	}
	min_v := min(a, b)
	max_v := max(a, b)
	diff := (max_v - min_v) + 1
	choice := i32(BaronyRNG_getF64(self) * f64(diff))
	return min_v + choice
}

// C++: int discrete(const unsigned int* chances, int size) - weighted pick.
// Asserts + returns 0 on empty/zero-chance; returns the FIRST index whose
// cumulative weight exceeds the choice.
@(export)
BaronyRNG_discrete :: proc "c" (self: ^Barony_RNG, chances: ^u32, size: i32) -> i32 {
	context = runtime.default_context()
	chances_slice := ([^]u32)(chances)[:size]
	if len(chances_slice) <= 0 {
		assert(false, "BaronyRNG::discrete() list is less-or-equal than 0")
		return 0
	}
	total: u32
	for c in chances_slice {
		total += c
	}
	if total == 0 {
		assert(false, "BaronyRNG::discrete() chances of picking anything are 0")
		return 0
	}
	choice := u32(BaronyRNG_getF64(self) * f64(total))
	for c in 0..<len(chances_slice) {
		if chances_slice[c] > choice {
			return i32(c)
		}
		choice -= chances_slice[c]
	}
	assert(false, "BaronyRNG::discrete() nothing was picked. this should never happen")
	return 0
}

// C++: int normal(int mean, int deviation) - Box-Muller.
@(export)
BaronyRNG_normal :: proc "c" (self: ^Barony_RNG, mean: i32, deviation: i32) -> i32 {
	context = runtime.default_context()
	m := f64(mean)
	d := f64(deviation)
	f1 := BaronyRNG_getF64(self)
	f2 := BaronyRNG_getF64(self)
	norm := math.cos_f64(2.0 * math.PI * f2) * math.sqrt_f64(-2.0 * math.ln_f64(f1))
	return i32(math.round_f64(norm * d + m))
}

// C++: size_t bytesRead() const
@(export)
BaronyRNG_bytesRead :: proc "c" (self: ^Barony_RNG) -> uint {
	context = runtime.default_context()
	return uint(self.bytes_read)
}

// C++: bool isSeeded() const - inline accessor, NOT forwarded (no symbol).
// Odin reads self.seeded directly.

// C++: void setMarker() const (debug)
@(export)
BaronyRNG_setMarker :: proc "c" (self: ^Barony_RNG) {
	context = runtime.default_context()
	when ODIN_DEBUG {
		mem.copy(&marker[0], &self.buf[0], len(marker))
	}
}

// C++: void checkMarker() const (debug)
@(export)
BaronyRNG_checkMarker :: proc "c" (self: ^Barony_RNG) {
	context = runtime.default_context()
	when ODIN_DEBUG {
		if mem.compare(marker[:], self.buf[:]) == 0 {
			// printlog("reached marker")
		}
	}
}

// C++: void testSeedHealth() const - prints % bits set + the bit string
@(export)
BaronyRNG_testSeedHealth :: proc "c" (self: ^Barony_RNG) {
	context = runtime.default_context()
	context = runtime.default_context()
	seed_str := make([dynamic]u8)
	defer delete(seed_str)
	sum := f64(0)
	for c in 0..<256 {
		for b in 0..<8 {
			if (self.buf[c] & (1 << u8(b))) != 0 {
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
