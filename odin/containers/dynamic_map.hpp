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
    void      barony_dynamic_map_strf32_init(DynamicMapRaw*);
    void      barony_dynamic_map_strf32_put(DynamicMapRaw*, DynamicString, float);
    bool      barony_dynamic_map_strf32_get(DynamicMapRaw*, DynamicString, float*);
    bool      barony_dynamic_map_strf32_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strf32_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strf32_len(DynamicMapRaw*);
    void      barony_dynamic_map_strf32_destroy(DynamicMapRaw*);
    float*    barony_dynamic_map_strf32_entry(DynamicMapRaw*, DynamicString);
    int32_t   barony_dynamic_map_strf32_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, float* val_ptrs, int32_t count);

    // i32 -> string (map<int,string>): keys are [4]byte ints, values owned
    void      barony_dynamic_map_i32str_init(DynamicMapRaw*);
    void      barony_dynamic_map_i32str_put(DynamicMapRaw*, const void* key, DynamicString);
    bool      barony_dynamic_map_i32str_get(DynamicMapRaw*, const void* key, DynamicString*);
    bool      barony_dynamic_map_i32str_erase(DynamicMapRaw*, const void* key);
    void      barony_dynamic_map_i32str_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_i32str_len(DynamicMapRaw*);
    void      barony_dynamic_map_i32str_destroy(DynamicMapRaw*);
    DynamicString* barony_dynamic_map_i32str_entry(DynamicMapRaw*, const void* key);
    int32_t   barony_dynamic_map_i32str_entries(DynamicMapRaw*, int* key_ptrs, void** val_ptrs, int32_t* val_lens, int32_t count);
    bool      barony_dynamic_map_i32str_find(DynamicMapRaw*, const void* key, DynamicString*, int32_t*);

    // DynamicSet shims (std::set replacement; map[T]struct{} on the Odin side)
    void      barony_dynamic_set_i32_init(DynamicMapRaw*);
    bool      barony_dynamic_set_i32_insert(DynamicMapRaw*, int);
    bool      barony_dynamic_set_i32_contains(DynamicMapRaw*, int);
    bool      barony_dynamic_set_i32_erase(DynamicMapRaw*, int);
    void      barony_dynamic_set_i32_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_set_i32_len(DynamicMapRaw*);
    void      barony_dynamic_set_i32_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_set_i32_entries(DynamicMapRaw*, int* values, int32_t count);
    void      barony_dynamic_set_str_init(DynamicMapRaw*);
    bool      barony_dynamic_set_str_insert(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_set_str_contains(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_set_str_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_set_str_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_set_str_len(DynamicMapRaw*);
    void      barony_dynamic_set_str_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_set_str_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, int32_t count);
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

    // find() iterator (std::map-like) � first = interned key, second = value
    struct KV { const char* first; DynamicString second; };
    struct Iterator {
        KV kv{};
        bool valid = false;
        const KV* operator->() const { return &kv; }
    };
    Iterator find(const char* key) const {
        Iterator it;
        void* kp = nullptr; int32_t kl = 0; void* vp = nullptr; int32_t vl = 0;
        // strstr find shim: get stored key + value
        DynamicString k(key), v;
        if (barony_dynamic_map_strstr_get(const_cast<DynamicMapRaw*>(&raw), k, &v)) {
            // get the stored key ptr via the entries snapshot of the found key
            int32_t n = (int32_t)size();
            if (n > 0) {
                std::vector<void*> kps(n); std::vector<int32_t> kls(n); std::vector<void*> vps(n); std::vector<int32_t> vls(n);
                int32_t got = barony_dynamic_map_strstr_entries(const_cast<DynamicMapRaw*>(&raw), kps.data(), kls.data(), vps.data(), vls.data(), n);
                for (int32_t i = 0; i < got; ++i) {
                    if (kls[i] == (int32_t)std::strlen(key) && std::memcmp(kps[i], key, kls[i]) == 0) {
                        it.kv.first = (const char*)kps[i];
                        it.kv.second = DynamicString((const char*)vps[i], vls[i]);
                        it.valid = true;
                        break;
                    }
                }
            }
        }
        return it;
    }
    Iterator find(const std::string& key) const { return find(key.c_str()); }
    Iterator end() const { return Iterator{}; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.valid != b.valid; }
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.valid == b.valid; }

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

// ---------------------------------------------------------------------------
// map<string, float> — GameUI enemy-bar height/screenDistance offsets.
// Float values (no ownership). Keys interned. Same API shape as DynamicMapI32.
// ---------------------------------------------------------------------------
class DynamicMapF32 {
public:
    DynamicMapRaw raw{};

    DynamicMapF32() { barony_dynamic_map_strf32_init(&raw); }
    ~DynamicMapF32() { barony_dynamic_map_strf32_destroy(&raw); }

    DynamicMapF32(const DynamicMapF32& other) : raw{} {
        barony_dynamic_map_strf32_init(&raw);
        copyFrom(other);
    }
    DynamicMapF32& operator=(const DynamicMapF32& other) {
        if (this != &other) { barony_dynamic_map_strf32_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapF32(DynamicMapF32&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapF32& operator=(DynamicMapF32&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strf32_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    float& operator[](const char* key) {
        return *barony_dynamic_map_strf32_entry(&raw, DynamicString(key));
    }
    float& operator[](const DynamicString& key) {
        return *barony_dynamic_map_strf32_entry(&raw, key);
    }
    float& operator[](const std::string& key) {
        return *barony_dynamic_map_strf32_entry(&raw, DynamicString(key.c_str()));
    }

    bool contains(const char* key) const {
        float v;
        return barony_dynamic_map_strf32_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v);
    }
    bool contains(const DynamicString& key) const {
        float v;
        return barony_dynamic_map_strf32_get(const_cast<DynamicMapRaw*>(&raw), key, &v);
    }
    bool contains(const std::string& key) const {
        float v;
        return barony_dynamic_map_strf32_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v);
    }

    int64_t size() const { return barony_dynamic_map_strf32_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strf32_clear(&raw); }
    bool erase(const char* key) { return barony_dynamic_map_strf32_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strf32_erase(&raw, key); }
    bool erase(const std::string& key) { return barony_dynamic_map_strf32_erase(&raw, DynamicString(key.c_str())); }

    struct Entry { const char* key; int64_t key_len; float value; };
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<float> vv(n);
        int32_t got = barony_dynamic_map_strf32_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = (const char*)kp[i];
            out[i].key_len = kl[i];
            out[i].value = vv[i];
        }
        return got;
    }

private:
    void copyFrom(const DynamicMapF32& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<float> vv(n);
        int32_t got = barony_dynamic_map_strf32_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strf32_put(&raw, key, vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<int, string> — ID-to-name maps (main.hpp entries/tmpEntries, mod_tools
// itemIDToString etc). Int keys are [4]byte (no interning). Values are OWNED
// deep copies (the value slot is a DynamicString& that RAII-assigns, so it
// must free safely). Same API shape as DynamicMapStr.
// ---------------------------------------------------------------------------
class DynamicMapI32Str {
public:
    DynamicMapRaw raw{};

    DynamicMapI32Str() { barony_dynamic_map_i32str_init(&raw); }
    ~DynamicMapI32Str() { barony_dynamic_map_i32str_destroy(&raw); }

    DynamicMapI32Str(const DynamicMapI32Str& other) : raw{} {
        barony_dynamic_map_i32str_init(&raw);
        copyFrom(other);
    }
    DynamicMapI32Str& operator=(const DynamicMapI32Str& other) {
        if (this != &other) { barony_dynamic_map_i32str_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapI32Str(DynamicMapI32Str&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapI32Str& operator=(DynamicMapI32Str&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_i32str_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    // operator[]: mutable value slot (owned string)
    DynamicString& operator[](int key) {
        return *barony_dynamic_map_i32str_entry(&raw, &key);
    }

    // at: value by deep copy (safe)
    DynamicString at(int key) const {
        DynamicString out;
        barony_dynamic_map_i32str_get(const_cast<DynamicMapRaw*>(&raw), &key, &out);
        return out;
    }

    bool contains(int key) const {
        DynamicString v;
        return barony_dynamic_map_i32str_get(const_cast<DynamicMapRaw*>(&raw), &key, &v);
    }

    struct KV { int first; DynamicString second; };
    struct Iterator {
        KV kv{};
        bool valid = false;
        const KV* operator->() const { return &kv; }
    };
    Iterator find(int key) const {
        Iterator it;
        DynamicString v;
        int32_t vlen = 0;
        if (barony_dynamic_map_i32str_find(const_cast<DynamicMapRaw*>(&raw), &key, &v, &vlen)) {
            it.kv.first = key;
            // deep-copy: v may point into map-owned storage; the Iterator's
            // DynamicString must OWN its buffer (dtor frees it)
            it.kv.second = DynamicString(v.c_str());
            it.valid = true;
        }
        return it;
    }
    Iterator end() const { return Iterator{}; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.valid != b.valid; }
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.valid == b.valid; }

    int64_t size() const { return barony_dynamic_map_i32str_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_i32str_clear(&raw); }
    bool erase(int key) { return barony_dynamic_map_i32str_erase(&raw, &key); }

    struct Entry { int key; const char* value; int64_t value_len; };
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<int> kp(n);
        std::vector<void*> vp(n);
        std::vector<int32_t> vl(n);
        int32_t got = barony_dynamic_map_i32str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), vp.data(), vl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = kp[i];
            out[i].value = (const char*)vp[i];
            out[i].value_len = vl[i];
        }
        return got;
    }

private:
    void copyFrom(const DynamicMapI32Str& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<int> kp(n);
        std::vector<void*> vp(n);
        std::vector<int32_t> vl(n);
        int32_t got = barony_dynamic_map_i32str_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), vp.data(), vl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            int key = kp[i];
            DynamicString value((const char*)vp[i], vl[i]);
            barony_dynamic_map_i32str_put(&raw, &key, value);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, LightDef> — light.hpp lightDefs. LightDef is a POD (layout
// matches the Odin mirror exactly). Value copied by value; keys interned.
// NOTE: the DynamicMapLightDef class lives in src/light.hpp (next to
// LightDef); the shims are declared here.
// ---------------------------------------------------------------------------
struct DynamicMapLightDefRaw;  // forward (defined in light.hpp)

// ---------------------------------------------------------------------------
// DynamicSet — std::set replacement (Odin map[T]struct{}, Raw_Map layout).
// DynamicSetI32 = set<int>; DynamicSetStr = set<std::string> (interned keys,
// process-lifetime stable, never freed).
// ---------------------------------------------------------------------------
class DynamicSetI32 {
public:
    DynamicMapRaw raw{};

    DynamicSetI32() { barony_dynamic_set_i32_init(&raw); }
    ~DynamicSetI32() { barony_dynamic_set_i32_destroy(&raw); }
    DynamicSetI32(const DynamicSetI32& other) : raw{} {
        barony_dynamic_set_i32_init(&raw);
        copyFrom(other);
    }
    DynamicSetI32& operator=(const DynamicSetI32& other) {
        if (this != &other) { barony_dynamic_set_i32_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicSetI32(DynamicSetI32&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicSetI32& operator=(DynamicSetI32&& other) noexcept {
        if (this != &other) {
            barony_dynamic_set_i32_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    bool insert(int v) { return barony_dynamic_set_i32_insert(&raw, v); }
    bool contains(int v) const { return barony_dynamic_set_i32_contains(const_cast<DynamicMapRaw*>(&raw), v); }
    bool erase(int v) { return barony_dynamic_set_i32_erase(&raw, v); }
    int64_t size() const { return barony_dynamic_set_i32_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_set_i32_clear(&raw); }

    // iteration/copy support: snapshot values into the caller's buffer
    int32_t entries(int* out, int32_t max) const {
        return barony_dynamic_set_i32_entries(const_cast<DynamicMapRaw*>(&raw), out, max);
    }
private:
    void copyFrom(const DynamicSetI32& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<int> vals(n);
        int32_t got = barony_dynamic_set_i32_entries(const_cast<DynamicMapRaw*>(&other.raw), vals.data(), n);
        for (int32_t i = 0; i < got; ++i) barony_dynamic_set_i32_insert(&raw, vals[i]);
    }
};

class DynamicSetStr {
public:
    DynamicMapRaw raw{};

    DynamicSetStr() { barony_dynamic_set_str_init(&raw); }
    ~DynamicSetStr() { barony_dynamic_set_str_destroy(&raw); }
    DynamicSetStr(const DynamicSetStr& other) : raw{} {
        barony_dynamic_set_str_init(&raw);
        copyFrom(other);
    }
    DynamicSetStr& operator=(const DynamicSetStr& other) {
        if (this != &other) { barony_dynamic_set_str_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicSetStr(DynamicSetStr&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicSetStr& operator=(DynamicSetStr&& other) noexcept {
        if (this != &other) {
            barony_dynamic_set_str_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    bool insert(const char* v) { return barony_dynamic_set_str_insert(&raw, DynamicString(v)); }
    bool insert(const DynamicString& v) { return barony_dynamic_set_str_insert(&raw, v); }
    bool insert(const std::string& v) { return barony_dynamic_set_str_insert(&raw, DynamicString(v.c_str())); }
    bool contains(const char* v) const { return barony_dynamic_set_str_contains(const_cast<DynamicMapRaw*>(&raw), DynamicString(v)); }
    bool contains(const DynamicString& v) const { return barony_dynamic_set_str_contains(const_cast<DynamicMapRaw*>(&raw), v); }
    bool contains(const std::string& v) const { return barony_dynamic_set_str_contains(const_cast<DynamicMapRaw*>(&raw), DynamicString(v.c_str())); }
    bool erase(const char* v) { return barony_dynamic_set_str_erase(&raw, DynamicString(v)); }
    bool erase(const DynamicString& v) { return barony_dynamic_set_str_erase(&raw, v); }
    bool erase(const std::string& v) { return barony_dynamic_set_str_erase(&raw, DynamicString(v.c_str())); }
    int64_t size() const { return barony_dynamic_set_str_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_set_str_clear(&raw); }

    struct Entry { const char* key; int64_t key_len; };
    int32_t entries(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        int32_t got = barony_dynamic_set_str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = (const char*)kp[i];
            out[i].key_len = kl[i];
        }
        return got;
    }

private:
    void copyFrom(const DynamicSetStr& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        int32_t got = barony_dynamic_set_str_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_set_str_insert(&raw, key);
        }
    }
};
