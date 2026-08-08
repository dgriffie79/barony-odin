#include <cstdio>
#include <cstdint>
struct DynamicArray { void* data; int64_t len; int64_t cap; void* a; void* b; };
extern "C" {
    void barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void*, int64_t);
    void barony_dynamic_array_erase(DynamicArray*, int32_t, int64_t);
    int32_t barony_dynamic_array_insert(DynamicArray*, int32_t, const void*, int64_t);
    void barony_dynamic_array_destroy(DynamicArray*);
}
template <typename T> void push(DynamicArray& a, const T& v) { barony_dynamic_array_append(&a, &v, sizeof(T)); }
void dump(const DynamicArray& a, const char* label) {
    printf("%s: [", label);
    int* p = (int*)a.data;
    for (long long i = 0; i < a.len/4; ++i) printf("%d ", p[i]);
    printf("]\n");
}
int main() {
    DynamicArray a{}; barony_dynamic_array_init(&a);
    for (int i = 0; i < 10; ++i) push(a, i);
    barony_dynamic_array_erase(&a, 4, sizeof(int)); dump(a, "after erase4");
    barony_dynamic_array_erase(&a, 0, sizeof(int)); dump(a, "after erase0");
    barony_dynamic_array_erase(&a, (int32_t)(a.len/4 - 1), sizeof(int)); dump(a, "after eraselast");
    int v99 = 99; barony_dynamic_array_insert(&a, 2, &v99, sizeof(v99)); dump(a, "after ins99@2");
    int v7 = 7;   barony_dynamic_array_insert(&a, 0, &v7, sizeof(v7));   dump(a, "after ins7@0");
    int v5 = 5;   barony_dynamic_array_insert(&a, (int32_t)(a.len/4), &v5, sizeof(v5)); dump(a, "after ins5@end");
    barony_dynamic_array_destroy(&a);
    return 0;
}
