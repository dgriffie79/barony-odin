// dynamic_map.hpp — C++ RAII mirror of Odin's native hash map for string keys
// (Raw_Map, 32 bytes) with a std::map-like API. Every method is a thin inline
// marshaling to an Odin containers shim.
//
// Replaces std::map<std::string, int32_t> in shared structs (Item.attributes,
// tmpItem_t.attributes, stat_t.attributes, Entity.scriptVariables, ...).
//
// KEY OWNERSHIP: Odin's map[string]V stores keys as views; std::map deep-copies.
// The shims INTERN string keys into a shared global interner (strings.intern),
// so map keys are stable for the process lifetime and deduplicated. This gives
// std::map semantics: keys survive the source DynamicString. The interner is
// shared across ALL maps (one copy of "ATK" globally).
//
// LAYOUT: {data, len, allocator(2 ptrs)} = 32 bytes = Odin Raw_Map exactly.
// C++ and Odin operate on the same memory. The ported Odin code uses
// map[string]i32 + strings.intern with the same semantics.
//
// UNORDERED: Odin's map is a hash table — std::map's ordering is NOT preserved
// (verified: the codebase doesn't rely on it for these maps).
//
// RAII: ctor inits, dtor destroys. copy deep-copies via the entries shim.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include "dynamic_string.hpp"

class DynamicMapI32;
struct DynamicMapRaw;  // forward decl for the shim signatures

// Odin containers shims (declared before the class so methods can call them)
extern "C" {
    void      barony_dynamic_map_stri32_init(DynamicMapRaw*);
    void      barony_dynamic_map_stri32_put(DynamicMapRaw*, DynamicString, int32_t);
    bool      barony_dynamic_map_stri32_get(DynamicMapRaw*, DynamicString, int32_t*);
    bool      barony_dynamic_map_stri32_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_stri32_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_stri32_len(DynamicMapRaw*);
    void      barony_dynamic_map_stri32_destroy(DynamicMapRaw*);
    int32_t*  barony_dynamic_map_stri32_entry(DynamicMapRaw*, DynamicString);
    int32_t   barony_dynamic_map_stri32_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, int32_t* val_ptrs, int32_t count);
}

// 32 bytes on x64 — matches Odin Raw_Map {data, len, allocator}
struct DynamicMapRaw {
    void*   data;
    int64_t len;
    void*   alloc[2];
};

// map<string, int32_t> — Item.attributes, stat_t.attributes, etc.
class DynamicMapI32 {
public:
    DynamicMapRaw raw{};

    DynamicMapI32() { barony_dynamic_map_stri32_init(&raw); }
    ~DynamicMapI32() { barony_dynamic_map_stri32_destroy(&raw); }

    // copy: deep copy (std::map semantics)
    DynamicMapI32(const DynamicMapI32& other) : raw{} {
        barony_dynamic_map_stri32_init(&raw);
        copyFrom(other);
    }
    DynamicMapI32& operator=(const DynamicMapI32& other) {
        if (this != &other) { barony_dynamic_map_stri32_clear(&raw); copyFrom(other); }
        return *this;
    }
    // move
    DynamicMapI32(DynamicMapI32&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapI32& operator=(DynamicMapI32&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_stri32_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    // ---- std::map-like API ----
    // operator[]: inserts default (0) if missing, returns STABLE reference
    // (via the Odin map_entry value pointer — same slot on re-access).
    int32_t& operator[](const char* key) {
        return *barony_dynamic_map_stri32_entry(&raw, DynamicString(key));
    }
    int32_t& operator[](const DynamicString& key) {
        return *barony_dynamic_map_stri32_entry(&raw, key);
    }

    bool contains(const char* key) const {
        int32_t v;
        return barony_dynamic_map_stri32_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v);
    }
    bool contains(const DynamicString& key) const {
        int32_t v;
        return barony_dynamic_map_stri32_get(const_cast<DynamicMapRaw*>(&raw), key, &v);
    }

    int64_t size() const { return barony_dynamic_map_stri32_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_stri32_clear(&raw); }
    bool erase(const char* key) { return barony_dynamic_map_stri32_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_stri32_erase(&raw, key); }

    // ---- iteration ----
    struct Entry { const char* key; int64_t key_len; int32_t value; };

    // Snapshot all entries into a caller-provided buffer. Returns count.
    // Keys point into interned storage (valid while the map lives).
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        // stack buffers for the shim (keys/lens/vals)
        // — the shim needs arrays of size n; use a two-pass with a temp
        // (the shim writes into caller arrays; we need heap for arbitrary n)
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n), vv(n);
        int32_t got = barony_dynamic_map_stri32_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = (const char*)kp[i];
            out[i].key_len = kl[i];
            out[i].value = vv[i];
        }
        return got;
    }

private:
    void copyFrom(const DynamicMapI32& other) {
        // snapshot other, put each entry into this
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n), vv(n);
        int32_t got = barony_dynamic_map_stri32_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_stri32_put(&raw, key, vv[i]);
        }
    }
};
