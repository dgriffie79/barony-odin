// dynamic_array.hpp — C++ mirror of Odin's Raw_Dynamic_Array (40 bytes on x64)
// + inline helpers that call the Odin containers shims.
//
// Replaces std::vector<T> members in shared structs so C++ and Odin operate
// on the same memory layout. Growth is Odin-owned (the shims); C++ only reads
// .data/.len/.cap and calls the shims to mutate.
#pragma once
#include "dynamic_string.hpp"
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

    // string-aware array (elements are DynamicString, deep-owned)
    void    barony_dynamic_array_str_init(DynamicArray*);
    void    barony_dynamic_array_str_append(DynamicArray*, DynamicString*);
    bool    barony_dynamic_array_str_get(DynamicArray*, int32_t, DynamicString*);
    void    barony_dynamic_array_str_set(DynamicArray*, int32_t, DynamicString*);
    void    barony_dynamic_array_str_erase(DynamicArray*, int32_t);
    void    barony_dynamic_array_str_clear(DynamicArray*);
    void    barony_dynamic_array_str_destroy(DynamicArray*);
    int32_t barony_dynamic_array_str_len(DynamicArray*);
    void    barony_dynamic_array_str_copy(DynamicArray* dst, DynamicArray* src);
    int32_t barony_dynamic_array_str_entries(DynamicArray*, DynamicString* val_ptrs, int32_t count);
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

// ---------------------------------------------------------------------------
// DynamicArrayStr — std::vector<DynamicString> replacement. Elements are
// DynamicString (16B each) — the array OWNS their buffers. All mutations go
// through the str shims (deep-free on erase/clear/destroy/pop, deep-copy on
// append/set/copy).
// ---------------------------------------------------------------------------
class DynamicArrayStr {
public:
    DynamicArray raw{};

    DynamicArrayStr() { barony_dynamic_array_str_init(&raw); }
    ~DynamicArrayStr() { barony_dynamic_array_str_destroy(&raw); }
    DynamicArrayStr(const DynamicArrayStr& other) : raw{} {
        barony_dynamic_array_str_init(&raw);
        *this = other;
    }
    DynamicArrayStr& operator=(const DynamicArrayStr& other) {
        if (this != &other) { barony_dynamic_array_str_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }
        return *this;
    }
    DynamicArrayStr(DynamicArrayStr&& other) noexcept : raw(other.raw) {
        other.raw = DynamicArray{};
    }
    DynamicArrayStr& operator=(DynamicArrayStr&& other) noexcept {
        if (this != &other) {
            barony_dynamic_array_str_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicArray{};
        }
        return *this;
    }

    int64_t size() const { return barony_dynamic_array_str_len(const_cast<DynamicArray*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_array_str_clear(&raw); }
    void push_back(const DynamicString& v) { barony_dynamic_array_str_append(&raw, const_cast<DynamicString*>(&v)); }
    void push_back(const char* v) { DynamicString s(v); barony_dynamic_array_str_append(&raw, &s); }
    void push_back(const std::string& v) { DynamicString s(v.c_str()); barony_dynamic_array_str_append(&raw, &s); }
    void erase(int64_t index) { barony_dynamic_array_str_erase(&raw, (int32_t)index); }
    void pop_back() {
        if (size() > 0) { DynamicString e; barony_dynamic_array_str_get(&raw, (int32_t)(size() - 1), &e); barony_dynamic_array_str_erase(&raw, (int32_t)(size() - 1)); }
    }

    // operator[] — returns a deep-copied element (safe; mutations via set())
    DynamicString at(int64_t i) const {
        DynamicString out;
        barony_dynamic_array_str_get(const_cast<DynamicArray*>(&raw), (int32_t)i, &out);
        return out;
    }
    void set(int64_t i, const DynamicString& v) { barony_dynamic_array_str_set(&raw, (int32_t)i, const_cast<DynamicString*>(&v)); }
    void set(int64_t i, const char* v) { DynamicString s(v); barony_dynamic_array_str_set(&raw, (int32_t)i, &s); }
    void set(int64_t i, const std::string& v) { DynamicString s(v.c_str()); barony_dynamic_array_str_set(&raw, (int32_t)i, &s); }

    // snapshot: deep-copy all elements into the caller's vector
    void snapshot(std::vector<DynamicString>& out) const {
        int64_t n = size();
        out.clear();
        if (n <= 0) return;
        out.resize((size_t)n);
        barony_dynamic_array_str_entries(const_cast<DynamicArray*>(&raw), out.data(), (int32_t)n);
    }
};
