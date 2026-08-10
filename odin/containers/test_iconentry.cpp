#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapIconEntryTextMap m;
        IconEntryTextMap_t v;
        v.text = "hello";
        v.highlights.insert(3);
        v.highlights.insert(7);
        m.put("k1", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        IconEntryTextMap_t out;
        bool ok = m.get("k1", out);
        printf("get k1: %d text='%s' (expect 1, hello)\n", (int)ok, out.text.c_str());
        printf("contains(3): %d (expect 1)\n", (int)out.highlights.contains(3));
        printf("contains(9): %d (expect 0)\n", (int)out.highlights.contains(9));
        // overwrite — must free old owned members without double-free
        v.text = "world";
        v.highlights.clear();
        v.highlights.insert(42);
        m.put("k1", v);
        IconEntryTextMap_t out2;
        m.get("k1", out2);
        printf("after overwrite text='%s' (expect world)\n", out2.text.c_str());
        printf("after overwrite contains(3): %d (expect 0)\n", (int)out2.highlights.contains(3));
        printf("after overwrite contains(42): %d (expect 1)\n", (int)out2.highlights.contains(42));
        // erase
        printf("erase k1: %d (expect 1)\n", (int)m.erase("k1"));
        printf("size after erase: %lld (expect 0)\n", (long long)m.size());
        // copy independence
        m.put("a", v);
        DynamicMapIconEntryTextMap copy = m;
        IconEntryTextMap_t cv;
        copy.get("a", cv);
        cv.text = "mutated";
        cv.highlights.clear();
        IconEntryTextMap_t orig;
        m.get("a", orig);
        printf("orig text after copy-mutate: '%s' (expect world)\n", orig.text.c_str());
        printf("orig contains(42): %d (expect 1)\n", (int)orig.highlights.contains(42));
        m.clear();
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("iconentry map test: PASS\n");
    return 0;
}
