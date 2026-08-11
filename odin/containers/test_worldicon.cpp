#include <cstdio>
#include "dynamic_map.hpp"
// test-local definition (the game defines it in interface.cpp with colorblind logic)
bool colorblind_lobby = false;
DynamicString& WorldIconEntry_tMirror::getPlayerIconPath(const int playernum) {
    switch (playernum) {
        case 1: return pathPlayer1;
        case 2: return pathPlayer2;
        case 3: return pathPlayer3;
        case 4: return pathPlayer4;
        default: return pathPlayerX;
    }
}
const DynamicString& WorldIconEntry_tMirror::getPlayerIconPath(const int playernum) const {
    return const_cast<WorldIconEntry_tMirror*>(this)->getPlayerIconPath(playernum);
}

int main() {
    {
        DynamicMapWorldIconEntry m;
        WorldIconEntry_tMirror v;
        v.pathDefault = "default.png";
        v.pathPlayer1 = "p1.png";
        v.id = 7;
        m.put("k1", v);
        printf("size: %lld (expect 1)\n", (long long)m.size());
        // operator[] read + mutate (in-place via entry)
        auto& e = m["k1"];
        printf("default='%s' (expect default.png)\n", e.pathDefault.c_str());
        e.pathPlayer2 = "p2.png";
        printf("getPlayerIconPath(0)='%s' (expect p1.png)\n", e.getPlayerIconPath(0).c_str());
        // get (deep copy)
        WorldIconEntry_tMirror out;
        m.get("k1", out);
        printf("get pathPlayer2='%s' (expect p2.png)\n", out.pathPlayer2.c_str());
        // contains overloads
        printf("contains cstr: %d (expect 1)\n", (int)m.contains("k1"));
        printf("contains DynamicString: %d (expect 1)\n", (int)m.contains(DynamicString("k1")));
        printf("contains std::string: %d (expect 1)\n", (int)m.contains(std::string("k1")));
        printf("contains nope: %d (expect 0)\n", (int)m.contains("nope"));
        // overwrite via put (frees old owned)
        v.pathDefault = "new.png";
        m.put("k1", v);
        WorldIconEntry_tMirror o2;
        m.get("k1", o2);
        printf("after overwrite default='%s' (expect new.png)\n", o2.pathDefault.c_str());
        // copy independence
        DynamicMapWorldIconEntry copy = m;
        copy["k1"].pathDefault = "mutated";
        WorldIconEntry_tMirror orig;
        m.get("k1", orig);
        printf("orig default after copy-mutate: '%s' (expect new.png)\n", orig.pathDefault.c_str());
        // erase + clear
        printf("erase k1: %d (expect 1)\n", (int)m.erase("k1"));
        printf("size after erase: %lld (expect 0)\n", (long long)m.size());
        m.clear();
        printf("cleared: %lld (expect 0)\n", (long long)m.size());
    }
    printf("worldicon map test: PASS\n");
    return 0;
}
