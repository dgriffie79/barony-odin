#include <cstdio>
#include <cstdint>
#include <cstddef>
struct BaronyRNG {
    bool seeded;
    uint8_t seed[256];
    uint8_t seed_size;
    uint8_t buf[256];
    uint8_t i1;
    uint8_t i2;
    size_t bytes_read;
};
int main(){ printf("sizeof(BaronyRNG)=%zu\n", sizeof(BaronyRNG)); return 0; }
