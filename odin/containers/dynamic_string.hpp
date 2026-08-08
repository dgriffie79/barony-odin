// dynamic_string.hpp — C++ mirror of Odin's string ({data,len}, 16 bytes on x64)
// + inline helpers that call the Odin containers shims.
//
// Replaces std::string in shared structs and signatures. The Odin shims always
// keep data[len] == 0 (NUL-terminated), so c_str() is O(1) — just return data.
// Layout matches Odin's Raw_String exactly, so C++ and Odin operate on the
// same memory.
#pragma once
#include <cstdint>
#include <cstddef>

// 16 bytes on x64 — matches Odin Raw_String {data, len}
struct DynamicString {
    char*   data;
    int64_t len;

    // Odin shims guarantee data[len] == 0, so this is O(1) and correct
    // (std::string::c_str() semantics — stable while not mutated).
    const char* c_str() const { return data ? data : ""; }

    int64_t size() const { return len; }
    int64_t length() const { return len; }
    bool    empty() const { return len == 0; }
};

extern "C" {
    void         barony_dynamic_string_init(DynamicString*);
    void         barony_dynamic_string_from_cstr(DynamicString*, const char*);
    void         barony_dynamic_string_set_len(DynamicString*, int32_t);
    const char*  barony_dynamic_string_c_str(DynamicString*);
    void         barony_dynamic_string_append(DynamicString*, const void* bytes, int64_t n);
    void         barony_dynamic_string_destroy(DynamicString*);
}

// convenience inline helpers
inline void dstr_from_cstr(DynamicString& s, const char* c) { barony_dynamic_string_from_cstr(&s, c); }
inline void dstr_append(DynamicString& s, const char* c) { barony_dynamic_string_append(&s, c, (int64_t)strlen(c)); }
inline void dstr_append(DynamicString& s, const void* bytes, int64_t n) { barony_dynamic_string_append(&s, bytes, n); }
inline void dstr_clear(DynamicString& s) { s.len = 0; if (s.data) s.data[0] = 0; }
inline void dstr_destroy(DynamicString& s) { barony_dynamic_string_destroy(&s); }
