#include <cstdio>
#include <string>
#include "dynamic_map.hpp"

// Regression for DynamicSetI32 (set<int>) and DynamicSetStr (set<string>):
// insert/contains/erase/clear/copy/move/entries. String keys are interned
// (process-lifetime stable, never freed by the set).
int main() {
    {
        DynamicSetI32 s;
        printf("i32: insert(5)=%d (expect 1)\n", (int)s.insert(5));
        printf("i32: insert(7)=%d (expect 1)\n", (int)s.insert(7));
        printf("i32: insert(5) dup=%d (expect 0)\n", (int)s.insert(5));
        printf("i32: size=%lld (expect 2)\n", (long long)s.size());
        printf("i32: contains(5)=%d (expect 1)\n", (int)s.contains(5));
        printf("i32: contains(9)=%d (expect 0)\n", (int)s.contains(9));
        // entries snapshot
        int vals[8];
        int32_t n = s.entries(vals, 8);
        printf("i32: entries=%d (expect 2), vals=[%d,%d]\n", n, n>0?vals[0]:-1, n>1?vals[1]:-1);
        bool saw5=false, saw7=false;
        for (int i=0;i<n;i++){ if(vals[i]==5)saw5=true; if(vals[i]==7)saw7=true; }
        printf("i32: entries contain 5&7: %d (expect 1)\n", (int)(saw5&&saw7));
        // erase
        printf("i32: erase(7)=%d (expect 1)\n", (int)s.erase(7));
        printf("i32: erase(7) again=%d (expect 0)\n", (int)s.erase(7));
        printf("i32: size after erase=%lld (expect 1)\n", (long long)s.size());
        // copy independence
        DynamicSetI32 copy = s;
        copy.insert(99);
        printf("i32: orig contains 99=%d (expect 0)\n", (int)s.contains(99));
        printf("i32: copy contains 99=%d (expect 1)\n", (int)copy.contains(99));
        // move
        DynamicSetI32 moved = std::move(copy);
        printf("i32: moved size=%lld (expect 2)\n", (long long)moved.size());
        // clear
        s.clear();
        printf("i32: cleared size=%lld (expect 0)\n", (long long)s.size());
    }
    {
        DynamicSetStr s;
        printf("str: insert('foo')=%d (expect 1)\n", (int)s.insert("foo"));
        printf("str: insert('bar')=%d (expect 1)\n", (int)s.insert("bar"));
        printf("str: insert('foo') dup=%d (expect 0)\n", (int)s.insert("foo"));
        printf("str: size=%lld (expect 2)\n", (long long)s.size());
        printf("str: contains('foo')=%d (expect 1)\n", (int)s.contains("foo"));
        printf("str: contains('baz')=%d (expect 0)\n", (int)s.contains("baz"));
        printf("str: contains(DynamicString('bar'))=%d (expect 1)\n", (int)s.contains(DynamicString("bar")));
        // entries
        DynamicSetStr::Entry ents[8];
        int32_t n = s.entries(ents, 8);
        printf("str: entries=%d (expect 2)\n", n);
        bool sawFoo=false, sawBar=false;
        for (int i=0;i<n;i++){
            std::string k(ents[i].key, ents[i].key_len);
            if (k=="foo") sawFoo=true;
            if (k=="bar") sawBar=true;
        }
        printf("str: entries contain foo&bar: %d (expect 1)\n", (int)(sawFoo&&sawBar));
        // erase
        printf("str: erase('bar')=%d (expect 1)\n", (int)s.erase("bar"));
        printf("str: erase('bar') again=%d (expect 0)\n", (int)s.erase("bar"));
        // copy + move
        DynamicSetStr copy = s;
        copy.insert("baz");
        printf("str: orig contains baz=%d (expect 0)\n", (int)s.contains("baz"));
        printf("str: copy contains baz=%d (expect 1)\n", (int)copy.contains("baz"));
        DynamicSetStr moved = std::move(copy);
        printf("str: moved size=%lld (expect 2)\n", (long long)moved.size());
        s.clear();
        printf("str: cleared size=%lld (expect 0)\n", (long long)s.size());
    }
    printf("dynamic set test: PASS\n");
    return 0;
}
