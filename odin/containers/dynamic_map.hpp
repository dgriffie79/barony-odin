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
#include <string>
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
    bool      barony_dynamic_map_stri32_find(DynamicMapRaw*, DynamicString, void**, int32_t*, int32_t*);
    int32_t   barony_dynamic_map_stri32_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, int32_t* val_ptrs, int32_t count);
    void      barony_dynamic_map_strstr_init(DynamicMapRaw*);
    void      barony_dynamic_map_strstr_put(DynamicMapRaw*, DynamicString, DynamicString);
    bool      barony_dynamic_map_strstr_get(DynamicMapRaw*, DynamicString, DynamicString*);
    bool      barony_dynamic_map_strstr_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strstr_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strstr_len(DynamicMapRaw*);
    void      barony_dynamic_map_strstr_destroy(DynamicMapRaw*);
    DynamicString* barony_dynamic_map_strstr_entry(DynamicMapRaw*, DynamicString);
    int32_t   barony_dynamic_map_strstr_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void** val_ptrs, int32_t* val_lens, int32_t count);
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
    // find(): returns an iterator-like; use find(k) != end() then find->second
    // (first = key, second = value). Matches std::map::find usage.
    // CRITICAL: first points to the INTERNED key (process-lifetime stable),
    // NOT a copy — a temp iterator's first must outlive the iterator (callers
    // store find->first.c_str() pointers). Interned keys never free, so this
    // is safe.
    struct KV { const char* first; int64_t first_len; int32_t second; };
    struct Iterator {
        KV kv{};
        bool valid = false;
        const KV* operator->() const { return &kv; }
    };
    Iterator find(const char* key) const {
        Iterator it;
        void* kp = nullptr; int32_t kl = 0, vv = 0;
        if (barony_dynamic_map_stri32_find(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &kp, &kl, &vv)) {
            it.kv.first = (const char*)kp;  // interned, process-lifetime stable
            it.kv.first_len = kl;
            it.kv.second = vv;
            it.valid = true;
        }
        return it;
    }
    Iterator find(const std::string& key) const {
        return find(key.c_str());
    }
    Iterator end() const { return Iterator{}; }

    // operator[]: inserts default (0) if missing, returns STABLE reference
    // (via the Odin map_entry value pointer — same slot on re-access).
    int32_t& operator[](const char* key) {
        return *barony_dynamic_map_stri32_entry(&raw, DynamicString(key));
    }
    int32_t& operator[](const DynamicString& key) {
        return *barony_dynamic_map_stri32_entry(&raw, key);
    }
    // bridge: accept std::string keys (until callers convert)
    int32_t& operator[](const std::string& key) {
        return *barony_dynamic_map_stri32_entry(&raw, DynamicString(key.c_str()));
    }

    bool contains(const char* key) const {
        int32_t v;
        return barony_dynamic_map_stri32_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v);
    }
    bool contains(const DynamicString& key) const {
        int32_t v;
        return barony_dynamic_map_stri32_get(const_cast<DynamicMapRaw*>(&raw), key, &v);
    }
    // bridge: accept std::string keys (until callers convert)
    bool contains(const std::string& key) const {
        int32_t v;
        return barony_dynamic_map_stri32_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v);
    }

    int64_t size() const { return barony_dynamic_map_stri32_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_stri32_clear(&raw); }
    bool erase(const char* key) { return barony_dynamic_map_stri32_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_stri32_erase(&raw, key); }
    // iterator comparison for find() != end()
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.valid != b.valid; }
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.valid == b.valid; }

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

// ---------------------------------------------------------------------------
// map<string, string> — stat_t.attributes, input bindings, etc.
// BOTH keys AND values are interned (values are often temporaries like
// std::to_string results — they'd dangle without interning).
// ---------------------------------------------------------------------------
class DynamicMapStr {
public:
    DynamicMapRaw raw{};

    DynamicMapStr() { barony_dynamic_map_strstr_init(&raw); }
    ~DynamicMapStr() { barony_dynamic_map_strstr_destroy(&raw); }

    DynamicMapStr(const DynamicMapStr& other) : raw{} {
        barony_dynamic_map_strstr_init(&raw);
        copyFrom(other);
    }
    DynamicMapStr& operator=(const DynamicMapStr& other) {
        if (this != &other) { barony_dynamic_map_strstr_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapStr(DynamicMapStr&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapStr& operator=(DynamicMapStr&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strstr_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    // operator[]: returns a DynamicString reference to the interned value
    DynamicString& operator[](const char* key) {
        return *barony_dynamic_map_strstr_entry(&raw, DynamicString(key));
    }
    DynamicString& operator[](const DynamicString& key) {
        return *barony_dynamic_map_strstr_entry(&raw, key);
    }
    DynamicString& operator[](const std::string& key) {
        return *barony_dynamic_map_strstr_entry(&raw, DynamicString(key.c_str()));
    }

    // at: returns the value as a DynamicString (by value, for safety)
    DynamicString at(const char* key) const {
        DynamicString out;
        barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
        return out;
    }
    DynamicString at(const std::string& key) const {
        DynamicString out;
        barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &out);
        return out;
    }

    bool contains(const char* key) const {
        DynamicString v;
        return barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v);
    }
    bool contains(const DynamicString& key) const {
        DynamicString v;
        return barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), key, &v);
    }
    bool contains(const std::string& key) const {
        DynamicString v;
        return barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v);
    }

    int64_t size() const { return barony_dynamic_map_strstr_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strstr_clear(&raw); }
    bool erase(const char* key) { return barony_dynamic_map_strstr_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strstr_erase(&raw, key); }
    bool erase(const std::string& key) { return barony_dynamic_map_strstr_erase(&raw, DynamicString(key.c_str())); }

    struct Entry { const char* key; int64_t key_len; const char* value; int64_t value_len; };
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<void*> kp(n), vp(n);
        std::vector<int32_t> kl(n), vl(n);
        int32_t got = barony_dynamic_map_strstr_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vp.data(), vl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = (const char*)kp[i];
            out[i].key_len = kl[i];
            out[i].value = (const char*)vp[i];
            out[i].value_len = vl[i];
        }
        return got;
    }

private:
    void copyFrom(const DynamicMapStr& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n), vp(n);
        std::vector<int32_t> kl(n), vl(n);
        int32_t got = barony_dynamic_map_strstr_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vp.data(), vl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            DynamicString value((const char*)vp[i], vl[i]);
            barony_dynamic_map_strstr_put(&raw, key, value);
        }
    }
};
