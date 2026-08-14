// check_parse.cpp — parse stdin JSON with rapidjson, report OK/FAIL.
// Used as the oracle cross-check for the Odin writer's output.
#include "rapidjson/document.h"

#include <cstdio>
#include <string>

int main() {
    std::string in;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), stdin)) > 0) in.append(buf, n);

    rapidjson::Document d;
    d.Parse(in.c_str());
    if (d.HasParseError()) {
        std::printf("PARSE ERROR offset %zu code %d\n", d.GetErrorOffset(), (int)d.GetParseError());
        return 1;
    }
    std::printf("PARSE OK\n");
    return 0;
}
