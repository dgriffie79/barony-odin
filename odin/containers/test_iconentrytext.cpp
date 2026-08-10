#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapIconEntryText m;
        IconEntryText_tMirror v;
        v.bannerText = "banner";
        v.bannerHighlights.insert(1);
        v.bannerHighlights.insert(2);
        v.worldMsg = "msg";
        v.worldMsgEmote = "emote";
        v.worldIconTag = "tag";
        m.put("k1", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        IconEntryText_tMirror out;
        bool ok = m.get("k1", out);
        printf("get: %d banner='%s' (expect 1, banner)\n", (int)ok, out.bannerText.c_str());
        printf("worldMsg='%s' (expect msg)\n", out.worldMsg.c_str());
        printf("worldIconTag='%s' (expect tag)\n", out.worldIconTag.c_str());
        printf("contains(1): %d (expect 1)\n", (int)out.bannerHighlights.contains(1));
        printf("contains(9): %d (expect 0)\n", (int)out.bannerHighlights.contains(9));
        // overwrite (frees old owned members)
        v.bannerText = "new";
        v.bannerHighlights.clear();
        v.bannerHighlights.insert(42);
        m.put("k1", v);
        m.get("k1", out);
        printf("after overwrite banner='%s' (expect new)\n", out.bannerText.c_str());
        printf("after overwrite contains(1): %d (expect 0)\n", (int)out.bannerHighlights.contains(1));
        printf("after overwrite contains(42): %d (expect 1)\n", (int)out.bannerHighlights.contains(42));
        // contains (string overloads)
        printf("contains std::string 'k1': %d (expect 1)\n", (int)m.contains(std::string("k1")));
        printf("contains 'nope': %d (expect 0)\n", (int)m.contains("nope"));
        // copy independence
        DynamicMapIconEntryText copy = m;
        IconEntryText_tMirror cv;
        copy.get("k1", cv);
        cv.bannerText = "mutated";
        cv.bannerHighlights.clear();
        IconEntryText_tMirror orig;
        m.get("k1", orig);
        printf("orig banner after copy-mutate: '%s' (expect new)\n", orig.bannerText.c_str());
        printf("orig contains(42): %d (expect 1)\n", (int)orig.bannerHighlights.contains(42));
        // erase
        printf("erase k1: %d (expect 1)\n", (int)m.erase("k1"));
        printf("size after erase: %lld (expect 0)\n", (long long)m.size());
        m.clear();
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("iconentrytext map test: PASS\n");
    return 0;
}
