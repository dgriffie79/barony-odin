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
    printf("arr at %p size=%zu\n", &arr, sizeof(arr));
    barony_dynamic_array_init(&arr);
    printf("after init: data=%p len=%lld cap=%lld\n", arr.data, (long long)arr.len, (long long)arr.cap);
    int v = 42;
    int32_t r = barony_dynamic_array_append(&arr, &v, sizeof(v));
    printf("append r=%d data=%p len=%lld cap=%lld\n", r, arr.data, (long long)arr.len, (long long)arr.cap);
    int64_t s = barony_dynamic_array_sum_ints(&arr, 1);
    printf("sum=%lld\n", (long long)s);
    barony_dynamic_array_destroy(&arr);
    printf("DONE\n");
    return 0;
}
