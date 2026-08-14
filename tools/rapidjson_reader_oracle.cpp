// rapidjson_reader_oracle.cpp — dump rapidjson's complete number/type
// classification as a machine-readable spec. This is the contract the Odin
// reader must reproduce. Output: one line per token, tab-separated:
//   token \t IsInt IsUint IsInt64 IsUint64 IsDouble IsNumber IsBool IsString IsNull IsArray IsObject \t value
#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>

int main() {
    const char* tokens[] = {
        // integers (no . or e)
        "0", "1", "123",
        "-1", "-123", "-0",
        "2147483647", "-2147483648",
        "2147483648", "4294967295", "4294967296",
        "-2147483649",
        "9223372036854775807", "-9223372036854775808",
        "9223372036854775808", "18446744073709551615",
        "-9223372036854775809", "18446744073709551616",
        // doubles (has . or e)
        "1.0", "1.5", "0.0", "-0.0", "0.1", "123.0", "123.5",
        "1e20", "1e-7", "1e5", "1.5e3", "1E5", "1e+5", "1e05", "1e0",
        "3.141592653589793", "1e400", "1e-400",
        // bool/null/string/array/object
        "true", "false", "null", "\"str\"", "[]", "{}",
        // malformed (parse errors -> rapidjson leaves doc null)
        "+1", "01", ".5", "1.", "1e", "--1", "1,2", "nan", "Infinity",
    };

    for (const char* tok : tokens) {
        rapidjson::Document d;
        d.Parse(tok);
        const rapidjson::Value& v = d;
        std::printf("%s", tok);
        if (d.HasParseError()) {
            std::printf("\tPARSE_ERROR\n");
            continue;
        }
        std::printf("\t%d %d %d %d %d %d %d %d %d %d %d",
            v.IsInt(), v.IsUint(), v.IsInt64(), v.IsUint64(), v.IsDouble(),
            v.IsNumber(), v.IsBool(), v.IsString(), v.IsNull(), v.IsArray(), v.IsObject());
        if (v.IsInt())    std::printf("\tInt:%d", v.GetInt());
        if (v.IsUint())   std::printf("\tUint:%u", v.GetUint());
        if (v.IsInt64())  std::printf("\tInt64:%lld", (long long)v.GetInt64());
        if (v.IsUint64()) std::printf("\tUint64:%llu", (unsigned long long)v.GetUint64());
        if (v.IsDouble()) std::printf("\tDouble:%.17g", v.GetDouble());
        if (v.IsBool())   std::printf("\tBool:%d", v.GetBool());
        if (v.IsString()) std::printf("\tString:%s", v.GetString());
        if (v.IsNull())   std::printf("\tNull");
        if (v.IsArray())  std::printf("\tArray");
        if (v.IsObject()) std::printf("\tObject");
        std::printf("\n");
    }
    return 0;
}
