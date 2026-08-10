#include <cstdio>
#include <string>
#include "dynamic_map.hpp"

// Regression for map<int,string> (DynamicMapI32Str): owned values, RAII-safe
// operator[] writes, erase/clear/destroy free owned buffers (no double-free),
// find/at deep-copy semantics.
int main() {
    {
        DynamicMapI32Str m;
        m[1] = "foo";
        m[2] = "bar";
        m[3] = std::string("baz");
        printf("size: %lld (expect 3)\n", (long long)m.size());
        printf("[1]: '%s' (expect foo)\n", m[1].c_str());
        printf("at(2): '%s' (expect bar)\n", m.at(2).c_str());
        printf("contains(3): %d (expect 1)\n", (int)m.contains(3));
        printf("contains(99): %d (expect 0)\n", (int)m.contains(99));
        // overwrite via operator[] (owned-value reassignment)
        for (int i = 0; i < 100; ++i) {
            m[1] = std::string("val") + std::to_string(i);
        }
        printf("after 100 overwrites [1]: '%s' (expect val99)\n", m[1].c_str());
        // find
        auto it = m.find(2);
        printf("find(2): valid=%d val='%s' (expect 1, bar)\n", it.valid, it->second.c_str());
        auto notfound = m.find(99);
        printf("find(99): valid=%d (expect 0)\n", notfound.valid);
        // entryList
        DynamicMapI32Str::Entry entries[8];
        int32_t n = m.entryList(entries, 8);
        printf("entryList: %d entries\n", n);
        // erase
        printf("erase(2): %d (expect 1)\n", (int)m.erase(2));
        printf("erase(2) again: %d (expect 0)\n", (int)m.erase(2));
        printf("after erase size: %lld (expect 2)\n", (long long)m.size());
        // copy
        DynamicMapI32Str copy = m;
        printf("copy size: %lld (expect 2)\n", (long long)copy.size());
        printf("copy[3]: '%s' (expect baz)\n", copy[3].c_str());
        copy[1] = "changed";
        printf("orig[1] after copy change: '%s' (expect val99 - independent)\n", m[1].c_str());
        // clear
        m.clear();
        printf("after clear size: %lld (expect 0)\n", (long long)m.size());
    }
    printf("i32str map test: PASS\n");
    return 0;
}
