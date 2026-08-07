## Odin port scope (this repository)

This repository is the C++ reference tree for the Barony -> Odin port. The build
system here is Meson (the upstream uses CMake), and the following subsystems are
**deliberately not ported** — treat them as dead ends:

- **Steamworks** (`STEAMWORKS`) — Steam lobbies/P2P, workshop, leaderboards,
  achievements, and the Steam-backed daily/challenge content. Requires Valve's
  proprietary SDK and AppID 371970 access. Not needed for the port.
- **Epic Online Services** (`USE_EOS`) — crossplay. Requires the EOS SDK and
  Epic dev credentials. Not ported.
- **PlayFab** (`USE_PLAYFAB`) — telemetry/leaderboards/challenges. Requires
  Steam/Epic auth tickets plus PlayFab title credentials (BUILD_ENV_PFTID /
  BUILD_ENV_PFHID). Not ported; the retail build does not use it either.
- **FMOD** — audio engine, replaced by OpenAL.
- **Opus voice chat** (`USE_OPUS`) — only meaningful with FMOD voice chat.
- **libcurl** (`USE_LIBCURL`) — only used to cache Steam Workshop preview
  images; dead without Steam.
- **Nintendo** (`NINTENDO`) — Switch platform build (mod-book filtering,
  ROM read paths). Never enabled here; the port targets desktop only.
- **Tremor** (`USE_TREMOR`) — fixed-point Vorbis decoder alternative to
  libvorbis for OpenAL music decoding. Off by default; libvorbis (float) is
  used instead. Harmless to leave off for the port.
- **ImGui** (`USE_IMGUI`, `/devmenu`) — the cheat-gated debug overlay (console
  command GUI + HUD frame-timer profiler). Removed from the reference entirely
  (code + vendored `src/imgui/`): it is not worth porting, and the Odin port
  would build any debug tooling natively. Note: upstream `IMGUI` Config define
  is set to 0 here.

The `curl`, `playfab`, `opus`, and `tremor` Meson options remain only to
preserve the upstream option surface; enabling them does not produce a working
online build.

### Full upstream (CMake) option inventory

For completeness, these are all the upstream CMake options and their status in
this Meson port:

| CMake option | Default | Meson status | Notes |
|---|---|---|---|
| `STEAMWORKS` | OFF | dropped | dead end (see above) |
| `EOS` | OFF | dropped | dead end (see above) |
| `PLAYFAB` | OFF | kept as inert option | dead end (see above) |
| `THEORAPLAYER` | OFF | replaced by `theoraplay` | vendored TheoraPlay (C) |
| `CURL` | OFF | kept as inert option | dead end (see above) |
| `OPUS` | OFF | kept as inert option | dead end (see above) |
| `FMOD_ENABLED` | ON | dropped | replaced by OpenAL |
| `OPENAL_ENABLED` | OFF | `openal` (default ON) | the port's audio engine |
| `TREMOR_ENABLED` | OFF | `tremor` (default OFF) | inert; libvorbis used |
| `EDITOR_ENABLED` | ON | `editor` (default ON) | editor exe built |
| `GAME_ENABLED` | ON | always on | game exe always built |
| `DATA_DIR` | OFF | `base_data_dir` | install data dir; our port uses
  `base_data_dir` option instead |
| `PANDORA_ENABLED` | OFF | hardcoded 0 | OpenPandora handheld; not a target |
| `OPTIMIZATION_LEVEL` | `-O3` | n/a (Meson `buildtype`) | GCC/Clang flag; Meson
  handles optimization via `buildtype` |
| `BARONY_WIN32_LIBRARIES` | env | n/a (vcpkg) | extra lib dir for VS; vcpkg
  paths used instead |
| `BARONY_DATADIR` | env | `base_data_dir` | VS debugger working dir; port uses
  `-datadir` argument |
| `NINTENDO` | n/a | never defined | Switch platform; desktop only |
| `BUILD_ENV_*` (EOS/PlayFab) | env | empty | dead-end credentials |
| `IMGUI` | 1 | set to 0 | removed; debug overlay not ported (see above) |

### Platforms

- **Windows** — primary target, actively built/tested (MSVC + vcpkg debug).
- **Linux / macOS** — build plumbing is in place (the Meson `host_system`
  branches for `darwin`/`linux` set `APPLE`/`LINUX`, `-rpath`, `-malign-double`,
  and the correct OpenGL linkage), but **untested** — no CI or local machine
  available. The ports are kept and should work, but are unverified. Fixes
  welcome if a platform issue surfaces.

What *is* ported and verified against the retail Steam data dir:
- OpenAL audio (replaces FMOD)
- TheoraPlay .ogv sign video playback (vendored C library; retail ships 122
  .ogv files, 120 video-only, 2 with silent audio tracks)
- **Direct-IP / LAN multiplayer** over SDL2_net (UDP sockets) — the
  `directConnect` path is fully independent of Steam/EOS lobby services.
  The online lobby browser (Steam lobby search / EOS crossplay) is not
  ported; hosting/joining by IP address works.
