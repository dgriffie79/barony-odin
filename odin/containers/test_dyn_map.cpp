// test_dyn_map.cpp — verify DynamicMapI32 (interned-key map) against std::map
// semantics, with TEMPORARY keys (the case that would dangle with view storage).
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include "dynamic_map.hpp"

static int failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("  FAIL: %s\n", msg); ++failures; } } while (0)

int main() {
    // ---- basic put/get with literals ----
    {
        DynamicMapI32 m;
        m["ATK"] = 10;
        m["AC"] = 5;
        m["KNOCKBACK"] = 1;
        CHECK(m.size() == 3, "size after 3 puts");
        CHECK(m["ATK"] == 10, "read ATK");
        CHECK(m["AC"] == 5, "read AC");
        CHECK(m.contains("KNOCKBACK"), "contains KNOCKBACK");
        CHECK(!m.contains("MISSING"), "not contains MISSING");
        // operator[] inserts default on missing
        CHECK(m["NEW"] == 0, "operator[] missing -> 0");
        CHECK(m.size() == 4, "size after [] insert");
    }

    // ---- TEMPORARY KEYS (the critical case) ----
    {
        DynamicMapI32 m;
        for (int i = 0; i < 100; ++i) {
            std::string key = "key_" + std::to_string(i);  // temporary std::string
            m[key.c_str()] = i;                             // key dies after this line
        }
        CHECK(m.size() == 100, "size with temp keys");
        bool all = true;
        for (int i = 0; i < 100; ++i) {
            std::string key = "key_" + std::to_string(i);
            if (m[key.c_str()] != i) { all = false; break; }
        }
        CHECK(all, "temp keys survive (interned)");
        // duplicate keys dedup to one entry
        m["key_5"] = 555;
        CHECK(m.size() == 100, "duplicate key updates, no growth");
        CHECK(m["key_5"] == 555, "updated value");
    }

    // ---- operator[] reference stability (the auto& spellID case) ----
    {
        DynamicMapI32 m;
        m["foci_spell"] = 1;
        auto& spellID = m["foci_spell"];   // bind a reference
        CHECK(spellID == 1, "ref read");
        spellID = 42;                       // write through the reference
        CHECK(m["foci_spell"] == 42, "ref write persists");
        // re-access gives the same slot
        auto& spellID2 = m["foci_spell"];
        CHECK(&spellID == &spellID2, "reference is stable across []");
    }

    // ---- copy semantics ----
    {
        DynamicMapI32 a;
        a["X"] = 1;
        a["Y"] = 2;
        DynamicMapI32 b(a);                 // copy ctor
        CHECK(b.size() == 2 && b["X"] == 1 && b["Y"] == 2, "copy ctor");
        a["X"] = 100;                        // mutate source
        CHECK(b["X"] == 1, "copy is deep (source mutation isolated)");
        DynamicMapI32 c;
        c = a;                               // copy assign
        CHECK(c["X"] == 100 && c["Y"] == 2, "copy assign");
        c = c;                               // self-assign
        CHECK(c["X"] == 100, "self copy-assign");
    }

    // ---- move semantics ----
    {
        DynamicMapI32 a;
        a["M"] = 7;
        DynamicMapI32 b(std::move(a));
        CHECK(b["M"] == 7 && a.empty(), "move ctor");
        DynamicMapI32 c;
        c = std::move(b);
        CHECK(c["M"] == 7 && b.empty(), "move assign");
    }

    // ---- erase / clear ----
    {
        DynamicMapI32 m;
        m["A"] = 1;
        m["B"] = 2;
        CHECK(m.erase("A"), "erase existing");
        CHECK(!m.contains("A") && m.size() == 1, "erased");
        CHECK(!m.erase("ZZZ"), "erase missing -> false");
        m.clear();
        CHECK(m.empty(), "clear");
    }

    // ---- iteration (snapshot) ----
    {
        DynamicMapI32 m;
        m["alpha"] = 1;
        m["beta"] = 2;
        m["gamma"] = 3;
        DynamicMapI32::Entry entries[8];
        int32_t n = m.entryList(entries, 8);
        CHECK(n == 3, "entryList count");
        // verify all 3 present (order is hash-based, so check membership)
        bool found[3] = {false, false, false};
        for (int32_t i = 0; i < n; ++i) {
            if (std::strcmp(entries[i].key, "alpha") == 0) { found[0] = true; CHECK(entries[i].value == 1, "alpha val"); }
            if (std::strcmp(entries[i].key, "beta") == 0) { found[1] = true; CHECK(entries[i].value == 2, "beta val"); }
            if (std::strcmp(entries[i].key, "gamma") == 0) { found[2] = true; CHECK(entries[i].value == 3, "gamma val"); }
        }
        CHECK(found[0] && found[1] && found[2], "all entries present");
        // keys are NUL-terminated interned strings
        CHECK(entries[0].key_len == strlen(entries[0].key), "key NUL-terminated");
    }

    // ---- compare against std::map ground truth ----
    {
        DynamicMapI32 dm;
        std::map<std::string, int> sm;
        const char* keys[] = {"AC", "ATK", "RATE_OF_FIRE", "ARMOR_PIERCE", "KNOCKBACK", "FRAGILE", "EFF_PARALYZE"};
        for (int i = 0; i < 7; ++i) {
            std::string tmp = std::string(keys[i]) + std::to_string(i);  // temp keys
            dm[tmp.c_str()] = i * 3;
            sm[tmp] = i * 3;
        }
        CHECK(dm.size() == (int64_t)sm.size(), "size matches std::map");
        bool match = true;
        for (auto& kv : sm) {
            if (dm[kv.first.c_str()] != kv.second) { match = false; break; }
        }
        CHECK(match, "values match std::map");
        // erase match
        dm.erase("AC0");
        sm.erase("AC0");
        CHECK(dm.size() == (int64_t)sm.size(), "erase matches std::map");
    }

    // ---- find() iterator pattern (find != end(), find->second) ----
    {
        DynamicMapI32 m;
        m["ATK"] = 10;
        auto find = m.find("ATK");
        CHECK(find != m.end(), "find existing != end");
        CHECK(find->second == 10, "find->second");
        auto missing = m.find("NOPE");
        CHECK(missing == m.end(), "find missing == end");
        // find->first.c_str() must be STABLE after the iterator dies
        // (callers store it — the compendium crash bug)
        const char* stored;
        {
            auto f2 = m.find("ATK");
            stored = f2->first;
        }  // f2 dies here — stored must still be valid (interned)
        CHECK(std::strcmp(stored, "ATK") == 0, "find->first.c_str() stable after iterator death");
        CHECK(m["ATK"] == 10, "map still intact");
    }

    if (failures == 0) {
        printf("dyn map test: PASS (all checks)\n");
        return 0;
    } else {
        printf("dyn map test: %d FAILURES\n", failures);
        return 1;
    }
}
