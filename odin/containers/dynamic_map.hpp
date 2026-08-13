// dynamic_map.hpp — C++ mirror of Odin's native maps (Raw_Map layout) + typed
// wrappers that call the generic str-key / i32-key shim families.
//
// GENERIC (since D3l): the per-value-type shim families are gone. There are
// TWO exported families on the Odin side:
//   barony_dynamic_map_str_*  (string keys; value_kind selects the V type)
//   barony_dynamic_map_i32_*  ([4]byte keys; value_kind selects the V type)
// The C++ classes collapse into TWO templates below; the typedefs keep the
// pre-consolidation class names so game code is untouched.
//
// Key ownership: string keys are INTERNED on the Odin side (process-lifetime
// stable, deduped). Values: POD kinds are copied by value; owned kinds are
// deep-copied (free/copy via the shared Element_Ops table).
#pragma once
#include <cstdint>
#include "dynamic_array.hpp"
#include <memory>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>
#include <string_view>
#include "dynamic_string.hpp"

struct DynamicMapRaw;  // forward decl for the shim signatures

// Odin containers shims (declared before the class so methods can call them)
extern "C" {
    // ---- generic str-key family (value_kind selects V) ----
    void      barony_dynamic_map_str_init(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_str_put(DynamicMapRaw*, DynamicString, const void* value, int32_t value_kind);
    bool      barony_dynamic_map_str_get(DynamicMapRaw*, DynamicString, void* out, int32_t value_kind);
    bool      barony_dynamic_map_str_erase(DynamicMapRaw*, DynamicString, int32_t value_kind);
    void      barony_dynamic_map_str_clear(DynamicMapRaw*, int32_t value_kind);
    int32_t   barony_dynamic_map_str_len(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_str_destroy(DynamicMapRaw*, int32_t value_kind);
    void*     barony_dynamic_map_str_entry(DynamicMapRaw*, DynamicString, int32_t value_kind);
    int32_t   barony_dynamic_map_str_entries(DynamicMapRaw*, void** key_ptrs, int32_t* key_lens, void* val_ptrs, int32_t count, int32_t value_kind);
    bool      barony_dynamic_map_str_find(DynamicMapRaw*, DynamicString, void** out_key, int32_t* out_key_len, void* out_val, int32_t value_kind);

    // ---- generic [4]byte-key family (value_kind selects V) ----
    void      barony_dynamic_map_i32_init(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_i32_put(DynamicMapRaw*, const void* key, const void* value, int32_t value_kind);
    bool      barony_dynamic_map_i32_get(DynamicMapRaw*, const void* key, void* out, int32_t value_kind);
    bool      barony_dynamic_map_i32_erase(DynamicMapRaw*, const void* key, int32_t value_kind);
    void      barony_dynamic_map_i32_clear(DynamicMapRaw*, int32_t value_kind);
    int32_t   barony_dynamic_map_i32_len(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_i32_destroy(DynamicMapRaw*, int32_t value_kind);
    void*     barony_dynamic_map_i32_entry(DynamicMapRaw*, const void* key, int32_t value_kind);
    int32_t   barony_dynamic_map_i32_entries(DynamicMapRaw*, void** key_ptrs, void* val_ptrs, int32_t count, int32_t value_kind);
    bool      barony_dynamic_map_i32_find(DynamicMapRaw*, const void* key, void* out_val, int32_t* out_val_len, int32_t value_kind);
    void      barony_dynamic_map_i32_for_each(DynamicMapRaw*, int32_t value_kind, void* cb, void* userdata);

    void      barony_dynamic_map_str_for_each(DynamicMapRaw*, int32_t value_kind, void* cb, void* userdata);

    // ---- generic ptr-key family (rawptr keys; value_kind selects V) ----
    void      barony_dynamic_map_ptr_init(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_ptr_put(DynamicMapRaw*, const void* key, const void* value, int32_t value_kind);
    bool      barony_dynamic_map_ptr_get(DynamicMapRaw*, const void* key, void* out, int32_t value_kind);
    int32_t   barony_dynamic_map_ptr_len(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_ptr_clear(DynamicMapRaw*, int32_t value_kind);
    void      barony_dynamic_map_ptr_destroy(DynamicMapRaw*, int32_t value_kind);
    void*     barony_dynamic_map_ptr_entry(DynamicMapRaw*, const void* key, int32_t value_kind);
    int32_t   barony_dynamic_map_ptr_entries(DynamicMapRaw*, void** key_ptrs, void* val_ptrs, int32_t count, int32_t value_kind);
    bool      barony_dynamic_map_ptr_erase(DynamicMapRaw*, const void* key, int32_t value_kind);

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

// ---- sets (defined first: IconEntryTextMap_t uses DynamicSetI32) ----
class DynamicSetI32 {
public:
    DynamicMapRaw raw{};

    DynamicSetI32() { barony_dynamic_set_i32_init(&raw); }
    DynamicSetI32(std::initializer_list<int> init) {
        barony_dynamic_set_i32_init(&raw);
        for (int v : init) barony_dynamic_set_i32_insert(&raw, v);
    }
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
    // find()/end() (std::set-like): find(x) != end()
    struct Iterator {
        std::shared_ptr<std::vector<int>> snap;  // snapshot for begin(); null for find()
        int value = 0; bool valid = false;
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const int* operator->() const { return snap ? &(*snap)[idx] : &value; }
        int operator*() const { return snap ? (*snap)[idx] : value; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            if (!o.snap) return snap ? idx < snap->size() : valid;
            if (!snap) return o.snap ? o.idx < o.snap->size() : false;
            if (snap.get() != o.snap.get()) return !(idx >= snap->size() && o.idx == END);
            return idx != o.idx;
        }
        bool operator==(const Iterator& o) const { return !(*this != o); }
    };
    Iterator begin() const {
        Iterator it;
        int32_t n = (int32_t)size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<int>>();
            it.snap->resize((size_t)n);
            int32_t got = barony_dynamic_set_i32_entries(const_cast<DynamicMapRaw*>(&raw), it.snap->data(), n);
            if (got < n) it.snap->resize((size_t)got);
        }
        return it;
    }
    Iterator find(int v) const { Iterator it; it.valid = barony_dynamic_set_i32_contains(const_cast<DynamicMapRaw*>(&raw), v); it.value = v; return it; }
    Iterator end() const { Iterator it; it.valid = false; it.idx = Iterator::END; return it; }
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

// ---- mirror value structs (shared layout with Odin) ----
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

struct DiscoveryAnim_tMirror {
    uint32_t startTicks = 0;
    uint32_t processedOnTick = 0;
    DynamicString name;

    DiscoveryAnim_tMirror();   // defined in interface.cpp (needs global ticks)
};

struct SpecialNPCEntry_tMirror {
    DynamicString internalName;
    DynamicString name;
    DynamicString shortname;
    DynamicSetI32 modelIndexes;
    int baseModel = 0;
    DynamicString uniqueIcon;
};

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

struct ItemLocalization_tMirror {
    DynamicString name_identified;
    DynamicString name_unidentified;
};

struct Achievement_tMirror {
    DynamicString name;
    bool unlocked = false;
    int64_t unlockTime = 0;
};

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

struct Class_tMirror {
    int dlc = 0;
    const char* image = nullptr;
    const char* image_highlighted = nullptr;
    const char* image_locked = nullptr;
};

// Dither_t — 8B POD mirror of Entity::Dither / Chunk::Dither (both
// {int value; Uint32 lastUpdateTick;}). Value for the pointer-keyed
// dithering maps. The non-zero Chunk::Dither default (value=10) is handled at
// the call site (chunk dithering always writes value before first read), so
// the mirror is POD and value-initializes to zero like Entity::Dither.
struct Dither_t {
    int32_t value = 0;
    uint32_t lastUpdateTick = 0;
};

// DynamicStringPair_t — 32B owning mirror of std::pair<DynamicString,DynamicString>.
// Value for map<string, pair<string,string>> (mapDisplayNamesDescriptions).
struct DynamicStringPair_t {
    DynamicString first;
    DynamicString second;
};

// ChunkDither_t — same 8B layout, but its map value kind (MK_ChunkDither)
// defaults `value` to 10 on insert (Chunk::Dither { value = MAX; }), matching
// the std::unordered_map value-initialization for chunk dithering.
struct ChunkDither_t {
    int32_t value = 10;
    uint32_t lastUpdateTick = 0;
};



// ---- value kinds (must match value_kind mapping in dynamic_map.odin) ----
enum MapValueKind {
    MK_I32 = 0,
    MK_F32 = 1,
    MK_U32 = 2,
    MK_String = 3,
    MK_LightDef = 4,
    MK_IconEntryTextMap = 5,
    MK_IconEntryText = 6,
    MK_WorldIconEntry = 7,
    MK_DiscoveryAnim = 8,
    MK_SpecialNPC = 9,
    MK_ColliderDmg = 10,
    MK_ItemLoc = 11,
    MK_Achievement = 12,
    MK_AchievementData = 13,
    MK_IconEntry = 14,
    MK_IconEntryCallout = 15,
    MK_Binding = 16,
    MK_Class = 17,
    MK_DynArrayStr = 18,
    MK_DynArrayS32 = 19,
    MK_StatueLimbArray = 20,
    MK_StoreSlotsArray = 21,
    MK_MonsterTrapIgnore = 22,
    MK_SetOfI32 = 23,
    MK_Lootbag = 24,
    MK_EnemyHPDetails = 25,
    MK_U64 = 26,
    MK_GlyphData = 27,
    MK_Statistic = 28,
    MK_Ptr = 29,                 // 8-byte pointer values (Frame*, node_t*, etc.)
    MK_F64 = 30,                 // double / real_t (x64)
    MK_FormationInfo = 31,       // FormationInfo_t (20B POD)
    MK_AnimatedTile = 32,        // AnimatedTile (32B POD)
    MK_AdditionalOffset = 33,    // AdditionalOffset_t (48B POD)
    MK_PlayerRaceHostility = 34, // PlayerRaceHostility_t (40B POD)
    MK_SpellElement = 35,        // spellElement_t (168B POD)
    MK_Effect = 36,              // ParticleTimerEffect_t::Effect_t (40B POD)
    MK_EffectLocations = 37,     // ParticleTimerEffect_t::EffectLocations_t (40B POD)
    MK_CalloutParticle = 38,     // CalloutParticle_t (160B POD)
    MK_ModelOffset = 39,         // ModelOffset_t (160B, owning: 2 nested maps)
    MK_EffectDefinitionEntry = 40, // StatusEffectQueue_t::EffectDefinitionEntry_t (248B owning)
    MK_ParticleTimerEffect = 41, // ParticleTimerEffect_t (32B, owning: effectMap)
    MK_IconLookup = 42,          // MonsterData_t::MonsterDataEntry_t::IconLookup_t (32B, owning: 2 DynamicStrings)
    MK_MonsterDataEntry = 43,    // MonsterData_t::MonsterDataEntry_t (200B, owning: strings+maps+sets)
    MK_MonsterAllies = 44,       // MonsterAllyFormation_t::MonsterAllies_t (72B, owning: 2 i32 maps)
    MK_Dither = 45,              // Dither_t (8B POD; pointer-keyed dithering maps)
    MK_ChunkDither = 46,         // ChunkDither_t (8B POD; default value=10)
    MK_I32Map = 47,              // nested map<int,int> (32B Raw_Map, owning)
    MK_Bool = 48,                // bool (1B POD)
    MK_U32Map = 49,              // nested map<Uint32,Uint32> (32B Raw_Map, owning)
    MK_U32MapEmitterHit = 50,    // nested map<Uint32,map<Uint32,ParticleEmitterHit_t>> (8B POD inner)
    MK_StringPair = 51,          // pair<DynamicString,DynamicString> (32B, owning: 2 strings)
    MK_EntityColliderData = 52,  // EditorEntityData_t::EntityColliderData_t (owning: strings+arrays+maps+sets)
    MK_SpellItem = 53,           // ItemTooltips_t::spellItem_t (owning: strings+arrays+sets)
    MK_ItemTooltip = 54,         // ItemTooltips_t::ItemTooltip_t (owning: icons+strings+maps)
};

// value_kind_of<V> — compile-time kind for the shim's value_kind arg.
template <typename V> struct MapValueKindOf { static constexpr int value = MK_I32; };
template <> struct MapValueKindOf<int32_t> { static constexpr int value = MK_I32; };
template <> struct MapValueKindOf<float> { static constexpr int value = MK_F32; };
template <> struct MapValueKindOf<uint32_t> { static constexpr int value = MK_U32; };
template <> struct MapValueKindOf<uint64_t> { static constexpr int value = MK_U64; };
template <> struct MapValueKindOf<DynamicString> { static constexpr int value = MK_String; };
template <> struct MapValueKindOf<IconEntryTextMap_t> { static constexpr int value = MK_IconEntryTextMap; };
template <> struct MapValueKindOf<IconEntryText_tMirror> { static constexpr int value = MK_IconEntryText; };
template <> struct MapValueKindOf<WorldIconEntry_tMirror> { static constexpr int value = MK_WorldIconEntry; };
template <> struct MapValueKindOf<DiscoveryAnim_tMirror> { static constexpr int value = MK_DiscoveryAnim; };
template <> struct MapValueKindOf<SpecialNPCEntry_tMirror> { static constexpr int value = MK_SpecialNPC; };
template <> struct MapValueKindOf<ColliderDmgProperties_tMirror> { static constexpr int value = MK_ColliderDmg; };
template <> struct MapValueKindOf<ItemLocalization_tMirror> { static constexpr int value = MK_ItemLoc; };
template <> struct MapValueKindOf<Achievement_tMirror> { static constexpr int value = MK_Achievement; };
template <> struct MapValueKindOf<AchievementData_tMirror> { static constexpr int value = MK_AchievementData; };
template <> struct MapValueKindOf<binding_tMirror> { static constexpr int value = MK_Binding; };
template <> struct MapValueKindOf<Class_tMirror> { static constexpr int value = MK_Class; };
template <> struct MapValueKindOf<Dither_t> { static constexpr int value = MK_Dither; };
template <> struct MapValueKindOf<ChunkDither_t> { static constexpr int value = MK_ChunkDither; };
template <> struct MapValueKindOf<DynamicStringPair_t> { static constexpr int value = MK_StringPair; };
template <> struct MapValueKindOf<DynamicArrayStr> { static constexpr int value = MK_DynArrayStr; };
template <> struct MapValueKindOf<DynamicArrayS32> { static constexpr int value = MK_DynArrayS32; };
template <> struct MapValueKindOf<DynamicSetI32> { static constexpr int value = MK_SetOfI32; };
template <> struct MapValueKindOf<bool> { static constexpr int value = MK_Bool; };

// 8-byte pointer values (Frame*, node_t*, image_t*, struct pointers) are POD in a u64 slot.
template <typename T> struct MapValueKindOf<T*> { static constexpr int value = MK_Ptr; };
template <> struct MapValueKindOf<double> { static constexpr int value = MK_F64; };

// Entry value type: string maps expose const char* (view into map storage);
// other maps expose V by value.
template <typename V> struct MapEntryValue { using type = V; };
template <> struct MapEntryValue<DynamicString> { using type = DynamicString; };

// Zero-copy iteration trampoline. The Odin for_each shims walk live map
// entries and call a C-ABI callback with (key, key_len, live value ptr,
// userdata). The trampoline forwards to a typed std::function-style sink.
namespace barony_map_detail {
struct ForeachSink {
    virtual ~ForeachSink() = default;
    virtual void call(const void* key, int32_t key_len, void* value) = 0;
};
inline void foreachTrampoline(const void* key, int32_t key_len, void* value, void* userdata) {
    static_cast<ForeachSink*>(userdata)->call(key, key_len, value);
}
}

// ---------------------------------------------------------------------------
// DynamicMapStrT<V> — std::map<string,V> replacement (string keys).
// ---------------------------------------------------------------------------
template <typename V>
class DynamicMapStrT {
public:
    DynamicMapRaw raw{};

    DynamicMapStrT() { barony_dynamic_map_str_init(&raw, MapValueKindOf<V>::value); }
    ~DynamicMapStrT() { barony_dynamic_map_str_destroy(&raw, MapValueKindOf<V>::value); }
    DynamicMapStrT(const DynamicMapStrT& other) : raw{} {
        barony_dynamic_map_str_init(&raw, MapValueKindOf<V>::value);
        copyFrom(other);
    }
    DynamicMapStrT& operator=(const DynamicMapStrT& other) {
        if (this != &other) { barony_dynamic_map_str_clear(&raw, MapValueKindOf<V>::value); copyFrom(other); }
        return *this;
    }
    DynamicMapStrT(DynamicMapStrT&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapStrT& operator=(DynamicMapStrT&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_str_destroy(&raw, MapValueKindOf<V>::value);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    // ---- get/put (get/put-family maps) ----
    bool get(const char* key, V& out) const {
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &out, MapValueKindOf<V>::value);
    }
    bool get(const DynamicString& key, V& out) const {
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), key, &out, MapValueKindOf<V>::value);
    }
    bool get(const std::string& key, V& out) const {
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &out, MapValueKindOf<V>::value);
    }
    void put(const char* key, const V& v) {
        barony_dynamic_map_str_put(&raw, DynamicString(key), const_cast<V*>(&v), MapValueKindOf<V>::value);
    }
    void put(const DynamicString& key, const V& v) {
        barony_dynamic_map_str_put(&raw, key, const_cast<V*>(&v), MapValueKindOf<V>::value);
    }
    void put(const std::string& key, const V& v) {
        barony_dynamic_map_str_put(&raw, DynamicString(key.c_str()), const_cast<V*>(&v), MapValueKindOf<V>::value);
    }

    // at(): return the value for key (deep copy by value; default if missing)
    V at(const char* key) const {
        V v{};
        barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v, MapValueKindOf<V>::value);
        return v;
    }
    V at(const DynamicString& key) const {
        V v{};
        barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), key, &v, MapValueKindOf<V>::value);
        return v;
    }
    V at(const std::string& key) const {
        V v{};
        barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v, MapValueKindOf<V>::value);
        return v;
    }

    // ---- find/operator[] (find-family maps) ----
    struct KV { const char* first; int64_t first_len; V second; };
    struct Iterator {
        std::shared_ptr<std::vector<KV>> snap;   // snapshot for begin(); null for find()
        KV kv{};                                  // single result for find()
        bool valid = false;                       // find() result validity
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const KV* operator->() const { return snap ? &(*snap)[idx] : &kv; }
        const KV& operator*() const { return snap ? (*snap)[idx] : kv; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            if (!o.snap) return snap ? idx < snap->size() : valid;      // o is find-end / end()
            if (!snap) return o.snap ? o.idx < o.snap->size() : false;
            if (snap.get() != o.snap.get()) return !(idx >= snap->size() && o.idx == END);
            return idx != o.idx;
        }
        bool operator==(const Iterator& o) const { return !(*this != o); }
    };
    Iterator find(const char* key) const {
        Iterator it;
        void* kp = nullptr; int32_t kl = 0;
        if (barony_dynamic_map_str_find(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &kp, &kl, &it.kv.second, MapValueKindOf<V>::value)) {
            it.kv.first = (const char*)kp;
            it.kv.first_len = kl;
            it.valid = true;
        }
        return it;
    }
    Iterator find(const std::string& key) const { return find(key.c_str()); }
    Iterator end() const { Iterator it; it.valid = false; it.idx = Iterator::END; return it; }

    V& operator[](const char* key) {
        return *reinterpret_cast<V*>(barony_dynamic_map_str_entry(&raw, DynamicString(key), MapValueKindOf<V>::value));
    }
    V& operator[](const DynamicString& key) {
        return *reinterpret_cast<V*>(barony_dynamic_map_str_entry(&raw, key, MapValueKindOf<V>::value));
    }
    V& operator[](const std::string& key) {
        return *reinterpret_cast<V*>(barony_dynamic_map_str_entry(&raw, DynamicString(key.c_str()), MapValueKindOf<V>::value));
    }

    // ---- common ----
    bool contains(const char* key) const {
        V v{};
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key), &v, MapValueKindOf<V>::value);
    }
    bool contains(const DynamicString& key) const {
        V v{};
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), key, &v, MapValueKindOf<V>::value);
    }
    bool contains(const std::string& key) const {
        V v{};
        return barony_dynamic_map_str_get(const_cast<DynamicMapRaw*>(&raw), DynamicString(key.c_str()), &v, MapValueKindOf<V>::value);
    }
    int64_t size() const { return barony_dynamic_map_str_len(const_cast<DynamicMapRaw*>(&raw), MapValueKindOf<V>::value); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_str_clear(&raw, MapValueKindOf<V>::value); }
    bool erase(const char* key) { return barony_dynamic_map_str_erase(&raw, DynamicString(key), MapValueKindOf<V>::value); }
    bool erase(const DynamicString& key) { return barony_dynamic_map_str_erase(&raw, key, MapValueKindOf<V>::value); }
    bool erase(const std::string& key) { return barony_dynamic_map_str_erase(&raw, DynamicString(key.c_str()), MapValueKindOf<V>::value); }

    // ---- iteration ----
    struct Entry { const char* key; int64_t key_len; typename MapEntryValue<V>::type value; int64_t value_len; };
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = (const char*)kp[i];
            out[i].key_len = kl[i];
            out[i].value = std::move(vv[i]);
            if constexpr (std::is_same_v<V, DynamicString>) {
                out[i].value_len = out[i].value.len;
            } else {
                out[i].value_len = 0;
            }
        }
        return got;
    }

    // keys() (binding maps): collect interned keys
    void keys(std::vector<const char*>& out) const {
        out.clear();
        int32_t n = (int32_t)size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) out.push_back((const char*)kp[i]);
    }

    // Zero-copy iteration over live entries. The callback receives
    // (std::string_view key, V& value) where value is the LIVE map slot.
    // Do not insert/erase entries during the callback (mutating the value is fine).
    template <typename F>
    void forEach(F&& f) const {
        struct Sink : barony_map_detail::ForeachSink {
            F& f;
            explicit Sink(F& f_) : f(f_) {}
            void call(const void* key, int32_t key_len, void* value) override {
                f(std::string_view((const char*)key, (size_t)key_len), *reinterpret_cast<V*>(value));
            }
        } sink{ f };
        barony_dynamic_map_str_for_each(const_cast<DynamicMapRaw*>(&raw), MapValueKindOf<V>::value,
            (void*)&barony_map_detail::foreachTrampoline, &sink);
    }

    Iterator begin() const {
        Iterator it;
        int32_t n = (int32_t)size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<KV>>();
            it.snap->resize((size_t)n);
            std::vector<void*> kp(n);
            std::vector<int32_t> kl(n);
            std::vector<V> vv(n);
            int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), kl.data(), vv.data(), n, MapValueKindOf<V>::value);
            for (int32_t i = 0; i < got; ++i) {
                (*it.snap)[i].first = (const char*)kp[i];
                (*it.snap)[i].first_len = kl[i];
                (*it.snap)[i].second = vv[i];
            }
        }
        return it;
    }

private:
    void copyFrom(const DynamicMapStrT& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<int32_t> kl(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_str_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), kl.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) {
            DynamicString key((const char*)kp[i], kl[i]);
            put(key, vv[i]);
        }
    }
};

// ---------------------------------------------------------------------------
// DynamicMapI32T<V> — std::map<int,V> replacement ([4]byte keys).
// ---------------------------------------------------------------------------
template <typename V>
class DynamicMapI32T {
public:
    DynamicMapRaw raw{};

    DynamicMapI32T() { barony_dynamic_map_i32_init(&raw, MapValueKindOf<V>::value); }
    ~DynamicMapI32T() { barony_dynamic_map_i32_destroy(&raw, MapValueKindOf<V>::value); }
    DynamicMapI32T(const DynamicMapI32T& other) : raw{} {
        barony_dynamic_map_i32_init(&raw, MapValueKindOf<V>::value);
        copyFrom(other);
    }
    DynamicMapI32T& operator=(const DynamicMapI32T& other) {
        if (this != &other) { barony_dynamic_map_i32_clear(&raw, MapValueKindOf<V>::value); copyFrom(other); }
        return *this;
    }
    DynamicMapI32T(DynamicMapI32T&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapI32T& operator=(DynamicMapI32T&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_i32_destroy(&raw, MapValueKindOf<V>::value);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    V& operator[](int key) {
        return *reinterpret_cast<V*>(barony_dynamic_map_i32_entry(&raw, &key, MapValueKindOf<V>::value));
    }
    V at(int key) const {
        V v{};
        barony_dynamic_map_i32_get(const_cast<DynamicMapRaw*>(&raw), &key, &v, MapValueKindOf<V>::value);
        return v;
    }
    bool contains(int key) const {
        V v{};
        return barony_dynamic_map_i32_get(const_cast<DynamicMapRaw*>(&raw), &key, &v, MapValueKindOf<V>::value);
    }
    bool get(int key, V& out) const {
        return barony_dynamic_map_i32_get(const_cast<DynamicMapRaw*>(&raw), &key, &out, MapValueKindOf<V>::value);
    }
    void put(int key, const V& v) {
        barony_dynamic_map_i32_put(&raw, &key, const_cast<V*>(&v), MapValueKindOf<V>::value);
    }
    int64_t size() const { return barony_dynamic_map_i32_len(const_cast<DynamicMapRaw*>(&raw), MapValueKindOf<V>::value); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_i32_clear(&raw, MapValueKindOf<V>::value); }
    bool erase(int key) { return barony_dynamic_map_i32_erase(&raw, &key, MapValueKindOf<V>::value); }

    // find-family
    struct KV { int first; int64_t first_len; V second; };
    struct Iterator {
        std::shared_ptr<std::vector<KV>> snap;   // snapshot for begin(); null for find()
        KV kv{};                                  // single result for find()
        bool valid = false;
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const KV* operator->() const { return snap ? &(*snap)[idx] : &kv; }
        const KV& operator*() const { return snap ? (*snap)[idx] : kv; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            if (!o.snap) return snap ? idx < snap->size() : valid;
            if (!snap) return o.snap ? o.idx < o.snap->size() : false;
            if (snap.get() != o.snap.get()) return !(idx >= snap->size() && o.idx == END);
            return idx != o.idx;
        }
        bool operator==(const Iterator& o) const { return !(*this != o); }
    };
    Iterator begin() const {
        Iterator it;
        int32_t n = (int32_t)size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<KV>>();
            it.snap->resize((size_t)n);
            std::vector<int> kp(n);
            std::vector<V> vv(n);
            int32_t got = barony_dynamic_map_i32_entries(const_cast<DynamicMapRaw*>(&raw), (void**)kp.data(), vv.data(), n, MapValueKindOf<V>::value);
            for (int32_t i = 0; i < got; ++i) {
                (*it.snap)[i].first = kp[i];
                (*it.snap)[i].first_len = 4;
                (*it.snap)[i].second = vv[i];
            }
        }
        return it;
    }
    Iterator find(int key) const {
        Iterator it;
        int32_t vl = 0;
        if (barony_dynamic_map_i32_find(const_cast<DynamicMapRaw*>(&raw), &key, &it.kv.second, &vl, MapValueKindOf<V>::value)) {
            it.kv.first = key;
            it.valid = true;
        }
        return it;
    }
    Iterator end() const { Iterator it; it.valid = false; it.idx = Iterator::END; return it; }

    // entryList
    struct Entry { int key; int64_t key_len; typename MapEntryValue<V>::type value; int64_t value_len; };
    int32_t entryList(Entry* out, int32_t max) const {
        int32_t n = (int32_t)size();
        if (n > max) n = max;
        if (n <= 0) return 0;
        std::vector<int> kp(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_i32_entries(const_cast<DynamicMapRaw*>(&raw), (void**)kp.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) {
            out[i].key = kp[i];
            out[i].key_len = 4;
            out[i].value = std::move(vv[i]);
            if constexpr (std::is_same_v<V, DynamicString>) {
                out[i].value_len = out[i].value.len;
            } else {
                out[i].value_len = 0;
            }
        }
        return got;
    }

    // Zero-copy iteration over live entries. The callback receives
    // (int key, V& value) where value is the LIVE map slot.
    // Do not insert/erase entries during the callback (mutating the value is fine).
    template <typename F>
    void forEach(F&& f) const {
        struct Sink : barony_map_detail::ForeachSink {
            F& f;
            explicit Sink(F& f_) : f(f_) {}
            void call(const void* key, int32_t, void* value) override {
                f(*(const int*)key, *reinterpret_cast<V*>(value));
            }
        } sink{ f };
        barony_dynamic_map_i32_for_each(const_cast<DynamicMapRaw*>(&raw), MapValueKindOf<V>::value,
            (void*)&barony_map_detail::foreachTrampoline, &sink);
    }

private:
    void copyFrom(const DynamicMapI32T& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<int> kp(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_i32_entries(const_cast<DynamicMapRaw*>(&other.raw), (void**)kp.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) put(kp[i], vv[i]);
    }
};

// ---------------------------------------------------------------------------
// DynamicMapPtrT<V> — std::unordered_map<KeyT, V> replacement with rawptr keys
// (pointer-keyed maps: unordered_map<view_t*, Dither>, etc.). The key type is
// the pointer itself (non-owning); Odin hashes the address. C++ passes the key
// by value as a rawptr (8 bytes on x64, matches a pointer).
// ---------------------------------------------------------------------------
template <typename V>
class DynamicMapPtrT {
public:
    DynamicMapRaw raw{};

    DynamicMapPtrT() { barony_dynamic_map_ptr_init(&raw, MapValueKindOf<V>::value); }
    ~DynamicMapPtrT() { barony_dynamic_map_ptr_destroy(&raw, MapValueKindOf<V>::value); }
    DynamicMapPtrT(const DynamicMapPtrT& other) : raw{} {
        barony_dynamic_map_ptr_init(&raw, MapValueKindOf<V>::value);
        copyFrom(other);
    }
    DynamicMapPtrT& operator=(const DynamicMapPtrT& other) {
        if (this != &other) { barony_dynamic_map_ptr_clear(&raw, MapValueKindOf<V>::value); copyFrom(other); }
        return *this;
    }
    DynamicMapPtrT(DynamicMapPtrT&& other) noexcept : raw(other.raw) {
        other.raw = DynamicMapRaw{};
    }
    DynamicMapPtrT& operator=(DynamicMapPtrT&& other) noexcept {
        if (this != &other) {
            barony_dynamic_map_ptr_destroy(&raw, MapValueKindOf<V>::value);
            raw = other.raw;
            other.raw = DynamicMapRaw{};
        }
        return *this;
    }

    template <typename K>
    V& operator[](K key) {
        return *reinterpret_cast<V*>(barony_dynamic_map_ptr_entry(&raw, (const void*)key, MapValueKindOf<V>::value));
    }
    template <typename K>
    bool contains(K key) const {
        V v{};
        return barony_dynamic_map_ptr_get(const_cast<DynamicMapRaw*>(&raw), (const void*)key, &v, MapValueKindOf<V>::value);
    }
    template <typename K>
    bool get(K key, V& out) const {
        return barony_dynamic_map_ptr_get(const_cast<DynamicMapRaw*>(&raw), (const void*)key, &out, MapValueKindOf<V>::value);
    }
    template <typename K>
    void put(K key, const V& v) {
        barony_dynamic_map_ptr_put(&raw, (const void*)key, const_cast<V*>(&v), MapValueKindOf<V>::value);
    }
    int64_t size() const { return barony_dynamic_map_ptr_len(const_cast<DynamicMapRaw*>(&raw), MapValueKindOf<V>::value); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_map_ptr_clear(&raw, MapValueKindOf<V>::value); }
    template <typename K>
    bool erase(K key) { return barony_dynamic_map_ptr_erase(&raw, (const void*)key, MapValueKindOf<V>::value); }

    // swap (used by Chunk move ctor/assign: `dithering.swap(rhs.dithering)`)
    void swap(DynamicMapPtrT& other) noexcept {
        DynamicMapRaw tmp = raw;
        raw = other.raw;
        other.raw = tmp;
    }

    // find-family (snapshot iterator, std::unordered_map-like)
    struct KV { const void* first; V second; };
    struct Iterator {
        std::shared_ptr<std::vector<KV>> snap;   // snapshot for begin(); null for find()
        KV kv{};                                  // single result for find()
        bool valid = false;
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const KV* operator->() const { return snap ? &(*snap)[idx] : &kv; }
        const KV& operator*() const { return snap ? (*snap)[idx] : kv; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        bool operator!=(const Iterator& o) const {
            if (!o.snap) return snap ? idx < snap->size() : valid;
            if (!snap) return o.snap ? o.idx < o.snap->size() : false;
            if (snap.get() != o.snap.get()) return !(idx >= snap->size() && o.idx == END);
            return idx != o.idx;
        }
        bool operator==(const Iterator& o) const { return !(*this != o); }
    };
    template <typename K>
    Iterator find(K key) const {
        Iterator it;
        V v{};
        if (barony_dynamic_map_ptr_get(const_cast<DynamicMapRaw*>(&raw), (const void*)key, &v, MapValueKindOf<V>::value)) {
            it.kv.first = (const void*)key;
            it.kv.second = v;
            it.valid = true;
        }
        return it;
    }
    Iterator end() const { Iterator it; it.valid = false; it.idx = Iterator::END; return it; }

    Iterator begin() const {
        Iterator it;
        int32_t n = (int32_t)size();
        if (n > 0) {
            it.snap = std::make_shared<std::vector<KV>>();
            it.snap->resize((size_t)n);
            std::vector<void*> kp(n);
            std::vector<V> vv(n);
            int32_t got = barony_dynamic_map_ptr_entries(const_cast<DynamicMapRaw*>(&raw), kp.data(), vv.data(), n, MapValueKindOf<V>::value);
            for (int32_t i = 0; i < got; ++i) {
                (*it.snap)[i].first = kp[i];
                (*it.snap)[i].second = vv[i];
            }
        }
        return it;
    }

private:
    void copyFrom(const DynamicMapPtrT& other) {
        int32_t n = (int32_t)other.size();
        if (n <= 0) return;
        std::vector<void*> kp(n);
        std::vector<V> vv(n);
        int32_t got = barony_dynamic_map_ptr_entries(const_cast<DynamicMapRaw*>(&other.raw), kp.data(), vv.data(), n, MapValueKindOf<V>::value);
        for (int32_t i = 0; i < got; ++i) put(kp[i], vv[i]);
    }
};

// ---- typedefs preserve the pre-consolidation class names (game code untouched) ----
using DynamicMapI32 = DynamicMapStrT<int32_t>;                    // map<string,int32_t>
using DynamicMapStr = DynamicMapStrT<DynamicString>;              // map<string,string>
using DynamicMapF32 = DynamicMapStrT<float>;                      // map<string,float>
using DynamicMapIconEntryTextMap = DynamicMapStrT<IconEntryTextMap_t>;
using DynamicMapIconEntryText = DynamicMapStrT<IconEntryText_tMirror>;
using DynamicMapWorldIconEntry = DynamicMapStrT<WorldIconEntry_tMirror>;
using DynamicMapDiscoveryAnim = DynamicMapStrT<DiscoveryAnim_tMirror>;
using DynamicMapSpecialNPC = DynamicMapStrT<SpecialNPCEntry_tMirror>;
using DynamicMapColliderDmg = DynamicMapStrT<ColliderDmgProperties_tMirror>;
using DynamicMapItemLoc = DynamicMapStrT<ItemLocalization_tMirror>;
using DynamicMapAchievement = DynamicMapStrT<Achievement_tMirror>;
using DynamicMapAchievementData = DynamicMapStrT<AchievementData_tMirror>;
using DynamicMapBinding = DynamicMapStrT<binding_tMirror>;
using DynamicMapClass = DynamicMapStrT<Class_tMirror>;
using DynamicMapStrArrStr = DynamicMapStrT<DynamicArrayStr>;       // map<string, vector<string>>
using DynamicMapI32Str = DynamicMapI32T<DynamicString>;           // map<int,string>
using DynamicMapStringPair = DynamicMapStrT<DynamicStringPair_t>;  // map<string, pair<string,string>>
using DynamicMapStrI32Map = DynamicMapStrT<DynamicMapI32T<int>>;   // map<string, map<int,int>>

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
template <> struct MapValueKindOf<IconEntry_tMirror> { static constexpr int value = MK_IconEntry; };
template <> struct MapValueKindOf<IconEntryCallout_tMirror> { static constexpr int value = MK_IconEntryCallout; };
using DynamicMapIconEntryList = DynamicMapStrT<IconEntry_tMirror>;
using DynamicMapIconEntryCallout = DynamicMapStrT<IconEntryCallout_tMirror>;

// map<int, map<int,int>>: the nested inner map as a value kind. DynamicMapI32T<int>
// serves as the inner-map value (Raw_Map 32B == Odin map[[4]byte]i32).
// CRITICAL: MapValueKindOf<DynamicMapI32T<int>> = MK_I32Map is what makes the
// OUTER map treat the 32B inner map as an owned nested map. The inner map's own
// method calls dispatch on MapValueKindOf<int> (MK_I32), never on this
// specialization, so a standalone int->int map (spellTomeIDToAppearance) is
// unaffected.
template <> struct MapValueKindOf<DynamicMapI32T<int>> { static constexpr int value = MK_I32Map; };
using DynamicMapI32Map = DynamicMapI32T<DynamicMapI32T<int>>;

// map<Uint32, map<Uint32, Uint32>>: nested u32->u32 inner map as a value kind.
template <> struct MapValueKindOf<DynamicMapI32T<uint32_t>> { static constexpr int value = MK_U32Map; };
using DynamicMapU32Map = DynamicMapI32T<DynamicMapI32T<uint32_t>>;


