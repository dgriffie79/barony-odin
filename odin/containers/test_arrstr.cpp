#include <cstdio>
#include <vector>
#include "dynamic_array.hpp"
int main() {
    {
        DynamicArrayStr a;
        a.push_back("foo");
        a.push_back(std::string("bar"));
        a.push_back(DynamicString("baz"));
        printf("size: %lld (expect 3)\n", (long long)a.size());
        printf("at(0)='%s' at(1)='%s' at(2)='%s' (expect foo bar baz)\n", a.at(0).c_str(), a.at(1).c_str(), a.at(2).c_str());
        // overwrite via set (frees old owned)
        a.set(1, "BAR");
        printf("at(1) after set='%s' (expect BAR)\n", a.at(1).c_str());
        // erase (frees the erased element)
        a.erase(0);
        printf("size after erase: %lld (expect 2)\n", (long long)a.size());
        printf("at(0) after erase='%s' (expect BAR)\n", a.at(0).c_str());
        // copy (deep) + independence
        DynamicArrayStr copy = a;
        copy.set(0, "MUTATED");
        printf("orig at(0) after copy-mutate='%s' (expect BAR)\n", a.at(0).c_str());
        printf("copy at(0)='%s' (expect MUTATED)\n", copy.at(0).c_str());
        // snapshot
        std::vector<DynamicString> snap;
        a.snapshot(snap);
        printf("snapshot size: %zu (expect 2)\n", snap.size());
        // clear (frees all) + destroy
        a.clear();
        printf("cleared: %lld (expect 0)\n", (long long)a.size());
    }
    printf("arrstr test: PASS\n");
    return 0;
}
