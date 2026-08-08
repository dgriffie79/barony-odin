// dynamic_string.hpp — C++ RAII mirror of Odin's string ({data,len}, 16 bytes)
// with a std::string-like API. Every method is a thin inline that marshals to
// an Odin containers shim — no C++ logic lives here, only ABI glue + object
// lifecycle (when to call alloc/free).
//
// RAII: ctor calls init, dtor calls destroy, copy/move are correct. This makes
// the bulk std::string → DynamicString type-swap safe (ownership is automatic,
// no manual destroy at thousands of call sites).
//
// IMPORTANT (porting): this RAII layer is bridge scaffolding for the C++
// transition. When a C++ file ports to Odin, DynamicString → Odin `string` and
// the RAII disappears (Odin has no destructors) — Odin strings are views; the
// port restructures lifetime naturally. The Odin shims are the single
// implementation; these methods contain no logic that needs re-porting.
//
// Layout: {data, len} = 16 bytes = Odin Raw_String. Odin shims guarantee
// data[len] == 0, so c_str() is O(1) — just return data.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>

// forward decl so the extern "C" shim block can use DynamicString* params
class DynamicString;

// Odin containers shims (declared before the class so methods can call them)
extern "C" {
    void         barony_dynamic_string_init(DynamicString*);
    void         barony_dynamic_string_from_cstr(DynamicString*, const char*);
    void         barony_dynamic_string_from_bytes(DynamicString*, const void*, int);
    const char*  barony_dynamic_string_c_str(const DynamicString*);
    void         barony_dynamic_string_append(DynamicString*, const void*, int64_t);
    void         barony_dynamic_string_clear(DynamicString*);
    void         barony_dynamic_string_copy(DynamicString*, const DynamicString*);
    bool         barony_dynamic_string_equal(const DynamicString*, const DynamicString*);
    bool         barony_dynamic_string_equal_cstr(const DynamicString*, const char*);
    int32_t      barony_dynamic_string_compare(const DynamicString*, const DynamicString*);
    int64_t      barony_dynamic_string_find(const DynamicString*, const void*, int64_t, int);
    void         barony_dynamic_string_substr(DynamicString*, const DynamicString*, int, int);
    void         barony_dynamic_string_destroy(DynamicString*);
}

// 16 bytes on x64 — matches Odin Raw_String {data, len}
class DynamicString {
public:
    char*   data;
    int64_t len;

    // ---- construction / destruction (RAII) ----
    DynamicString() : data(nullptr), len(0) { barony_dynamic_string_init(this); }
    explicit DynamicString(const char* cstr) : data(nullptr), len(0) { assign(cstr); }
    DynamicString(const char* bytes, int64_t n) : data(nullptr), len(0) { assign(bytes, n); }
    ~DynamicString() { barony_dynamic_string_destroy(this); }

    // copy: deep copy (std::string semantics)
    DynamicString(const DynamicString& other) : data(nullptr), len(0) { barony_dynamic_string_copy(this, &other); }
    DynamicString& operator=(const DynamicString& other) {
        if (this != &other) barony_dynamic_string_copy(this, &other);
        return *this;
    }

    // move: transfer the buffer, null the source (no deep copy, no double-free)
    DynamicString(DynamicString&& other) noexcept : data(other.data), len(other.len) {
        other.data = nullptr;
        other.len = 0;
    }
    DynamicString& operator=(DynamicString&& other) noexcept {
        if (this != &other) {
            barony_dynamic_string_destroy(this);
            data = other.data;
            len = other.len;
            other.data = nullptr;
            other.len = 0;
        }
        return *this;
    }

    // ---- assignment / mutation ----
    DynamicString& operator=(const char* cstr) { assign(cstr); return *this; }
    DynamicString& assign(const char* cstr) { barony_dynamic_string_from_cstr(this, cstr); return *this; }
    DynamicString& assign(const char* bytes, int64_t n) { barony_dynamic_string_from_bytes(this, bytes, (int)n); return *this; }
    void from_cstr(const char* cstr) { barony_dynamic_string_from_cstr(this, cstr); }

    DynamicString& operator+=(const char* cstr) { barony_dynamic_string_append(this, cstr, (int64_t)std::strlen(cstr)); return *this; }
    DynamicString& operator+=(const DynamicString& other) { barony_dynamic_string_append(this, other.data, other.len); return *this; }
    DynamicString& append(const char* cstr) { barony_dynamic_string_append(this, cstr, (int64_t)std::strlen(cstr)); return *this; }
    DynamicString& append(const char* bytes, int64_t n) { barony_dynamic_string_append(this, bytes, (int)n); return *this; }
    DynamicString& append(const DynamicString& other) { barony_dynamic_string_append(this, other.data, other.len); return *this; }

    void clear() { barony_dynamic_string_clear(this); }
    void destroy() { barony_dynamic_string_destroy(this); }

    // ---- observers ----
    // Odin shims guarantee data[len] == 0, so this is O(1) — std::string::c_str() semantics
    const char* c_str() const { return data ? data : ""; }
    int64_t size() const { return len; }
    int64_t length() const { return len; }
    bool empty() const { return len == 0; }

    // ---- search / compare ----
    int64_t find(const char* needle) const { return barony_dynamic_string_find(this, needle, (int64_t)std::strlen(needle), 0); }
    int64_t find(const char* needle, int64_t start) const { return barony_dynamic_string_find(this, needle, (int64_t)std::strlen(needle), (int)start); }
    int64_t find(const DynamicString& needle) const { return barony_dynamic_string_find(this, needle.data, needle.len, 0); }
    int compare(const DynamicString& other) const { return barony_dynamic_string_compare(this, &other); }
    int compare(const char* cstr) const {
        DynamicString tmp(cstr);
        return barony_dynamic_string_compare(this, &tmp);
    }
    DynamicString substr(int64_t start, int64_t n = -1) const {
        DynamicString out;
        int64_t count = (n < 0) ? (len - start) : n;
        barony_dynamic_string_substr(&out, this, (int)start, (int)count);
        return out;
    }

    // ---- operators ----
    bool operator==(const DynamicString& other) const { return barony_dynamic_string_equal(this, &other); }
    bool operator!=(const DynamicString& other) const { return !barony_dynamic_string_equal(this, &other); }
    bool operator==(const char* cstr) const { return barony_dynamic_string_equal_cstr(this, cstr); }
    bool operator!=(const char* cstr) const { return !barony_dynamic_string_equal_cstr(this, cstr); }
    bool operator<(const DynamicString& other) const { return barony_dynamic_string_compare(this, &other) < 0; }
    bool operator>(const DynamicString& other) const { return barony_dynamic_string_compare(this, &other) > 0; }
    bool operator<=(const DynamicString& other) const { return barony_dynamic_string_compare(this, &other) <= 0; }
    bool operator>=(const DynamicString& other) const { return barony_dynamic_string_compare(this, &other) >= 0; }

private:
    // raw access to the Odin shims (declared above, in extern "C")
    friend bool operator==(const char* a, const DynamicString& b);
};

// symmetric ==/!= for "literal" == s
inline bool operator==(const char* a, const DynamicString& b) { return b == a; }
inline bool operator!=(const char* a, const DynamicString& b) { return b != a; }

// std::string-compatible free helpers
inline DynamicString operator+(const DynamicString& a, const char* b) { DynamicString r(a); r += b; return r; }
inline DynamicString operator+(const char* a, const DynamicString& b) { DynamicString r(a); r += b; return r; }
inline DynamicString operator+(const DynamicString& a, const DynamicString& b) { DynamicString r(a); r += b; return r; }
