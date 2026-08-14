#include "rapidjson/document.h"
#include <cstdio>
int main() {
    const char* json = "{\"a\":1,\"a\":2,\"b\":3,\"a\":4}";
    rapidjson::Document d;
    d.Parse(json);
    std::printf("HasParseError=%d\n", (int)d.HasParseError());
    std::printf("HasMember(a)=%d\n", (int)d.HasMember("a"));
    std::printf("d[a].GetInt()=%d\n", d["a"].GetInt());
    std::printf("MemberCount=%u\n", d.MemberCount());
    std::printf("iteration order:\n");
    for (auto it = d.MemberBegin(); it != d.MemberEnd(); ++it) {
        std::printf("  %s = %d\n", it->name.GetString(), it->value.GetInt());
    }
    return 0;
}
