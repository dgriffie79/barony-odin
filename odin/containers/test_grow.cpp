#include <cstdio>
#include <cstdint>
struct DynamicArray { void* data; int64_t len; int64_t cap; void* a; void* b; };
extern "C" {
    void barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void*, int64_t);
    void barony_dynamic_array_destroy(DynamicArray*);
    int64_t barony_dynamic_array_sum_ints(const DynamicArray*, int32_t);
}
int main() {
    DynamicArray arr{};
    barony_dynamic_array_init(&arr);
    for (int i = 0; i < 100; ++i) {
        int v = i * 2;
        int32_t r = barony_dynamic_array_append(&arr, &v, sizeof(v));
        if (r == 0) { printf("FAIL at %d\n", i); return 1; }
    }
    printf("len=%lld cap=%lld data=%p\n", (long long)arr.len, (long long)arr.cap, arr.data);
    int64_t s = barony_dynamic_array_sum_ints(&arr, 100);
    printf("sum=%lld (expect 9900)\n", (long long)s);
    barony_dynamic_array_destroy(&arr);
    printf("DONE\n");
    return 0;
}
