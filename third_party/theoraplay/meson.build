# Vendored TheoraPlay (https://github.com/icculus/theoraplay) — zlib license.
# A small C library wrapping Ogg Theora/Ogg Vorbis decoding.
# Barony uses it to play .ogv animated signs (see VideoManager_t in src/mod_tools.*).

theoraplay_inc = include_directories('.')

theoraplay_lib = static_library('theoraplay',
  'theoraplay.c',
  include_directories : theoraplay_inc,
  dependencies : [ogg, theora, vorbis],
  c_args : ['-DTHEORAPLAY_ONLY_SINGLE_THREADED=0'],
)

# Provide a dependency object so the game/editor can link it.
theoraplay_dep = declare_dependency(
  include_directories : theoraplay_inc,
  link_with : theoraplay_lib,
  dependencies : [ogg, theora, vorbis],
)
