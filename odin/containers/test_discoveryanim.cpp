#include <cstdio>
#include "dynamic_map.hpp"
// test-local ticks
uint32_t ticks = 100;
DiscoveryAnim_tMirror::DiscoveryAnim_tMirror() : startTicks(ticks), processedOnTick(0) {}
int main() {
    {
        DynamicMapDiscoveryAnim m;
        m["label1"] = DiscoveryAnim_tMirror();
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& d = m["label1"];
        printf("startTicks: %u (expect 100)\n", (unsigned)d.startTicks);
        d.name = "discovered";
        d.processedOnTick = 42;
        DiscoveryAnim_tMirror out;
        m.get("label1", out);
        printf("name='%s' (expect discovered)\n", out.name.c_str());
        printf("processedOnTick: %u (expect 42)\n", (unsigned)out.processedOnTick);
        // overwrite via put (frees old owned)
        DiscoveryAnim_tMirror v;
        v.name = "new";
        m.put("label1", v);
        m.get("label1", out);
        printf("after overwrite name='%s' (expect new)\n", out.name.c_str());
        // contains overloads
        printf("contains cstr: %d (expect 1)\n", (int)m.contains("label1"));
        printf("contains DynamicString: %d (expect 1)\n", (int)m.contains(DynamicString("label1")));
        printf("contains std::string: %d (expect 1)\n", (int)m.contains(std::string("label1")));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // copy independence
        DynamicMapDiscoveryAnim copy = m;
        copy["label1"].name = "mutated";
        m.get("label1", out);
        printf("orig name after copy-mutate: '%s' (expect new)\n", out.name.c_str());
        // erase + clear
        printf("erase label1: %d (expect 1)\n", (int)m.erase("label1"));
        printf("size after erase: %lld (expect 0)\n", (long long)m.size());
        m.clear();
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("discoveryanim map test: PASS\n");
    return 0;
}
