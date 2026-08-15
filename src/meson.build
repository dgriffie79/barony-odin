# Barony game/editor sources (from the original CMakeLists.txt)

# Root src/ files used by the game
game_sources = files(
  'main.cpp',
  'init.cpp',
  'collision.cpp',
  'light.cpp',
  'maps.cpp',
  'cursors.cpp',
  'draw.cpp',
  'opengl.cpp',
  'objects.cpp',
  'list.cpp',
  'files.cpp',
  'items.cpp',
  'paths.cpp',
  'charclass.cpp',
  'net.cpp',
  'game.cpp',
  'stat.cpp',
  'acttorch.cpp',
  'actplayer.cpp',
  'actmonster.cpp',
  'actitem.cpp',
  'acthudweapon.cpp',
  'actgold.cpp',
  'actgib.cpp',
  'actgeneral.cpp',
  'actdoor.cpp',
  'actladder.cpp',
  'input.cpp',
  'item_usage_funcs.cpp',
  'entity.cpp',
  'messages.cpp',
  'actflame.cpp',
  'actcampfire.cpp',
  'actfountain.cpp',
  'actsink.cpp',
  'monster_rat.cpp',
  'monster_goblin.cpp',
  'monster_slime.cpp',
  'monster_human.cpp',
  'monster_scorpion.cpp',
  'monster_succubus.cpp',
  'monster_troll.cpp',
  'monster_shopkeeper.cpp',
  'monster_skeleton.cpp',
  'monster_minotaur.cpp',
  'monster_ghoul.cpp',
  'monster_demon.cpp',
  'monster_spider.cpp',
  'monster_lich.cpp',
  'monster_imp.cpp',
  'monster_gnome.cpp',
  'monster_devil.cpp',
  'monster_automaton.cpp',
  'monster_cockatrice.cpp',
  'monster_crystalgolem.cpp',
  'monster_goatman.cpp',
  'monster_incubus.cpp',
  'monster_insectoid.cpp',
  'monster_kobold.cpp',
  'monster_lichfire.cpp',
  'monster_lichice.cpp',
  'monster_scarab.cpp',
  'monster_sentrybot.cpp',
  'monster_shadow.cpp',
  'monster_vampire.cpp',
  'monster_mimic.cpp',
  'monster_bugbear.cpp',
  'monster_bat.cpp',
  'shops.cpp',
  'mechanisms.cpp',
  'actgate.cpp',
  'actchest.cpp',
  'actsprite.cpp',
  'menu.cpp',
  'savepng.cpp',
  'actarrow.cpp',
  'actarrowtrap.cpp',
  'actboulder.cpp',
  'actheadstone.cpp',
  'actthrown.cpp',
  'actbeartrap.cpp',
  'actspeartrap.cpp',
  'actwallbuster.cpp',
  'book.cpp',
  'init_game.cpp',
  'prng.cpp',
  'scores.cpp',
  'steam.cpp',
  'hash.cpp',
  'player.cpp',
  'entity_shared.cpp',
  'stat_shared.cpp',
  'item_tool.cpp',
  'actsummontrap.cpp',
  'actpowercrystal.cpp',
  'monster_shared.cpp',
  'actpedestal.cpp',
  'actteleporter.cpp',
  'actmagictrap.cpp',
  'mod_tools.cpp',
  'json.cpp',
  'eos.cpp',
  'lobbies.cpp',
  'shader.cpp',
  'playfab.cpp',
  'monster_d.cpp',
  'monster_m.cpp',
  'monster_s.cpp',
  'monster_g.cpp',
  'monster_summons.cpp',
  'monster_moth.cpp',
  'monster_duck.cpp',
)

# Editor sources (subset of src/, compiled with -DEDITOR)
editor_sources = files(
  'main.cpp',
  'init.cpp',
  'light.cpp',
  'buttons.cpp',
  'cursors.cpp',
  'draw.cpp',
  'opengl.cpp',
  'objects.cpp',
  'entity_editor.cpp',
  'input.cpp',
  'list.cpp',
  'hash.cpp',
  'files.cpp',
  'editor.cpp',
  'entity_shared.cpp',
  'stat_editor.cpp',
  'stat_shared.cpp',
  'items_editor.cpp',
  'json.cpp',
  'eos_editor.cpp',
  'mod_tools.cpp',
  'prng.cpp',
  'shader.cpp',
  'ui/Button.cpp',
  'ui/Field.cpp',
  'ui/Font.cpp',
  'ui/Frame.cpp',
  'ui/Image.cpp',
  'ui/Slider.cpp',
  'ui/Text.cpp',
  'ui/Widget.cpp',
  'ui/LoadingScreen.cpp',
  'engine/audio/defines.cpp',
  'engine/audio/init_audio.cpp',
  'engine/audio/sound.cpp',
)

# Subdirectory sources
magic_sources = files(
  'magic/magic.cpp',
  'magic/actmagic.cpp',
  'magic/spell.cpp',
  'magic/setupSpells.cpp',
  'magic/act_HandMagic.cpp',
  'magic/castSpell.cpp',
)
interface_sources = files(
  'interface/interface.cpp',
  'interface/screenshot.cpp',
  'interface/clickdescription.cpp',
  'interface/consolecommand.cpp',
  'interface/drawminimap.cpp',
  'interface/drawstatus.cpp',
  'interface/magicgui.cpp',
  'interface/updatecharactersheet.cpp',
  'interface/updaterightsidebar.cpp',
  'interface/updatechestinventory.cpp',
  'interface/identify_and_appraise.cpp',
  'interface/playerinventory.cpp',
  'interface/bookgui.cpp',
  'interface/shopgui.cpp',
  'interface/removecurse.cpp',
  'interface/ui_general.cpp',
)
ui_sources = files(
  'ui/Button.cpp',
  'ui/Field.cpp',
  'ui/Font.cpp',
  'ui/Frame.cpp',
  'ui/Image.cpp',
  'ui/Slider.cpp',
  'ui/Text.cpp',
  'ui/Widget.cpp',
  'ui/GameUI.cpp',
  'ui/LoadingScreen.cpp',
  'ui/MainMenu.cpp',
)
audio_sources = files(
  'engine/audio/defines.cpp',
  'engine/audio/init_audio.cpp',
  'engine/audio/sound.cpp',
  'engine/audio/sound_game.cpp',
  'engine/audio/music.cpp',
)

# (engine/ has no direct sources; only engine/audio/)

# ---------------------------------------------------------------------------
# Executables
# ---------------------------------------------------------------------------

# Shared include dirs (so #include "Config.hpp", "main.hpp", subdir headers work)
inc = include_directories(
  '.',
  'magic',
  'interface',
  'ui',
  'engine',
  'engine/audio',
)

# Config.hpp is generated into the build dir root by configure_file.
build_inc = include_directories('..')  # relative to src/ -> builddir/

# The game + editor share most sources; editor uses -DEDITOR and a subset.
# To keep things simple and correct, build each library from its own list.
#
# The C++ code is built as STATIC LIBRARIES (libbarony_game.a / libbarony_editor.a)
# that get linked INTO the Odin executable. One static link means both directions
# resolve at link time: Odin calls C++ (barony_main) and C++ objects can call
# Odin-exported procs (@(export)) — the reverse-call pattern the port needs.
# (Shared DLLs can't do DLL->exe imports cleanly.) The process entry point lives
# in Odin; the C++ entry point was renamed main -> barony_main (see game.cpp /
# editor.cpp). SDL_MAIN_HANDLED (meson_force_include.h) stops SDL from hijacking
# main, and barony_main calls SDL_SetMainReady() itself, so SDL2main is NOT linked
# (Odin provides the real main).

# Game static library
barony_deps = [
  sdl2, sdl2_image, sdl2_net, sdl2_ttf, physfs, libpng, threads, opengl, glew, vorbisfile,
]
barony_deps += audio_deps
if theoraplay_enabled
  barony_deps += theoraplay_dep
endif
if curl_enabled
  barony_deps += curl
endif

barony_game_lib = static_library('barony_game',
  game_sources + magic_sources + interface_sources + ui_sources + audio_sources,
  include_directories : [inc, build_inc],
  dependencies : barony_deps,
  cpp_args : common_cpp_args + (theoraplay_enabled ? ['-DUSE_THEORA_VIDEO'] : []),
  link_args : common_link_args,
)

# Editor static library (optional)
if editor_enabled
  editor_deps = [
    sdl2, sdl2_image, sdl2_net, sdl2_ttf, physfs, libpng, threads, opengl, glew, vorbisfile,
  ]
  editor_deps += audio_deps
  barony_editor_lib = static_library('barony_editor',
    editor_sources,
    include_directories : [inc, build_inc],
    dependencies : editor_deps,
    cpp_args : common_cpp_args + ['-DEDITOR'],
    link_args : common_link_args,
  )
endif

# ---------------------------------------------------------------------------
# Odin drivers (the actual barony.exe / editor.exe)
#
# Odin owns the process entry point and calls barony_main() (the renamed C++
# main). The C++ static lib is linked with /WHOLEARCHIVE (so ALL objects are
# included, not just those referenced from Odin), plus the vcpkg import libs
# the static lib needs. One Odin package (odin/: main.odin + game.odin +
# editor.odin + prng.odin) builds both binaries: the game by default, the
# editor with -define:EDITOR=true. game.odin/editor.odin import the static
# lib by relative path from odin/.
# ---------------------------------------------------------------------------
if host_system == 'windows'
  odin_exe = find_program('odin', 'C:/dev/odin/odin.exe', required : false)
  if odin_exe.found()
    odin_root = meson.current_source_dir() / '..' / 'odin'

    # vcpkg debug import libs (must match the runtime DLLs: SDL2d, OpenAL32, etc.)
    vcpkg_debug_lib = 'C:/dev/vcpkg/installed/x64-windows/debug/lib'
    # The C++ static libs are compiled with /MDd (debug dynamic CRT); Odin defaults
    # to /defaultlib:libcmt (static release CRT), so the debug DLL CRT import libs
    # must be added explicitly to satisfy the C++ objects' __imp_* references.
    # (8.3 short paths are used because Odin's -extra-linker-flags does not
    # preserve quotes around paths with spaces.)
    msvc_lib = 'C:/PROGRA~1/MIB055~1/18/COMMUN~1/VC/Tools/MSVC/1451~1.362/lib/x64'
    ucrt_lib = 'C:/PROGRA~2/WI3CF2~1/10/Lib/100280~1.0/ucrt/x64'
    # Common third-party + CRT import libs (the /WHOLEARCHIVE archive differs per exe)
    odin_dep_libs = [
      msvc_lib / 'msvcprtd.lib',
      msvc_lib / 'vcruntimed.lib',
      ucrt_lib / 'ucrtd.lib',
      '/NODEFAULTLIB:libcmt',
      '/NODEFAULTLIB:libucrt',
      vcpkg_debug_lib / 'SDL2d.lib',
      vcpkg_debug_lib / 'SDL2_imaged.lib',
      vcpkg_debug_lib / 'SDL2_netd.lib',
      vcpkg_debug_lib / 'SDL2_ttfd.lib',
      vcpkg_debug_lib / 'physfs.lib',
      vcpkg_debug_lib / 'libpng16d.lib',
      vcpkg_debug_lib / 'glew32d.lib',
      vcpkg_debug_lib / 'freetyped.lib',
      vcpkg_debug_lib / 'bz2d.lib',
      vcpkg_debug_lib / 'brotlicommon.lib',
      vcpkg_debug_lib / 'brotlidec.lib',
      vcpkg_debug_lib / 'brotlienc.lib',
      vcpkg_debug_lib / 'fmtd.lib',
      vcpkg_debug_lib / 'OpenAL32.lib',
      vcpkg_debug_lib / 'ogg.lib',
      vcpkg_debug_lib / 'vorbis.lib',
      vcpkg_debug_lib / 'vorbisenc.lib',
      vcpkg_debug_lib / 'vorbisfile.lib',
      'opengl32.lib',
      '/DEFAULTLIB:wsock32.lib',
      '/DEFAULTLIB:ws2_32.lib',
      '/DEFAULTLIB:shell32.lib',
      '/DEFAULTLIB:user32.lib',
      '/DEFAULTLIB:gdi32.lib',
      '/DEFAULTLIB:winmm.lib',
      '/DEFAULTLIB:imm32.lib',
      '/DEFAULTLIB:ole32.lib',
      '/DEFAULTLIB:oleaut32.lib',
      '/DEFAULTLIB:version.lib',
      '/DEFAULTLIB:uuid.lib',
      '/DEFAULTLIB:advapi32.lib',
      '/DEFAULTLIB:setupapi.lib',
    ]

    # Per-exe link flags: /WHOLEARCHIVE the matching static lib + common deps.
    game_link_flags = ['/WHOLEARCHIVE:' + meson.current_build_dir() / 'libbarony_game.a'] + odin_dep_libs
    editor_link_flags = ['/WHOLEARCHIVE:' + meson.current_build_dir() / 'libbarony_editor.a'] + odin_dep_libs

    if theoraplay_enabled
      # The game lib's mod_tools.cpp uses TheoraPlay (USE_THEORA_VIDEO); the
      # editor does not. Link the vendored archive + theora/vorbis codec libs
      # into the game exe only.
      game_link_flags += meson.current_build_dir() / '..' / 'third_party' / 'theoraplay' / 'libtheoraplay.a'
      game_link_flags += vcpkg_debug_lib / 'theora.lib'
      game_link_flags += vcpkg_debug_lib / 'theoradec.lib'
      game_link_flags += vcpkg_debug_lib / 'theoraenc.lib'
    endif

    # Odin containers shims (dynamic array/map/string) — C++ code in the
    # static libs calls these; they must be linked into the final exe.

    # Odin exe for the game -> builddir/src/barony.exe
    odin_game_exe = custom_target('odin_game_exe',
      output : 'barony.exe',
      input : [
        odin_root / 'main.odin',
        odin_root / 'game.odin',
        odin_root / 'prng.odin',
        odin_root / 'containers' / 'dynamic_array.odin',
        odin_root / 'containers' / 'dynamic_map.odin',
        odin_root / 'containers' / 'dynamic_string.odin',
        odin_root / 'json_shim' / 'json_shim.odin',
      ],
      depends : [barony_game_lib],
      command : [
        odin_exe, 'build', odin_root,
        '-out:' + meson.current_build_dir() / 'barony.exe',
        '-debug',
        '-extra-linker-flags:' + ' '.join(game_link_flags),
      ],
      build_by_default : true,
    )

    if editor_enabled
      # Odin exe for the editor -> builddir/src/editor.exe
      odin_editor_exe = custom_target('odin_editor_exe',
        output : 'editor.exe',
        input : [
          odin_root / 'main.odin',
          odin_root / 'editor.odin',
          odin_root / 'containers' / 'dynamic_array.odin',
          odin_root / 'containers' / 'dynamic_map.odin',
          odin_root / 'containers' / 'dynamic_string.odin',
          odin_root / 'json_shim' / 'json_shim.odin',
        ],
        depends : [barony_editor_lib],
        command : [
          odin_exe, 'build', odin_root,
          '-out:' + meson.current_build_dir() / 'editor.exe',
          '-debug',
          '-define:EDITOR=true',
          '-extra-linker-flags:' + ' '.join(editor_link_flags),
        ],
        build_by_default : true,
      )
    endif
  endif
endif
