#include <cstdio>
#include <cstdint>
struct DynamicArray { void* data; int64_t len; int64_t cap; void* a; void* b; };
extern "C" {
    void barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void*, int64_t);
    void barony_dynamic_array_erase(DynamicArray*, int32_t, int64_t);
    void barony_dynamic_array_destroy(DynamicArray*);
}
template <typename T> void push(DynamicArray& a, const T& v) { barony_dynamic_array_append(&a, &v, sizeof(T)); }
void dump(const DynamicArray& a, const char* label) {
    printf("%s: len=%lld elems=%lld [", label, (long long)a.len, (long long)(a.len/4));
    int* p = (int*)a.data;
    for (long long i = 0; i < a.len/4; ++i) printf("%d ", p[i]);
    printf("]\n");
}
int main() {
    DynamicArray a{}; barony_dynamic_array_init(&a);
    for (int i = 0; i < 10; ++i) push(a, i);
    dump(a, "after append");
    barony_dynamic_array_erase(&a, 4, sizeof(int));
    dump(a, "after erase(4)");
    barony_dynamic_array_destroy(&a);
    return 0;
}
