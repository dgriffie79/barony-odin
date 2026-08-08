// test_dyn_map_str.cpp — verify DynamicMapStr (map<string,string>) against
// std::map semantics, with TEMPORARY keys AND values (the dangling case).
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include "dynamic_map.hpp"

static int failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("  FAIL: %s\n", msg); ++failures; } } while (0)

int main() {
    // ---- basic put/get ----
    {
        DynamicMapStr m;
        m["special_npc"] = "merlin";
        m["slime_type"] = "terrain_spawn_override";
        CHECK(m.size() == 2, "size after 2 puts");
        CHECK(m["special_npc"] == "merlin", "read value");
        CHECK(m.contains("slime_type"), "contains");
        CHECK(!m.contains("missing"), "not contains");
    }

    // ---- TEMPORARY VALUES (the std::to_string case) ----
    {
        DynamicMapStr m;
        for (int i = 0; i < 100; ++i) {
            std::string val = std::to_string(i * 1000);  // temporary value
            std::string key = "key" + std::to_string(i); // temporary key
            m[key.c_str()] = val.c_str();                // both die after this
        }
        CHECK(m.size() == 100, "size with temp keys+values");
        bool all = true;
        for (int i = 0; i < 100; ++i) {
            std::string key = "key" + std::to_string(i);
            std::string expect = std::to_string(i * 1000);
            if (m[key.c_str()] != expect.c_str()) { all = false; break; }
        }
        CHECK(all, "temp keys+values survive (interned)");
    }

    // ---- operator[] reference stability ----
    {
        DynamicMapStr m;
        m["lich_num_summons"] = "1";
        auto& v = m["lich_num_summons"];
        CHECK(v == "1", "ref read");
        v = "5";  // write through the reference (like std::to_string assign)
        CHECK(m["lich_num_summons"] == "5", "ref write persists");
        auto& v2 = m["lich_num_summons"];
        CHECK(&v == &v2, "ref stable across []");
    }

    // ---- at() returns value ----
    {
        DynamicMapStr m;
        m["dummy_ticks"] = "123";
        CHECK(m.at("dummy_ticks") == "123", "at()");
    }

    // ---- copy semantics ----
    {
        DynamicMapStr a;
        a["A"] = "1";
        a["B"] = "2";
        DynamicMapStr b(a);
        CHECK(b.size() == 2 && b["A"] == "1", "copy ctor");
        a["A"] = "100";
        CHECK(b["A"] == "1", "copy deep (source isolated)");
        DynamicMapStr c;
        c = a;
        CHECK(c["A"] == "100", "copy assign");
    }

    // ---- erase / clear ----
    {
        DynamicMapStr m;
        m["X"] = "1";
        m["Y"] = "2";
        CHECK(m.erase("X"), "erase");
        CHECK(!m.contains("X") && m.size() == 1, "erased");
        m.clear();
        CHECK(m.empty(), "clear");
    }

    // ---- iteration ----
    {
        DynamicMapStr m;
        m["alpha"] = "a";
        m["beta"] = "bb";
        m["gamma"] = "ccc";
        DynamicMapStr::Entry entries[8];
        int32_t n = m.entryList(entries, 8);
        CHECK(n == 3, "entryList count");
        bool found[3] = {false, false, false};
        for (int32_t i = 0; i < n; ++i) {
            if (std::strcmp(entries[i].key, "alpha") == 0) { found[0] = true; CHECK(std::strcmp(entries[i].value, "a") == 0, "alpha val"); }
            if (std::strcmp(entries[i].key, "beta") == 0) { found[1] = true; CHECK(std::strcmp(entries[i].value, "bb") == 0, "beta val"); }
            if (std::strcmp(entries[i].key, "gamma") == 0) { found[2] = true; CHECK(std::strcmp(entries[i].value, "ccc") == 0, "gamma val"); }
        }
        CHECK(found[0] && found[1] && found[2], "all entries present");
    }

    // ---- vs std::map ground truth ----
    {
        DynamicMapStr dm;
        std::map<std::string, std::string> sm;
        for (int i = 0; i < 10; ++i) {
            std::string k = "stat" + std::to_string(i);
            std::string v = std::to_string(i * 7);
            dm[k.c_str()] = v.c_str();
            sm[k] = v;
        }
        CHECK(dm.size() == (int64_t)sm.size(), "size matches");
        bool match = true;
        for (auto& kv : sm) {
            if (dm[kv.first.c_str()] != kv.second.c_str()) { match = false; break; }
        }
        CHECK(match, "values match std::map");
    }

    if (failures == 0) {
        printf("dyn map str test: PASS (all checks)\n");
        return 0;
    } else {
        printf("dyn map str test: %d FAILURES\n", failures);
        return 1;
    }
}
