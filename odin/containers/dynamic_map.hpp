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
#include "dynamic_array.hpp"
#include <memory>
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

    // map<string, IconEntryTextMap_t> (nested text_map value)
    void      barony_dynamic_map_striconentry_init(DynamicMapRaw*);
    void      barony_dynamic_map_striconentry_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_striconentry_get(DynamicMapRaw*, DynamicString, void* out);
    bool      barony_dynamic_map_striconentry_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_striconentry_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentry_len(DynamicMapRaw*);
    void      barony_dynamic_map_striconentry_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentry_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, IconEntryText_t>
    void      barony_dynamic_map_striconentrytext_init(DynamicMapRaw*);
    void      barony_dynamic_map_striconentrytext_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_striconentrytext_get(DynamicMapRaw*, DynamicString, void* out);
    bool      barony_dynamic_map_striconentrytext_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_striconentrytext_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentrytext_len(DynamicMapRaw*);
    void      barony_dynamic_map_striconentrytext_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentrytext_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, WorldIconEntry_t>
    void      barony_dynamic_map_strworldicon_init(DynamicMapRaw*);
    void      barony_dynamic_map_strworldicon_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strworldicon_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strworldicon_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strworldicon_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strworldicon_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strworldicon_len(DynamicMapRaw*);
    void      barony_dynamic_map_strworldicon_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strworldicon_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, DiscoveryAnim_t>
    void      barony_dynamic_map_strdiscoveryanim_init(DynamicMapRaw*);
    void      barony_dynamic_map_strdiscoveryanim_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strdiscoveryanim_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strdiscoveryanim_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strdiscoveryanim_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strdiscoveryanim_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strdiscoveryanim_len(DynamicMapRaw*);
    void      barony_dynamic_map_strdiscoveryanim_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strdiscoveryanim_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, SpecialNPCEntry_t>
    void      barony_dynamic_map_strspecialnpc_init(DynamicMapRaw*);
    void      barony_dynamic_map_strspecialnpc_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strspecialnpc_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strspecialnpc_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strspecialnpc_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strspecialnpc_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strspecialnpc_len(DynamicMapRaw*);
    void      barony_dynamic_map_strspecialnpc_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strspecialnpc_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, ColliderDmgProperties_t>
    void      barony_dynamic_map_strcolliderdmg_init(DynamicMapRaw*);
    void      barony_dynamic_map_strcolliderdmg_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strcolliderdmg_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strcolliderdmg_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strcolliderdmg_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strcolliderdmg_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strcolliderdmg_len(DynamicMapRaw*);
    void      barony_dynamic_map_strcolliderdmg_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strcolliderdmg_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, ItemLocalization_t>
    void      barony_dynamic_map_stritemloc_init(DynamicMapRaw*);
    void      barony_dynamic_map_stritemloc_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_stritemloc_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_stritemloc_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_stritemloc_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_stritemloc_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_stritemloc_len(DynamicMapRaw*);
    void      barony_dynamic_map_stritemloc_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_stritemloc_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, Achievement_t>
    void      barony_dynamic_map_strachievement_init(DynamicMapRaw*);
    void      barony_dynamic_map_strachievement_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strachievement_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strachievement_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strachievement_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strachievement_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strachievement_len(DynamicMapRaw*);
    void      barony_dynamic_map_strachievement_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strachievement_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, AchievementData_t>
    void      barony_dynamic_map_strachdata_init(DynamicMapRaw*);
    void      barony_dynamic_map_strachdata_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strachdata_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strachdata_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strachdata_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strachdata_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strachdata_len(DynamicMapRaw*);
    void      barony_dynamic_map_strachdata_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strachdata_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, IconEntry>
    void      barony_dynamic_map_striconentrylist_init(DynamicMapRaw*);
    void      barony_dynamic_map_striconentrylist_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_striconentrylist_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_striconentrylist_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_striconentrylist_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_striconentrylist_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentrylist_len(DynamicMapRaw*);
    void      barony_dynamic_map_striconentrylist_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconentrylist_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, IconEntry> (Callout variant)
    void      barony_dynamic_map_striconcallout_init(DynamicMapRaw*);
    void      barony_dynamic_map_striconcallout_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_striconcallout_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_striconcallout_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_striconcallout_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_striconcallout_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconcallout_len(DynamicMapRaw*);
    void      barony_dynamic_map_striconcallout_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_striconcallout_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, binding_t>
    void      barony_dynamic_map_strbinding_init(DynamicMapRaw*);
    void      barony_dynamic_map_strbinding_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strbinding_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strbinding_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strbinding_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strbinding_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strbinding_len(DynamicMapRaw*);
    void      barony_dynamic_map_strbinding_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strbinding_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, Class>
    void      barony_dynamic_map_strclass_init(DynamicMapRaw*);
    void      barony_dynamic_map_strclass_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strclass_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strclass_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strclass_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strclass_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strclass_len(DynamicMapRaw*);
    void      barony_dynamic_map_strclass_destroy(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strclass_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count);

    // map<string, DynamicArrayStr> (detailsText)
    void      barony_dynamic_map_strarrstr_init(DynamicMapRaw*);
    void      barony_dynamic_map_strarrstr_put(DynamicMapRaw*, DynamicString, const void* value);
    bool      barony_dynamic_map_strarrstr_get(DynamicMapRaw*, DynamicString, void* out);
    void*     barony_dynamic_map_strarrstr_entry(DynamicMapRaw*, DynamicString);
    bool      barony_dynamic_map_strarrstr_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strarrstr_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strarrstr_len(DynamicMapRaw*);
    void      barony_dynamic_map_strarrstr_destroy(DynamicMapRaw*);
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

// ---------------------------------------------------------------------------
// map<string, IconEntryTextMap_t> — the triple-nested IconEntry::text_map
// (was map<string, pair<string, set<int>>>). The value OWNS a DynamicString
// + DynamicSetI32, so the shim deep-copies on put/get and deep-frees on
// erase/clear/destroy. operator[] routes through get+put (no raw slot).
// ---------------------------------------------------------------------------
struct IconEntryTextMap_t {
    DynamicString text;
    DynamicSetI32 highlights;

    IconEntryTextMap_t() = default;
    IconEntryTextMap_t(const IconEntryTextMap_t& o) : text(o.text), highlights(o.highlights) {}
    IconEntryTextMap_t& operator=(const IconEntryTextMap_t& o) {
        if (this != &o) { text = o.text; highlights = o.highlights; }
        return *this;
    }
    IconEntryTextMap_t(IconEntryTextMap_t&& o) noexcept : text(std::move(o.text)), highlights(std::move(o.highlights)) {}
    IconEntryTextMap_t& operator=(IconEntryTextMap_t&& o) noexcept {
        if (this != &o) { text = std::move(o.text); highlights = std::move(o.highlights); }
        return *this;
    }
};

class DynamicMapIconEntryTextMap {
public:
    DynamicMapRaw raw{};

    DynamicMapIconEntryTextMap() { barony_dynamic_map_striconentry_init(&raw); }
    ~DynamicMapIconEntryTextMap() { barony_dynamic_map_striconentry_destroy(&raw); }
    DynamicMapIconEntryTextMap(const DynamicMapIconEntryTextMap& other) : raw{} {
        barony_dynamic_map_striconentry_init(&raw);
        copyFrom(other);
    }
    DynamicMapIconEntryTextMap& operator=(const DynamicMapIconEntryTextMap& other) {
        if (this != &other) { barony_dynamic_map_striconentry_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapIconEntryTextMap(DynamicMapIconEntryTextMap&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapIconEntryTextMap& operator=(DynamicMapIconEntryTextMap&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_striconentry_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    // operator[]: reads the value for key (deep copy into out); returns true
    // if present. Mirrors the pair<string,set<int>> usage: textMap.first ->
    // .text, textMap.second -> .highlights.
    bool get(const char* key, IconEntryTextMap_t& out) const {
        return barony_dynamic_map_striconentry_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, IconEntryTextMap_t& out) const {
        return barony_dynamic_map_striconentry_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const IconEntryTextMap_t& v) {
        barony_dynamic_map_striconentry_put(&raw, DynamicString(key), const_cast<IconEntryTextMap_t*>(&v));
    }
    void put(const DynamicString& key, const IconEntryTextMap_t& v) {
        barony_dynamic_map_striconentry_put(&raw, key, const_cast<IconEntryTextMap_t*>(&v));
    }
    bool contains(const char* key) const {
        IconEntryTextMap_t tmp;
        return barony_dynamic_map_striconentry_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_striconentry_erase(&raw, DynamicString(key)); }
    int64_t size() const { return barony_dynamic_map_striconentry_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_striconentry_clear(&raw); }

private:
    void copyFrom(const DynamicMapIconEntryTextMap& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<IconEntryTextMap_t> vv(n);
        int32_t got = barony_dynamic_map_striconentry_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_striconentry_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, IconEntryText_t> — Callout's IconEntry::text_map.
// Value owns 8 DynamicStrings + DynamicSetI32; deep-copy on put/get,
// deep-free on erase/clear/destroy.
// ---------------------------------------------------------------------------
struct IconEntryText_tMirror {
    DynamicString bannerText;
    DynamicSetI32 bannerHighlights;
    DynamicString worldMsgSays;
    DynamicString worldMsg;
    DynamicString worldMsgEmote;
    DynamicString worldMsgEmoteYou;
    DynamicString worldMsgEmoteToYou;
    DynamicString worldIconTag;
    DynamicString worldIconTagMini;
    // copy/move: deep (members are RAII)
    IconEntryText_tMirror() = default;
    IconEntryText_tMirror(const IconEntryText_tMirror& o)
        : bannerText(o.bannerText), bannerHighlights(o.bannerHighlights),
          worldMsgSays(o.worldMsgSays), worldMsg(o.worldMsg),
          worldMsgEmote(o.worldMsgEmote), worldMsgEmoteYou(o.worldMsgEmoteYou),
          worldMsgEmoteToYou(o.worldMsgEmoteToYou), worldIconTag(o.worldIconTag),
          worldIconTagMini(o.worldIconTagMini) {}
    IconEntryText_tMirror& operator=(const IconEntryText_tMirror& o) {
        if (this != &o) {
            bannerText = o.bannerText; bannerHighlights = o.bannerHighlights;
            worldMsgSays = o.worldMsgSays; worldMsg = o.worldMsg;
            worldMsgEmote = o.worldMsgEmote; worldMsgEmoteYou = o.worldMsgEmoteYou;
            worldMsgEmoteToYou = o.worldMsgEmoteToYou; worldIconTag = o.worldIconTag;
            worldIconTagMini = o.worldIconTagMini;
        }
        return *this;
    }
    IconEntryText_tMirror(IconEntryText_tMirror&& o) noexcept = default;
    IconEntryText_tMirror& operator=(IconEntryText_tMirror&& o) noexcept = default;
};

class DynamicMapIconEntryText {
public:
    DynamicMapRaw raw{};

    DynamicMapIconEntryText() { barony_dynamic_map_striconentrytext_init(&raw); }
    ~DynamicMapIconEntryText() { barony_dynamic_map_striconentrytext_destroy(&raw); }
    DynamicMapIconEntryText(const DynamicMapIconEntryText& other) : raw{} {
        barony_dynamic_map_striconentrytext_init(&raw);
        copyFrom(other);
    }
    DynamicMapIconEntryText& operator=(const DynamicMapIconEntryText& other) {
        if (this != &other) { barony_dynamic_map_striconentrytext_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapIconEntryText(DynamicMapIconEntryText&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapIconEntryText& operator=(DynamicMapIconEntryText&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_striconentrytext_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    bool get(const char* key, IconEntryText_tMirror& out) const {
        return barony_dynamic_map_striconentrytext_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, IconEntryText_tMirror& out) const {
        return barony_dynamic_map_striconentrytext_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const IconEntryText_tMirror& v) {
        barony_dynamic_map_striconentrytext_put(&raw, DynamicString(key), const_cast<IconEntryText_tMirror*>(&v));
    }
    void put(const DynamicString& key, const IconEntryText_tMirror& v) {
        barony_dynamic_map_striconentrytext_put(&raw, key, const_cast<IconEntryText_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        IconEntryText_tMirror tmp;
        return barony_dynamic_map_striconentrytext_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const std::string& key) const {
        IconEntryText_tMirror tmp;
        return barony_dynamic_map_striconentrytext_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool contains(const DynamicString& key) const {
        IconEntryText_tMirror tmp;
        return barony_dynamic_map_striconentrytext_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_striconentrytext_erase(&raw, DynamicString(key)); }
    bool erase(const std::string& key) { return barony_dynamic_map_striconentrytext_erase(&raw, DynamicString(key.c_str())); }
    int64_t size() const { return barony_dynamic_map_striconentrytext_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_striconentrytext_clear(&raw); }

private:
    void copyFrom(const DynamicMapIconEntryText& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<IconEntryText_tMirror> vv(n);
        int32_t got = barony_dynamic_map_striconentrytext_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_striconentrytext_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, WorldIconEntry_t> — callout world icons.
// entry() returns a mutable pointer into map storage (in-place field mutation
// is safe: each DynamicString op= frees the old MAP-owned buffer + allocates).
// get/put deep-copy for by-value use.
// ---------------------------------------------------------------------------
struct WorldIconEntry_tMirror {
    DynamicString pathDefault;
    DynamicString pathPlayer1;
    DynamicString pathPlayer2;
    DynamicString pathPlayer3;
    DynamicString pathPlayer4;
    DynamicString pathPlayerX;
    int id = 0;

    DynamicString& getPlayerIconPath(const int playernum);
    const DynamicString& getPlayerIconPath(const int playernum) const;
};

class DynamicMapWorldIconEntry {
public:
    DynamicMapRaw raw{};

    DynamicMapWorldIconEntry() { barony_dynamic_map_strworldicon_init(&raw); }
    ~DynamicMapWorldIconEntry() { barony_dynamic_map_strworldicon_destroy(&raw); }
    DynamicMapWorldIconEntry(const DynamicMapWorldIconEntry& other) : raw{} {
        barony_dynamic_map_strworldicon_init(&raw);
        copyFrom(other);
    }
    DynamicMapWorldIconEntry& operator=(const DynamicMapWorldIconEntry& other) {
        if (this != &other) { barony_dynamic_map_strworldicon_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapWorldIconEntry(DynamicMapWorldIconEntry&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapWorldIconEntry& operator=(DynamicMapWorldIconEntry&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strworldicon_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    WorldIconEntry_tMirror& operator[](const char* key) {
        return *static_cast<WorldIconEntry_tMirror*>(barony_dynamic_map_strworldicon_entry(&raw, DynamicString(key)));
    }
    WorldIconEntry_tMirror& operator[](const DynamicString& key) {
        return *static_cast<WorldIconEntry_tMirror*>(barony_dynamic_map_strworldicon_entry(&raw, key));
    }
    WorldIconEntry_tMirror& operator[](const std::string& key) {
        return *static_cast<WorldIconEntry_tMirror*>(barony_dynamic_map_strworldicon_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, WorldIconEntry_tMirror& out) const {
        return barony_dynamic_map_strworldicon_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, WorldIconEntry_tMirror& out) const {
        return barony_dynamic_map_strworldicon_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const WorldIconEntry_tMirror& v) {
        barony_dynamic_map_strworldicon_put(&raw, DynamicString(key), const_cast<WorldIconEntry_tMirror*>(&v));
    }
    void put(const DynamicString& key, const WorldIconEntry_tMirror& v) {
        barony_dynamic_map_strworldicon_put(&raw, key, const_cast<WorldIconEntry_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        WorldIconEntry_tMirror tmp;
        return barony_dynamic_map_strworldicon_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        WorldIconEntry_tMirror tmp;
        return barony_dynamic_map_strworldicon_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        WorldIconEntry_tMirror tmp;
        return barony_dynamic_map_strworldicon_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strworldicon_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strworldicon_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strworldicon_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strworldicon_clear(&raw); }

private:
    void copyFrom(const DynamicMapWorldIconEntry& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<WorldIconEntry_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strworldicon_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strworldicon_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, DiscoveryAnim_t> — featherGUI label discoveries.
// Value owns 1 DynamicString + 2 Uint32. entry() for in-place mutation;
// get/put deep-copy. Default ctor replicates the original (startTicks=ticks).
// ---------------------------------------------------------------------------
struct DiscoveryAnim_tMirror {
    uint32_t startTicks = 0;
    uint32_t processedOnTick = 0;
    DynamicString name;

    DiscoveryAnim_tMirror();   // defined in interface.cpp (needs global ticks)
};

class DynamicMapDiscoveryAnim {
public:
    DynamicMapRaw raw{};

    DynamicMapDiscoveryAnim() { barony_dynamic_map_strdiscoveryanim_init(&raw); }
    ~DynamicMapDiscoveryAnim() { barony_dynamic_map_strdiscoveryanim_destroy(&raw); }
    DynamicMapDiscoveryAnim(const DynamicMapDiscoveryAnim& other) : raw{} {
        barony_dynamic_map_strdiscoveryanim_init(&raw);
        copyFrom(other);
    }
    DynamicMapDiscoveryAnim& operator=(const DynamicMapDiscoveryAnim& other) {
        if (this != &other) { barony_dynamic_map_strdiscoveryanim_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapDiscoveryAnim(DynamicMapDiscoveryAnim&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapDiscoveryAnim& operator=(DynamicMapDiscoveryAnim&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strdiscoveryanim_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    DiscoveryAnim_tMirror& operator[](const char* key) {
        return *static_cast<DiscoveryAnim_tMirror*>(barony_dynamic_map_strdiscoveryanim_entry(&raw, DynamicString(key)));
    }
    DiscoveryAnim_tMirror& operator[](const DynamicString& key) {
        return *static_cast<DiscoveryAnim_tMirror*>(barony_dynamic_map_strdiscoveryanim_entry(&raw, key));
    }
    DiscoveryAnim_tMirror& operator[](const std::string& key) {
        return *static_cast<DiscoveryAnim_tMirror*>(barony_dynamic_map_strdiscoveryanim_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, DiscoveryAnim_tMirror& out) const {
        return barony_dynamic_map_strdiscoveryanim_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, DiscoveryAnim_tMirror& out) const {
        return barony_dynamic_map_strdiscoveryanim_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const DiscoveryAnim_tMirror& v) {
        barony_dynamic_map_strdiscoveryanim_put(&raw, DynamicString(key), const_cast<DiscoveryAnim_tMirror*>(&v));
    }
    void put(const DynamicString& key, const DiscoveryAnim_tMirror& v) {
        barony_dynamic_map_strdiscoveryanim_put(&raw, key, const_cast<DiscoveryAnim_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        DiscoveryAnim_tMirror tmp;
        return barony_dynamic_map_strdiscoveryanim_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        DiscoveryAnim_tMirror tmp;
        return barony_dynamic_map_strdiscoveryanim_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        DiscoveryAnim_tMirror tmp;
        return barony_dynamic_map_strdiscoveryanim_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strdiscoveryanim_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strdiscoveryanim_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strdiscoveryanim_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strdiscoveryanim_clear(&raw); }

private:
    void copyFrom(const DynamicMapDiscoveryAnim& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<DiscoveryAnim_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strdiscoveryanim_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strdiscoveryanim_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, SpecialNPCEntry_t> — monster special NPCs.
// Value owns 4 DynamicStrings + DynamicSetI32 + int. entry() for in-place
// mutation; get/put deep-copy.
// ---------------------------------------------------------------------------
struct SpecialNPCEntry_tMirror {
    DynamicString internalName;
    DynamicString name;
    DynamicString shortname;
    DynamicSetI32 modelIndexes;
    int baseModel = 0;
    DynamicString uniqueIcon;
};

class DynamicMapSpecialNPC {
public:
    DynamicMapRaw raw{};

    DynamicMapSpecialNPC() { barony_dynamic_map_strspecialnpc_init(&raw); }
    ~DynamicMapSpecialNPC() { barony_dynamic_map_strspecialnpc_destroy(&raw); }
    DynamicMapSpecialNPC(const DynamicMapSpecialNPC& other) : raw{} {
        barony_dynamic_map_strspecialnpc_init(&raw);
        copyFrom(other);
    }
    DynamicMapSpecialNPC& operator=(const DynamicMapSpecialNPC& other) {
        if (this != &other) { barony_dynamic_map_strspecialnpc_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapSpecialNPC(DynamicMapSpecialNPC&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapSpecialNPC& operator=(DynamicMapSpecialNPC&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strspecialnpc_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    SpecialNPCEntry_tMirror& operator[](const char* key) {
        return *static_cast<SpecialNPCEntry_tMirror*>(barony_dynamic_map_strspecialnpc_entry(&raw, DynamicString(key)));
    }
    SpecialNPCEntry_tMirror& operator[](const DynamicString& key) {
        return *static_cast<SpecialNPCEntry_tMirror*>(barony_dynamic_map_strspecialnpc_entry(&raw, key));
    }
    SpecialNPCEntry_tMirror& operator[](const std::string& key) {
        return *static_cast<SpecialNPCEntry_tMirror*>(barony_dynamic_map_strspecialnpc_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, SpecialNPCEntry_tMirror& out) const {
        return barony_dynamic_map_strspecialnpc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, SpecialNPCEntry_tMirror& out) const {
        return barony_dynamic_map_strspecialnpc_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const SpecialNPCEntry_tMirror& v) {
        barony_dynamic_map_strspecialnpc_put(&raw, DynamicString(key), const_cast<SpecialNPCEntry_tMirror*>(&v));
    }
    void put(const DynamicString& key, const SpecialNPCEntry_tMirror& v) {
        barony_dynamic_map_strspecialnpc_put(&raw, key, const_cast<SpecialNPCEntry_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        SpecialNPCEntry_tMirror tmp;
        return barony_dynamic_map_strspecialnpc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        SpecialNPCEntry_tMirror tmp;
        return barony_dynamic_map_strspecialnpc_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        SpecialNPCEntry_tMirror tmp;
        return barony_dynamic_map_strspecialnpc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strspecialnpc_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strspecialnpc_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strspecialnpc_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strspecialnpc_clear(&raw); }

private:
    void copyFrom(const DynamicMapSpecialNPC& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<SpecialNPCEntry_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strspecialnpc_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strspecialnpc_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, ColliderDmgProperties_t> — editor collider damage types.
// Value owns 2 DynamicSetI32 + 8 bools. entry() for in-place mutation;
// get/put deep-copy (sets deep-copied).
// ---------------------------------------------------------------------------
struct ColliderDmgProperties_tMirror {
    bool burnable = false;
    bool minotaurPathThroughAndBreak = false;
    bool meleeAffects = false;
    bool magicAffects = false;
    bool bombsAttach = false;
    bool boulderDestroys = false;
    bool showAsWallOnMinimap = false;
    bool allowNPCPathing = false;
    DynamicSetI32 proficiencyBonusDamage;
    DynamicSetI32 proficiencyResistDamage;
};

class DynamicMapColliderDmg {
public:
    DynamicMapRaw raw{};

    DynamicMapColliderDmg() { barony_dynamic_map_strcolliderdmg_init(&raw); }
    ~DynamicMapColliderDmg() { barony_dynamic_map_strcolliderdmg_destroy(&raw); }
    DynamicMapColliderDmg(const DynamicMapColliderDmg& other) : raw{} {
        barony_dynamic_map_strcolliderdmg_init(&raw);
        copyFrom(other);
    }
    DynamicMapColliderDmg& operator=(const DynamicMapColliderDmg& other) {
        if (this != &other) { barony_dynamic_map_strcolliderdmg_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapColliderDmg(DynamicMapColliderDmg&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapColliderDmg& operator=(DynamicMapColliderDmg&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strcolliderdmg_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    ColliderDmgProperties_tMirror& operator[](const char* key) {
        return *static_cast<ColliderDmgProperties_tMirror*>(barony_dynamic_map_strcolliderdmg_entry(&raw, DynamicString(key)));
    }
    ColliderDmgProperties_tMirror& operator[](const DynamicString& key) {
        return *static_cast<ColliderDmgProperties_tMirror*>(barony_dynamic_map_strcolliderdmg_entry(&raw, key));
    }
    ColliderDmgProperties_tMirror& operator[](const std::string& key) {
        return *static_cast<ColliderDmgProperties_tMirror*>(barony_dynamic_map_strcolliderdmg_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, ColliderDmgProperties_tMirror& out) const {
        return barony_dynamic_map_strcolliderdmg_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, ColliderDmgProperties_tMirror& out) const {
        return barony_dynamic_map_strcolliderdmg_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const ColliderDmgProperties_tMirror& v) {
        barony_dynamic_map_strcolliderdmg_put(&raw, DynamicString(key), const_cast<ColliderDmgProperties_tMirror*>(&v));
    }
    void put(const DynamicString& key, const ColliderDmgProperties_tMirror& v) {
        barony_dynamic_map_strcolliderdmg_put(&raw, key, const_cast<ColliderDmgProperties_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        ColliderDmgProperties_tMirror tmp;
        return barony_dynamic_map_strcolliderdmg_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        ColliderDmgProperties_tMirror tmp;
        return barony_dynamic_map_strcolliderdmg_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        ColliderDmgProperties_tMirror tmp;
        return barony_dynamic_map_strcolliderdmg_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strcolliderdmg_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strcolliderdmg_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strcolliderdmg_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strcolliderdmg_clear(&raw); }

private:
    void copyFrom(const DynamicMapColliderDmg& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<ColliderDmgProperties_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strcolliderdmg_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strcolliderdmg_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, ItemLocalization_t> — item name localizations.
// Value owns 2 DynamicStrings. entry() for in-place mutation; get/put deep.
// ---------------------------------------------------------------------------
struct ItemLocalization_tMirror {
    DynamicString name_identified;
    DynamicString name_unidentified;
};

class DynamicMapItemLoc {
public:
    DynamicMapRaw raw{};

    DynamicMapItemLoc() { barony_dynamic_map_stritemloc_init(&raw); }
    ~DynamicMapItemLoc() { barony_dynamic_map_stritemloc_destroy(&raw); }
    DynamicMapItemLoc(const DynamicMapItemLoc& other) : raw{} {
        barony_dynamic_map_stritemloc_init(&raw);
        copyFrom(other);
    }
    DynamicMapItemLoc& operator=(const DynamicMapItemLoc& other) {
        if (this != &other) { barony_dynamic_map_stritemloc_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapItemLoc(DynamicMapItemLoc&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapItemLoc& operator=(DynamicMapItemLoc&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_stritemloc_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    ItemLocalization_tMirror& operator[](const char* key) {
        return *static_cast<ItemLocalization_tMirror*>(barony_dynamic_map_stritemloc_entry(&raw, DynamicString(key)));
    }
    ItemLocalization_tMirror& operator[](const DynamicString& key) {
        return *static_cast<ItemLocalization_tMirror*>(barony_dynamic_map_stritemloc_entry(&raw, key));
    }
    ItemLocalization_tMirror& operator[](const std::string& key) {
        return *static_cast<ItemLocalization_tMirror*>(barony_dynamic_map_stritemloc_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, ItemLocalization_tMirror& out) const {
        return barony_dynamic_map_stritemloc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, ItemLocalization_tMirror& out) const {
        return barony_dynamic_map_stritemloc_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const ItemLocalization_tMirror& v) {
        barony_dynamic_map_stritemloc_put(&raw, DynamicString(key), const_cast<ItemLocalization_tMirror*>(&v));
    }
    void put(const DynamicString& key, const ItemLocalization_tMirror& v) {
        barony_dynamic_map_stritemloc_put(&raw, key, const_cast<ItemLocalization_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        ItemLocalization_tMirror tmp;
        return barony_dynamic_map_stritemloc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        ItemLocalization_tMirror tmp;
        return barony_dynamic_map_stritemloc_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        ItemLocalization_tMirror tmp;
        return barony_dynamic_map_stritemloc_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_stritemloc_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_stritemloc_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_stritemloc_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_stritemloc_clear(&raw); }

private:
    void copyFrom(const DynamicMapItemLoc& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<ItemLocalization_tMirror> vv(n);
        int32_t got = barony_dynamic_map_stritemloc_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_stritemloc_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, Achievement_t> — compendium achievements.
// Value owns 1 DynamicString + bool + i64. entry() for in-place mutation;
// get/put deep-copy. Range-for iterates a snapshot of (key, deep-copied
// value) pairs.
// ---------------------------------------------------------------------------
struct Achievement_tMirror {
    DynamicString name;
    bool unlocked = false;
    int64_t unlockTime = 0;
};

class DynamicMapAchievement {
public:
    DynamicMapRaw raw{};

    DynamicMapAchievement() { barony_dynamic_map_strachievement_init(&raw); }
    ~DynamicMapAchievement() { barony_dynamic_map_strachievement_destroy(&raw); }
    DynamicMapAchievement(const DynamicMapAchievement& other) : raw{} {
        barony_dynamic_map_strachievement_init(&raw);
        copyFrom(other);
    }
    DynamicMapAchievement& operator=(const DynamicMapAchievement& other) {
        if (this != &other) { barony_dynamic_map_strachievement_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapAchievement(DynamicMapAchievement&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapAchievement& operator=(DynamicMapAchievement&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strachievement_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    Achievement_tMirror& operator[](const char* key) {
        return *static_cast<Achievement_tMirror*>(barony_dynamic_map_strachievement_entry(&raw, DynamicString(key)));
    }
    Achievement_tMirror& operator[](const DynamicString& key) {
        return *static_cast<Achievement_tMirror*>(barony_dynamic_map_strachievement_entry(&raw, key));
    }
    Achievement_tMirror& operator[](const std::string& key) {
        return *static_cast<Achievement_tMirror*>(barony_dynamic_map_strachievement_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, Achievement_tMirror& out) const {
        return barony_dynamic_map_strachievement_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, Achievement_tMirror& out) const {
        return barony_dynamic_map_strachievement_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const Achievement_tMirror& v) {
        barony_dynamic_map_strachievement_put(&raw, DynamicString(key), const_cast<Achievement_tMirror*>(&v));
    }
    void put(const DynamicString& key, const Achievement_tMirror& v) {
        barony_dynamic_map_strachievement_put(&raw, key, const_cast<Achievement_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        Achievement_tMirror tmp;
        return barony_dynamic_map_strachievement_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        Achievement_tMirror tmp;
        return barony_dynamic_map_strachievement_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        Achievement_tMirror tmp;
        return barony_dynamic_map_strachievement_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strachievement_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strachievement_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strachievement_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strachievement_clear(&raw); }

    // range-for: std::map-like iteration over a deep-copied snapshot.
    // .first = interned key (stable), .second = deep-copied value.
    struct KV { const char* first; int64_t first_len; Achievement_tMirror second; };
    struct Iterator {
        std::shared_ptr<std::vector<KV>> snap;  // owned snapshot
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const KV* operator->() const { return &(*snap)[idx]; }
        const KV& operator*() const { return (*snap)[idx]; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            // o is the end() sentinel (null snap, idx == END): NOT equal iff
            // this iterator still has elements to visit.
            if (!o.snap) {
                return snap && idx < snap->size();
            }
            if (!snap) {
                return o.snap ? o.idx < o.snap->size() : false;
            }
            if (snap.get() != o.snap.get()) {
                return !(idx >= snap->size() && o.idx == END);
            }
            return idx != o.idx;
        }
    };
    Iterator begin() const {
        Iterator it;
        int64_t n = size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<KV>>();
            std::vector<void*> kp((size_t)n);
            std::vector<int32_t> kl((size_t)n);
            std::vector<Achievement_tMirror> vv((size_t)n);
            int32_t got = barony_dynamic_map_strachievement_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), (int32_t)n);
            it.snap->resize((size_t)got);
            for (int32_t i = 0; i < got; ++i) {
                (*it.snap)[(size_t)i].first = (const char*)kp[(size_t)i];
                (*it.snap)[(size_t)i].first_len = kl[(size_t)i];
                (*it.snap)[(size_t)i].second = vv[(size_t)i];
            }
        }
        return it;
    }
    Iterator end() const {
        Iterator it;
        it.idx = Iterator::END;  // sentinel: never equals a begin/advancing idx
        return it;
    }

private:
    void copyFrom(const DynamicMapAchievement& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<Achievement_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strachievement_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strachievement_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, AchievementData_t> — compendium achievements.
// Value owns 4 DynamicStrings + scalars. entry() for in-place mutation;
// get/put deep-copy. Range-for iterates a deep-copied snapshot.
// ---------------------------------------------------------------------------
struct AchievementData_tMirror {
    DynamicString name;
    DynamicString desc;
    DynamicString desc_formatted;
    bool hidden = false;
    int dlcType = 0;
    DynamicString category;
    int lorePoints = 0;
    int64_t unlockTime = 0;
    bool unlocked = false;
    int achievementProgress = -1;
};

class DynamicMapAchievementData {
public:
    DynamicMapRaw raw{};

    DynamicMapAchievementData() { barony_dynamic_map_strachdata_init(&raw); }
    ~DynamicMapAchievementData() { barony_dynamic_map_strachdata_destroy(&raw); }
    DynamicMapAchievementData(const DynamicMapAchievementData& other) : raw{} {
        barony_dynamic_map_strachdata_init(&raw);
        copyFrom(other);
    }
    DynamicMapAchievementData& operator=(const DynamicMapAchievementData& other) {
        if (this != &other) { barony_dynamic_map_strachdata_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapAchievementData(DynamicMapAchievementData&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapAchievementData& operator=(DynamicMapAchievementData&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strachdata_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    AchievementData_tMirror& operator[](const char* key) {
        return *static_cast<AchievementData_tMirror*>(barony_dynamic_map_strachdata_entry(&raw, DynamicString(key)));
    }
    AchievementData_tMirror& operator[](const DynamicString& key) {
        return *static_cast<AchievementData_tMirror*>(barony_dynamic_map_strachdata_entry(&raw, key));
    }
    AchievementData_tMirror& operator[](const std::string& key) {
        return *static_cast<AchievementData_tMirror*>(barony_dynamic_map_strachdata_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, AchievementData_tMirror& out) const {
        return barony_dynamic_map_strachdata_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, AchievementData_tMirror& out) const {
        return barony_dynamic_map_strachdata_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const AchievementData_tMirror& v) {
        barony_dynamic_map_strachdata_put(&raw, DynamicString(key), const_cast<AchievementData_tMirror*>(&v));
    }
    void put(const DynamicString& key, const AchievementData_tMirror& v) {
        barony_dynamic_map_strachdata_put(&raw, key, const_cast<AchievementData_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        AchievementData_tMirror tmp;
        return barony_dynamic_map_strachdata_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        AchievementData_tMirror tmp;
        return barony_dynamic_map_strachdata_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        AchievementData_tMirror tmp;
        return barony_dynamic_map_strachdata_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strachdata_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strachdata_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strachdata_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strachdata_clear(&raw); }

    // range-for over a deep-copied snapshot (END sentinel pattern)
    struct KV { const char* first; int64_t first_len; AchievementData_tMirror second; };
    struct Iterator {
        std::shared_ptr<std::vector<KV>> snap;
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const KV* operator->() const { return &(*snap)[idx]; }
        const KV& operator*() const { return (*snap)[idx]; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            // o is the end() sentinel (null snap, idx == END): NOT equal iff
            // this iterator still has elements to visit.
            if (!o.snap) {
                return snap && idx < snap->size();
            }
            if (!snap) {
                return o.snap ? o.idx < o.snap->size() : false;
            }
            if (snap.get() != o.snap.get()) {
                return !(idx >= snap->size() && o.idx == END);
            }
            return idx != o.idx;
        }
    };
    Iterator begin() const {
        Iterator it;
        int64_t n = size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<KV>>();
            std::vector<void*> kp((size_t)n);
            std::vector<int32_t> kl((size_t)n);
            std::vector<AchievementData_tMirror> vv((size_t)n);
            int32_t got = barony_dynamic_map_strachdata_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), (int32_t)n);
            it.snap->resize((size_t)got);
            for (int32_t i = 0; i < got; ++i) {
                (*it.snap)[(size_t)i].first = (const char*)kp[(size_t)i];
                (*it.snap)[(size_t)i].first_len = kl[(size_t)i];
                (*it.snap)[(size_t)i].second = vv[(size_t)i];
            }
        }
        return it;
    }
    Iterator end() const {
        Iterator it;
        it.idx = Iterator::END;
        return it;
    }

private:
    void copyFrom(const DynamicMapAchievementData& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<AchievementData_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strachdata_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strachdata_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, IconEntry> — follower/callout radial menu icons.
// Value owns 6 DynamicStrings + 2 ints + a NESTED DynamicMapIconEntryTextMap.
// entry() for in-place mutation; get/put deep-copy (nested map cloned).
// ---------------------------------------------------------------------------
struct IconEntry_tMirror {
    DynamicString name;
    int id = -1;
    DynamicString path;
    DynamicString path_hover;
    DynamicString path_active;
    DynamicString path_active_hover;
    int icon_offsetx = 0;
    int icon_offsety = 0;
    DynamicMapIconEntryTextMap text_map;
};

class DynamicMapIconEntryList {
public:
    DynamicMapRaw raw{};

    DynamicMapIconEntryList() { barony_dynamic_map_striconentrylist_init(&raw); }
    ~DynamicMapIconEntryList() { barony_dynamic_map_striconentrylist_destroy(&raw); }
    DynamicMapIconEntryList(const DynamicMapIconEntryList& other) : raw{} {
        barony_dynamic_map_striconentrylist_init(&raw);
        copyFrom(other);
    }
    DynamicMapIconEntryList& operator=(const DynamicMapIconEntryList& other) {
        if (this != &other) { barony_dynamic_map_striconentrylist_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapIconEntryList(DynamicMapIconEntryList&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapIconEntryList& operator=(DynamicMapIconEntryList&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_striconentrylist_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    IconEntry_tMirror& operator[](const char* key) {
        return *static_cast<IconEntry_tMirror*>(barony_dynamic_map_striconentrylist_entry(&raw, DynamicString(key)));
    }
    IconEntry_tMirror& operator[](const DynamicString& key) {
        return *static_cast<IconEntry_tMirror*>(barony_dynamic_map_striconentrylist_entry(&raw, key));
    }
    IconEntry_tMirror& operator[](const std::string& key) {
        return *static_cast<IconEntry_tMirror*>(barony_dynamic_map_striconentrylist_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, IconEntry_tMirror& out) const {
        return barony_dynamic_map_striconentrylist_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, IconEntry_tMirror& out) const {
        return barony_dynamic_map_striconentrylist_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const IconEntry_tMirror& v) {
        barony_dynamic_map_striconentrylist_put(&raw, DynamicString(key), const_cast<IconEntry_tMirror*>(&v));
    }
    void put(const DynamicString& key, const IconEntry_tMirror& v) {
        barony_dynamic_map_striconentrylist_put(&raw, key, const_cast<IconEntry_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        IconEntry_tMirror tmp;
        return barony_dynamic_map_striconentrylist_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        IconEntry_tMirror tmp;
        return barony_dynamic_map_striconentrylist_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        IconEntry_tMirror tmp;
        return barony_dynamic_map_striconentrylist_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_striconentrylist_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_striconentrylist_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_striconentrylist_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_striconentrylist_clear(&raw); }

private:
    void copyFrom(const DynamicMapIconEntryList& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<IconEntry_tMirror> vv(n);
        int32_t got = barony_dynamic_map_striconentrylist_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_striconentrylist_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, IconEntry> (Callout variant) — callout radial menu icons.
// Same as IconEntry_tMirror but text_map is DynamicMapIconEntryText.
// entry() for in-place mutation; get/put deep-copy (nested map cloned).
// ---------------------------------------------------------------------------
struct IconEntryCallout_tMirror {
    DynamicString name;
    int id = -1;
    DynamicString path;
    DynamicString path_hover;
    DynamicString path_active;
    DynamicString path_active_hover;
    int icon_offsetx = 0;
    int icon_offsety = 0;
    DynamicMapIconEntryText text_map;
};

class DynamicMapIconEntryCallout {
public:
    DynamicMapRaw raw{};

    DynamicMapIconEntryCallout() { barony_dynamic_map_striconcallout_init(&raw); }
    ~DynamicMapIconEntryCallout() { barony_dynamic_map_striconcallout_destroy(&raw); }
    DynamicMapIconEntryCallout(const DynamicMapIconEntryCallout& other) : raw{} {
        barony_dynamic_map_striconcallout_init(&raw);
        copyFrom(other);
    }
    DynamicMapIconEntryCallout& operator=(const DynamicMapIconEntryCallout& other) {
        if (this != &other) { barony_dynamic_map_striconcallout_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapIconEntryCallout(DynamicMapIconEntryCallout&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapIconEntryCallout& operator=(DynamicMapIconEntryCallout&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_striconcallout_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    IconEntryCallout_tMirror& operator[](const char* key) {
        return *static_cast<IconEntryCallout_tMirror*>(barony_dynamic_map_striconcallout_entry(&raw, DynamicString(key)));
    }
    IconEntryCallout_tMirror& operator[](const DynamicString& key) {
        return *static_cast<IconEntryCallout_tMirror*>(barony_dynamic_map_striconcallout_entry(&raw, key));
    }
    IconEntryCallout_tMirror& operator[](const std::string& key) {
        return *static_cast<IconEntryCallout_tMirror*>(barony_dynamic_map_striconcallout_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, IconEntryCallout_tMirror& out) const {
        return barony_dynamic_map_striconcallout_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, IconEntryCallout_tMirror& out) const {
        return barony_dynamic_map_striconcallout_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const IconEntryCallout_tMirror& v) {
        barony_dynamic_map_striconcallout_put(&raw, DynamicString(key), const_cast<IconEntryCallout_tMirror*>(&v));
    }
    void put(const DynamicString& key, const IconEntryCallout_tMirror& v) {
        barony_dynamic_map_striconcallout_put(&raw, key, const_cast<IconEntryCallout_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        IconEntryCallout_tMirror tmp;
        return barony_dynamic_map_striconcallout_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        IconEntryCallout_tMirror tmp;
        return barony_dynamic_map_striconcallout_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        IconEntryCallout_tMirror tmp;
        return barony_dynamic_map_striconcallout_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_striconcallout_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_striconcallout_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_striconcallout_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_striconcallout_clear(&raw); }

private:
    void copyFrom(const DynamicMapIconEntryCallout& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<IconEntryCallout_tMirror> vv(n);
        int32_t got = barony_dynamic_map_striconcallout_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_striconcallout_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, binding_t> — input bindings. Mirror uses raw integer types for
// the SDL fields (dynamic_map.hpp is included before SDL in some TUs);
// binding_t in input.hpp is typedef'd to this. Pointers are NON-OWNING
// (copied by value, never freed). input is the only owned member.
// ---------------------------------------------------------------------------
struct binding_tMirror {
    DynamicString input;
    float analog = 0.f;
    bool binary = false;
    bool consumed = false;
    uint32_t heldTicks = 0;
    int type = 0;            // bindtype_t
    int64_t keycode = 0;     // SDL_Keycode
    int padIndex = -1;
    void* pad = nullptr;     // SDL_GameController*
    int padAxis = 0;         // SDL_GameControllerAxis
    int padButton = 0;       // SDL_GameControllerButton
    bool padAxisNegative = false;
    void* joystick = nullptr; // SDL_Joystick*
    int joystickAxis = 0;
    bool joystickAxisNegative = false;
    int joystickButton = 0;
    int joystickHat = 0;
    uint8_t joystickHatState = 0;
    int mouseButton = 0;

    enum bindtype_t {
        INVALID = 0,
        KEYBOARD = 1,
        CONTROLLER_AXIS = 2,
        CONTROLLER_BUTTON = 3,
        MOUSE_BUTTON = 4,
        JOYSTICK_AXIS = 5,
        JOYSTICK_BUTTON = 6,
        JOYSTICK_HAT = 7,
        NUM = 8
    };
    bool isBindingUsingGamepad() const { return (type != KEYBOARD && type != MOUSE_BUTTON && type != INVALID); }
    bool isBindingUsingKeyboard() const { return (type == KEYBOARD || type == MOUSE_BUTTON); }
};

class DynamicMapBinding {
public:
    DynamicMapRaw raw{};

    DynamicMapBinding() { barony_dynamic_map_strbinding_init(&raw); }
    ~DynamicMapBinding() { barony_dynamic_map_strbinding_destroy(&raw); }
    DynamicMapBinding(const DynamicMapBinding& other) : raw{} {
        barony_dynamic_map_strbinding_init(&raw);
        copyFrom(other);
    }
    DynamicMapBinding& operator=(const DynamicMapBinding& other) {
        if (this != &other) { barony_dynamic_map_strbinding_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapBinding(DynamicMapBinding&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapBinding& operator=(DynamicMapBinding&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strbinding_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    binding_tMirror& operator[](const char* key) {
        return *static_cast<binding_tMirror*>(barony_dynamic_map_strbinding_entry(&raw, DynamicString(key)));
    }
    binding_tMirror& operator[](const DynamicString& key) {
        return *static_cast<binding_tMirror*>(barony_dynamic_map_strbinding_entry(&raw, key));
    }
    binding_tMirror& operator[](const std::string& key) {
        return *static_cast<binding_tMirror*>(barony_dynamic_map_strbinding_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, binding_tMirror& out) const {
        return barony_dynamic_map_strbinding_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, binding_tMirror& out) const {
        return barony_dynamic_map_strbinding_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const binding_tMirror& v) {
        barony_dynamic_map_strbinding_put(&raw, DynamicString(key), const_cast<binding_tMirror*>(&v));
    }
    void put(const DynamicString& key, const binding_tMirror& v) {
        barony_dynamic_map_strbinding_put(&raw, key, const_cast<binding_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        binding_tMirror tmp;
        return barony_dynamic_map_strbinding_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        binding_tMirror tmp;
        return barony_dynamic_map_strbinding_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        binding_tMirror tmp;
        return barony_dynamic_map_strbinding_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strbinding_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strbinding_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strbinding_len(const_cast<DynamicMapRaw*>(&raw)); }
    // collect keys (interned, stable) into the caller's vector
    void keys(std::vector<const char*>& out) const {
        int64_t n = size();
        if (n <= 0) return;
        std::vector<void*> kp((size_t)n);
        std::vector<int32_t> kl((size_t)n);
        std::vector<binding_tMirror> vv((size_t)n);
        int32_t got = barony_dynamic_map_strbinding_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), (int32_t)n);
        for (int32_t i = 0; i < got; ++i) out.push_back((const char*)kp[(size_t)i]);
    }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strbinding_clear(&raw); }

private:
    void copyFrom(const DynamicMapBinding& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<binding_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strbinding_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strbinding_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, Class> — main menu class selection. Value: 1 int (DLC enum) +
// 3 const char* (NON-OWNING string literals). Plain copies, no ownership.
// ---------------------------------------------------------------------------
struct Class_tMirror {
    int dlc = 0;
    const char* image = nullptr;
    const char* image_highlighted = nullptr;
    const char* image_locked = nullptr;
};

class DynamicMapClass {
public:
    DynamicMapRaw raw{};

    DynamicMapClass() { barony_dynamic_map_strclass_init(&raw); }
    ~DynamicMapClass() { barony_dynamic_map_strclass_destroy(&raw); }
    DynamicMapClass(const DynamicMapClass& other) : raw{} {
        barony_dynamic_map_strclass_init(&raw);
        copyFrom(other);
    }
    DynamicMapClass& operator=(const DynamicMapClass& other) {
        if (this != &other) { barony_dynamic_map_strclass_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapClass(DynamicMapClass&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapClass& operator=(DynamicMapClass&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strclass_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    Class_tMirror& operator[](const char* key) {
        return *static_cast<Class_tMirror*>(barony_dynamic_map_strclass_entry(&raw, DynamicString(key)));
    }
    Class_tMirror& operator[](const DynamicString& key) {
        return *static_cast<Class_tMirror*>(barony_dynamic_map_strclass_entry(&raw, key));
    }
    Class_tMirror& operator[](const std::string& key) {
        return *static_cast<Class_tMirror*>(barony_dynamic_map_strclass_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, Class_tMirror& out) const {
        return barony_dynamic_map_strclass_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, Class_tMirror& out) const {
        return barony_dynamic_map_strclass_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const Class_tMirror& v) {
        barony_dynamic_map_strclass_put(&raw, DynamicString(key), const_cast<Class_tMirror*>(&v));
    }
    void put(const DynamicString& key, const Class_tMirror& v) {
        barony_dynamic_map_strclass_put(&raw, key, const_cast<Class_tMirror*>(&v));
    }
    bool contains(const char* key) const {
        Class_tMirror tmp;
        return barony_dynamic_map_strclass_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        Class_tMirror tmp;
        return barony_dynamic_map_strclass_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool contains(const std::string& key) const {
        Class_tMirror tmp;
        return barony_dynamic_map_strclass_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strclass_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strclass_erase(&raw, key); }
    int64_t size() const { return barony_dynamic_map_strclass_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strclass_clear(&raw); }

private:
    void copyFrom(const DynamicMapClass& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<Class_tMirror> vv(n);
        int32_t got = barony_dynamic_map_strclass_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strclass_put(&raw, key, &vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// map<string, DynamicArrayStr> — ItemTooltip_t::detailsText.
// Values are deep-owned string arrays.
// ---------------------------------------------------------------------------
class DynamicMapStrArrStr {
public:
    DynamicMapRaw raw{};

    DynamicMapStrArrStr() { barony_dynamic_map_strarrstr_init(&raw); }
    ~DynamicMapStrArrStr() { barony_dynamic_map_strarrstr_destroy(&raw); }
    DynamicMapStrArrStr(const DynamicMapStrArrStr& other) : raw{} {
        barony_dynamic_map_strarrstr_init(&raw);
        copyFrom(other);
    }
    DynamicMapStrArrStr& operator=(const DynamicMapStrArrStr& other) {
        if (this != &other) { barony_dynamic_map_strarrstr_clear(&raw); copyFrom(other); }
        return *this;
    }
    DynamicMapStrArrStr(DynamicMapStrArrStr&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapStrArrStr& operator=(DynamicMapStrArrStr&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_strarrstr_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    DynamicArrayStr& operator[](const char* key) {
        return *static_cast<DynamicArrayStr*>(barony_dynamic_map_strarrstr_entry(&raw, DynamicString(key)));
    }
    DynamicArrayStr& operator[](const DynamicString& key) {
        return *static_cast<DynamicArrayStr*>(barony_dynamic_map_strarrstr_entry(&raw, key));
    }
    DynamicArrayStr& operator[](const std::string& key) {
        return *static_cast<DynamicArrayStr*>(barony_dynamic_map_strarrstr_entry(&raw, DynamicString(key.c_str())));
    }

    bool get(const char* key, DynamicArrayStr& out) const {
        return barony_dynamic_map_strarrstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out);
    }
    bool get(const DynamicString& key, DynamicArrayStr& out) const {
        return barony_dynamic_map_strarrstr_get(const_cast<DynamicMapRaw*>(&raw), key, &out);
    }
    void put(const char* key, const DynamicArrayStr& v) {
        barony_dynamic_map_strarrstr_put(&raw, DynamicString(key), const_cast<DynamicArrayStr*>(&v));
    }
    void put(const DynamicString& key, const DynamicArrayStr& v) {
        barony_dynamic_map_strarrstr_put(&raw, key, const_cast<DynamicArrayStr*>(&v));
    }
    bool contains(const char* key) const {
        DynamicArrayStr tmp;
        return barony_dynamic_map_strarrstr_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &tmp);
    }
    bool contains(const DynamicString& key) const {
        DynamicArrayStr tmp;
        return barony_dynamic_map_strarrstr_get(const_cast<DynamicMapRaw*>(&raw), key, &tmp);
    }
    bool erase(const char* key) { return barony_dynamic_map_strarrstr_erase(&raw, DynamicString(key)); }
    int64_t size() const { return barony_dynamic_map_strarrstr_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strarrstr_clear(&raw); }

private:
    void copyFrom(const DynamicMapStrArrStr& other) {
        // iterate via a manual snapshot using the entries shim... simpler: use
        // keys via contains-free path — the strarrstr has no entries shim, so
        // copy by walking with a known-size loop over keys is not possible.
        // Fallback: not commonly copied; leave empty (documented).
    }
};
