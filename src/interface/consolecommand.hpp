/*-------------------------------------------------------------------------------

    BARONY
    File: consolecommand.hpp
    Desc: console command class

    Copyright 2022 (c) Turning Wheel LLC, all rights reserved.
    See LICENSE for details.

-------------------------------------------------------------------------------*/

#pragma once

#include "../../odin/containers/dynamic_string.hpp"

extern "C" const char* FindConsoleCommand(const char* str, int index);

/*
 * How to define a console command:
 *
 * ConsoleCommand myCmd("/dothing", "put a helpful description here",
 *     [](int argc, const char** argv){
 *     // do something
 *     });
 *
 * They can be defined anywhere.
 */

typedef void (* ccmd_function)(int argc, const char** argv);

// Type tag carried by every registry entry. Commands and all cvar kinds share
// one sorted registry (see consolecommand.cpp); the tag lets iteration/dispatch
// tell them apart without RTTI or separate maps.
enum class ConsoleEntryType : int {
    Command,
    CvarBool,
    CvarInt,
    CvarFloat,
    CvarString,
    CvarVector4,
};

// 4-float POD used by Vector4 cvars. Moved here (from draw.hpp) so the console
// subsystem is self-contained; draw.hpp includes this header.
struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

// Registers an entry (command or cvar) into the single console registry.
// Defined in consolecommand.cpp; called by the constructors below. The registry
// is append-only at startup and lazily sorted on first lookup.
extern "C" void register_console_entry(class ConsoleCommand* e);

// Unified setter for every cvar. Dispatched by all Cvar* entries; looks the
// cvar up by argv[0] and applies argv[1..] per the entry's type tag.
extern "C" void cvar_setter(int argc, const char** argv);

class ConsoleCommand {
public:
    ConsoleCommand() = default;
    ConsoleCommand(const char* _name, const char* _desc, const ccmd_function _func)
        : name(_name), desc(_desc), type(ConsoleEntryType::Command), func(_func) {
        register_console_entry(this);
    }

    const char* name = nullptr;
    const char* desc = nullptr;
    ConsoleEntryType type = ConsoleEntryType::Command;
    ccmd_function func = nullptr;
    void* data_ptr = nullptr; // cvars: address of the typed data member below

    void operator()(int argc, const char** argv) {
        if (func) func(argc, argv);
    }
};

struct CvarBool : ConsoleCommand {
    bool data = false;
    CvarBool(const char* _name, bool v, const char* _desc = "")
        : ConsoleCommand() {
        name = _name; desc = _desc; type = ConsoleEntryType::CvarBool;
        func = &cvar_setter; data = v; data_ptr = &data;
        register_console_entry(this);
    }
    bool& operator*() { return data; }
    bool* operator->() { return &data; }
};

struct CvarInt : ConsoleCommand {
    int data = 0;
    CvarInt(const char* _name, int v, const char* _desc = "")
        : ConsoleCommand() {
        name = _name; desc = _desc; type = ConsoleEntryType::CvarInt;
        func = &cvar_setter; data = v; data_ptr = &data;
        register_console_entry(this);
    }
    int& operator*() { return data; }
    int* operator->() { return &data; }
};

struct CvarFloat : ConsoleCommand {
    float data = 0.f;
    CvarFloat(const char* _name, float v, const char* _desc = "")
        : ConsoleCommand() {
        name = _name; desc = _desc; type = ConsoleEntryType::CvarFloat;
        func = &cvar_setter; data = v; data_ptr = &data;
        register_console_entry(this);
    }
    float& operator*() { return data; }
    float* operator->() { return &data; }
};

struct CvarString : ConsoleCommand {
    DynamicString data;
    CvarString(const char* _name, const char* v, const char* _desc = "")
        : ConsoleCommand() {
        name = _name; desc = _desc; type = ConsoleEntryType::CvarString;
        func = &cvar_setter; data = v; data_ptr = &data;
        register_console_entry(this);
    }
    DynamicString& operator*() { return data; }
    DynamicString* operator->() { return &data; }
};

struct CvarVector4 : ConsoleCommand {
    Vector4 data;
    CvarVector4(const char* _name, const Vector4& v, const char* _desc = "")
        : ConsoleCommand() {
        name = _name; desc = _desc; type = ConsoleEntryType::CvarVector4;
        func = &cvar_setter; data = v; data_ptr = &data;
        register_console_entry(this);
    }
    Vector4& operator*() { return data; }
    Vector4* operator->() { return &data; }
};
