// rapidjson_struct_walk.cpp — parse a JSON document with rapidjson and walk it
// depth-first, dumping every scalar as:  path<TAB>kind<TAB>flags<TAB>value
// This is the oracle the Odin structural reader must match.
#include "rapidjson/document.h"

#include <cstdio>
#include <cstring>
#include <string>

static void dump_value(const rapidjson::Value& v, const std::string& path) {
    switch (v.GetType()) {
    case rapidjson::kNullType:
        std::printf("%s\tnull\n", path.c_str());
        break;
    case rapidjson::kTrueType:
        std::printf("%s\tbool\ttrue\n", path.c_str());
        break;
    case rapidjson::kFalseType:
        std::printf("%s\tbool\tfalse\n", path.c_str());
        break;
    case rapidjson::kStringType:
        std::printf("%s\tstring\t%s\n", path.c_str(), v.GetString());
        break;
    case rapidjson::kNumberType:
        std::printf("%s\tnumber\t%d %d %d %d %d",
            path.c_str(), v.IsInt(), v.IsUint(), v.IsInt64(), v.IsUint64(), v.IsDouble());
        if (v.IsInt())    std::printf("\tInt:%d", v.GetInt());
        if (v.IsUint())   std::printf("\tUint:%u", v.GetUint());
        if (v.IsInt64())  std::printf("\tInt64:%lld", (long long)v.GetInt64());
        if (v.IsUint64()) std::printf("\tUint64:%llu", (unsigned long long)v.GetUint64());
        if (v.IsDouble()) std::printf("\tDouble:%.17g", v.GetDouble());
        std::printf("\n");
        break;
    case rapidjson::kArrayType: {
        for (rapidjson::SizeType i = 0; i < v.Size(); ++i) {
            dump_value(v[i], path + "[" + std::to_string(i) + "]");
        }
        break;
    }
    case rapidjson::kObjectType: {
        for (rapidjson::Value::ConstMemberIterator it = v.MemberBegin(); it != v.MemberEnd(); ++it) {
            dump_value(it->value, path + "." + it->name.GetString());
        }
        break;
    }
    default:
        break;
    }
}

int main(int argc, char** argv) {
    const char* filename = argc > 1 ? argv[1] : "tools/test_doc.json";
    FILE* f = std::fopen(filename, "rb");
    if (!f) { std::fprintf(stderr, "cannot open %s\n", filename); return 2; }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::string data(size, '\0');
    std::fread(&data[0], 1, size, f);
    std::fclose(f);

    rapidjson::Document d;
    d.Parse(data.c_str());
    if (d.HasParseError()) {
        std::printf("PARSE ERROR at offset %zu\n", d.GetErrorOffset());
        return 2;
    }
    dump_value(d, "$");
    return 0;
}
