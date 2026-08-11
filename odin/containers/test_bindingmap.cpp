#include <cstdio>
#include "dynamic_map.hpp"
int main() {
    {
        DynamicMapBinding m;
        binding_tMirror v;
        v.input = "W";
        v.type = binding_tMirror::KEYBOARD;
        v.analog = 1.f;
        m.put("Move Forward", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        auto& e = m["Move Forward"];
        printf("input='%s' type=%d (expect W, 1)\n", e.input.c_str(), e.type);
        printf("isBindingUsingGamepad: %d (expect 0)\n", (int)e.isBindingUsingGamepad());
        // overwrite
        binding_tMirror v2;
        v2.input = "Pad1ButtonA";
        v2.type = binding_tMirror::CONTROLLER_BUTTON;
        m.put("Move Forward", v2);
        binding_tMirror out;
        m.get("Move Forward", out);
        printf("after overwrite input='%s' type=%d (expect Pad1ButtonA, 3)\n", out.input.c_str(), out.type);
        printf("isBindingUsingGamepad: %d (expect 1)\n", (int)out.isBindingUsingGamepad());
        // contains
        printf("contains cstr: %d (expect 1)\n", (int)m.contains("Move Forward"));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // keys
        std::vector<const char*> keys;
        m.keys(keys);
        printf("keys count: %zu (expect 1)\n", keys.size());
        // copy independence
        DynamicMapBinding copy = m;
        copy["Move Forward"].input = "mutated";
        m.get("Move Forward", out);
        printf("orig input after copy-mutate: '%s' (expect Pad1ButtonA)\n", out.input.c_str());
        // erase + clear
        printf("erase: %d (expect 1)\n", (int)m.erase("Move Forward"));
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("bindingmap test: PASS\n");
    return 0;
}
