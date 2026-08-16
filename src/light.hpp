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
// Odin map[string]LightDef shims (generic str-key family, kind MK_LightDef)
extern "C" {
    void      barony_dynamic_map_str_init(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_str_put(DynamicMapRaw*, DynamicString, const void* value, int32_t value_kind);
    bool      barony_dynamic_map_str_get(DynamicMapRaw*, DynamicString, void* out, int32_t value_kind);
    bool      barony_dynamic_map_str_erase(DynamicMapRaw*, DynamicString, int32_t value_kind);
    void      barony_dynamic_map_str_clear(DynamicMapRaw*, int32_t value_kind);
    int32_t   barony_dynamic_map_str_len(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_str_destroy(DynamicMapRaw*, int32_t value_kind);
    void*     barony_dynamic_map_str_entry(DynamicMapRaw*, DynamicString, int32_t value_kind);
    int32_t   barony_dynamic_map_str_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count, int32_t value_kind);
}

// map<string, LightDef> — RAII mirror of the Odin map[string]LightDef
// (generic str-key family, kind MK_LightDef; LightDef is POD)
class DynamicMapLightDef {
public:
    DynamicMapRaw raw{};
    DynamicMapLightDef() { barony_dynamic_map_str_init(&raw, MK_LightDef); }
    ~DynamicMapLightDef() { barony_dynamic_map_str_destroy(&raw, MK_LightDef); }
    DynamicMapLightDef(const DynamicMapLightDef& o) : raw{} {
        barony_dynamic_map_str_init(&raw, MK_LightDef);
        int32_t n = (int32_t)barony_dynamic_map_str_len(const_cast<DynamicMapRaw*>(&o.raw), MK_LightDef);
        if (n <= 0) return;
        std::vector<void*> kp(n); std::vector<int32_t> kl(n); std::vector<LightDef> vv(n);
        int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&o.raw), kp.data(), kl.data(), vv.data(), n, MK_LightDef);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            barony_dynamic_map_str_put(&raw, key, &vv[i], MK_LightDef);
        }
    }
    DynamicMapLightDef& operator=(const DynamicMapLightDef& o) {
        if (this != &o) { barony_dynamic_map_str_clear(&raw, MK_LightDef); *this = DynamicMapLightDef(o); }
        return *this;
    }
    DynamicMapLightDef(DynamicMapLightDef&& o) noexcept : raw(o.raw) { o.raw = DynamicMapRaw{}; }
    DynamicMapLightDef& operator=(DynamicMapLightDef&& o) noexcept {
        if (this != &o) { barony_dynamic_map_str_destroy(&raw, MK_LightDef); raw = o.raw; o.raw = DynamicMapRaw{}; }
        return *this;
    }
    LightDef& operator[](const char* key) { return *reinterpret_cast<LightDef*>(barony_dynamic_map_str_entry(&raw, DynamicString(key), MK_LightDef)); }
    LightDef& operator[](const DynamicString& key) { return *reinterpret_cast<LightDef*>(barony_dynamic_map_str_entry(&raw, key, MK_LightDef)); }
    LightDef& operator[](const std::string& key) { return *reinterpret_cast<LightDef*>(barony_dynamic_map_str_entry(&raw, DynamicString(key.c_str()), MK_LightDef)); }
    bool contains(const char* key) const;
    bool contains(const DynamicString& key) const;
    bool contains(const std::string& key) const;
    int64_t size() const { return barony_dynamic_map_str_len(const_cast<DynamicMapRaw*>(&raw), MK_LightDef); }
    bool empty() const;
    void clear();
    bool erase(const char* key) { return barony_dynamic_map_str_erase(&raw, DynamicString(key), MK_LightDef); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_str_erase(&raw, key, MK_LightDef); }
    // find() iterator (std::map-like)
    struct KV { const char* first; LightDef second; };
    struct Iterator { KV kv{}; bool valid = false; const KV* operator->() const { return &kv; } };
    Iterator find(const char* key) const;
    Iterator find(const std::string& key) const;
    Iterator end() const;
    friend bool operator!=(const Iterator& a, const Iterator& b) { return a.valid != b.valid; }
    friend bool operator==(const Iterator& a, const Iterator& b) { return a.valid == b.valid; }
};

extern DynamicMapLightDef lightDefs;
