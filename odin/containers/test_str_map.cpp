// test_str_map.cpp — exercise string-keyed native map (Item.attributes case) from C++.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>

// DynamicString {data,len} — 16 bytes, ABI-identical to Odin string
struct DynamicString { const char* data; int64_t len; };
struct DynamicMap { void* data; int64_t len; int64_t alloc[2]; };

extern "C" {
    void barony_dynamic_map_stri32_init(void* m);
    void barony_dynamic_map_stri32_put(void* m, DynamicString key, int32_t value);
    bool barony_dynamic_map_stri32_get(void* m, DynamicString key, int32_t* out);
    bool barony_dynamic_map_stri32_erase(void* m, DynamicString key);
    void barony_dynamic_map_stri32_clear(void* m);
    int32_t barony_dynamic_map_stri32_len(void* m);
    void barony_dynamic_map_stri32_destroy(void* m);
}

int main() {
    bool ok = true;
    DynamicMap m{};
    std::map<std::string,int> ref;
    auto dstr = [](const char* s) -> DynamicString { return {s, (int64_t)strlen(s)}; };

    // insert keys (like Item.attributes: "AC", "ATK", "RATE_OF_FIRE")
    const char* keys[] = {"AC", "RATE_OF_FIRE", "ATK", "ARMOR_PIERCE", "KNOCKBACK", "FRAGILE", "EFF_PARALYZE"};
    for (int i = 0; i < 7; ++i) {
        barony_dynamic_map_stri32_put(&m, dstr(keys[i]), i*5);
        ref[keys[i]] = i*5;
    }
    bool good = barony_dynamic_map_stri32_len(&m) == (int32_t)ref.size();
    for (auto& kv : ref) {
        int32_t v = -1;
        bool found = barony_dynamic_map_stri32_get(&m, dstr(kv.first.c_str()), &v);
        if (!found || v != kv.second) { good = false; break; }
    }
    printf("insert 7 string keys: %s (len=%d vs %zu)\n", good ? "OK" : "FAIL",
        (int)barony_dynamic_map_stri32_len(&m), ref.size());
    ok &= good;

    // get missing
    int32_t v = -1;
    bool f = barony_dynamic_map_stri32_get(&m, dstr("DOES_NOT_EXIST"), &v);
    printf("get missing: ok=%d %s\n", f, (!f) ? "OK" : "FAIL");
    ok &= !f;

    // erase
    bool e = barony_dynamic_map_stri32_erase(&m, dstr("AC"));
    ref.erase("AC");
    printf("erase AC: %s\n", (e && barony_dynamic_map_stri32_len(&m)==(int32_t)ref.size()) ? "OK" : "FAIL");
    ok &= (e && barony_dynamic_map_stri32_len(&m)==(int32_t)ref.size());

    // clear + destroy
    barony_dynamic_map_stri32_clear(&m);
    ref.clear();
    printf("clear: %s\n", (barony_dynamic_map_stri32_len(&m)==0) ? "OK" : "FAIL");
    ok &= (barony_dynamic_map_stri32_len(&m)==0);
    barony_dynamic_map_stri32_destroy(&m);

    printf(ok ? "string-map test: PASS\n" : "string-map test: FAIL\n");
    return ok ? 0 : 1;
}
