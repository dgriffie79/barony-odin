project('barony', 'cpp', 'c',
  version : '2.0.2',
  default_options : [
    'cpp_std=c++17',
    'c_std=c11',
    'buildtype=debug',
    'warning_level=2',
  ],
)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
# Sound engine: OpenAL (required for now; FMOD removed in the Odin port)
openal_enabled = get_option('openal')
# Tremor (fixed-point vorbis decoder) instead of libvorbis for OpenAL
tremor_enabled = get_option('tremor')
# TheoraPlay video playback (vendored in third_party/theoraplay)
theoraplay_enabled = get_option('theoraplay')
# Optional online features
curl_enabled = get_option('curl')
playfab_enabled = get_option('playfab')
opus_enabled = get_option('opus')
# Build the editor executable (in addition to the game)
editor_enabled = get_option('editor')

if playfab_enabled and not curl_enabled
  error('playfab requires curl to be enabled')
endif

# ---------------------------------------------------------------------------
# Compiler / platform flags
# ---------------------------------------------------------------------------
cc = meson.get_compiler('cpp')
host_system = host_machine.system()

common_cpp_args = []
common_link_args = []

if host_system == 'windows'
  add_project_arguments('-DWINDOWS', language : 'cpp')
  # Avoid Win32 macro collisions with rapidjson (GetObject->GetObjectA) and
  # std::min/std::max. NOMINMAX is not enough for GetObject (wingdi.h).
  add_project_arguments('-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX', language : 'cpp')
  # Forced include undefines the GetObject macro (wingdi.h defines it even
  # with WIN32_LEAN_AND_MEAN).
  add_project_arguments('-FI' + meson.current_source_dir() / 'meson_force_include.h', language : 'cpp')
  # Barony uses SDL_main on Windows; SDL2 provides it when linked with SDL2main.
  # (SDL2's pkg-config pulls in SDL2main automatically for console apps.)
elif host_system == 'darwin'
  add_project_arguments('-DAPPLE', language : 'cpp')
  common_link_args += ['-rpath', '@loader_path/../Frameworks']
else
  add_project_arguments('-DLINUX', language : 'cpp')
  # -malign-double historically used on Linux; harmless to keep.
  common_cpp_args += ['-malign-double']
  common_link_args += ['-Wl,-rpath=$ORIGIN']
endif

# Disable warnings the Barony codebase trips on (GCC/Clang only; MSVC rejects these).
if cc.get_id() != 'msvc'
  common_cpp_args += [
    '-Wno-pedantic',
    '-Wno-empty-body',
    '-Wno-string-plus-int',
    '-Wno-parentheses',
    '-Wno-format-security',
    '-Wno-multichar',
    '-Wno-inconsistent-missing-override',
  ]
endif

# Base data dir: where the game looks for data at runtime.
base_data_dir = get_option('base_data_dir')
if base_data_dir == ''
  base_data_dir = meson.current_source_dir()
endif
# Normalize to forward slashes so the C string literal doesn't treat backslashes
# as escape sequences (e.g. 'C:\dev\barony' -> 'C:devbarony'). Windows accepts both.
base_data_dir = base_data_dir.replace('\\', '/')
add_project_arguments('-DBASE_DATA_DIR="' + base_data_dir + '"', language : 'cpp')

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
sdl2 = dependency('sdl2', required : true)
# SDL2's header redefines main->SDL_main; vcpkg puts SDL2main in manual-link/
# (must be explicitly linked, it provides the real main() entry point).
sdl2main = cc.find_library('SDL2main', dirs : ['C:/dev/vcpkg/installed/x64-windows/lib/manual-link'], required : true)
sdl2_image = dependency('SDL2_image', required : true)
# SDL2_net ships no .pc with vcpkg; use its CMake config.
sdl2_net = dependency('SDL2_net', method : 'cmake', required : true)
sdl2_ttf = dependency('SDL2_ttf', required : true)
# physfs ships no .pc with vcpkg and its CMake config isn't Meson-friendly.
# Use the raw library + include path (vcpkg's include dir is added globally below).
physfs = cc.find_library('physfs', required : true)
# GLEW for OpenGL extension loading (Barony's shader/framebuffer code uses it).
glew = dependency('glew', required : true)
# vorbisfile (ov_*) is used by the OpenAL path to decode .ogg music.
vorbisfile = dependency('vorbisfile', required : true)
libpng = dependency('libpng', required : true)
threads = dependency('threads', required : true)

# OpenGL
if host_system == 'darwin'
  opengl = dependency('OpenGL', required : true)
elif host_system == 'windows'
  # MSVC links OpenGL via opengl32.lib (provided by the Windows SDK)
  opengl = cc.find_library('opengl32', required : true)
else
  opengl = cc.find_library('GL', required : true)
endif

# Codec deps: always available (OpenAL uses vorbis/ogg for audio; TheoraPlay uses
# ogg/theora/vorbis for video).
ogg = dependency('ogg', required : true)
vorbis = dependency('vorbis', required : true)
theora = dependency('theora', required : true)

# Include vcpkg headers (so physfs.h, theora/ headers, etc. are found).
# The .pc files already point at the right dirs for the pkg-config deps.
vcpkg_inc = include_directories('C:/dev/vcpkg/installed/x64-windows/include')
add_project_arguments('-I' + 'C:/dev/vcpkg/installed/x64-windows/include', language : 'cpp')

# OpenAL + vorbis (or tremor)
audio_deps = []
if openal_enabled
  openal = dependency('openal', required : true)
  audio_deps += openal
  if tremor_enabled
    add_project_arguments('-DUSE_TREMOR', language : 'cpp')
    tremor = dependency('tremor', required : true)
    audio_deps += tremor
  else
    audio_deps += [vorbis, ogg]
  endif
  add_project_arguments('-DUSE_OPENAL', language : 'cpp')
endif

# Optional features
if theoraplay_enabled
  # USE_THEORA_VIDEO is added to the game target only (not the editor); the
  # VideoManager_t code is inside #ifndef EDITOR in mod_tools.hpp.
  subdir('third_party/theoraplay')
else
  theoraplay_dep = disabler()
endif

if curl_enabled
  add_project_arguments('-DUSE_LIBCURL', language : 'cpp')
  curl = dependency('libcurl', required : true)
endif

if opus_enabled
  add_project_arguments('-DUSE_OPUS', language : 'cpp')
  opus = dependency('opus', required : true)
endif

# ---------------------------------------------------------------------------
# Config.hpp generation (replaces CMake's configure_file of src/Config.hpp.in)
# ---------------------------------------------------------------------------
config_data = configuration_data()
config_data.set('Windows', host_system == 'windows' ? 1 : 0)
config_data.set('Apple', host_system == 'darwin' ? 1 : 0)
config_data.set('Linux', host_system == 'linux' ? 1 : 0)
config_data.set('Bsd', host_system in ['freebsd', 'netbsd', 'dragonfly'] ? 1 : 0)
config_data.set('Haiku', host_system == 'haiku' ? 1 : 0)
config_data.set('STEAMWORKS_ENABLED', 0)
config_data.set('EOS_ENABLED', 0)
config_data.set('PLAYFAB_ENABLED', playfab_enabled ? 1 : 0)
config_data.set('IMGUI', 0)
config_data.set('CURL_ENABLED', curl_enabled ? 1 : 0)
config_data.set('THEORAPLAYER_ENABLED', 0)
config_data.set('OPUS_ENABLED', opus_enabled ? 1 : 0)
config_data.set('FMOD', 0)
config_data.set('OPENAL', openal_enabled ? 1 : 0)
config_data.set('TREMOR', tremor_enabled ? 1 : 0)
config_data.set('PANDORA', 0)
config_data.set('NOT_DWORD_DEFINED', 1)
config_data.set('BUILD_ENV_PR', '')
config_data.set('BUILD_ENV_SA', '')
config_data.set('BUILD_ENV_DE', '')
config_data.set('BUILD_ENV_CC', '')
config_data.set('BUILD_ENV_CS', '')
config_data.set('BUILD_ENV_GSE', '')
config_data.set('BUILD_ENV_PFTID', '')
config_data.set('BUILD_ENV_PFHID', '')

configure_file(
  input : 'src/Config.hpp.in',
  output : 'Config.hpp',
  configuration : config_data,
)

# ---------------------------------------------------------------------------
# Runtime DLL copying (Windows/vcpkg)
# ---------------------------------------------------------------------------
if host_system == 'windows'
  # Copy the runtime DLLs (from the vcpkg tree used for the build) next to the
  # executables. Debug builds use the debug bin dir; release uses the release one.
  vcpkg_root = get_option('vcpkg_root')
  if get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
    vcpkg_bin = vcpkg_root / 'debug' / 'bin'
  else
    vcpkg_bin = vcpkg_root / 'bin'
  endif
  meson.add_install_script('tools/copy_runtime_dlls.sh', vcpkg_bin, meson.current_build_dir() / 'src')
endif

# ---------------------------------------------------------------------------
# Subdirectories
# ---------------------------------------------------------------------------
subdir('src')
