# Porting Guide — Barony C++ → Odin

This document is the durable handoff for the C++→Odin port. The C++ tree is
the green reference; Odin is the target. Read this before doing port work.

## Goal

Port Barony from C++ to Odin, file-by-file. Odin is the executable
(`barony.exe`/`editor.exe`); the C++ builds as static libs
(`libbarony_game.a`/`libbarony_editor.a`) linked into the Odin exe. The C++
tree stays green and runnable until each file is ported, then the ported file's
C++ version is DELETED (replace, never wrap).

## Architecture

- `src/` — the C++ reference (stays green, runnable via `barony.exe`)
- `odin/` — one Odin package (the executable)
  - `main.odin` — driver: calls `barony_main`, `run_barony()` argv marshaling
  - `game.odin` / `editor.odin` — foreign decls for `barony_main`
    (game vs editor selected by `when #config(EDITOR, false)`)
  - `prng.odin` — ported RNG (RC4), verified byte-exact (unused yet)
  - `containers/` — container shims (see below)
- `builddir/` — Meson build output (gitignored)

## The C boundary (what crosses C++↔Odin)

Odin `foreign` is C ABI only — no mangled C++ symbols, no std::vector layout,
no methods. Crossings are `extern "C"` + plain types.

**Rule:** a function crosses the boundary only if C-mappable (raw pointers,
primitives, the shared container types below). Non-C-mappable functions (STL
in signature, templates, lambdas) never cross — they port with their callers.

**Rule (replace, never wrap):** when porting a function, DELETE the C++ version
and write the Odin proc. Never keep a C++ shell that delegates to Odin.

**Reverse calls:** with static linking, C++ objects can call Odin `@(export)`
procs (one-way C++→Odin). This is how ported leaves are used before their
callers port. The rejected pattern is a *surviving C++ API* that delegates.

## Container shims (odin/containers/) — the de-STL foundation

The shared structs (Entity, Stat, Item, Player, etc.) have STL members. These
are replaced with C++ mirrors of Odin's container layouts, so C++ and Odin
operate on the same memory:

| C++ member | C++ mirror type | Odin type | Size | Shims (Odin, `@(export)`) |
|---|---|---|---|---|
| `std::vector<T>` | `DynamicArray` | `[dynamic]T` | 40B | `barony_dynamic_array_*` |
| `std::map/unordered_map<K,V>` | `DynamicMap` (32B) | `map[[4]byte]V` / `map[string]V` | 32B | `barony_dynamic_map_*` |
| `std::string` | `DynamicString` | `string` | 16B | `barony_dynamic_string_*` |

All verified from C++ against std containers. Files:
- `odin/containers/dynamic_array.odin` + `dynamic_array.hpp` (C++ mirror + `dynarray_push/at/size` helpers)
- `odin/containers/dynamic_map.odin` — uses Odin's NATIVE map
- `odin/containers/dynamic_string.odin` + `dynamic_string.hpp` (C++ mirror, `.c_str()` returns data — shims guarantee NUL)

### Key design decisions
- **Maps use Odin's native hash map**, not hand-rolled pairs. Numeric keys
  (`int`/`Uint32`/enum, 4 bytes) → `map[[4]byte]V` (C++ passes key by pointer);
  string keys → `map[string]V` (DynamicString `{data,len}` is ABI-identical to
  Odin `string`, passes by value).
- **Strings are replaced BEFORE maps** so string-keyed maps are native
  `map[string]V` (no custom string-key path).
- **Odin shims must set `context = runtime.default_context()`** at the top of
  every proc — they're called from C++ where the Odin context is uninitialized.
- **Use out-params for multi-value returns.** `proc "c" -> (int, bool)` has
  ambiguous MSVC ABI; use a single bool return + pointer out-param.
- **`[N]byte` is a valid map key** (arrays are hashable in Odin).
- Growth is Odin-owned: C++ never reallocs a shared container buffer; it only
  reads `.data/.len/.cap` and calls shims to mutate.

### Shims must match these C-ABI rules
- Single return values only (out-params for multiple).
- `context = runtime.default_context()` first.
- Key pointers passed as `^[4]byte` (by pointer), not by value.

## de-STL ordering (established)

1. **Vectors** (`std::vector` → `DynamicArray`) — done for a pilot, bulk pending
2. **Strings** (`std::string` → `DynamicString`) — shims done, pilot pending
3. **Maps** (`std::map/unordered_map` → native map) — shims done, pending

**Pilot vector** (proves the pipeline in the real game):
- `EnsembleSounds_t` in `sound.hpp` — 4 member vectors → DynamicArray,
  call sites → `barony_dynamic_array_*`. Game boots, sounds load. ✅
- `src/meson.build` links `odin/containers/containers.lib` into game+editor.
  (Later changed: containers is now IMPORTED by the odin package instead of
  linked as a separate lib — linking a separately-built lib duplicated Odin's
  runtime objects (LNK2005). See commit bd8c67d.)

**String pilot (DONE, verified in-game):** `SaveGameInfo.customseed_string`
(scores.hpp) + `SeededRun.seedString` (mod_tools.hpp) → DynamicString — the
first live members to leave std::string. ~25 call sites. Required:
- **FileInterface DynamicString serialization**: `value(DynamicString&)`
  overloads in all 4 state structs + FileInterface dispatch +
  `writeStringInternal/readStringInternal` (JSON + Binary). Read matches the
  FORMAT (`[u32 len][len bytes]`), not the std::string reserve+operator[] size
  bug — sets len correctly, round-trips identically.
- **Build fix**: containers package imported by odin/game.odin + editor.odin;
  containers.lib dropped from link flags (runtime duplication).
- **DynamicString class additions the pilot forced**: non-explicit ctor from
  const char* (std::string implicit-conversion semantics — Language::get()/
  literals pass into DynamicString params), find(char), at(), operator[].
- **Verified**: USER ran the game, started a map with a custom seed — the map
  MATCHED the reference Steam version with the same seed. Strongest possible
  validation (seed string → djb2Hash → mapgen all through DynamicString).
- Commit d1a3b2f.

## std::string survey findings (string-first ordering)

- Core structs barely use std::string: `Stat::name` is `char[128]`; the
  `Player.name` std::string is EOS-gated (dead); `Item.item_name_*` has 0 sites.
- The 40 struct string members are ALL in the UI layer (MainMenu, Button,
  Frame, Slider, Field, Widget, Font, Image, Text) — port last.
- `std::string` in non-UI game logic: ~1,336 occurrences (mod_tools 474,
  menu 109, actgeneral 102, input 98, files 87, ...).
- `c_str()` = ~2,344 sites, mostly passing to C functions (printlog, SDL,
  PhysFS, fopen). With DynamicString's guaranteed NUL, `.c_str()` is `return data`.
- `vector<string>` = 35 sites.

## Verified facts / gotchas

- **`DynamicString` layout** = Odin `Raw_String` `{data,len}` 16B. Shims
  allocate `len+1` and set `data[len]=0`, so `.c_str()` is O(1).
- **`DynamicArray` layout** = Odin `Raw_Dynamic_Array` 40B
  `{data,len,cap,allocator(2 ptrs)}`.
- **`DynamicMap` layout** = Odin `Raw_Map` 32B `{data,len,allocator(2 ptrs)}`.
- **MSVC `std::string` = 32B with SSO** — its layout is NOT `{data,len}`; you
  cannot reinterpret a std::string as DynamicString. Must replace the type.
- **MSVC `std::map` = tree layout** — not shareable; hence native Odin maps.
- **Odin `append` uses the array's STORED allocator** (`_reserve_dynamic_array_unsafe`
  falls back to context only if allocator field is nil). Setting `context =
  runtime.default_context()` at shim entry makes it valid.
- **Odin `int` is 8 bytes on x64; C++ `int` is 4.** Any shim crossing the ABI
  with ints must use `i32`/`i64` explicitly. `[^]int` pointer arithmetic strides
  by 8 — writing `values[n]` lands at 2×n in a C++ `int[]` (silent garbage).
  Affected: set/map entries shims, anything with `int` params. (Found in the
  DynamicSet entries shim; same class as the i32str entries `[^][4]byte` bug.)
- **`DynamicSet` (std::set replacement)** = Odin `map[T]struct{}` (same 32B
  Raw_Map layout). `for key in s^` iterates KEYS (not values — values are
  empty structs). i32/str families in dynamic_map.odin; `barony_dynamic_set_*`.
- **Odin's `heap_allocator` is an aligned allocator** — `realloc`-grown memory
  is NOT compatible with it. C++ must never `realloc` a shared buffer.
- **`std::vector::erase(it)` returns next iterator = same index** in index terms.
  The erase loop `it = v.erase(it)` → `erase(&v, i)` and DON'T increment.
- **Maps with unused order** (Item.attributes etc.) → native `map` (hash,
  unordered) is fine. Order-dependent maps (allGameSpells, console commands)
  are global, not in shared structs — port with their files.
- **PRESERVE default initializers when converting std::string → DynamicString**
  (bit us TWICE — compendium_sorting, notificationFont). The member-decl swap
  keeps `= "..."`, but the .cpp STATIC-def conversion dropped it:
  `std::string X::name = "default";` → `DynamicString X::name;` lost the
  default → empty at startup (compendium empty until sort clicked). ALWAYS
  convert statics as `DynamicString X::name = "default";`. Same class as the
  earlier magic_cookie default bug. When bulk-converting, audit all statics
  with non-empty defaults (git show c8a4f95 + grep).
- **DynamicMapI32 find() returns a COPY (KV snapshot), not a mutable slot** —
  `find()->second = x` writes the copy and is silently lost. Use `map[key] = x`
  (operator[] gives a mutable ref) for writes; `find()->second` for reads.
  (std::map::find returns an iterator into the map; ours is a snapshot.)
- **find() iterator's `first` points at interned storage** (process-lifetime
  stable) — safe to store; never a copy that dies with the iterator.

## Meson build

- `meson compile -C builddir` builds: C++ static libs → Odin exes.
- `src/meson.build`: `static_library('barony_game'/'barony_editor')` +
  custom_targets running `odin build` with `/WHOLEARCHIVE` + vcpkg debug
  import libs + debug CRT (`ucrtd`/`msvcprtd`/`vcruntimed`,
  `/NODEFAULTLIB:libcmt libucrt`) + theoraplay (game only).
- `odin/containers/containers.lib` linked into both exes (shims).
- **8.3 short paths** for MSVC/SDK libs (Odin's `-extra-linker-flags` doesn't
  preserve quotes around spaces).
- Run: `cd builddir/src && barony.exe -datadir="C:\Program Files (x86)\Steam\steamapps\common\Barony\"`

## Debug workflow

- Crash logs: `crashlogs/` (log.txt + .dmp). Analyze with `cdb` + PDB in
  `builddir/src`.
- The exe is fully static (C++ + Odin in one binary) — one PDB.
- Debug builds default (`/Od`, `/MDd`, `/Z7`); vcpkg debug DLLs (`*d.dll`).

## Porting rules (from experience)

1. Port leaves first (self-contained, C-mappable); the RNG (`prng.odin`) is the
   verified example.
2. Every ported proc: `@(export)` + verified against a C++ harness (byte-exact
   where deterministic — RNG is the model).
3. Shared structs must share memory/layout with Odin — use the container
   mirrors, never keep std:: containers in a struct that crosses.
4. Delete the C++ function when ported; never leave a delegating shell.
5. `std::async`/`std::future` vectors are local, not shared — port with their
   file (don't de-STL them).
6. When in doubt, verify the behavior against the C++ reference (simulate in
   tests, don't guess).

## Next steps (in order)

1. **String pilot** ✅ (customseed_string + seedString → DynamicString,
   FileInterface serialization, verified in-game — commit d1a3b2f).
2. **Bulk string replacement** — the next big push. The pilot locked the
   transform spec: DynamicString's std::string-compatible API means the bulk
   pass is mostly TYPE-SWAPS (std::string → DynamicString; the 2,164 `+=`,
   467 `==`, find/substr/compare sites fix themselves via operators). Only
   explicit transforms needed: std::to_string() (225 sites) → a
   DynamicString-returning helper, and any std::string params/returns at
   boundaries. Use ast-grep (0.44.1), build-gated per batch, per-file order:
   non-UI shared-struct owners first (mod_tools 106 members, input, files,
   scores, player), UI layer (MainMenu 568, GameUI 349, Button/Frame/Field)
   last since it ports last.
3. **Bulk vector replacement** — rules for `std::vector<T>` → DynamicArray,
   `.push_back` → shim, range-for → index loop. (vector<string> converts in
   the string pass: element type becomes DynamicString.)
4. **Map replacement** — native `map[[4]byte]V` / `map[string]V` (string keys
   ride on DynamicString after the string pass).
5. Then port files bottom-up, deleting C++ as callers dry up.
