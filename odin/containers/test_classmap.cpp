#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapClass m;
        Class_tMirror c;
        c.dlc = 0;
        c.image = "icon.png";
        c.image_highlighted = "iconOn.png";
        c.image_locked = "iconLocked.png";
        m.put("barbarian", c);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["barbarian"];
        printf("dlc=%d image='%s' hl='%s' (expect 0, icon.png, iconOn.png)\n", e.dlc, e.image, e.image_highlighted);
        printf("contains: %d (expect 1)\n", (int)m.contains("barbarian"));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // overwrite
        Class_tMirror c2;
        c2.dlc = 1;
        c2.image = "icon2.png";
        m.put("barbarian", c2);
        Class_tMirror out;
        m.get("barbarian", out);
        printf("after overwrite dlc=%d image='%s' (expect 1, icon2.png)\n", out.dlc, out.image);
        // copy independence (non-owning pointers, plain copy)
        DynamicMapClass copy = m;
        copy["barbarian"].image = "mutated.png";
        m.get("barbarian", out);
        printf("orig image after copy-mutate: '%s' (expect icon2.png)\n", out.image);
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("barbarian"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("classmap test: PASS\n");
    return 0;
}
