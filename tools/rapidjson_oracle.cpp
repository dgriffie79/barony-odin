// rapidjson_oracle.cpp — empirically characterize rapidjson's exact behavior
// so the Odin JSON engine can be built to match it byte-for-byte / type-for-type.
//
// Prints, for a fixed corpus:
//   A) parse-time number/type classification (IsInt/IsUint/IsInt64/IsUint64/IsDouble/...)
//   B) writer scalar emission (Int/Uint/Int64/Uint64/Double/Float/Bool/String)
//   C) PrettyWriter formatting (default vs SetIndent(' ',2) + kFormatSingleLineArray)
//
// Compile (header-only, no link deps beyond the CRT):
//   cl /nologo /std:c++17 /EHsc /I<vcpkg>/include tools/rapidjson_oracle.cpp
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <cstdio>
#include <cstring>
#include <string>

static void dump_bytes(const char* label, const char* s) {
    std::printf("%s = [", label);
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '\n': std::printf("\\n"); break;
            case '\r': std::printf("\\r"); break;
            case '\t': std::printf("\\t"); break;
            case ' ':  std::printf("·");  break;
            default:   std::putchar(*p);  break;
        }
    }
    std::printf("]\n");
}

static void classify(const char* json) {
    rapidjson::Document d;
    d.Parse(json);
    const rapidjson::Value& v = d;
    std::printf("%-28s", json);
    std::printf(" IsNull=%d IsBool=%d IsNumber=%d IsInt=%d IsUint=%d IsInt64=%d IsUint64=%d IsDouble=%d IsString=%d IsArray=%d IsObject=%d",
        v.IsNull(), v.IsBool(), v.IsNumber(),
        v.IsInt(), v.IsUint(), v.IsInt64(), v.IsUint64(), v.IsDouble(),
        v.IsString(), v.IsArray(), v.IsObject());
    if (v.IsInt())    std::printf(" Int=%d", v.GetInt());
    if (v.IsUint())   std::printf(" Uint=%u", v.GetUint());
    if (v.IsInt64())  std::printf(" Int64=%lld", (long long)v.GetInt64());
    if (v.IsUint64()) std::printf(" Uint64=%llu", (unsigned long long)v.GetUint64());
    if (v.IsDouble()) std::printf(" Double=%.17g", v.GetDouble());
    if (v.IsString()) std::printf(" String=\"%s\"", v.GetString());
    std::printf("\n");
}

int main() {
    std::printf("=== A) parse-time classification ===\n");
    const char* nums[] = {
        "0", "1", "123", "-1", "-123", "2147483647", "-2147483648",
        "2147483648", "4294967295", "4294967296", "9223372036854775807",
        "-9223372036854775808", "18446744073709551615",
        "1.0", "1.5", "0.0", "-0.0", "0.1", "1e20", "1e-7", "3.141592653589793",
        "1e400", "1e-400", "true", "false", "\"str\"", "null", "[]", "{}",
    };
    for (const char* n : nums) classify(n);

    std::printf("\n=== B) writer scalar emission ===\n");
    {
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();
        w.Key("i_neg");   w.Int(-1);
        w.Key("i_zero");  w.Int(0);
        w.Key("i_pos");   w.Int(123);
        w.Key("u_zero");  w.Uint(0u);
        w.Key("u_pos");   w.Uint(123u);
        w.Key("u_max");   w.Uint(4294967295u);
        w.Key("i64");     w.Int64(-9223372036854775807LL - 1);
        w.Key("u64");     w.Uint64(18446744073709551615ULL);
        w.Key("d_one");   w.Double(1.0);
        w.Key("d_half");  w.Double(1.5);
        w.Key("d_zero");  w.Double(0.0);
        w.Key("d_neg0");  w.Double(-0.0);
        w.Key("d_tenth"); w.Double(0.1);
        w.Key("d_third"); w.Double(1.0 / 3.0);
        w.Key("d_pi");    w.Double(3.141592653589793);
        w.Key("d_e20");   w.Double(1e20);
        w.Key("d_e-7");   w.Double(1e-7);
        w.Key("d_big");   w.Double(1.7976931348623157e308);
        w.Key("d_small"); w.Double(5e-324);
        w.Key("f_tenth"); w.Double(0.1f);   // float promoted to double (game path)
        w.Key("b_true");  w.Bool(true);
        w.Key("b_false"); w.Bool(false);
        w.Key("s_plain"); w.String("hello");
        w.Key("s_esc");   w.String("a\"b\\c\nd\te");
        w.EndObject();
        dump_bytes("compact", sb.GetString());
    }

    std::printf("\n=== C) PrettyWriter formatting ===\n");
    {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
        w.StartObject();
        w.Key("a"); w.Int(1);
        w.Key("arr"); w.StartArray(); w.Int(1); w.Int(2); w.Int(3); w.EndArray();
        w.Key("nested"); w.StartObject(); w.Key("x"); w.Double(1.5); w.EndObject();
        w.EndObject();
        dump_bytes("pretty_default", sb.GetString());
    }
    {
        rapidjson::StringBuffer sb;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
        w.SetIndent(' ', 2);
        w.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
        w.StartObject();
        w.Key("a"); w.Int(1);
        w.Key("arr"); w.StartArray(); w.Int(1); w.Int(2); w.Int(3); w.EndArray();
        w.Key("nested"); w.StartObject(); w.Key("x"); w.Double(1.5); w.EndObject();
        w.EndObject();
        dump_bytes("pretty_compact", sb.GetString());
    }

    return 0;
}
