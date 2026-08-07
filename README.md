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

The `curl`, `playfab`, `opus`, and `tremor` Meson options remain only to
preserve the upstream option surface; enabling them does not produce a working
online build.

What *is* ported and verified against the retail Steam data dir:
- OpenAL audio (replaces FMOD)
- TheoraPlay .ogv sign video playback (vendored C library; retail ships 122
  .ogv files, 120 video-only, 2 with silent audio tracks)
- **Direct-IP / LAN multiplayer** over SDL2_net (UDP sockets) — the
  `directConnect` path is fully independent of Steam/EOS lobby services.
  The online lobby browser (Steam lobby search / EOS crossplay) is not
  ported; hosting/joining by IP address works.
