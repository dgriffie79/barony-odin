/*-------------------------------------------------------------------------------

	BARONY
	File: light.cpp
	Desc: light spawning code

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "main.hpp"
#include "light.hpp"
#include "draw.hpp"

/*-------------------------------------------------------------------------------

	lightSphereShadow

	Adds a circle of light to the lightmap at x and y with the supplied
	radius and color; casts shadows against walls

-------------------------------------------------------------------------------*/

light_t* lightSphereShadow(int index, Sint32 x, Sint32 y, Sint32 radius, float r, float g, float b, float a, float exp)
{
	light_t* light = newLight(index, x, y, radius);
    r = r * 255.f;
    g = g * 255.f;
    b = b * 255.f;
    a = a * 255.f;

	for (int v = y - radius; v <= y + radius; ++v) {
		for (int u = x - radius; u <= x + radius; ++u) {
			if (u >= 0 && v >= 0 && u < map.width && v < map.height) {
				const int dx = u - x;
				const int dy = v - y;
				const int dxabs = abs(dx);
				const int dyabs = abs(dy);
				real_t a0 = dyabs * .5;
				real_t b0 = dxabs * .5;
				int u2 = u;
				int v2 = v;
                
                // check origin is okay
				bool wallhit = true;
				const int mapindex = v * MAPLAYERS + u * MAPLAYERS * map.height;
				for (int z = 0; z < MAPLAYERS; z++) {
					if ( !map.tiles[mapindex + z] || map.tiles[mapindex + z] == TRANSPARENT_TILE ) {
						wallhit = false;
						break;
					}
				}
				if (wallhit == true) {
					continue;
				}
                
                // line test
                if (dxabs >= dyabs) { // the line is more horizontal than vertical
					for (int i = 0; i < dxabs; ++i) {
						u2 -= sgn(dx);
						b0 += dyabs;
						if (b0 >= dxabs) {
							b0 -= dxabs;
							v2 -= sgn(dy);
						}
						if (u2 >= 0 && u2 < map.width && v2 >= 0 && v2 < map.height) {
							if ( map.tiles[OBSTACLELAYER + v2 * MAPLAYERS + u2 * MAPLAYERS * map.height]
                                && map.tiles[OBSTACLELAYER + v2 * MAPLAYERS + u2 * MAPLAYERS * map.height] != TRANSPARENT_TILE ) {
								wallhit = true;
								break;
							}
						}
					}
				}
                else { // the line is more vertical than horizontal
					for (int i = 0; i < dyabs; ++i) {
						v2 -= sgn(dy);
						a0 += dxabs;
						if (a0 >= dyabs) {
							a0 -= dyabs;
							u2 -= sgn(dx);
						}
						if (u2 >= 0 && u2 < map.width && v2 >= 0 && v2 < map.height) {
							if (map.tiles[OBSTACLELAYER + v2 * MAPLAYERS + u2 * MAPLAYERS * map.height]
                                && map.tiles[OBSTACLELAYER + v2 * MAPLAYERS + u2 * MAPLAYERS * map.height] != TRANSPARENT_TILE) {
								wallhit = true;
								break;
							}
						}
					}
				}
                
                // light tile if it passed line test
				if (wallhit == false || (wallhit == true && u2 == u && v2 == v)) {
                    const float dist = exp != 1.f ? powf(dx * dx + dy * dy, exp) : dx * dx + dy * dy;
                    const auto falloff = std::min<float>(dist / radius, 1.0f);
                    const auto soff = (dy + radius) + (dx + radius) * (radius * 2 + 1);
                    auto& s = light->tiles[soff];
					s.x += r - r * falloff;
                    s.y += g - g * falloff;
                    s.z += b - b * falloff;
                    s.w += a - a * falloff;
                    const auto doff = v + u * map.height;
                    if (index) {
                        auto& d = *dynarray_at<vec4_t>(lightmaps[index], doff);
                        d.x += s.x;
                        d.y += s.y;
                        d.z += s.z;
                        d.w += s.w;
                    } else {
                        for (int c = 0; c < MAXPLAYERS + 1; ++c) {
                            auto& d = *dynarray_at<vec4_t>(lightmaps[c], doff);
                            d.x += s.x;
                            d.y += s.y;
                            d.z += s.z;
                            d.w += s.w;
                        }
                    }
                }
			}
		}
	}
	return light;
}

/*-------------------------------------------------------------------------------

	lightSphere

	Adds a circle of light to the lightmap at x and y with the supplied
	radius and color; casts no shadows

-------------------------------------------------------------------------------*/

light_t* lightSphere(int index, Sint32 x, Sint32 y, Sint32 radius, float r, float g, float b, float a, float exp)
{
	light_t* light = newLight(index, x, y, radius);
    r = r * 255.f;
    g = g * 255.f;
    b = b * 255.f;
    a = a * 255.f;

	for (int v = y - radius; v <= y + radius; ++v) {
		for (int u = x - radius; u <= x + radius; ++u) {
			if (u >= 0 && v >= 0 && u < map.width && v < map.height) {
				const int dx = u - x;
				const int dy = v - y;
                const float dist = exp != 1.f ? powf(dx * dx + dy * dy, exp) : dx * dx + dy * dy;
                const auto falloff = std::min<float>(dist / radius, 1.0f);
                const auto soff = (dy + radius) + (dx + radius) * (radius * 2 + 1);
                auto& s = light->tiles[soff];
                s.x += r - r * falloff;
                s.y += g - g * falloff;
                s.z += b - b * falloff;
                s.w += a - a * falloff;
                const auto doff = v + u * map.height;
                if (index) {
                    auto& d = *dynarray_at<vec4_t>(lightmaps[index], doff);
                    d.x += s.x;
                    d.y += s.y;
                    d.z += s.z;
                    d.w += s.w;
                } else {
                    for (int c = 0; c < MAXPLAYERS + 1; ++c) {
                        auto& d = *dynarray_at<vec4_t>(lightmaps[c], doff);
                        d.x += s.x;
                        d.y += s.y;
                        d.z += s.z;
                        d.w += s.w;
                    }
                }
			}
		}
	}
	return light;
}

#include "../odin/json_shim/json_shim.hpp"
#include "files.hpp"

DynamicMapLightDef lightDefs;
bool loadLights(bool forceLoadBaseDirectory) {
    if ( !PHYSFS_getRealDir("/data/lights.json") )
    {
        printlog("[JSON]: Error: Could not find file: data/lights.json");
        return false;
    }

    std::string inputPath = PHYSFS_getRealDir("/data/lights.json");
    if ( forceLoadBaseDirectory )
    {
        inputPath = BASE_DATA_DIR;
    }
    else
    {
        if ( inputPath != BASE_DATA_DIR )
        {
            loadLights(true); // force load the base directory first, then modded paths later.
        }
        else
        {
            forceLoadBaseDirectory = true;
        }
    }

    inputPath.append("/data/lights.json");

    File* fp = FileIO::open(inputPath.c_str(), "rb");
    if ( !fp )
    {
        printlog("[JSON]: Error: Could not open json file %s", inputPath.c_str());
        return false;
    }

    if ( forceLoadBaseDirectory )
    {
        lightDefs.clear();
    }
    
    char buf[65536];
    int count = (int)fp->read(buf, sizeof(buf[0]), sizeof(buf));
    buf[count] = '\0';
    FileIO::close(fp);

    void* jsonReader = json_reader_parse(buf);
    if ( jsonReader )
    {
        void* root = json_node_root(jsonReader);
        void* lights = json_node_get_member(root, "lights");
        if ( lights && json_node_is_object(lights) ) {
            uint32_t lightCount = json_node_member_count(lights);
            for ( uint32_t i = 0; i < lightCount; ++i ) {
                LightDef def;
                const char* name = json_node_member_name_at(lights, i);
                void* value = json_node_member_value_at(lights, i);
                int32_t radius; json_node_get_int(json_node_get_member(value, "radius"), &radius); def.radius = radius;
                float r; json_node_get_float(json_node_get_member(value, "r"), &r); def.r = r;
                float g; json_node_get_float(json_node_get_member(value, "g"), &g); def.g = g;
                float b; json_node_get_float(json_node_get_member(value, "b"), &b); def.b = b;
                if ( json_node_has_member(value, "a") )
                {
                    float a; json_node_get_float(json_node_get_member(value, "a"), &a); def.a = a;
                }
                else
                {
                    def.a = 0.f;
                }
                float exp; json_node_get_float(json_node_get_member(value, "falloff_exp"), &exp); def.falloff_exp = exp;
                bool shadows; json_node_get_bool(json_node_get_member(value, "shadows"), &shadows); def.shadows = shadows;
                lightDefs[name] = def;
            }
        }
        json_reader_destroy(jsonReader);
    }
    
    return true;
}

#ifndef EDITOR
#include "interface/consolecommand.hpp"
static ConsoleCommand ccmd_reloadLights("/reloadlights", "reload light json",
    [](int argc, const char* argv[]){
    loadLights();
    });
#endif

light_t* addLight(Sint32 x, Sint32 y, const char* name, int range_bonus, int index) {
    if (!name || !name[0]) {
        return nullptr;
    }
    auto find = lightDefs.find(name);
    if (find == lightDefs.end()) {
        return nullptr;
    }
    const auto& def = find->second;
    if (def.shadows) {
        return lightSphereShadow(index, x, y, std::max(def.radius + range_bonus, 1), def.r, def.g, def.b, def.a, def.falloff_exp);
    } else {
        return lightSphere(index, x, y, std::max(def.radius + range_bonus, 1), def.r, def.g, def.b, def.a, def.falloff_exp);
    }
}

bool DynamicMapLightDef::contains(const char* key) const { LightDef v; return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v, MK_LightDef); }

extern "C" bool DynamicMapLightDef_contains_3(const DynamicMapLightDef* self, const std::string & key) { return self->contains(key); }


extern "C" bool DynamicMapLightDef_contains_2(const DynamicMapLightDef* self, const DynamicString & key) { return self->contains(key); }


extern "C" bool DynamicMapLightDef_contains(const DynamicMapLightDef* self, const char * key) { return self->contains(key); }


bool DynamicMapLightDef::contains(const DynamicString& key) const { LightDef v; return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), key, &v, MK_LightDef); }

bool DynamicMapLightDef::contains(const std::string& key) const { LightDef v; return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v, MK_LightDef); }

bool DynamicMapLightDef::empty() const { return size() == 0; }

extern "C" bool DynamicMapLightDef_empty(const DynamicMapLightDef* self) { return self->empty(); }


void DynamicMapLightDef::clear() { barony_dynamic_map_str_clear(&raw, MK_LightDef); }

extern "C" void DynamicMapLightDef_clear(DynamicMapLightDef* self) { return self->clear(); }


DynamicMapLightDef::Iterator DynamicMapLightDef::find(const char* key) const {
        Iterator it;
        int32_t n = (int32_t)size();
        if (n > 0) {
            std::vector<void*> kp(n); std::vector<int32_t> kl(n); std::vector<LightDef> vv(n);
            int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n, MK_LightDef);
            for (int32_t i = 0; i < got; ++i) {
                if (kl[i] == (int32_t)std::strlen(key) && std::memcmp(kp[i], key, kl[i]) == 0) {
                    it.kv.first = (const char*)kp[i]; it.kv.second = vv[i]; it.valid = true; break;
                }
            }
        }
        return it;
    }

extern "C" DynamicMapLightDef::Iterator DynamicMapLightDef_find_2(const DynamicMapLightDef* self, const std::string & key) { return self->find(key); }


extern "C" DynamicMapLightDef::Iterator DynamicMapLightDef_find(const DynamicMapLightDef* self, const char * key) { return self->find(key); }


DynamicMapLightDef::Iterator DynamicMapLightDef::find(const std::string& key) const { return find(key.c_str()); }

DynamicMapLightDef::Iterator DynamicMapLightDef::end() const { return Iterator{}; }

extern "C" DynamicMapLightDef::Iterator DynamicMapLightDef_end(const DynamicMapLightDef* self) { return self->end(); }


// DynamicMapLightDef operators + Iterator::operator-> are inline in light.hpp.
extern "C" DynamicMapLightDef & DynamicMapLightDef_assign(DynamicMapLightDef* self, const DynamicMapLightDef & o) { return self->operator=(o); }
extern "C" DynamicMapLightDef & DynamicMapLightDef_assign_2(DynamicMapLightDef* self, DynamicMapLightDef && o) { return self->operator=(static_cast<DynamicMapLightDef&&>(o)); }
extern "C" const DynamicMapLightDef::KV * DynamicMapLightDef_Iterator_arrow(const DynamicMapLightDef::Iterator* self) { return self->operator->(); }
