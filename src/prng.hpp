/*-------------------------------------------------------------------------------

	BARONY
	File: prng.hpp
	Desc: prototypes for prng.cpp, pseudo-random number generation

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include <stddef.h>
#include <stdint.h>

static uint8_t marker[256];

class BaronyRNG {
public:
    BaronyRNG() = default;
    BaronyRNG(const BaronyRNG&) = default;
    BaronyRNG(BaronyRNG&&) = default;
    ~BaronyRNG() = default;

    void seedTime();                      // seed according to a 32-bit time value
    void seedBytes(const void*, size_t);  // seed given byte buffer (uses 256 bytes at most)
    void getBytes(void*, size_t);         // fill a buffer with pseudo-random bytes

    // fill a buffer of given size with our seed value (for instance to reseed)
    // return the size of the seed or -1 if the buffer was not large enough
    int getSeed(void*, size_t) const;

    // test function, print quality of the RNG seed to log
    void testSeedHealth() const;

    // test function, set rng marker
    void setMarker() const;

    // check against marker and print to log if they are equal
    void checkMarker() const;

    // report number of bytes read since seeding
    size_t bytesRead() const;

    bool isSeeded() const { return seeded; };

    uint8_t  getU8();   // get number in range [0 - 2^8)
    uint16_t getU16();  // get number in range [0 - 2^16)
    uint32_t getU32();  // get number in range [0 - 2^32)
    uint64_t getU64();  // get number in range [0 - 2^64)
    int8_t   getI8();   // get number in range [-(2^8)/2 - (2^8)/2)
    int16_t  getI16();  // get number in range [-(2^16)/2 - (2^16)/2)
    int32_t  getI32();  // get number in range [-(2^32)/2 - (2^32)/2)
    int64_t  getI64();  // get number in range [-(2^64)/2 - (2^64)/2)
    float    getF32();  // get number in range [0.0 - 1.0)
    double   getF64();  // get number in range [0.0 - 1.0)

    // return number between 0 - RAND_MAX
    int rand();

    // uniform distribution
    // pick a number from a to b (or b to a) inclusive
    int uniform(int a, int b);

    // sampling distribution
    // pick a number using a list of weights to pick each number
    int discrete(const unsigned int* chances, int size);

    // normal distribution
    // given a mean and standard deviation, pick a number
    int normal(int mean, int deviation);

private:
    bool seeded = false; // initialized or not
    uint8_t seed[256];   // seed
    uint8_t seed_size;   // seed size
    uint8_t buf[256];    // rng buffer
    uint8_t i1;          // rng index 1
    uint8_t i2;          // rng index 2
    size_t bytes_read;   // number of bytes read since seeding


    public:
    void seedImpl(const void*, size_t);
};

extern "C" BaronyRNG local_rng; // RNG for anything that does not require client synchronization
extern "C" BaronyRNG net_rng;   // RNG which must always be synchronized among all clients
extern "C" BaronyRNG map_rng;   // used strictly during map generation
extern "C" BaronyRNG map_server_rng; // used during map generation for server only to seed local entity rng
extern "C" BaronyRNG map_sequence_rng; // used to determine the next map seed

// Flattened C-ABI entry points (Odin @(export) proc "c"). C++ method bodies
// forward to these; the Odin side owns the implementation.
extern "C" {
void BaronyRNG_seedTime(BaronyRNG* self);
void BaronyRNG_seedBytes(BaronyRNG* self, const void* key, size_t size);
void BaronyRNG_getBytes(BaronyRNG* self, void* data, size_t size);
int  BaronyRNG_getSeed(const BaronyRNG* self, void* out, size_t size);
void BaronyRNG_testSeedHealth(const BaronyRNG* self);
void BaronyRNG_setMarker(const BaronyRNG* self);
void BaronyRNG_checkMarker(const BaronyRNG* self);
size_t BaronyRNG_bytesRead(const BaronyRNG* self);
uint8_t  BaronyRNG_getU8(BaronyRNG* self);
uint16_t BaronyRNG_getU16(BaronyRNG* self);
uint32_t BaronyRNG_getU32(BaronyRNG* self);
uint64_t BaronyRNG_getU64(BaronyRNG* self);
int8_t  BaronyRNG_getI8(BaronyRNG* self);
int16_t BaronyRNG_getI16(BaronyRNG* self);
int32_t BaronyRNG_getI32(BaronyRNG* self);
int64_t BaronyRNG_getI64(BaronyRNG* self);
float  BaronyRNG_getF32(BaronyRNG* self);
double BaronyRNG_getF64(BaronyRNG* self);
int BaronyRNG_rand(BaronyRNG* self);
int BaronyRNG_uniform(BaronyRNG* self, int a, int b);
int BaronyRNG_discrete(BaronyRNG* self, const unsigned int* chances, int size);
int BaronyRNG_normal(BaronyRNG* self, int mean, int deviation);
void BaronyRNG_seedImpl(BaronyRNG* self, const void* key, size_t size);
}