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
#include <string>
#include <cstdio>
#include <cstdlib>

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
    int64_t      barony_dynamic_string_find_first_of(const DynamicString*, const char*, int);
    void         barony_dynamic_string_erase(DynamicString*, int, int);
    void         barony_dynamic_string_substr(DynamicString*, const DynamicString*, int, int);
    void         barony_dynamic_string_destroy(DynamicString*);
}

// 16 bytes on x64 — matches Odin Raw_String {data, len}
class DynamicString {
public:
    char*   data;
    int64_t len;

    // std::string::npos equivalent — find() returns this on no-match
    static const int64_t npos = -1;

    // ---- construction / destruction (RAII) ----
    DynamicString() : data(nullptr), len(0) { barony_dynamic_string_init(this); }
    // non-explicit: implicit conversion from const char* (std::string semantics)
    DynamicString(const char* cstr) : data(nullptr), len(0) { assign(cstr); }
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

    // iteration (std::string-compatible char iteration)
    char* begin() { return data ? data : nullptr; }
    char* end() { return data ? data + len : nullptr; }
    const char* begin() const { return data ? data : nullptr; }
    const char* end() const { return data ? data + len : nullptr; }

    // bridge: implicit conversion to std::string (unconverted callers assign
    // DynamicString values into std::string vars — e.g. titleText = getHover...)
    operator std::string() const { return c_str(); }

    // ---- search / compare ----
    int64_t find(const char* needle) const { return barony_dynamic_string_find(this, needle, (int64_t)std::strlen(needle), 0); }
    int64_t find(const char* needle, int64_t start) const { return barony_dynamic_string_find(this, needle, (int64_t)std::strlen(needle), (int)start); }
    int64_t find(const DynamicString& needle) const { return barony_dynamic_string_find(this, needle.data, needle.len, 0); }
    // find a single char (std::string::find(char))
    int64_t find(char c) const { return barony_dynamic_string_find(this, &c, 1, 0); }
    // find first of any char in the set (std::string::find_first_of)
    int64_t find_first_of(const char* set, int64_t start = 0) const { return barony_dynamic_string_find_first_of(this, set, (int)start); }
    // erase [pos, pos+count) in place (std::string::erase); count=-1 = to end
    DynamicString& erase(int64_t pos, int64_t count = -1) {
        if (count < 0) count = len - pos;
        barony_dynamic_string_erase(this, (int)pos, (int)count); return *this; }
    // mutable char access (std::string::operator[]/at)
    char& at(int64_t i) { return data[i]; }
    char& operator[](int64_t i) { return data[i]; }
    const char& operator[](int64_t i) const { return data[i]; }
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

// bridge: std::string += DynamicString (unconverted callers appending values)
inline std::string& operator+=(std::string& a, const DynamicString& b) { a += b.c_str(); return a; }
inline std::string operator+(const std::string& a, const DynamicString& b) { return a + b.c_str(); }

// std::to_string replacement — returns a DynamicString (no std:: allocator
// involvement; snprintf formats into an Odin-allocated buffer via from_cstr)
inline DynamicString to_string(int v) { char buf[32]; snprintf(buf, sizeof(buf), "%d", v); return DynamicString(buf); }
inline DynamicString to_string(unsigned int v) { char buf[32]; snprintf(buf, sizeof(buf), "%u", v); return DynamicString(buf); }
inline DynamicString to_string(long v) { char buf[32]; snprintf(buf, sizeof(buf), "%ld", v); return DynamicString(buf); }
inline DynamicString to_string(long long v) { char buf[32]; snprintf(buf, sizeof(buf), "%lld", v); return DynamicString(buf); }
inline DynamicString to_string(unsigned long long v) { char buf[32]; snprintf(buf, sizeof(buf), "%llu", v); return DynamicString(buf); }
inline DynamicString to_string(float v) { char buf[64]; snprintf(buf, sizeof(buf), "%f", v); return DynamicString(buf); }
inline DynamicString to_string(double v) { char buf[64]; snprintf(buf, sizeof(buf), "%f", v); return DynamicString(buf); }

// std::stoi/std::stof replacement — parse a DynamicString to a number
inline int stoi(const DynamicString& s, size_t* pos = nullptr, int base = 10) {
    const char* end = nullptr;
    long r = std::strtol(s.c_str(), const_cast<char**>(&end), base);
    if (pos) *pos = end ? (size_t)(end - s.c_str()) : s.size();
    return (int)r;
}
inline long stol(const DynamicString& s, size_t* pos = nullptr, int base = 10) {
    const char* end = nullptr;
    long r = std::strtol(s.c_str(), const_cast<char**>(&end), base);
    if (pos) *pos = end ? (size_t)(end - s.c_str()) : s.size();
    return r;
}
inline float stof(const DynamicString& s, size_t* pos = nullptr) {
    const char* end = nullptr;
    float r = std::strtof(s.c_str(), const_cast<char**>(&end));
    if (pos) *pos = end ? (size_t)(end - s.c_str()) : s.size();
    return r;
}
inline double stod(const DynamicString& s, size_t* pos = nullptr) {
    const char* end = nullptr;
    double r = std::strtod(s.c_str(), const_cast<char**>(&end));
    if (pos) *pos = end ? (size_t)(end - s.c_str()) : s.size();
    return r;
}
