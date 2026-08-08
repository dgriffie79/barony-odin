// dynamic_array.hpp — C++ mirror of Odin's Raw_Dynamic_Array (40 bytes on x64)
// + inline helpers that call the Odin containers shims.
//
// Replaces std::vector<T> members in shared structs so C++ and Odin operate
// on the same memory layout. Growth is Odin-owned (the shims); C++ only reads
// .data/.len/.cap and calls the shims to mutate.
#pragma once
#include <cstdint>
#include <cstddef>

// 40 bytes on x64 — matches Odin Raw_Dynamic_Array exactly
struct DynamicArray {
    void*   data;          // element bytes
    int64_t len;           // bytes (not elements!)
    int64_t cap;           // bytes
    void*   alloc_proc;    // Odin Allocator.procedure
    void*   alloc_data;    // Odin Allocator.data
};

extern "C" {
    void    barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void* elem, int64_t elem_size);
    void    barony_dynamic_array_erase(DynamicArray*, int32_t index, int64_t elem_size);
    int32_t barony_dynamic_array_insert(DynamicArray*, int32_t index, const void*, int64_t);
    void    barony_dynamic_array_clear(DynamicArray*);
    int32_t barony_dynamic_array_resize(DynamicArray*, int64_t elem_size, int32_t new_len);
    void    barony_dynamic_array_pop_back(DynamicArray*, int64_t elem_size);
    void    barony_dynamic_array_destroy(DynamicArray*);
    int32_t barony_dynamic_array_copy(DynamicArray* dst, const DynamicArray* src);
}

// typed helpers (hide void* casting)
template <typename T>
inline void dynarray_push(DynamicArray& a, const T& v) {
    barony_dynamic_array_append(&a, &v, (int64_t)sizeof(T));
}
template <typename T>
inline T* dynarray_at(DynamicArray& a, int64_t i) {
    return reinterpret_cast<T*>(a.data) + i;
}
template <typename T>
inline const T* dynarray_at(const DynamicArray& a, int64_t i) {
    return reinterpret_cast<const T*>(a.data) + i;
}
template <typename T>
inline int64_t dynarray_size(const DynamicArray& a) {
    return a.len / (int64_t)sizeof(T);
}
