#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapIconEntryCallout m;
        m["look"] = IconEntryCallout_tMirror();
        m["look"].name = "look";
        m["look"].path_active = "act.png";
        IconEntryText_tMirror tm;
        tm.bannerText = "banner";
        tm.bannerHighlights.insert(2);
        m["look"].text_map.put("key1", tm);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["look"];
        printf("name='%s' path_active='%s' (expect look, act.png)\n", e.name.c_str(), e.path_active.c_str());
        IconEntryText_tMirror tmout;
        bool tok = m["look"].text_map.get("key1", tmout);
        printf("nested text_map get: %d banner='%s' (expect 1, banner)\n", (int)tok, tmout.bannerText.c_str());
        printf("nested contains(2): %d (expect 1)\n", (int)tmout.bannerHighlights.contains(2));
        // copy independence (nested deep-copied)
        DynamicMapIconEntryCallout copy = m;
        copy["look"].name = "mutated";
        copy["look"].text_map.erase("key1");
        IconEntryCallout_tMirror orig;
        m.get("look", orig);
        printf("orig name after copy-mutate: '%s' (expect look)\n", orig.name.c_str());
        IconEntryText_tMirror otm;
        bool ook = orig.text_map.get("key1", otm);
        printf("orig nested intact: %d (expect 1)\n", (int)ook);
        // overwrite
        IconEntryCallout_tMirror v2;
        v2.name = "look2";
        m.put("look", v2);
        IconEntryCallout_tMirror o2;
        m.get("look", o2);
        printf("after overwrite name='%s' (expect look2)\n", o2.name.c_str());
        printf("after overwrite nested empty: %lld (expect 0)\n", (long long)o2.text_map.size());
        printf("erase: %d (expect 1)\n", (int)m.erase("look"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("iconcallout map test: PASS\n");
    return 0;
}
