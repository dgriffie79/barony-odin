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



    // string-aware array (elements are DynamicString, deep-owned)

    void    barony_dynamic_array_str_init(DynamicArray*);

    void    barony_dynamic_array_str_append(DynamicArray*, DynamicString*);

    bool    barony_dynamic_array_str_get(DynamicArray*, int32_t, DynamicString*);

    void    barony_dynamic_array_str_set(DynamicArray*, int32_t, DynamicString*);

    void    barony_dynamic_array_str_erase(DynamicArray*, int32_t);

    void    barony_dynamic_array_str_clear(DynamicArray*);

    void    barony_dynamic_array_str_destroy(DynamicArray*);

    int32_t barony_dynamic_array_str_len(DynamicArray*);

    void    barony_dynamic_array_str_copy(DynamicArray* dst, DynamicArray* src);

    int32_t barony_dynamic_array_str_entries(DynamicArray*, DynamicString* val_ptrs, int32_t count);



    // ItemTooltipIcons_t array (3 DynamicStrings + u32, deep-owned)

    void    barony_dynamic_array_icon_init(DynamicArray*);

    void    barony_dynamic_array_icon_append(DynamicArray*, ItemTooltipIcons_tMirror*);

    bool    barony_dynamic_array_icon_get(DynamicArray*, int32_t, ItemTooltipIcons_tMirror*);

    void    barony_dynamic_array_icon_set(DynamicArray*, int32_t, ItemTooltipIcons_tMirror*);

    void    barony_dynamic_array_icon_erase(DynamicArray*, int32_t);

    void    barony_dynamic_array_icon_clear(DynamicArray*);

    void    barony_dynamic_array_icon_destroy(DynamicArray*);

    int32_t barony_dynamic_array_icon_len(DynamicArray*);

    void    barony_dynamic_array_icon_copy(DynamicArray*, DynamicArray*);

    int32_t barony_dynamic_array_icon_entries(DynamicArray*, ItemTooltipIcons_tMirror*, int32_t);


    // DropdownOption_t array (4 DynamicStrings, deep-owned)
    void    barony_dynamic_array_option_init(DynamicArray*);
    void    barony_dynamic_array_option_append(DynamicArray*, DropdownOption_tMirror*);
    bool    barony_dynamic_array_option_get(DynamicArray*, int32_t, DropdownOption_tMirror*);
    void    barony_dynamic_array_option_set(DynamicArray*, int32_t, DropdownOption_tMirror*);
    void    barony_dynamic_array_option_erase(DynamicArray*, int32_t);
    void    barony_dynamic_array_option_clear(DynamicArray*);
    void    barony_dynamic_array_option_destroy(DynamicArray*);
    int32_t barony_dynamic_array_option_len(DynamicArray*);
    void    barony_dynamic_array_option_copy(DynamicArray*, DynamicArray*);
    int32_t barony_dynamic_array_option_entries(DynamicArray*, DropdownOption_tMirror*, int32_t);

    // Entry Variable_t array (i32 + DynamicString + 3 i32, value owned)
    void    barony_dynamic_array_entryvar_init(DynamicArray*);
    void    barony_dynamic_array_entryvar_append(DynamicArray*, EntryVariable_tMirror*);
    bool    barony_dynamic_array_entryvar_get(DynamicArray*, int32_t, EntryVariable_tMirror*);
    void    barony_dynamic_array_entryvar_set(DynamicArray*, int32_t, EntryVariable_tMirror*);
    void    barony_dynamic_array_entryvar_erase(DynamicArray*, int32_t);
    void    barony_dynamic_array_entryvar_clear(DynamicArray*);
    void    barony_dynamic_array_entryvar_destroy(DynamicArray*);
    int32_t barony_dynamic_array_entryvar_len(DynamicArray*);
    void    barony_dynamic_array_entryvar_copy(DynamicArray*, DynamicArray*);
    int32_t barony_dynamic_array_entryvar_entries(DynamicArray*, EntryVariable_tMirror*, int32_t);
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



// ---------------------------------------------------------------------------

// DynamicArrayStr — std::vector<DynamicString> replacement. Elements are

// DynamicString (16B each) — the array OWNS their buffers. All mutations go

// through the str shims (deep-free on erase/clear/destroy/pop, deep-copy on

// append/set/copy).

// ---------------------------------------------------------------------------

class DynamicArrayStr {

public:

    DynamicArray raw{};



    DynamicArrayStr() { barony_dynamic_array_str_init(&raw); }

    ~DynamicArrayStr() { barony_dynamic_array_str_destroy(&raw); }

    DynamicArrayStr(const DynamicArrayStr& other) : raw{} {

        barony_dynamic_array_str_init(&raw);

        *this = other;

    }

    DynamicArrayStr& operator=(const DynamicArrayStr& other) {

        if (this != &other) { barony_dynamic_array_str_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }

        return *this;

    }

    DynamicArrayStr(DynamicArrayStr&& other) noexcept : raw(other.raw) {

        other.raw = DynamicArray{};

    }

    DynamicArrayStr& operator=(DynamicArrayStr&& other) noexcept {

        if (this != &other) {

            barony_dynamic_array_str_destroy(&raw);

            raw = other.raw;

            other.raw = DynamicArray{};

        }

        return *this;

    }



    int64_t size() const { return barony_dynamic_array_str_len(const_cast<DynamicArray*>(&raw)); }

    bool empty() const { return size() == 0; }

    void clear() { barony_dynamic_array_str_clear(&raw); }

    void push_back(const DynamicString& v) { barony_dynamic_array_str_append(&raw, const_cast<DynamicString*>(&v)); }

    void push_back(const char* v) { DynamicString s(v); barony_dynamic_array_str_append(&raw, &s); }

    void push_back(const std::string& v) { DynamicString s(v.c_str()); barony_dynamic_array_str_append(&raw, &s); }

    void erase(int64_t index) { barony_dynamic_array_str_erase(&raw, (int32_t)index); }

    void pop_back() {

        if (size() > 0) { DynamicString e; barony_dynamic_array_str_get(&raw, (int32_t)(size() - 1), &e); barony_dynamic_array_str_erase(&raw, (int32_t)(size() - 1)); }

    }



    // operator[] — returns a deep-copied element (safe; mutations via set())

    DynamicString at(int64_t i) const {

        DynamicString out;

        barony_dynamic_array_str_get(const_cast<DynamicArray*>(&raw), (int32_t)i, &out);

        return out;

    }

    void set(int64_t i, const DynamicString& v) { barony_dynamic_array_str_set(&raw, (int32_t)i, const_cast<DynamicString*>(&v)); }

    void set(int64_t i, const char* v) { DynamicString s(v); barony_dynamic_array_str_set(&raw, (int32_t)i, &s); }

    void set(int64_t i, const std::string& v) { DynamicString s(v.c_str()); barony_dynamic_array_str_set(&raw, (int32_t)i, &s); }



    // snapshot: deep-copy all elements into the caller's vector

    void snapshot(std::vector<DynamicString>& out) const {

        int64_t n = size();

        out.clear();

        if (n <= 0) return;

        out.resize((size_t)n);

        barony_dynamic_array_str_entries(const_cast<DynamicArray*>(&raw), out.data(), (int32_t)n);

    }



    // range-for: iterate a deep-copied snapshot (END sentinel pattern)

    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = DynamicString;
        using difference_type = std::ptrdiff_t;
        using pointer = const DynamicString*;
        using reference = const DynamicString&;
        std::shared_ptr<std::vector<DynamicString>> snap;
        size_t idx = 0;
        static constexpr size_t END = SIZE_MAX;
        const DynamicString& operator*() const { return (*snap)[idx]; }
        Iterator& operator++() { if (snap && idx < snap->size()) ++idx; return *this; }
        Iterator operator++(int) { Iterator t = *this; ++*this; return t; }
        Iterator operator+(difference_type d) const { Iterator t = *this; t.idx = (size_t)((ptrdiff_t)t.idx + d); return t; }
        bool operator==(const Iterator& o) const { return !(*this != o); }
        bool operator!=(const Iterator& o) const {
            if (!o.snap) return snap && idx < snap->size();
            if (!snap) return o.snap ? o.idx < o.snap->size() : false;
            if (snap.get() != o.snap.get()) return !(idx >= snap->size() && o.idx == END);
            return idx != o.idx;
        }
    };;

    Iterator begin() const {

        Iterator it;

        int64_t n = size();

        if (n > 0) {

            it.snap = std::make_shared<std::vector<DynamicString>>();

            it.snap->resize((size_t)n);

            barony_dynamic_array_str_entries(const_cast<DynamicArray*>(&raw), it.snap->data(), (int32_t)n);

        }

        return it;

    }

    Iterator end() const {

        Iterator it;

        it.idx = Iterator::END;

        return it;

    }

};



// ---------------------------------------------------------------------------

// DynamicArrayS32 — std::vector<Sint32> replacement (POD elements, no

// ownership — raw byte moves are safe). Uses the base DynamicArray shims

// with elem_size = 4.

// ---------------------------------------------------------------------------

class DynamicArrayS32 {

public:

    DynamicArray raw{};



    DynamicArrayS32() { barony_dynamic_array_init(&raw); }

    ~DynamicArrayS32() { barony_dynamic_array_destroy(&raw); }

    DynamicArrayS32(const DynamicArrayS32& other) : raw{} {

        barony_dynamic_array_init(&raw);

        *this = other;

    }

    DynamicArrayS32& operator=(const DynamicArrayS32& other) {

        if (this != &other) { barony_dynamic_array_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }

        return *this;

    }

    DynamicArrayS32(DynamicArrayS32&& other) noexcept : raw(other.raw) {

        other.raw = DynamicArray{};

    }

    DynamicArrayS32& operator=(DynamicArrayS32&& other) noexcept {

        if (this != &other) {

            barony_dynamic_array_destroy(&raw);

            raw = other.raw;

            other.raw = DynamicArray{};

        }

        return *this;

    }



    int64_t size() const { return dynarray_size<int32_t>(raw); }

    bool empty() const { return size() == 0; }

    void clear() { barony_dynamic_array_clear(&raw); }

    void push_back(int32_t v) { dynarray_push<int32_t>(raw, v); }

    void erase(int64_t i) { barony_dynamic_array_erase(&raw, (int32_t)i, (int64_t)sizeof(int32_t)); }

    int32_t& operator[](int64_t i) { return *dynarray_at<int32_t>(raw, i); }

    const int32_t& operator[](int64_t i) const { return *dynarray_at<int32_t>(raw, i); }

    int32_t at(int64_t i) const { return *dynarray_at<int32_t>(raw, i); }

    int32_t* data() { return (int32_t*)raw.data; }

    const int32_t* data() const { return (const int32_t*)raw.data; }

};



// ---------------------------------------------------------------------------

// DynamicArrayU32 — std::vector<Uint32> replacement (POD).

// ---------------------------------------------------------------------------

class DynamicArrayU32 {

public:

    DynamicArray raw{};



    DynamicArrayU32() { barony_dynamic_array_init(&raw); }

    ~DynamicArrayU32() { barony_dynamic_array_destroy(&raw); }

    DynamicArrayU32(const DynamicArrayU32& other) : raw{} {

        barony_dynamic_array_init(&raw);

        *this = other;

    }

    DynamicArrayU32& operator=(const DynamicArrayU32& other) {

        if (this != &other) { barony_dynamic_array_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }

        return *this;

    }

    DynamicArrayU32(DynamicArrayU32&& other) noexcept : raw(other.raw) {

        other.raw = DynamicArray{};

    }

    DynamicArrayU32& operator=(DynamicArrayU32&& other) noexcept {

        if (this != &other) {

            barony_dynamic_array_destroy(&raw);

            raw = other.raw;

            other.raw = DynamicArray{};

        }

        return *this;

    }



    int64_t size() const { return dynarray_size<uint32_t>(raw); }

    bool empty() const { return size() == 0; }

    void clear() { barony_dynamic_array_clear(&raw); }

    void push_back(uint32_t v) { dynarray_push<uint32_t>(raw, v); }

    void erase(int64_t i) { barony_dynamic_array_erase(&raw, (int32_t)i, (int64_t)sizeof(uint32_t)); }

    uint32_t& operator[](int64_t i) { return *dynarray_at<uint32_t>(raw, i); }

    const uint32_t& operator[](int64_t i) const { return *dynarray_at<uint32_t>(raw, i); }

    uint32_t at(int64_t i) const { return *dynarray_at<uint32_t>(raw, i); }

};



// ---------------------------------------------------------------------------

// DynamicArrayIcon — std::vector<ItemTooltipIcons_t> replacement. Elements

// are 3 DynamicStrings + u32 (deep-owned). All mutations via the icon shims.

// ---------------------------------------------------------------------------

;



class DynamicArrayIcon {

public:

    DynamicArray raw{};



    DynamicArrayIcon() { barony_dynamic_array_icon_init(&raw); }

    ~DynamicArrayIcon() { barony_dynamic_array_icon_destroy(&raw); }

    DynamicArrayIcon(const DynamicArrayIcon& other) : raw{} {

        barony_dynamic_array_icon_init(&raw);

        *this = other;

    }

    DynamicArrayIcon& operator=(const DynamicArrayIcon& other) {

        if (this != &other) { barony_dynamic_array_icon_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }

        return *this;

    }

    DynamicArrayIcon(DynamicArrayIcon&& other) noexcept : raw(other.raw) {

        other.raw = DynamicArray{};

    }

    DynamicArrayIcon& operator=(DynamicArrayIcon&& other) noexcept {

        if (this != &other) {

            barony_dynamic_array_icon_destroy(&raw);

            raw = other.raw;

            other.raw = DynamicArray{};

        }

        return *this;

    }



    int64_t size() const { return barony_dynamic_array_icon_len(const_cast<DynamicArray*>(&raw)); }

    bool empty() const { return size() == 0; }

    void clear() { barony_dynamic_array_icon_clear(&raw); }

    void push_back(const ItemTooltipIcons_tMirror& v) { barony_dynamic_array_icon_append(&raw, const_cast<ItemTooltipIcons_tMirror*>(&v)); }

    void erase(int64_t i) { barony_dynamic_array_icon_erase(&raw, (int32_t)i); }

    ItemTooltipIcons_tMirror at(int64_t i) const {

        ItemTooltipIcons_tMirror out;

        barony_dynamic_array_icon_get(const_cast<DynamicArray*>(&raw), (int32_t)i, &out);

        return out;

    }

    void set(int64_t i, const ItemTooltipIcons_tMirror& v) { barony_dynamic_array_icon_set(&raw, (int32_t)i, const_cast<ItemTooltipIcons_tMirror*>(&v)); }

    ItemTooltipIcons_tMirror& operator[](int64_t i) {

        // NOTE: returns a temporary-based ref is unsafe for owned structs.
        // Use at()/set(). This only supports reads via the copy.
        static thread_local ItemTooltipIcons_tMirror _tmp;
        barony_dynamic_array_icon_get(&raw, (int32_t)i, &_tmp);
        return _tmp;
    }

    // live-slot iterators: operator* returns a pointer to the ACTUAL array
    // slot (mutations persist, &*it is the live element). Valid until the
    // array mutates.
    struct Iterator {
        DynamicArray* arr = nullptr;
        int64_t i = 0;
        ItemTooltipIcons_tMirror* operator->() const { return (ItemTooltipIcons_tMirror*)arr->data + i; }
        ItemTooltipIcons_tMirror& operator*() const { return *((ItemTooltipIcons_tMirror*)arr->data + i); }
        Iterator& operator++() { ++i; return *this; }
        bool operator!=(const Iterator& o) const { return arr != o.arr || i != o.i; }
    };
    Iterator begin() { return Iterator{&raw, 0}; }
    Iterator end() { return Iterator{&raw, size()}; }
    Iterator begin() const { return Iterator{const_cast<DynamicArray*>(&raw), 0}; }
    Iterator end() const { return Iterator{const_cast<DynamicArray*>(&raw), size()}; }
};


// ---------------------------------------------------------------------------
// DynamicArrayOption — std::vector<DropdownOption_t> replacement. Elements
// are 4 DynamicStrings (deep-owned). Live-slot iterators (like Icon).
// ---------------------------------------------------------------------------
;

class DynamicArrayOption {
public:
    DynamicArray raw{};

    DynamicArrayOption() { barony_dynamic_array_option_init(&raw); }
    ~DynamicArrayOption() { barony_dynamic_array_option_destroy(&raw); }
    DynamicArrayOption(const DynamicArrayOption& other) : raw{} {
        barony_dynamic_array_option_init(&raw);
        *this = other;
    }
    DynamicArrayOption& operator=(const DynamicArrayOption& other) {
        if (this != &other) { barony_dynamic_array_option_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }
        return *this;
    }
    DynamicArrayOption(DynamicArrayOption&& other) noexcept : raw(other.raw) {
        other.raw = DynamicArray{};
    }
    DynamicArrayOption& operator=(DynamicArrayOption&& other) noexcept {
        if (this != &other) {
            barony_dynamic_array_option_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicArray{};
        }
        return *this;
    }

    int64_t size() const { return barony_dynamic_array_option_len(const_cast<DynamicArray*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_array_option_clear(&raw); }
    void push_back(const DropdownOption_tMirror& v) { barony_dynamic_array_option_append(&raw, const_cast<DropdownOption_tMirror*>(&v)); }
    void erase(int64_t i) { barony_dynamic_array_option_erase(&raw, (int32_t)i); }
    DropdownOption_tMirror at(int64_t i) const {
        DropdownOption_tMirror out;
        barony_dynamic_array_option_get(const_cast<DynamicArray*>(&raw), (int32_t)i, &out);
        return out;
    }
    void set(int64_t i, const DropdownOption_tMirror& v) { barony_dynamic_array_option_set(&raw, (int32_t)i, const_cast<DropdownOption_tMirror*>(&v)); }
    DropdownOption_tMirror& operator[](int64_t i) {
        static thread_local DropdownOption_tMirror _tmp;
        barony_dynamic_array_option_get(&raw, (int32_t)i, &_tmp);
        return _tmp;
    }
    // live-slot iterators (mutations persist)
    struct Iterator {
        DynamicArray* arr = nullptr;
        int64_t i = 0;
        DropdownOption_tMirror* operator->() const { return (DropdownOption_tMirror*)arr->data + i; }
        DropdownOption_tMirror& operator*() const { return *((DropdownOption_tMirror*)arr->data + i); }
        Iterator& operator++() { ++i; return *this; }
        bool operator!=(const Iterator& o) const { return arr != o.arr || i != o.i; }
    };
    Iterator begin() { return Iterator{&raw, 0}; }
    Iterator end() { return Iterator{&raw, size()}; }
    Iterator begin() const { return Iterator{const_cast<DynamicArray*>(&raw), 0}; }
    Iterator end() const { return Iterator{const_cast<DynamicArray*>(&raw), size()}; }
};

// ---------------------------------------------------------------------------
// DynamicArrayEntryVar — std::vector<Entry_t::Variable_t> replacement.
// Element = i32 type + DynamicString value + 3 i32. value is owned.
// ---------------------------------------------------------------------------
;

class DynamicArrayEntryVar {
public:
    DynamicArray raw{};

    DynamicArrayEntryVar() { barony_dynamic_array_entryvar_init(&raw); }
    ~DynamicArrayEntryVar() { barony_dynamic_array_entryvar_destroy(&raw); }
    DynamicArrayEntryVar(const DynamicArrayEntryVar& other) : raw{} {
        barony_dynamic_array_entryvar_init(&raw);
        *this = other;
    }
    DynamicArrayEntryVar& operator=(const DynamicArrayEntryVar& other) {
        if (this != &other) { barony_dynamic_array_entryvar_copy(&raw, const_cast<DynamicArray*>(&other.raw)); }
        return *this;
    }
    DynamicArrayEntryVar(DynamicArrayEntryVar&& other) noexcept : raw(other.raw) {
        other.raw = DynamicArray{};
    }
    DynamicArrayEntryVar& operator=(DynamicArrayEntryVar&& other) noexcept {
        if (this != &other) {
            barony_dynamic_array_entryvar_destroy(&raw);
            raw = other.raw;
            other.raw = DynamicArray{};
        }
        return *this;
    }

    int64_t size() const { return barony_dynamic_array_entryvar_len(const_cast<DynamicArray*>(&raw)); }
    bool empty() const { return size() == 0; }
    void clear() { barony_dynamic_array_entryvar_clear(&raw); }
    void push_back(const EntryVariable_tMirror& v) { barony_dynamic_array_entryvar_append(&raw, const_cast<EntryVariable_tMirror*>(&v)); }
    void erase(int64_t i) { barony_dynamic_array_entryvar_erase(&raw, (int32_t)i); }
    EntryVariable_tMirror at(int64_t i) const {
        EntryVariable_tMirror out;
        barony_dynamic_array_entryvar_get(const_cast<DynamicArray*>(&raw), (int32_t)i, &out);
        return out;
    }
    void set(int64_t i, const EntryVariable_tMirror& v) { barony_dynamic_array_entryvar_set(&raw, (int32_t)i, const_cast<EntryVariable_tMirror*>(&v)); }
    EntryVariable_tMirror& operator[](int64_t i) {
        static thread_local EntryVariable_tMirror _tmp;
        barony_dynamic_array_entryvar_get(&raw, (int32_t)i, &_tmp);
        return _tmp;
    }
    // live-slot iterators
    struct Iterator {
        DynamicArray* arr = nullptr;
        int64_t i = 0;
        EntryVariable_tMirror* operator->() const { return (EntryVariable_tMirror*)arr->data + i; }
        EntryVariable_tMirror& operator*() const { return *((EntryVariable_tMirror*)arr->data + i); }
        Iterator& operator++() { ++i; return *this; }
        bool operator!=(const Iterator& o) const { return arr != o.arr || i != o.i; }
    };
    Iterator begin() { return Iterator{&raw, 0}; }
    Iterator end() { return Iterator{&raw, size()}; }
    Iterator begin() const { return Iterator{const_cast<DynamicArray*>(&raw), 0}; }
    Iterator end() const { return Iterator{const_cast<DynamicArray*>(&raw), size()}; }
};
