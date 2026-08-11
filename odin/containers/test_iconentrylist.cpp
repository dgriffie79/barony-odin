#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapIconEntryList m;
        // write via operator[] + field mutation
        m["tinker_aim"] = IconEntry_tMirror();
        m["tinker_aim"].name = "aim";
        m["tinker_aim"].id = 5;
        m["tinker_aim"].path_active = "active.png";
        // nested text_map write
        IconEntryTextMap_t tm;
        tm.text = "msg";
        tm.highlights.insert(1);
        m["tinker_aim"].text_map.put("key1", tm);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["tinker_aim"];
        printf("name='%s' id=%d path_active='%s' (expect aim, 5, active.png)\n", e.name.c_str(), e.id, e.path_active.c_str());
        IconEntryTextMap_t tmout;
        bool tok = m["tinker_aim"].text_map.get("key1", tmout);
        printf("nested text_map get: %d text='%s' (expect 1, msg)\n", (int)tok, tmout.text.c_str());
        printf("nested contains(1): %d (expect 1)\n", (int)tmout.highlights.contains(1));
        // contains overloads
        printf("contains cstr: %d (expect 1)\n", (int)m.contains("tinker_aim"));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // copy independence (deep-copies nested text_map too)
        DynamicMapIconEntryList copy = m;
        copy["tinker_aim"].name = "mutated";
        copy["tinker_aim"].text_map.erase("key1");
        IconEntry_tMirror orig;
        m.get("tinker_aim", orig);
        printf("orig name after copy-mutate: '%s' (expect aim)\n", orig.name.c_str());
        IconEntryTextMap_t otm;
        bool ook = orig.text_map.get("key1", otm);
        printf("orig nested text_map intact: %d (expect 1)\n", (int)ook);
        // overwrite via put (frees old owned incl nested)
        IconEntry_tMirror v2;
        v2.name = "aim2";
        m.put("tinker_aim", v2);
        IconEntry_tMirror o2;
        m.get("tinker_aim", o2);
        printf("after overwrite name='%s' (expect aim2)\n", o2.name.c_str());
        printf("after overwrite nested empty: %lld (expect 0)\n", (long long)o2.text_map.size());
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("tinker_aim"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("iconentrylist map test: PASS\n");
    return 0;
}
