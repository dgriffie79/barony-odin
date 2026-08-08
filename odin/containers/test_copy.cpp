#include <cstdio>
#include <cstdint>
#include <cstring>
struct DynamicArray { void* data; int64_t len; int64_t cap; void* a; void* b; };
extern "C" {
    void barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void*, int64_t);
    int32_t barony_dynamic_array_copy(DynamicArray*, const DynamicArray*);
    void barony_dynamic_array_destroy(DynamicArray*);
    int64_t barony_dynamic_array_sum_ints(const DynamicArray*, int32_t);
}
template <typename T>
void push(DynamicArray& a, const T& v) { barony_dynamic_array_append(&a, &v, sizeof(T)); }
int main() {
    DynamicArray a{}, b{};
    barony_dynamic_array_init(&a);
    barony_dynamic_array_init(&b);
    for (int i = 0; i < 50; ++i) push(a, i);
    // deep copy a -> b
    barony_dynamic_array_copy(&b, &a);
    printf("a: len=%lld data=%p  b: len=%lld data=%p\n",
        (long long)a.len, a.data, (long long)b.len, b.data);
    printf("distinct buffers: %s\n", (a.data != b.data) ? "YES" : "NO (BUG)");
    printf("b sum: %lld (expect 1225)\n", (long long)barony_dynamic_array_sum_ints(&b, 50));
    // destroy both — would double-free if shallow
    barony_dynamic_array_destroy(&a);
    barony_dynamic_array_destroy(&b);
    printf("PASS\n");
    return 0;
}
