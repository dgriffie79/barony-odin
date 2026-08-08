// test_map_string.cpp — exercise DynamicMap + DynamicString shims from C++, verify vs std containers.
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>

struct DynamicMap  { void* data; int64_t len; int64_t cap; void* a; void* b; };
struct DynamicString { void* data; int64_t len; };

extern "C" {
    void    barony_dynamic_map_init(DynamicMap*);
    int32_t barony_dynamic_map_find(DynamicMap*, const void* key, int64_t key_size,
                                    int32_t(*cmp)(const void*, const void*), int64_t value_size);
    int32_t barony_dynamic_map_put(DynamicMap*, const void* key, int64_t key_size,
                                    const void* value, int64_t value_size,
                                    int32_t(*cmp)(const void*, const void*));
    int32_t barony_dynamic_map_erase(DynamicMap*, const void* key, int64_t key_size,
                                     int32_t(*cmp)(const void*, const void*), int64_t value_size);
    void    barony_dynamic_map_clear(DynamicMap*);
    void    barony_dynamic_map_destroy(DynamicMap*);

    void    barony_dynamic_string_init(DynamicString*);
    void    barony_dynamic_string_from_cstr(DynamicString*, const char*);
    void    barony_dynamic_string_set_len(DynamicString*, int32_t);
    const char* barony_dynamic_string_c_str(DynamicString*);
    void    barony_dynamic_string_append(DynamicString*, const void*, int64_t);
    void    barony_dynamic_string_destroy(DynamicString*);
}

// int comparator (key = int32 stored as bytes)
int32_t cmp_int(const void* a, const void* b) {
    int32_t x, y; memcpy(&x, a, 4); memcpy(&y, b, 4);
    return x < y ? -1 : (x > y ? 1 : 0);
}

// --- Map test: std::map<int,int> vs DynamicMap<int,int> ---
int test_map() {
    bool ok = true;
    DynamicMap m{}; barony_dynamic_map_init(&m);
    std::map<int,int> ref;

    auto check = [&](const char* what) {
        bool good = (m.len == (int)ref.size());
        // verify sorted order + values
        int idx = 0;
        for (auto& kv : ref) {
            int* pairs = (int*)m.data;
            if (pairs[idx*2] != kv.first || pairs[idx*2+1] != kv.second) { good = false; break; }
            idx++;
        }
        printf("  %-22s %s (len=%lld vs %zu)\n", what, good ? "OK" : "FAIL", (long long)m.len, ref.size());
        ok &= good;
    };

    // insert out of order
    int keys[] = {5, 3, 8, 1, 9, 2, 7};
    for (int k : keys) { int v = k * 10; barony_dynamic_map_put(&m, &k, 4, &v, 4, cmp_int); ref[k] = v; }
    check("insert 7 out-of-order");

    // update existing
    int k3 = 3, v30 = 300;
    barony_dynamic_map_put(&m, &k3, 4, &v30, 4, cmp_int);
    ref[3] = 300;
    check("update key 3");

    // find
    int k8 = 8;
    int32_t fi = barony_dynamic_map_find(&m, &k8, 4, cmp_int, 4);
    int k99 = 99;
    int32_t fi99 = barony_dynamic_map_find(&m, &k99, 4, cmp_int, 4);
    printf("  find(8)=%d find(99)=%d %s\n", fi, fi99, (fi>=0 && fi99<0) ? "OK" : "FAIL");
    ok &= (fi >= 0 && fi99 < 0);

    // erase
    int k1 = 1;
    barony_dynamic_map_erase(&m, &k1, 4, cmp_int, 4);
    ref.erase(1);
    check("erase key 1");

    // clear
    barony_dynamic_map_clear(&m);
    ref.clear();
    check("clear");

    barony_dynamic_map_destroy(&m);
    printf("map test: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

// --- String test ---
int test_string() {
    bool ok = true;
    DynamicString s{}; barony_dynamic_string_init(&s);
    std::string ref;

    barony_dynamic_string_from_cstr(&s, "Hello");
    ref = "Hello";
    printf("  from_cstr: '%s' len=%lld %s\n", barony_dynamic_string_c_str(&s), (long long)s.len,
        (strcmp(barony_dynamic_string_c_str(&s), ref.c_str())==0 && s.len==5) ? "OK" : "FAIL");
    ok &= (strcmp(barony_dynamic_string_c_str(&s), "Hello")==0 && s.len==5);

    barony_dynamic_string_append(&s, " World", 6);
    ref += " World";
    printf("  append: '%s' len=%lld %s\n", barony_dynamic_string_c_str(&s), (long long)s.len,
        (strcmp(barony_dynamic_string_c_str(&s), ref.c_str())==0) ? "OK" : "FAIL");
    ok &= (strcmp(barony_dynamic_string_c_str(&s), "Hello World")==0);

    // c_str on empty-ish
    barony_dynamic_string_destroy(&s);
    barony_dynamic_string_init(&s);
    barony_dynamic_string_from_cstr(&s, "");
    printf("  empty: '%s' len=%lld %s\n", barony_dynamic_string_c_str(&s), (long long)s.len,
        (s.len==0) ? "OK" : "FAIL");
    ok &= (s.len==0);

    barony_dynamic_string_destroy(&s);
    printf("string test: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

int main() {
    int r = 0;
    r |= test_map();
    r |= test_string();
    printf(r==0 ? "\nALL PASS\n" : "\nSOME FAILED\n");
    return r;
}
