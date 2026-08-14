// verify_roundtrip.cpp — parse the Odin JSON writer's output with rapidjson and
// verify every field's type classification + bit-exact value round-trip.
// Reads JSON from stdin, prints PASS/FAIL per field, exits non-zero on any failure.
#include "rapidjson/document.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

static int failures = 0;

static void check_bool(const rapidjson::Value& d, const char* key, bool expect) {
    const rapidjson::Value& v = d[key];
    bool ok = v.IsBool() && v.GetBool() == expect;
    std::printf("  %-10s %s (IsBool=%d value=%d)\n", key, ok ? "PASS" : "FAIL", v.IsBool(), v.IsBool() ? v.GetBool() : -1);
    if (!ok) ++failures;
}

static void check_int(const rapidjson::Value& d, const char* key, int expect) {
    const rapidjson::Value& v = d[key];
    bool ok = v.IsInt() && v.GetInt() == expect;
    std::printf("  %-10s %s (IsInt=%d value=%d)\n", key, ok ? "PASS" : "FAIL", v.IsInt(), v.IsInt() ? v.GetInt() : -999);
    if (!ok) ++failures;
}

static void check_uint(const rapidjson::Value& d, const char* key, unsigned expect) {
    const rapidjson::Value& v = d[key];
    bool ok = v.IsUint() && v.GetUint() == expect;
    std::printf("  %-10s %s (IsUint=%d value=%u)\n", key, ok ? "PASS" : "FAIL", v.IsUint(), v.IsUint() ? v.GetUint() : 0u);
    if (!ok) ++failures;
}

static void check_double(const rapidjson::Value& d, const char* key, double expect) {
    const rapidjson::Value& v = d[key];
    // bit-exact compare (== is exact for doubles; also check signbit for -0.0)
    bool ok = v.IsDouble() && v.GetDouble() == expect;
    if (expect == 0.0) ok = ok && (std::signbit(v.GetDouble()) == std::signbit(expect));
    std::printf("  %-10s %s (IsDouble=%d value=%.17g)\n", key, ok ? "PASS" : "FAIL", v.IsDouble(), v.IsDouble() ? v.GetDouble() : -999.0);
    if (!ok) ++failures;
}

static void check_string(const rapidjson::Value& d, const char* key, const char* expect) {
    const rapidjson::Value& v = d[key];
    bool ok = v.IsString() && std::strcmp(v.GetString(), expect) == 0;
    std::printf("  %-10s %s (IsString=%d)\n", key, ok ? "PASS" : "FAIL", v.IsString());
    if (!ok) ++failures;
}

int main() {
    // read all stdin
    std::string input;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), stdin)) > 0) input.append(buf, n);

    rapidjson::Document d;
    d.Parse(input.c_str());
    if (d.HasParseError() || !d.IsObject()) {
        std::printf("PARSE ERROR at offset %zu\n", d.GetErrorOffset());
        return 2;
    }

    std::printf("ints/uints:\n");
    check_int(d, "i_neg", -1);
    check_int(d, "i_zero", 0);
    check_int(d, "i_pos", 123);
    check_uint(d, "u_zero", 0u);
    check_uint(d, "u_pos", 123u);
    check_uint(d, "u_max", 4294967295u);

    std::printf("doubles (bit-exact round-trip):\n");
    check_double(d, "d_one", 1.0);
    check_double(d, "d_half", 1.5);
    check_double(d, "d_zero", 0.0);
    check_double(d, "d_neg0", -0.0);
    check_double(d, "d_tenth", 0.1);
    check_double(d, "d_third", 1.0 / 3.0);
    check_double(d, "d_pi", 3.141592653589793);
    check_double(d, "d_e20", 1e20);
    check_double(d, "d_e-7", 1e-7);
    check_double(d, "d_big", 1.7976931348623157e308);
    check_double(d, "d_small", 5e-324);
    check_double(d, "f_tenth", 0.10000000149011612);

    std::printf("bools/strings:\n");
    check_bool(d, "b_true", true);
    check_bool(d, "b_false", false);
    check_string(d, "s_plain", "hello");
    check_string(d, "s_esc", "a\"b\\c\nd\te");

    std::printf("\n%s (%d failures)\n", failures == 0 ? "ALL PASS" : "FAILURES", failures);
    return failures == 0 ? 0 : 1;
}
