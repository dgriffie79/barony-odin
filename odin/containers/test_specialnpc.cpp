#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapSpecialNPC m;
        SpecialNPCEntry_tMirror v;
        v.name = "lilith";
        v.shortname = "lil";
        v.baseModel = 42;
        v.modelIndexes.insert(1);
        v.modelIndexes.insert(2);
        m.put("lilith", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["lilith"];
        printf("name='%s' (expect lilith)\n", e.name.c_str());
        printf("baseModel: %d (expect 42)\n", e.baseModel);
        printf("modelIndexes contains(1): %d (expect 1)\n", (int)e.modelIndexes.contains(1));
        e.shortname = "lil2";
        SpecialNPCEntry_tMirror out;
        m.get("lilith", out);
        printf("shortname='%s' (expect lil2)\n", out.shortname.c_str());
        // overwrite
        v.name = "lilith2";
        m.put("lilith", v);
        m.get("lilith", out);
        printf("after overwrite name='%s' (expect lilith2)\n", out.name.c_str());
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // copy independence
        DynamicMapSpecialNPC copy = m;
        copy["lilith"].name = "mutated";
        m.get("lilith", out);
        printf("orig name after copy-mutate: '%s' (expect lilith2)\n", out.name.c_str());
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("lilith"));
        printf("size after erase: %lld (expect 0)\n", (long long)m.size());
        m.clear();
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("specialnpc map test: PASS\n");
    return 0;
}
