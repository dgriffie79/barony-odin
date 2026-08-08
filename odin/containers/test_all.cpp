#include <cstdio>
#include <cstdint>
#include <vector>

struct DynamicArray { void* data; int64_t len; int64_t cap; void* a; void* b; };

extern "C" {
    void    barony_dynamic_array_init(DynamicArray*);
    int32_t barony_dynamic_array_append(DynamicArray*, const void*, int64_t);
    void    barony_dynamic_array_erase(DynamicArray*, int32_t index, int64_t elem_size);
    int32_t barony_dynamic_array_insert(DynamicArray*, int32_t index, const void*, int64_t);
    void    barony_dynamic_array_clear(DynamicArray*);
    int32_t barony_dynamic_array_resize(DynamicArray*, int64_t elem_size, int32_t new_len);
    void    barony_dynamic_array_pop_back(DynamicArray*, int64_t elem_size);
    void    barony_dynamic_array_destroy(DynamicArray*);
}

template <typename T> void push(DynamicArray& a, const T& v) { barony_dynamic_array_append(&a, &v, sizeof(T)); }
int64_t elems(const DynamicArray& a) { return a.len / (int64_t)sizeof(int); }

bool cmp(const DynamicArray& a, const std::vector<int>& v, const char* what) {
    bool ok = elems(a) == (int64_t)v.size();
    if (ok) for (size_t i = 0; i < v.size(); ++i)
        if (((int*)a.data)[i] != v[i]) { ok = false; break; }
    printf("%-18s %s (elems=%lld vs %zu)\n", what, ok ? "OK" : "FAIL", (long long)elems(a), v.size());
    return ok;
}

int main() {
    bool all = true;
    DynamicArray a{}; barony_dynamic_array_init(&a);
    std::vector<int> ref;

    for (int i = 0; i < 10; ++i) { push(a, i); ref.push_back(i); }
    all &= cmp(a, ref, "append 0..9");

    barony_dynamic_array_erase(&a, 4, sizeof(int));
    ref.erase(ref.begin() + 4);
    all &= cmp(a, ref, "erase idx 4");

    barony_dynamic_array_erase(&a, 0, sizeof(int));
    ref.erase(ref.begin() + 0);
    all &= cmp(a, ref, "erase idx 0");

    barony_dynamic_array_erase(&a, (int32_t)(elems(a) - 1), sizeof(int));
    ref.erase(ref.begin() + (ref.size() - 1));
    all &= cmp(a, ref, "erase last");

    int v99 = 99;
    barony_dynamic_array_insert(&a, 2, &v99, sizeof(v99));
    ref.insert(ref.begin() + 2, 99);
    all &= cmp(a, ref, "insert 99 @2");

    int v7 = 7;
    barony_dynamic_array_insert(&a, 0, &v7, sizeof(v7));
    ref.insert(ref.begin(), 7);
    all &= cmp(a, ref, "insert 7 @0");

    int v5 = 5;
    barony_dynamic_array_insert(&a, (int32_t)elems(a), &v5, sizeof(v5));
    ref.insert(ref.end(), 5);
    all &= cmp(a, ref, "insert 5 @end");

    barony_dynamic_array_resize(&a, sizeof(int), 20);
    ref.resize(20, 0);
    all &= cmp(a, ref, "resize 20");

    barony_dynamic_array_resize(&a, sizeof(int), 7);
    ref.resize(7);
    all &= cmp(a, ref, "resize 7");

    barony_dynamic_array_pop_back(&a, sizeof(int));
    ref.pop_back();
    barony_dynamic_array_pop_back(&a, sizeof(int));
    ref.pop_back();
    all &= cmp(a, ref, "pop_back x2");

    barony_dynamic_array_clear(&a);
    ref.clear();
    all &= cmp(a, ref, "clear");

    for (int i = 100; i < 103; ++i) { push(a, i); ref.push_back(i); }
    all &= cmp(a, ref, "re-append after clear");

    barony_dynamic_array_destroy(&a);
    printf(all ? "\nALL PASS\n" : "\nSOME FAILED\n");
    return all ? 0 : 1;
}
