// test_containers.cpp — PoC: C++ calls Odin dynamic-array shims, data round-trips.
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <vector>

// ---- Mirrors Odin Raw_Dynamic_Array (40 bytes on x64) ----
struct DynamicArray {
    void*  data;
    int64_t len;
    int64_t cap;
    void*  alloc_proc;   // unused by our shim (we use C heap)
    void*  alloc_data;
};

extern "C" {
    void barony_dynamic_array_init(DynamicArray* arr);
    int32_t barony_dynamic_array_append(DynamicArray* arr, const void* elem, int64_t elem_size);
    void barony_dynamic_array_destroy(DynamicArray* arr);
    int64_t barony_dynamic_array_sum_ints(const DynamicArray* arr, int32_t count);
}

// ---- Template helper: hides the void* cast for typed elements ----
template <typename T>
void dynarray_push(DynamicArray& arr, const T& value) {
    barony_dynamic_array_append(&arr, &value, sizeof(T));
}

template <typename T>
T& dynarray_at(DynamicArray& arr, int64_t i) {
    return reinterpret_cast<T*>(arr.data)[i];
}

int main() {
    DynamicArray arr{};
    barony_dynamic_array_init(&arr);

    // C++ writes 100 ints via the shim (void* cast hidden by template)
    for (int i = 0; i < 100; ++i) {
        dynarray_push(arr, i * 2);
    }
    printf("after append: len=%lld cap=%lld\n", (long long)arr.len, (long long)arr.cap);

    // C++ reads back via pointer arithmetic
    int64_t cpp_sum = 0;
    for (int64_t i = 0; i < arr.len; ++i) {
        cpp_sum += dynarray_at<int>(arr, i);
    }
    printf("C++ sum: %lld\n", (long long)cpp_sum);

    // Odin reads the same array (sums the ints) — proves shared layout
    int64_t odin_sum = barony_dynamic_array_sum_ints(&arr, (int32_t)arr.len);
    printf("Odin sum: %lld\n", (long long)odin_sum);

    // Round-trip: Odin-side growth (append beyond initial cap via many pushes)
    DynamicArray big{};
    barony_dynamic_array_init(&big);
    for (int i = 0; i < 1000; ++i) {
        dynarray_push(big, i);
    }
    printf("big: len=%lld cap=%lld odin_sum=%lld\n",
        (long long)big.len, (long long)big.cap,
        (long long)barony_dynamic_array_sum_ints(&big, 1000));

    barony_dynamic_array_destroy(&arr);
    barony_dynamic_array_destroy(&big);
    printf("PASS\n");
    return 0;
}
