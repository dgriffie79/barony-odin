![Linux-CI_fmod_steam](https://github.com/TurningWheel/Barony/workflows/Linux-CI_fmod_steam/badge.svg) ![Linux-CI_fmod_steam_eos](https://github.com/TurningWheel/Barony/workflows/Linux-CI_fmod_steam_eos/badge.svg)

# Update - 3rd October 2023

The current 'develop' branch contains in-development features for our latest update. For bugfixes + PRs, open them against 'master'.

# Compilation Instructions

The compilation instructions can be found in [INSTALL.md](INSTALL.md)

# Open-source Announcement Letter

Well here it is, as promised: the open source release of Barony. Keep in mind you still need a purchased copy of Barony to play this. I'd recommend that you thumb through all of the included text files to get a feeling of other things you'll need to build the game and check out the included licenses as well.

Many thanks go to Ciprian Elies for his original contributions to the game code, as well as for the build systems, config files, and support libraries that he developed for the project over the years. In the future, he plans to head up development on some new stuff for Barony, so keep an eye out for that.

This project was a first for both of us in many ways and it shows. Since all of the original code was written in C and hastily converted to C++ in the past few months, experienced C++ programmers may be horrified at some of the kludge we had to write to get some of the more basic systems working properly. There's not a lot of module organization either since I didn't understand how to properly write projects that scale when I started the code three years ago. Prepare to deal with lots of global variables that get used all over the project indiscriminately.

Despite the project's shortcomings, I'm reasonably proud of how the end product turned out. Writing good games is about more than just writing good code, though I guarantee we'll be taking all of the lessons learned from Barony into our next project.

I'm not sure how many people will be interested in working on this, and it may take a while for anything substantial to get going here, but I'd be pleased to see some coordinated efforts take place on this code sometime in the coming years.

Some project ideas:

 * Add an extra hard mode to the game.
 * Add a dungeon with infinite levels.
 * Create a dedicated server.
 * Multithread the packet handler.
 * Multithread the entity logic.
 * Add script support for entities and items.
 * Add persistent levels and servers.
 * Add fully 3D physics and world geometry.
 * Renovate the OpenGL code to a modern standard.

Have fun,

Sheridan
June 27th 2016

---

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

The `curl`, `playfab`, and `opus` Meson options remain only to preserve the
upstream option surface; enabling them does not produce a working online build.

What *is* ported and verified against the retail Steam data dir:
- OpenAL audio (replaces FMOD)
- TheoraPlay .ogv sign video playback (vendored C library; retail ships 122
  .ogv files, 120 video-only, 2 with silent audio tracks)
