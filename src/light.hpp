/*-------------------------------------------------------------------------------

	BARONY
	File: light.hpp
	Desc: prototypes for light.cpp, light-related types and prototypes

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/
#pragma once
#include "../odin/containers/dynamic_map.hpp"

typedef struct light_t
{
	Sint32 x, y;
	Sint32 radius;
	vec4_t* tiles;
    int index; // which lightmap this actually exists in

	// a pointer to the light's location in a list
	node_t* node;
} light_t;

light_t* lightSphereShadow(int index, Sint32 x, Sint32 y, Sint32 radius, float r, float g, float b, float a, float exp);
light_t* lightSphere(int index, Sint32 x, Sint32 y, Sint32 radius, float r, float g, float b, float a, float exp);
light_t* newLight(int index, Sint32 x, Sint32 y, Sint32 radius);
light_t* addLight(Sint32 x, Sint32 y, const char* name, int range_bonus = 0, int index = 0);
bool loadLights(bool forceLoadBaseDirectory = false);

struct LightDef {
    int radius = 0;
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
	float a = 0.f;
    float falloff_exp = 1.f;
    bool shadows = false;
};
// Odin map[string]LightDef shims (declared here — they need the LightDef type)
extern "C" {
    void      barony_dynamic_map_strlightdef_init(DynamicMapRaw*);
    void      barony_dynamic_map_strlightdef_put(DynamicMapRaw*, DynamicString, LightDef);
    bool      barony_dynamic_map_strlightdef_get(DynamicMapRaw*, DynamicString, LightDef*);
    bool      barony_dynamic_map_strlightdef_erase(DynamicMapRaw*, DynamicString);
    void      barony_dynamic_map_strlightdef_clear(DynamicMapRaw*);
    int32_t   barony_dynamic_map_strlightdef_len(DynamicMapRaw*);
    void      barony_dynamic_map_strlightdef_destroy(DynamicMapRaw*);
    LightDef* barony_dynamic_map_strlightdef_entry(DynamicMapRaw*, DynamicString);
    int32_t   barony_dynamic_map_strlightdef_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, LightDef* val_ptrs, int32_t count);
}

// map<string, LightDef> — RAII mirror of the Odin map[string]LightDef
class DynamicMapLightDef {
public:
    DynamicMapRaw raw{};
    DynamicMapLightDef() { barony_dynamic_map_strlightdef_init(&raw); }
    ~DynamicMapLightDef() { barony_dynamic_map_strlightdef_destroy(&raw); }
    DynamicMapLightDef(const DynamicMapLightDef& o) : raw{} {
        barony_dynamic_map_strlightdef_init(&raw);
        int32_t n = (int32_t)barony_dynamic_map_strlightdef_len(const_cast<DynamicMapRaw*>(&o.raw));
        if (n <= 0) return;
        std::vector<void*> kp(n); std::vector<int32_t> kl(n); std::vector<LightDef> vv(n);
        int32_t got = barony_dynamic_map_strlightdef_entries(const_cast<DynamicMapRaw*>(&o.raw), kp.data(), kl.data(), vv.data(), n);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_strlightdef_put(&raw, key, vv[i]);
        }
    }
    DynamicMapLightDef& operator=(const DynamicMapLightDef& o) {
        if (this != &o) { barony_dynamic_map_strlightdef_clear(&raw); *this = DynamicMapLightDef(o); }
        return *this;
    }
    DynamicMapLightDef(DynamicMapLightDef&& o) noexcept : raw(o.raw) { o.raw = DynamicMapRaw{}; }
    DynamicMapLightDef& operator=(DynamicMapLightDef&& o) noexcept {
        if (this != &o) { barony_dynamic_map_strlightdef_destroy(&raw); raw = o.raw; o.raw = DynamicMapRaw{}; }
        return *this;
    }
    LightDef& operator[](const char* key) { return *barony_dynamic_map_strlightdef_entry(&raw, DynamicString(key)); }
    LightDef& operator[](const DynamicString& key) { return *barony_dynamic_map_strlightdef_entry(&raw, key); }
    LightDef& operator[](const std::string& key) { return *barony_dynamic_map_strlightdef_entry(&raw, DynamicString(key.c_str())); }
    bool contains(const char* key) const { LightDef v; return barony_dynamic_map_strlightdef_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v); }
    bool contains(const DynamicString& key) const { LightDef v; return barony_dynamic_map_strlightdef_get(const_cast<DynamicMapRaw*>(&raw), key, &v); }
    bool contains(const std::string& key) const { LightDef v; return barony_dynamic_map_strlightdef_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v); }
    int64_t size() const { return barony_dynamic_map_strlightdef_len(const_cast<DynamicMapRaw*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_strlightdef_clear(&raw); }
    bool erase(const char* key) { return barony_dynamic_map_strlightdef_erase(&raw, DynamicString(key)); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_strlightdef_erase(&raw, key); }
    // find() iterator (std::map-like)
    struct KV { const char* first; LightDef second; };
    struct Iterator { KV kv{}; bool valid = false; const KV* operator->() const { return &kv; } };
    Iterator find(const char* key) const {
        Iterator it;
        // snapshot scan (small maps) — find the interned key ptr
        int32_t n = (int32_t)size();
        if (n > 0) {
            std::vector<void*> kp(n); std::vector<int32_t> kl(n); std::vector<LightDef> vv(n);
            int32_t got = barony_dynamic_map_strlightdef_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n);
            for (int32_t i = 0; i < got; ++i) {
                if (kl[i] == (int32_t)std::strlen(key) && std::memcmp(kp[i], key, kl[i]) == 0) {
                    it.kv.first = (const char*)kp[i]; it.kv.second = vv[i]; it.valid = true; break;
                }
            }
        }
        return it;
    }
    Iterator find(const std::string& key) const { return find(key.c_str()); }
    Iterator end() const { return Iterator{}; }
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.valid != b.valid; }
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.valid == b.valid; }
};

extern DynamicMapLightDef lightDefs;
