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



// 40 bytes on x64 — matches Odin Raw_Dynamic_Array exactly

struct DynamicArray {

    void*   data;          // element bytes

    int64_t len;           // bytes (not elements!)

    int64_t cap;           // bytes

    void*   alloc_proc;    // Odin Allocator.procedure

    void*   alloc_data;    // Odin Allocator.data

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

    int64_t size() const { return barony_dynamic_array_elem_len(const_cast<DynamicArray*>(&raw), sizeof(T)); }
    bool empty() const { return size() == 0; }
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
    template <typename U = T, std::enable_if_t<std::is_same_v<U, DynamicString>, int> = 0>
    void push_back(const char* v) { T s(v); barony_dynamic_array_elem_append(&raw, &s, sizeof(T), DynamicArrayKindOf<T>::value); }
    template <typename U = T, std::enable_if_t<std::is_same_v<U, DynamicString>, int> = 0>
    void push_back(const std::string& v) { T s(v.c_str()); barony_dynamic_array_elem_append(&raw, &s, sizeof(T), DynamicArrayKindOf<T>::value); }

    void erase(int64_t i) { barony_dynamic_array_elem_erase(&raw, (int32_t)i, sizeof(T), DynamicArrayKindOf<T>::value); }
    void pop_back() { if (size() > 0) erase(size() - 1); }

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

    // snapshot: deep-copy all elements into the caller's vector
    void snapshot(std::vector<T>& out) const {
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
