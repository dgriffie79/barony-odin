// json_shim.hpp — C++ declarations for the Odin JSON shim (odin/json_shim/).
// Replaces rapidjson's PrettyWriter/Document inside FileInterface (json.cpp)
// and the ad-hoc DOM parser/builder sites. The Odin procs are @(export) "c";
// C++ holds opaque handles (void*). Duplicate object keys are accepted;
// lookup returns the first match, iteration yields all members (like rapidjson).
#pragma once

#include <cstdint>
#include <cstring>

extern "C" {
    // --- streaming writer ---
    void*       json_writer_create(bool compact);
    void        json_writer_destroy(void* w);
    bool        json_writer_begin_object(void* w);
    void        json_writer_end_object(void* w);
    bool        json_writer_begin_array(void* w);
    void        json_writer_end_array(void* w);
    void        json_writer_key(void* w, const char* key);
    bool        json_writer_uint(void* w, uint32_t v);
    bool        json_writer_int(void* w, int32_t v);
    bool        json_writer_double(void* w, double v);
    bool        json_writer_bool(void* w, bool v);
    bool        json_writer_string(void* w, const char* s);
    const char* json_writer_get_string(void* w);

    // --- lockstep reader (FileInterface cursor) ---
    void* json_reader_parse(const char* data);
    void  json_reader_destroy(void* r);
    bool  json_reader_begin_object(void* r);
    void  json_reader_end_object(void* r);
    bool  json_reader_begin_array(void* r, uint32_t* size);
    void  json_reader_end_array(void* r);
    void  json_reader_property_name(void* r, const char* name);
    bool  json_reader_value_uint(void* r, uint32_t* out);
    bool  json_reader_value_int(void* r, int32_t* out);
    bool  json_reader_value_float(void* r, float* out);
    bool  json_reader_value_double(void* r, double* out);
    bool  json_reader_value_bool(void* r, bool* out);
    bool  json_reader_value_string(void* r, const char** out);

    // --- DOM access (read side) ---
    void* json_node_root(void* r);
    void* json_node_get_member(void* node, const char* key);   // first match, or null
    bool  json_node_has_member(void* node, const char* key);
    uint32_t json_node_member_count(void* node);
    const char* json_node_member_name_at(void* node, uint32_t i);
    void* json_node_member_value_at(void* node, uint32_t i);
    uint32_t json_node_array_size(void* node);
    void* json_node_element_at(void* node, uint32_t i);
    bool  json_node_is_object(void* node);
    bool  json_node_is_array(void* node);
    bool  json_node_is_string(void* node);
    bool  json_node_is_bool(void* node);
    bool  json_node_is_null(void* node);
    bool  json_node_is_number(void* node);
    bool  json_node_is_int(void* node);
    bool  json_node_is_uint(void* node);
    bool  json_node_is_int64(void* node);
    bool  json_node_is_uint64(void* node);
    bool  json_node_is_double(void* node);
    bool  json_node_get_int(void* node, int32_t* out);
    bool  json_node_get_uint(void* node, uint32_t* out);
    bool  json_node_get_int64(void* node, int64_t* out);
    bool  json_node_get_uint64(void* node, uint64_t* out);
    bool  json_node_get_double(void* node, double* out);
    bool  json_node_get_float(void* node, float* out);
    bool  json_node_get_bool(void* node, bool* out);
    bool  json_node_get_string(void* node, const char** out);

    // --- DOM builder (write side) ---
    void* json_node_create_object(void);
    void* json_node_create_array(void);
    void* json_node_create_int(int32_t v);
    void* json_node_create_uint(uint32_t v);
    void* json_node_create_int64(int64_t v);
    void* json_node_create_uint64(uint64_t v);
    void* json_node_create_double(double v);
    void* json_node_create_bool(bool v);
    void* json_node_create_string(const char* s);
    void* json_node_create_null(void);
    void  json_node_add_member(void* obj, const char* key, void* value);
    void  json_node_push_back(void* arr, void* value);
    void  json_node_erase_member(void* obj, const char* key);
    void  json_node_clear_array(void* arr);
    void  json_node_set_string(void* node, const char* s);
    void  json_node_destroy(void* node);

    // --- serializer ---
    const char* json_node_serialize(void* node, bool compact);
    void        json_string_free(const char* s);
    void*       json_node_parse_document(const char* data);
    void        json_node_remove_all_members(void* obj);
}

// Convenience helpers for the ad-hoc parser sites (rapidjson-style value reads).
static inline int32_t json_int(void* node, const char* key, int32_t def = 0) {
    int32_t v = def;
    json_node_get_int(json_node_get_member(node, key), &v);
    return v;
}
static inline const char* json_str(void* node, const char* key) {
    const char* s = nullptr;
    json_node_get_string(json_node_get_member(node, key), &s);
    return s;
}
static inline bool json_bool(void* node, const char* key, bool def = false) {
    bool b = def;
    json_node_get_bool(json_node_get_member(node, key), &b);
    return b;
}
static inline double json_double(void* node, const char* key, double def = 0.0) {
    double d = def;
    json_node_get_double(json_node_get_member(node, key), &d);
    return d;
}
static inline float json_float(void* node, const char* key, float def = 0.0f) {
    float f = def;
    json_node_get_float(json_node_get_member(node, key), &f);
    return f;
}

struct JsonMemberIt;
struct JsonValueIt;

// JsonNode — a lightweight non-owning view over a json_shim DOM node, mirroring
// the rapidjson read API (operator[], HasMember, GetInt/GetString/..., Member/
// Value iteration). Lets large parser sites convert ~1:1 from rapidjson.
// Tags mirroring rapidjson's kObjectType/kArrayType used with Value(...).
enum JsonTypeTag { ObjectTypeTag, ArrayTypeTag };

struct JsonNode {
    void* h = nullptr;
    JsonNode() = default;
    explicit JsonNode(void* _h) : h(_h) {}
    JsonNode(JsonTypeTag t) : h(t == ObjectTypeTag ? json_node_create_object() : json_node_create_array()) {}
    // rapidjson::Value(v) — primitive-typed construction
    JsonNode(int32_t v)  : h(json_node_create_int(v)) {}
    JsonNode(uint32_t v) : h(json_node_create_uint(v)) {}
    JsonNode(int64_t v)  : h(json_node_create_int64(v)) {}
    JsonNode(uint64_t v) : h(json_node_create_uint64(v)) {}
    JsonNode(double v)   : h(json_node_create_double(v)) {}
    JsonNode(bool v)     : h(json_node_create_bool(v)) {}
    JsonNode(const char* v) : h(json_node_create_string(v)) {}
    // rapidjson::Value(str, allocator) — copy-string form
    JsonNode(const char* v, void*) : h(json_node_create_string(v)) {}

    bool IsNull()    const { return json_node_is_null(h); }
    bool IsObject()  const { return json_node_is_object(h); }
    bool IsArray()   const { return json_node_is_array(h); }
    bool IsString()  const { return json_node_is_string(h); }
    bool IsBool()    const { return json_node_is_bool(h); }
    bool IsNumber()  const { return json_node_is_number(h); }
    bool IsInt()     const { return json_node_is_int(h); }
    bool IsUint()    const { return json_node_is_uint(h); }
    bool IsInt64()   const { return json_node_is_int64(h); }
    bool IsUint64()  const { return json_node_is_uint64(h); }
    bool IsDouble()  const { return json_node_is_double(h); }

    bool HasMember(const char* k) const { return json_node_has_member(h, k); }
    JsonNode operator[](const char* k) const { return JsonNode(json_node_get_member(h, k)); }
    JsonNode operator[](uint32_t i) const { return JsonNode(json_node_element_at(h, i)); }

    // rapidjson::GetArray()/GetObject() equivalents — return *this (the node IS
    // the array/object). Only valid if IsArray()/IsObject().
    JsonNode GetArray()  const { return *this; }
    JsonNode GetObject() const { return *this; }

    // write-side (rapidjson::Value mutation, mirrored from JsonValue)
    static JsonNode Object()            { return JsonNode(json_node_create_object()); }
    static JsonNode Array()             { return JsonNode(json_node_create_array()); }
    static JsonNode Int(int32_t v)      { return JsonNode(json_node_create_int(v)); }
    static JsonNode Uint(uint32_t v)    { return JsonNode(json_node_create_uint(v)); }
    static JsonNode Int64(int64_t v)    { return JsonNode(json_node_create_int64(v)); }
    static JsonNode Uint64(uint64_t v)  { return JsonNode(json_node_create_uint64(v)); }
    static JsonNode Double(double v)    { return JsonNode(json_node_create_double(v)); }
    static JsonNode Bool(bool v)        { return JsonNode(json_node_create_bool(v)); }
    static JsonNode Str(const char* s)  { return JsonNode(json_node_create_string(s)); }
    static JsonNode Null()              { return JsonNode(json_node_create_null()); }

    // rapidjson Value Set* — replace this node's payload (mutate in place)
    void SetObject() { *this = Object(); }
    void SetArray()  { *this = Array(); }
    // rapidjson Document::Parse / Accept / RemoveAllMembers
    void ParseStream(void*) { /* handled by callers via JsonDoc; no-op */ }
    void Parse(const char* s) { *this = JsonNode(json_node_parse_document(s)); }
    void RemoveAllMembers() { json_node_remove_all_members(h); }
    void Clear() { json_node_remove_all_members(h); }
    void EraseMember(const char* k) { json_node_erase_member(h, k); }
    void Accept(void*) {}  // serialization is done via json_node_serialize
    void SetInt(int32_t v)     { *this = Int(v); }
    void SetUint(uint32_t v)   { *this = Uint(v); }
    void SetInt64(int64_t v)   { *this = Int64(v); }
    void SetUint64(uint64_t v) { *this = Uint64(v); }
    void SetDouble(double v)   { *this = Double(v); }
    void SetBool(bool v)       { *this = Bool(v); }
    void SetString(const char* s, void*) { *this = Str(s); }
    void AddMember(const char* k, JsonNode v) { json_node_add_member(h, k, v.h); }
    void AddMember(JsonNode k, JsonNode v) { json_node_add_member(h, k.GetString(), v.h); }
    void AddMember(const char* k, int32_t v)     { AddMember(k, Int(v)); }
    void AddMember(const char* k, uint32_t v)    { AddMember(k, Uint(v)); }
    void AddMember(const char* k, int64_t v)     { AddMember(k, Int64(v)); }
    void AddMember(const char* k, uint64_t v)    { AddMember(k, Uint64(v)); }
    void AddMember(const char* k, double v)      { AddMember(k, Double(v)); }
    void AddMember(const char* k, bool v)        { AddMember(k, Bool(v)); }
    void AddMember(const char* k, const char* v) { AddMember(k, Str(v)); }
    void PushBack(JsonNode v)                    { json_node_push_back(h, v.h); }
    void PushBack(int32_t v)     { PushBack(Int(v)); }
    void PushBack(uint32_t v)    { PushBack(Uint(v)); }
    void PushBack(int64_t v)     { PushBack(Int64(v)); }
    void PushBack(uint64_t v)    { PushBack(Uint64(v)); }
    void PushBack(double v)      { PushBack(Double(v)); }
    void PushBack(bool v)        { PushBack(Bool(v)); }
    void PushBack(const char* v) { PushBack(Str(v)); }

    int32_t  GetInt()    const { int32_t v = 0;    json_node_get_int(h, &v);    return v; }
    uint32_t GetUint()   const { uint32_t v = 0;   json_node_get_uint(h, &v);   return v; }
    int64_t  GetInt64()  const { int64_t v = 0;    json_node_get_int64(h, &v);  return v; }
    uint64_t GetUint64() const { uint64_t v = 0;   json_node_get_uint64(h, &v); return v; }
    double   GetDouble() const { double v = 0;     json_node_get_double(h, &v); return v; }
    float    GetFloat()  const { float v = 0;      json_node_get_float(h, &v);  return v; }
    bool     GetBool()   const { bool v = false;   json_node_get_bool(h, &v);   return v; }
    const char* GetString() const { const char* v = nullptr; json_node_get_string(h, &v); return v; }

    uint32_t Size()        const { return json_node_array_size(h); }
    uint32_t MemberCount() const { return json_node_member_count(h); }

    JsonMemberIt MemberBegin() const;
    JsonMemberIt MemberEnd()   const;
    JsonMemberIt FindMember(const char* k) const;
    JsonValueIt   Begin()       const;
    JsonValueIt   End()         const;
};

// object-member iterator (mirrors rapidjson ConstMemberIterator)
struct JsonMemberRef {
    struct Name { const char* s = nullptr; const char* GetString() const { return s; } bool IsString() const { return s != nullptr; } };
    Name name;
    JsonNode value;
};
struct JsonMemberIt {
    void* objh = nullptr;
    uint32_t i = 0;
    JsonMemberRef ref;
    JsonMemberRef& operator*()  { return ref; }
    JsonMemberRef* operator->() { return &ref; }
    bool operator!=(const JsonMemberIt& o) const { return i != o.i; }
    JsonMemberIt& operator++() { ++i; load(); return *this; }
    void load() {
        ref.name.s = json_node_member_name_at(objh, i);
        ref.value = JsonNode(json_node_member_value_at(objh, i));
    }
};

// array-element iterator (mirrors rapidjson ConstValueIterator)
struct JsonValueIt {
    void* arrh = nullptr;
    uint32_t i = 0;
    JsonNode elem;
    JsonNode& operator*()  { return elem; }
    JsonNode* operator->() { return &elem; }
    bool operator!=(const JsonValueIt& o) const { return i != o.i; }
    JsonValueIt& operator++() { ++i; load(); return *this; }
    void load() { elem = JsonNode(json_node_element_at(arrh, i)); }
};

inline JsonMemberIt JsonNode::MemberBegin() const { JsonMemberIt it; it.objh = h; it.i = 0; it.load(); return it; }
inline JsonMemberIt JsonNode::MemberEnd()   const { JsonMemberIt it; it.objh = h; it.i = MemberCount(); return it; }
inline JsonMemberIt JsonNode::FindMember(const char* k) const {
    uint32_t n = MemberCount();
    for (uint32_t i = 0; i < n; ++i) {
        const char* name = json_node_member_name_at(h, i);
        if (name && std::strcmp(name, k) == 0) {
            JsonMemberIt it; it.objh = h; it.i = i; it.load(); return it;
        }
    }
    return MemberEnd();
}
inline JsonValueIt JsonNode::Begin() const { JsonValueIt it; it.arrh = h; it.i = 0; it.load(); return it; }
inline JsonValueIt JsonNode::End()   const { JsonValueIt it; it.arrh = h; it.i = Size(); return it; }

// JsonValue — builder mirror of rapidjson::Value (write side). Owns a heap node;
// copies share the same underlying node (like rapidjson Value copies).
struct JsonValue {
    void* h = nullptr;
    JsonValue() = default;
    explicit JsonValue(void* _h) : h(_h) {}

    static JsonValue Object()            { return JsonValue(json_node_create_object()); }
    static JsonValue Array()             { return JsonValue(json_node_create_array()); }
    static JsonValue Int(int32_t v)      { return JsonValue(json_node_create_int(v)); }
    static JsonValue Uint(uint32_t v)    { return JsonValue(json_node_create_uint(v)); }
    static JsonValue Int64(int64_t v)    { return JsonValue(json_node_create_int64(v)); }
    static JsonValue Uint64(uint64_t v)  { return JsonValue(json_node_create_uint64(v)); }
    static JsonValue Double(double v)    { return JsonValue(json_node_create_double(v)); }
    static JsonValue Bool(bool v)        { return JsonValue(json_node_create_bool(v)); }
    static JsonValue Str(const char* s)  { return JsonValue(json_node_create_string(s)); }
    static JsonValue Null()              { return JsonValue(json_node_create_null()); }

    void SetObject() {}
    void SetArray()  {}
    void AddMember(const char* k, JsonValue v) { json_node_add_member(h, k, v.h); }
    void PushBack(JsonValue v)                  { json_node_push_back(h, v.h); }

    // primitive convenience overloads
    void AddMember(const char* k, int32_t v)     { AddMember(k, Int(v)); }
    void AddMember(const char* k, uint32_t v)    { AddMember(k, Uint(v)); }
    void AddMember(const char* k, int64_t v)     { AddMember(k, Int64(v)); }
    void AddMember(const char* k, uint64_t v)    { AddMember(k, Uint64(v)); }
    void AddMember(const char* k, double v)      { AddMember(k, Double(v)); }
    void AddMember(const char* k, bool v)        { AddMember(k, Bool(v)); }
    void AddMember(const char* k, const char* v) { AddMember(k, Str(v)); }

    void PushBack(int32_t v)     { PushBack(Int(v)); }
    void PushBack(uint32_t v)    { PushBack(Uint(v)); }
    void PushBack(int64_t v)     { PushBack(Int64(v)); }
    void PushBack(uint64_t v)    { PushBack(Uint64(v)); }
    void PushBack(double v)      { PushBack(Double(v)); }
    void PushBack(bool v)        { PushBack(Bool(v)); }
    void PushBack(const char* v) { PushBack(Str(v)); }
};

// JsonBuilder — owns a root object node (mirrors rapidjson::Document write side).
struct JsonBuilder {
    void* root = nullptr;
    JsonBuilder() : root(json_node_create_object()) {}
    ~JsonBuilder() { if (root) json_node_destroy(root); }
    JsonBuilder(const JsonBuilder&) = delete;
    JsonBuilder& operator=(const JsonBuilder&) = delete;
    void SetObject() {}
    void AddMember(const char* k, JsonValue v) { json_node_add_member(root, k, v.h); }
    JsonValue operator[](const char* k) const { return JsonValue(json_node_get_member(root, k)); }
    const char* Serialize(bool compact) const { return json_node_serialize(root, compact); }
};

// JsonDoc — owns a json_shim reader; JsonNode is a non-owning view into it.
struct JsonDoc {
    void* reader = nullptr;
    JsonNode root;
    JsonDoc() = default;
    JsonDoc(const JsonDoc&) = delete;
    JsonDoc& operator=(const JsonDoc&) = delete;
    explicit JsonDoc(const char* data) {
        reader = json_reader_parse(data);
        if (reader) root = JsonNode(json_node_root(reader));
    }
    ~JsonDoc() { if (reader) json_reader_destroy(reader); }
    bool ok() const { return reader != nullptr && root.h != nullptr; }
};
