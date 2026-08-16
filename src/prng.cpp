#include "prng.hpp"
#include "main.hpp"
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <string.h>

BaronyRNG local_rng;
BaronyRNG net_rng;
BaronyRNG map_sequence_rng;

#ifndef EDITOR
#include "interface/consolecommand.hpp"
#include "net.hpp"
static BaronyRNG test_rng;

static ConsoleCommand test_rng_seed(
    "/test_rng_seed",
    "seed test rng",
    [](int argc, const char* argv[]){
    if (argc < 2) {
        test_rng.seedTime();
    } else {
        auto seed = strtol(argv[1], nullptr, 10);
        test_rng.seedBytes(&seed, sizeof(seed));
    }
    });

static ConsoleCommand test_rng_seed_health(
    "/test_rng_seed_health",
    "test rng seed health",
    [](int argc, const char* argv[]){
    test_rng.testSeedHealth();
    });

static ConsoleCommand test_rng_u8(
    "/test_rng_u8",
    "test rng u8",
    [](int argc, const char* argv[]){
    const int i = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : 100000;
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        auto result = test_rng.getU8();
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%d", (int)result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_i8(
    "/test_rng_i8",
    "test rng i8",
    [](int argc, const char* argv[]){
    const int i = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : 100000;
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        auto result = test_rng.getI8();
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%d", (int)result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_f32(
    "/test_rng_f32",
    "test rng f32",
    [](int argc, const char* argv[]){
    const int i = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : 100000;
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        auto result = test_rng.getF32();
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%.2f", result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_f64(
    "/test_rng_f64",
    "test rng f64",
    [](int argc, const char* argv[]){
    const int i = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : 100000;
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        auto result = test_rng.getF64();
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%.2f", result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_uniform(
    "/test_rng_uniform",
    "test rng with uniform(a, b, iterations)",
    [](int argc, const char* argv[]){
    const int a = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : -10;
    const int b = argc > 2 ? (int)strtol(argv[2], nullptr, 10) : 10;
    const int i = argc > 3 ? (int)strtol(argv[3], nullptr, 10) : 100000;
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        int result = test_rng.uniform(a, b);
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%d", result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_discrete(
    "/test_rng_discrete",
    "test rng with discrete({chances}, iterations)",
    [](int argc, const char* argv[]){
    if (argc < 3) {
        messagePlayer(clientnum, MESSAGE_MISC, "args: {chances} iterations");
        return;
    }
    std::vector<unsigned int> chances;
    for (int c = 1; c < argc - 1; ++c) {
        unsigned int chance = (int)strtol(argv[c], nullptr, 10);
        chances.push_back(chance);
    }
    const int i = (int)strtol(argv[argc - 1], nullptr, 10);
    real_t sum = 0.0;
    for (int c = 0; c < i; ++c) {
        int result = test_rng.discrete(chances.data(), chances.size());
        sum += result;
        //messagePlayer(clientnum, MESSAGE_MISC, "%d", result);
    }
    sum /= i;
    messagePlayer(clientnum, MESSAGE_MISC, "mean: %.2f", sum);
    });

static ConsoleCommand test_rng_normal(
    "/test_rng_normal",
    "test rng with normal(mean, deviation, iterations)",
    [](int argc, const char* argv[]){
    const int m = argc > 1 ? (int)strtol(argv[1], nullptr, 10) : 0;
    const int d = argc > 2 ? (int)strtol(argv[2], nullptr, 10) : 5;
    const int i = argc > 3 ? (int)strtol(argv[3], nullptr, 10) : 100000;
    std::map<int, int> hist{};
    for (int c = 0; c < i; ++c) {
        int result = test_rng.normal(m, d);
        ++hist[result];
        //messagePlayer(clientnum, MESSAGE_MISC, "%d", result);
    }
    for (auto p : hist) {
        int value = p.second / 200;
        if (value) {
            std::string s(value, '*');
            messagePlayer(clientnum, MESSAGE_MISC, "%5d %s", p.first, s.c_str());
        }
    }
    });
#endif

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

