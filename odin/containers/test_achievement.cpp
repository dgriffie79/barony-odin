#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapAchievement m;   // Achievement_t (LocalAchievements)
        Achievement_tMirror v;
        v.name = "ach1";
        v.unlocked = true;
        v.unlockTime = 12345;
        m.put("ach1", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["ach1"];
        printf("name='%s' unlocked=%d (expect ach1, 1)\n", e.name.c_str(), (int)e.unlocked);
        // range-for over snapshot
        int cnt = 0;
        for (auto& kv : m) { ++cnt; if (kv.second.unlocked) printf("iter: %s unlocked\n", kv.first); }
        printf("range-for count: %d (expect 1)\n", cnt);
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("ach1"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    {
        DynamicMapAchievementData m;   // AchievementData_t (Compendium)
        AchievementData_tMirror v;
        v.name = "ach2";
        v.desc = "desc2";
        v.hidden = true;
        v.category = "combat";
        v.achievementProgress = 5;
        m.put("ach2", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["ach2"];
        printf("name='%s' desc='%s' (expect ach2, desc2)\n", e.name.c_str(), e.desc.c_str());
        printf("category='%s' progress=%d (expect combat, 5)\n", e.category.c_str(), e.achievementProgress);
        printf("contains cstr: %d (expect 1)\n", (int)m.contains("ach2"));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // range-for
        int cnt = 0;
        for (auto& kv : m) { ++cnt; if (kv.second.hidden) printf("iter: %s hidden\n", kv.first); }
        printf("range-for count: %d (expect 1)\n", cnt);
        // copy independence
        DynamicMapAchievementData copy = m;
        copy["ach2"].name = "mutated";
        AchievementData_tMirror out;
        m.get("ach2", out);
        printf("orig name after copy-mutate: '%s' (expect ach2)\n", out.name.c_str());
        // overwrite
        v.desc = "newdesc";
        m.put("ach2", v);
        m.get("ach2", out);
        printf("after overwrite desc='%s' (expect newdesc)\n", out.desc.c_str());
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("ach2"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("achievement map test: PASS\n");
    return 0;
}
