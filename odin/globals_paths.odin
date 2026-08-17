// globals_paths.odin — Odin-owner storage for the extern globals of paths.hpp.
// Ownership flip (PORTING.md §3): C++ references these via extern "C";
// Odin owns the storage here with @(export).
package main

// extern "C" int* pathMapFlying;  -> Odin single owner
@(export)
pathMapFlying : ^i32

// extern "C" int* pathMapGrounded;  -> Odin single owner
@(export)
pathMapGrounded : ^i32

// extern "C" int pathMapZone;  -> Odin single owner
@(export)
pathMapZone : i32

// extern "C" int lastGeneratePathTries;  -> Odin single owner
@(export)
lastGeneratePathTries : i32
