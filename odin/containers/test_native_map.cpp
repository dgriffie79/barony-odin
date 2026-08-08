// test_native_map.cpp — exercise the native-map shims from C++, verify vs std::map.
#include <cstdio>
#include <cstdint>
#include <map>

struct DynamicMap { void* data; int64_t len; int64_t alloc[2]; };  // Raw_Map mirror = 32B

extern "C" {
    void    barony_dynamic_map_i32i32_init(void* m);
    void    barony_dynamic_map_i32i32_put(void* m, const void* key4, int32_t value);
    bool    barony_dynamic_map_i32i32_get(void* m, const void* key4, int32_t* out);
    bool    barony_dynamic_map_i32i32_erase(void* m, const void* key4);
    void    barony_dynamic_map_i32i32_clear(void* m);
    int32_t barony_dynamic_map_i32i32_len(void* m);
    void    barony_dynamic_map_i32i32_destroy(void* m);
}

int main() {
    bool ok = true;
    DynamicMap m{};
    std::map<int,int> ref;

    auto check = [&](const char* what) {
        bool good = (barony_dynamic_map_i32i32_len(&m) == (int32_t)ref.size());
        for (auto& kv : ref) {
            int32_t key = kv.first, v = -1;
            bool found = barony_dynamic_map_i32i32_get(&m, &key, &v);
            if (!found || v != kv.second) { good = false; break; }
        }
        printf("  %-22s %s (len=%d vs %zu)\n", what, good ? "OK" : "FAIL",
            (int)barony_dynamic_map_i32i32_len(&m), ref.size());
        ok &= good;
    };

    int keys[] = {5, 3, 8, 1, 9, 2, 7};
    for (int k : keys) { barony_dynamic_map_i32i32_put(&m, &k, k*10); ref[k] = k*10; }
    check("insert 7 out-of-order");

    int k3 = 3;
    barony_dynamic_map_i32i32_put(&m, &k3, 300);
    ref[3] = 300;
    check("update key 3");

    int k8 = 8, k99 = 99, v8 = -1, v99 = -1;
    bool f8 = barony_dynamic_map_i32i32_get(&m, &k8, &v8);
    bool f99 = barony_dynamic_map_i32i32_get(&m, &k99, &v99);
    printf("  find(8)=%d ok=%d find(99)=%d ok=%d %s\n", v8, f8, v99, f99,
        (f8 && v8==80 && !f99) ? "OK" : "FAIL");
    ok &= (f8 && v8==80 && !f99);

    int k1 = 1;
    bool erased = barony_dynamic_map_i32i32_erase(&m, &k1);
    ref.erase(1);
    printf("  erase(1)=%d %s\n", erased, (erased && barony_dynamic_map_i32i32_len(&m)==(int32_t)ref.size()) ? "OK" : "FAIL");
    ok &= (erased && barony_dynamic_map_i32i32_len(&m)==(int32_t)ref.size());

    barony_dynamic_map_i32i32_clear(&m);
    ref.clear();
    check("clear");

    barony_dynamic_map_i32i32_destroy(&m);
    printf(ok ? "map test: PASS\n" : "map test: FAIL\n");
    return ok ? 0 : 1;
}
