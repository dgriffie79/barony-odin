#include <cstdio>
#include <cstdint>
#include <cstddef>
struct RawDA { void* data; long long len; long long cap; void* a1; void* a2; }; // 40B
struct Lootbag { int spawn_x; int spawn_y; bool s1; bool s2; RawDA items; };
int main(){ printf("Lootbag=%zu RawDA=%zu\n", sizeof(Lootbag), sizeof(RawDA)); return 0; }
