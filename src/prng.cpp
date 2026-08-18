#include "prng.hpp"
#include "main.hpp"
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <string.h>

// BaronyRNG local_rng / net_rng / map_sequence_rng defs moved to Odin
// (prng.odin @(export)) - see prng.hpp extern "C" decls.

// test_rng_* console commands ported to Odin (prng.odin, init_test_rng_commands).

void BaronyRNG::testSeedHealth() const { return BaronyRNG_testSeedHealth(this); }


size_t BaronyRNG::bytesRead() const { return BaronyRNG_bytesRead(this); }


static inline void swap_byte(uint8_t& a, uint8_t& b) {
    uint8_t t = a;
    a = b;
    b = t;
}

void BaronyRNG::seedImpl(const void* key, size_t size) { BaronyRNG_seedImpl(this, key, size); }


void BaronyRNG::seedBytes(const void* key, size_t size) { BaronyRNG_seedBytes(this, key, size); }


void BaronyRNG::seedTime() { BaronyRNG_seedTime(this); }


int BaronyRNG::getSeed(void* out, size_t size) const { return BaronyRNG_getSeed(this, out, size); }


void BaronyRNG::getBytes(void* data_, size_t size) { BaronyRNG_getBytes(this, data_, size); }


uint8_t BaronyRNG::getU8() { return BaronyRNG_getU8(this); }


uint16_t BaronyRNG::getU16() { return BaronyRNG_getU16(this); }


uint32_t BaronyRNG::getU32() { return BaronyRNG_getU32(this); }


uint64_t BaronyRNG::getU64() { return BaronyRNG_getU64(this); }


int8_t BaronyRNG::getI8() { return BaronyRNG_getI8(this); }


int16_t BaronyRNG::getI16() { return BaronyRNG_getI16(this); }


int32_t BaronyRNG::getI32() { return BaronyRNG_getI32(this); }


int64_t BaronyRNG::getI64() { return BaronyRNG_getI64(this); }


float BaronyRNG::getF32() { return BaronyRNG_getF32(this); }


double BaronyRNG::getF64() { return BaronyRNG_getF64(this); }


int BaronyRNG::rand() { return BaronyRNG_rand(this); }


int BaronyRNG::uniform(int a, int b) { return BaronyRNG_uniform(this, a, b); }


int BaronyRNG::discrete(const unsigned int* chances, int size) { return BaronyRNG_discrete(this, chances, size); }


int BaronyRNG::normal(int mean, int deviation) { return BaronyRNG_normal(this, mean, deviation); }


void BaronyRNG::setMarker() const { BaronyRNG_setMarker(this); }


void BaronyRNG::checkMarker() const { BaronyRNG_checkMarker(this); }

