// dynamic_array.hpp — C++ mirror of Odin's Raw_Dynamic_Array (40 bytes on x64)

// + inline helpers that call the Odin containers shims.

//

// Replaces std::vector<T> members in shared structs so C++ and Odin operate

// on the same memory layout. Growth is Odin-owned (the shims); C++ only reads

// .data/.len/.cap and calls the shims to mutate.

#pragma once
#ifndef Uint32
typedef unsigned int Uint32;
#endif

#include "dynamic_string.hpp"

#include <cstdint>

#include <cstddef>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>



// 40 bytes on x64 — matches Odin Raw_Dynamic_Array exactly

struct DynamicArray {

    void*   data = nullptr;   // element bytes
    int64_t len = 0;          // bytes (not elements!)
    int64_t cap = 0;          // bytes
    void*   alloc_proc = nullptr; // Odin Allocator.procedure
    void*   alloc_data = nullptr; // Odin Allocator.data

};



struct ItemTooltipIcons_tMirror {
    DynamicString iconPath;
    DynamicString text;
    uint32_t textColor = 0xFFFFFFFF;
    DynamicString conditionalAttribute;
    void setColor(Uint32 c) { textColor = c; }
    void setConditionalAttribute(const std::string& attr) { conditionalAttribute = attr.c_str(); }
    ItemTooltipIcons_tMirror() = default;
    ItemTooltipIcons_tMirror(const std::string& _path, const std::string& _text) {
        iconPath = _path.c_str();
        text = _text.c_str();
    }
};

struct DropdownOption_tMirror {
    DynamicString text;
    DynamicString keyboardGlyph;
    DynamicString controllerGlyph;
    DynamicString action;
    DropdownOption_tMirror() = default;
    DropdownOption_tMirror(const std::string& _text, const std::string& _keyboardGlyph,
                           const std::string& _controllerGlyph, const std::string& _action) {
        text = _text.c_str();
        keyboardGlyph = _keyboardGlyph.c_str();
        controllerGlyph = _controllerGlyph.c_str();
        action = _action.c_str();
    }
};


struct EntryVariable_tMirror {
    int type = 0;
    DynamicString value;
    int numericValue = 0;
    int sizex = 0;
    int sizey = 0;
};


extern "C" {

    void    barony_dynamic_array_init(DynamicArray*);

    int32_t barony_dynamic_array_append(DynamicArray*, const void* elem, int64_t elem_size);

    void    barony_dynamic_array_erase(DynamicArray*, int32_t index, int64_t elem_size);

    int32_t barony_dynamic_array_insert(DynamicArray*, int32_t index, const void*, int64_t);

    void    barony_dynamic_array_clear(DynamicArray*);

    int32_t barony_dynamic_array_resize(DynamicArray*, int64_t elem_size, int32_t new_len);

    void    barony_dynamic_array_pop_back(DynamicArray*, int64_t elem_size);

    void    barony_dynamic_array_destroy(DynamicArray*);

    int32_t barony_dynamic_array_copy(DynamicArray* dst, const DynamicArray* src);



        // GENERIC element-aware array (elem_size + value_kind dispatch).
    // value_kind: 0=POD(raw bytes) 1=DynamicString 2=Icon 3=Option 4=EntryVar
    void    barony_dynamic_array_elem_init(DynamicArray*);
    int32_t barony_dynamic_array_elem_append(DynamicArray*, const void* elem, int64_t elem_size, int32_t value_kind);
    bool    barony_dynamic_array_elem_get(DynamicArray*, int32_t index, void* out, int64_t elem_size, int32_t value_kind);
    void    barony_dynamic_array_elem_set(DynamicArray*, int32_t index, const void* elem, int64_t elem_size, int32_t value_kind);
    void    barony_dynamic_array_elem_erase(DynamicArray*, int32_t index, int64_t elem_size, int32_t value_kind);
    void    barony_dynamic_array_elem_clear(DynamicArray*, int64_t elem_size, int32_t value_kind);
    void    barony_dynamic_array_elem_destroy(DynamicArray*, int64_t elem_size, int32_t value_kind);
    int32_t barony_dynamic_array_elem_len(DynamicArray*, int64_t elem_size);
    void    barony_dynamic_array_elem_copy(DynamicArray* dst, DynamicArray* src, int64_t elem_size, int32_t value_kind);
    int32_t barony_dynamic_array_elem_entries(DynamicArray*, void* val_ptrs, int32_t count, int64_t elem_size, int32_t value_kind);
}



// typed helpers (hide void* casting)

template <typename T>

inline void dynarray_push(DynamicArray& a, const T& v) {

    barony_dynamic_array_append(&a, &v, (int64_t)sizeof(T));

}

template <typename T>

inline T* dynarray_at(DynamicArray& a, int64_t i) {

    return reinterpret_cast<T*>(a.data) + i;

}

template <typename T>

inline const T* dynarray_at(const DynamicArray& a, int64_t i) {

    return reinterpret_cast<const T*>(a.data) + i;

}

template <typename T>

inline int64_t dynarray_size(const DynamicArray& a) {

    return a.len / (int64_t)sizeof(T);

}

// pointer-element helpers (non-owning pointer arrays like vector<Entity*>)
template <typename T>
inline T dynarray_pget(const DynamicArray& a, int64_t i) { T* p = (T*)a.data; return p[i]; }
template <typename T>
inline T*& dynarray_pget_ref(DynamicArray& a, int64_t i) { T* p = (T*)a.data; return p[i]; }
template <typename T>
inline int64_t dynarray_psize(const DynamicArray& a) { return a.len / (int64_t)sizeof(T*); }



// ---------------------------------------------------------------------------

// DynamicArrayStr — std::vector<DynamicString> replacement. Elements are

// DynamicString (16B each) — the array OWNS their buffers. All mutations go

// through the str shims (deep-free on erase/clear/destroy/pop, deep-copy on

// append/set/copy).


// ---------------------------------------------------------------------------
// DynamicArray<T> — single template replacement for std::vector<T> in shared
// structs. The element type T determines:
//   - kind_of<T>: which Element_Ops (free/copy) the Odin side applies
//   - sizeof(T):  the stride for byte walking
// POD types (int32_t, uint32_t, pointers, pairs) are kind 0 (raw bytes).
// Owning types (DynamicString, ItemTooltipIcons_tMirror, ...) deep-free/
// deep-copy via the Odin elem_* shims. This replaces the per-type families
// (DynamicArrayStr/S32/U32/Icon/Option/EntryVar) — the typedefs below keep
// the old names so game code is untouched.
// ---------------------------------------------------------------------------

// element kinds (must match Kind_* in dynamic_array.odin)
enum DynamicArrayKind {
    Kind_POD = 0,
    Kind_DynamicString = 1,
    Kind_Icon = 2,
    Kind_Option = 3,
    Kind_EntryVar = 4,
    Kind_FollowerDetails = 5,
    Kind_LevelT = 6,
    Kind_CodexItem = 7,
    Kind_ShopkeeperItem = 8,
    Kind_VariantPair = 9,
    Kind_MonsterCurveEntry = 10,
    Kind_LevelCurve = 11,
    Kind_TmpItem = 12,
    Kind_MapGeneration = 13,
    Kind_HotbarEntry = 14,
    Kind_HotbarEntryArray = 15,
    Kind_DynArrayStrArray = 16,
    Kind_SkillEffect = 17,
    Kind_SkillEntry = 18,
    Kind_PanelEntry = 19,
    Kind_AssistNotifPair = 20,
    Kind_AlchNotifPair = 21,
    Kind_CalloutPanel = 22,
    Kind_HiscoreStat = 23,
    Kind_HiscoreLootbag = 24,
    Kind_HiscorePlayer = 25,
    Kind_Book = 26,
    Kind_StoreSlots = 27,
    Kind_StatusEffectQueueEntry = 28,
    Kind_HiscoreAttributesPair = 29,
    Kind_HiscorePlayerEquipPair = 30,
    Kind_HiscoreNpcEquipPair = 31,
    Kind_HiscoreLootbagPair = 32,
    Kind_HiscoreCompendiumPair = 33,
    Kind_FollowerBarPair = 34,
    Kind_StringPair = 35,
    Kind_SurfacePtrStringPair = 36,
    Kind_MonsterStringPair = 37,
    Kind_SaveGameListEntry = 38,
};

template <typename T> struct DynamicArrayKindOf { static constexpr int value = Kind_POD; };
template <> struct DynamicArrayKindOf<DynamicString> { static constexpr int value = Kind_DynamicString; };
template <> struct DynamicArrayKindOf<ItemTooltipIcons_tMirror> { static constexpr int value = Kind_Icon; };
template <> struct DynamicArrayKindOf<DropdownOption_tMirror> { static constexpr int value = Kind_Option; };
template <> struct DynamicArrayKindOf<EntryVariable_tMirror> { static constexpr int value = Kind_EntryVar; };

template <typename T>
class DynamicArrayT {
public:
    DynamicArray raw{};

    DynamicArrayT() { barony_dynamic_array_elem_init(&raw); }
    DynamicArrayT(int64_t count, const T& value) {
        barony_dynamic_array_elem_init(&raw);
        for (int64_t i = 0; i < count; ++i) push_back(value);
    }
    DynamicArrayT(std::initializer_list<T> init) {
        barony_dynamic_array_elem_init(&raw);
        for (const T& v : init) push_back(v);
    }
    ~DynamicArrayT() { barony_dynamic_array_elem_destroy(&raw, sizeof(T), DynamicArrayKindOf<T>::value); }
    DynamicArrayT(const DynamicArrayT& other) : raw{} {
        barony_dynamic_array_elem_init(&raw);
        *this = other;
    }
    DynamicArrayT& operator=(const DynamicArrayT& other) {
        if (this != &other) { barony_dynamic_array_elem_copy(&raw, const_cast<DynamicArray*>(&other.raw), sizeof(T), DynamicArrayKindOf<T>::value); }
        return *this;
    }
    DynamicArrayT(DynamicArrayT&& other) noexcept : raw(other.raw) {
        other.raw = DynamicArray{};
    }
    DynamicArrayT& operator=(DynamicArrayT&& other) noexcept {
        if (this != &other) {
            barony_dynamic_array_elem_destroy(&raw, sizeof(T), DynamicArrayKindOf<T>::value);
            raw = other.raw;
            other.raw = DynamicArray{};
        }
        return *this;
    }

    void swap(DynamicArrayT& other) noexcept {
        DynamicArray tmp = raw;
        raw = other.raw;
        other.raw = tmp;
    }

    int64_t size() const { return barony_dynamic_array_elem_len(const_cast<DynamicArray*>(&raw), sizeof(T)); }
    bool empty() const { return size() == 0; }
    bool contains(const T& v) const {
        for (int64_t i = 0; i < size(); ++i) {
            if ((*this)[i] == v) return true;
        }
        return false;
    }
    T* data() { return (T*)raw.data; }
    const T* data() const { return (const T*)raw.data; }
    T& back() { return (*this)[size() - 1]; }
    const T& back() const { return (*this)[size() - 1]; }
    T& front() { return (*this)[0]; }
    const T& front() const { return (*this)[0]; }
    void clear() { barony_dynamic_array_elem_clear(&raw, sizeof(T), DynamicArrayKindOf<T>::value); }
    void reserve(int64_t n) { if (raw.cap < n * (int64_t)sizeof(T)) { barony_dynamic_array_elem_append(&raw, nullptr, 0, DynamicArrayKindOf<T>::value); } }
    void resize(int64_t n) {
        int64_t cur = size();
        if (n > cur) { T zero{}; for (int64_t i = cur; i < n; ++i) push_back(zero); }
        else if (n < cur) { for (int64_t i = cur - 1; i >= n; --i) erase(i); }
    }

    void push_back(const T& v) { barony_dynamic_array_elem_append(&raw, &v, sizeof(T), DynamicArrayKindOf<T>::value); }
    void push_front(const T& v) { insert((T*)raw.data, v); }
    // insert at position (std::vector::insert single-element semantics)
    T* insert(T* pos, const T& v) {
        int64_t i = (int64_t)(pos - (T*)raw.data);
        // grow by one at the end, then shift the tail right
        barony_dynamic_array_elem_append(&raw, &v, sizeof(T), DynamicArrayKindOf<T>::value);
        int64_t n = size() - 1; // last element index (the appended v)
        if (i < n) {
            T* base = (T*)raw.data;
            for (int64_t k = n; k > i; --k) base[k] = base[k - 1];
            base[i] = v;
        }
        return (T*)raw.data + i;
    }
    // insert an initializer_list at position (std::vector::insert(pos, ilist) semantics)
    void insert(T* pos, std::initializer_list<T> ilist) {
        T* p = pos;
        for (const T& v : ilist) {
            p = insert(p, v) + 1;
        }
    }
    template <typename U = T, std::enable_if_t<std::is_same_v<U, DynamicString>, int> = 0>
    void push_back(const char* v) { T s(v); barony_dynamic_array_elem_append(&raw, &s, sizeof(T), DynamicArrayKindOf<T>::value); }
    template <typename U = T, std::enable_if_t<std::is_same_v<U, DynamicString>, int> = 0>
    void push_back(const std::string& v) { T s(v.c_str()); barony_dynamic_array_elem_append(&raw, &s, sizeof(T), DynamicArrayKindOf<T>::value); }

    void erase(int64_t i) { barony_dynamic_array_elem_erase(&raw, (int32_t)i, sizeof(T), DynamicArrayKindOf<T>::value); }
    // erase(iterator) -> returns pointer to the next element (std::vector::erase semantics)
    T* erase(T* it) {
        int64_t i = (int64_t)(it - (T*)raw.data);
        barony_dynamic_array_elem_erase(&raw, (int32_t)i, sizeof(T), DynamicArrayKindOf<T>::value);
        return (T*)raw.data + i;
    }
    void pop_back() { if (size() > 0) erase(size() - 1); }
    void pop_front() { if (size() > 0) erase((int64_t)0); }
    // sort (std::list::sort equivalent). Sorts in place using std::sort over
    // the live element slots. For DynamicString elements operator< is defined
    // (barony_dynamic_string_compare), so the default comparator works.
    void sort() { std::sort((T*)raw.data, (T*)raw.data + size()); }
    template <typename Cmp>
    void sort(Cmp cmp) { std::sort((T*)raw.data, (T*)raw.data + size(), cmp); }
    // remove consecutive duplicates (std::list::unique semantics)
    void unique() {
        int64_t n = size();
        if (n <= 1) return;
        T* base = (T*)raw.data;
        int64_t write = 1;
        for (int64_t i = 1; i < n; ++i) {
            if (!(base[i] == base[write - 1])) {
                if (write != i) base[write] = base[i];
                ++write;
            }
        }
        while (size() > write) pop_back();
    }
    // reverse iteration (std::deque/vector rbegin/rend)
    using reverse_iterator = std::reverse_iterator<T*>;
    using const_reverse_iterator = std::reverse_iterator<const T*>;
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    // at: LIVE slot reference (std::vector::at semantics — mutable). For
    // owned types the returned ref is the stored element (field assigns
    // free/alloc their own buffers correctly).
    T& at(int64_t i) { return reinterpret_cast<T*>(raw.data)[i]; }
    const T& at(int64_t i) const { return reinterpret_cast<const T*>(raw.data)[i]; }
    void set(int64_t i, const T& v) { barony_dynamic_array_elem_set(&raw, (int32_t)i, &v, sizeof(T), DynamicArrayKindOf<T>::value); }

    // operator[] — LIVE slot reference. For owned types this is the live
    // element (mutations persist), matching the live-slot iterator semantics.
    T& operator[](int64_t i) { return reinterpret_cast<T*>(raw.data)[i]; }
    const T& operator[](int64_t i) const { return reinterpret_cast<const T*>(raw.data)[i]; }
    // NOTE: operator[] returns the live slot. For owned types, reading via []
    // gives the stored element (not a copy). at() is the deep-copy read.

    // snapshot: deep-copy all elements into the caller's container
    // (works with both std::vector<T> and DynamicArrayT<T> out-params)
    template<typename OutT>
    void snapshot(OutT& out) const {
        int64_t n = size();
        out.clear();
        if (n <= 0) return;
        out.resize((size_t)n);
        barony_dynamic_array_elem_entries(const_cast<DynamicArray*>(&raw), out.data(), (int32_t)n, sizeof(T), DynamicArrayKindOf<T>::value);
    }

    // range-for: live-slot iterators (operator* = actual array slot, mutations
    // persist; valid until the array mutates).
    T* begin() { return (T*)raw.data; }
    T* end() { return (T*)raw.data + size(); }
    const T* begin() const { return (const T*)raw.data; }
    const T* end() const { return (const T*)raw.data + size(); }};

// ---- typedefs preserve the pre-consolidation names (game code untouched) ----
using DynamicArrayStr    = DynamicArrayT<DynamicString>;
using DynamicArrayS32    = DynamicArrayT<int32_t>;
using DynamicArrayU32    = DynamicArrayT<uint32_t>;
using DynamicArrayIcon   = DynamicArrayT<ItemTooltipIcons_tMirror>;
using DynamicArrayOption = DynamicArrayT<DropdownOption_tMirror>;
using DynamicArrayEntryVar = DynamicArrayT<EntryVariable_tMirror>;


// generic pair<T,U> element helpers (for pair-valued POD arrays)
template <typename P>
inline P* dynarray_pair_at(DynamicArray& a, int64_t i) { return reinterpret_cast<P*>(a.data) + i; }
template <typename P>
inline const P* dynarray_pair_at(const DynamicArray& a, int64_t i) { return reinterpret_cast<const P*>(a.data) + i; }
template <typename P>
inline int64_t dynarray_pair_size(const DynamicArray& a) { return a.len / (int64_t)sizeof(P); }
template <typename P>
inline void dynarray_pair_push(DynamicArray& a, const P& v) { barony_dynamic_array_append(&a, &v, (int64_t)sizeof(P)); }

// Deep-copy append for owning element types (pairs with DynamicString/nested
// array members). Uses the element kind's copy proc so the array owns
// independent memory and the source temporary can be destroyed safely.
template <typename T>
inline void dynarray_push_owned(DynamicArray& a, const T& v) {
    barony_dynamic_array_elem_append(&a, &v, (int64_t)sizeof(T), DynamicArrayKindOf<T>::value);
}

// pair<int,int> element helpers (8-byte POD pairs)
typedef std::pair<int, int> int_pair_t;
inline int_pair_t* dynarray_pair_at(DynamicArray& a, int64_t i) { return (int_pair_t*)a.data + i; }
inline const int_pair_t* dynarray_pair_at(const DynamicArray& a, int64_t i) { return (const int_pair_t*)a.data + i; }
inline int64_t dynarray_pair_size(const DynamicArray& a) { return a.len / (int64_t)sizeof(int_pair_t); }
inline void dynarray_pair_push(DynamicArray& a, const int_pair_t& v) { barony_dynamic_array_append(&a, &v, (int64_t)sizeof(int_pair_t)); }

// pair<Uint32,Uint32> element helpers (8-byte)
typedef std::pair<unsigned, unsigned> u32pair_t;
inline u32pair_t* dynarray_u32pair_at(DynamicArray& a, int64_t i) { return (u32pair_t*)a.data + i; }
inline const u32pair_t* dynarray_u32pair_at(const DynamicArray& a, int64_t i) { return (const u32pair_t*)a.data + i; }
inline int64_t dynarray_u32pair_size(const DynamicArray& a) { return a.len / (int64_t)sizeof(u32pair_t); }
inline void dynarray_u32pair_push(DynamicArray& a, const u32pair_t& v) { barony_dynamic_array_append(&a, &v, (int64_t)sizeof(u32pair_t)); }
