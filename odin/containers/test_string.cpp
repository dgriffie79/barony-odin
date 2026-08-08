// test_string.cpp — verify the DynamicString RAII class + Odin shims
// against std::string semantics. Every method/shim gets exercised.
#include <cstdio>
#include <cstring>
#include <string>
#include <cassert>
#include "dynamic_string.hpp"

static int failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("  FAIL: %s\n", msg); ++failures; } else { /*printf("  ok: %s\n", msg);*/ } } while (0)

int main() {
    // ---- construction ----
    DynamicString a;                       // default
    CHECK(a.empty() && a.size() == 0, "default ctor empty");
    DynamicString b("hello");              // from cstr
    CHECK(b.size() == 5 && !b.empty(), "ctor from cstr size");
    CHECK(std::strcmp(b.c_str(), "hello") == 0, "ctor c_str content");
    DynamicString c("hello", 3);           // from bytes
    CHECK(c.size() == 3 && std::strcmp(c.c_str(), "hel") == 0, "ctor from bytes");

    // ---- copy semantics ----
    DynamicString d(b);                    // copy ctor
    CHECK(d == b && d.c_str() != b.c_str(), "copy ctor deep (content eq, ptr diff)");
    DynamicString e;
    e = b;                                  // copy assign
    CHECK(e == b && e.c_str() != b.c_str(), "copy assign deep");
    e = e;                                  // self-assign
    CHECK(e == b, "self copy-assign");

    // ---- move semantics ----
    DynamicString m1("temp");
    const char* m1ptr = m1.c_str();
    DynamicString m2(std::move(m1));
    CHECK(m2.c_str() == m1ptr, "move ctor transfers ptr");
    CHECK(m1.data == nullptr && m1.len == 0, "move ctor nulls source");
    DynamicString m3;
    m3 = std::move(m2);
    CHECK(m3.c_str() == m1ptr, "move assign transfers ptr");
    CHECK(m2.data == nullptr, "move assign nulls source");

    // ---- assignment / mutation ----
    a = "world";
    CHECK(a == "world" && a.size() == 5, "operator=(cstr)");
    a += "!";
    CHECK(a == "world!" && a.size() == 6, "operator+=(cstr)");
    DynamicString suffix("??");
    a += suffix;
    CHECK(a == "world!??" && a.size() == 8, "operator+=(DynamicString)");
    a.append("123", 2);
    CHECK(a == "world!??12", "append(bytes,n)");
    a.clear();
    CHECK(a.empty(), "clear empties");
    a.from_cstr("reset");
    CHECK(a == "reset", "from_cstr after clear");

    // ---- comparison ----
    DynamicString x("abc"), y("abd"), z("abc");
    CHECK(x == z && !(x == y), "== between strings");
    CHECK(x != y && !(x != z), "!= between strings");
    CHECK(x == "abc" && "abc" == x, "== with cstr both directions");
    CHECK(x != "abd" && "abd" != x, "!= with cstr both directions");
    CHECK(x < y && y > x, "< and >");
    CHECK(x <= z && x >= z, "<= and >=");
    CHECK(x.compare(y) < 0 && y.compare(x) > 0 && x.compare(z) == 0, "compare");
    DynamicString empty;
    CHECK(empty == "", "empty == cstr empty");
    DynamicString long1("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"); // > SSO (32B)
    DynamicString long2("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzy");
    CHECK(long1 != long2, "long string != (heap compare)");
    CHECK(long1 == DynamicString("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"), "long string ==");

    // ---- find ----
    DynamicString hay("the quick brown fox");
    CHECK(hay.find("quick") == 4, "find at 4");
    CHECK(hay.find("fox") == 16, "find at 16");
    CHECK(hay.find("nope") == -1, "find missing");
    CHECK(hay.find("quick", 5) == -1, "find with start past");
    CHECK(hay.find("the") == 0, "find at 0");

    // ---- substr ----
    DynamicString sub = hay.substr(4, 5);
    CHECK(sub == "quick", "substr(4,5)");
    DynamicString sub2 = hay.substr(16);
    CHECK(sub2 == "fox", "substr(16) to end");
    DynamicString sub3 = hay.substr(100);
    CHECK(sub3.empty(), "substr past end empty");

    // ---- operator+ ----
    DynamicString plus1 = DynamicString("foo") + "bar";
    CHECK(plus1 == "foobar", "string + cstr");
    DynamicString plus2 = "foo" + DynamicString("bar");
    CHECK(plus2 == "foobar", "cstr + string");
    DynamicString plus3 = DynamicString("foo") + DynamicString("bar");
    CHECK(plus3 == "foobar", "string + string");

    // ---- copy of a moved-from is safe ----
    DynamicString mm("orig");
    DynamicString moved(std::move(mm));
    DynamicString copy_of_moved(moved);
    CHECK(copy_of_moved == "orig", "copy of moved-from value still correct");

    // ---- lifecycle: many strings, no leaks/crashes ----
    for (int i = 0; i < 1000; ++i) {
        DynamicString tmp;
        tmp += "iteration ";
        tmp += std::to_string(i).c_str();
        CHECK(tmp.size() > 10, "loop string grows");
    }

    // ---- comparison against std::string ground truth ----
    {
        DynamicString ds("Barony test 123");
        std::string ss("Barony test 123");
        CHECK(ds.size() == (int64_t)ss.size(), "size matches std::string");
        CHECK(std::strcmp(ds.c_str(), ss.c_str()) == 0, "c_str matches std::string");
        CHECK(ds.find("test") == (int64_t)ss.find("test"), "find matches std::string");
        DynamicString dsub = ds.substr(7, 4);
        CHECK(dsub == ss.substr(7, 4).c_str(), "substr matches std::string");
        DynamicString dcat;
        dcat += ds;
        dcat += "!";
        std::string scat = ss + "!";
        CHECK(dcat == scat.c_str(), "concat matches std::string");
    }


    // ---- find(char) + at() (MainMenu float_warning_add path) ----
    {
        DynamicString warn("line1\nline2");
        int64_t nl = warn.find('\n');
        CHECK(nl == 5, "find(char) newline at 5");
        if (nl != -1) warn.at(nl) = ' ';
        CHECK(warn == "line1 line2", "at() replace newline");
        CHECK(warn[0] == 'l' && warn[1] == 'i', "operator[] read");
    }

    // ---- implicit ctor from const char* (non-explicit) ----
    {
        DynamicString implicit_ok = "implicit";
        CHECK(implicit_ok == "implicit", "implicit ctor from cstr");
        // lambda-style: function taking DynamicString by value from a cstr
        auto takes_dstr = [](DynamicString s) { return s.size(); };
        CHECK(takes_dstr("hello") == 5, "implicit ctor into by-value param");
    }

    if (failures == 0) {
        printf("string test: PASS (all checks)\n");
        return 0;
    } else {
        printf("string test: %d FAILURES\n", failures);
        return 1;
    }
}
