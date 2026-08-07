#!/usr/bin/env bash
# Copies runtime DLLs from the vcpkg tree into the build dir next to the exe.
# Usage: copy_runtime_dlls.sh <vcpkg_bin_dir> <dest_dir>
set -euo pipefail

VCPKG_BIN="$1"
DEST="$2"

# Direct dependencies of barony.exe (from dumpbin /dependents)
DIRECT=(
  SDL2d.dll SDL2_imaged.dll SDL2_netd.dll SDL2_ttfd.dll
  physfs.dll libpng16d.dll glew32d.dll vorbisfile.dll OpenAL32.dll
)

# Transitive deps needed by the above
TRANSITIVE=(
  ogg.dll vorbis.dll vorbisenc.dll theora.dll theoradec.dll theoraenc.dll
  freetyped.dll zd.dll fmtd.dll bz2d.dll brotlidec.dll brotlicommon.dll brotlienc.dll
)

mkdir -p "$DEST"
for dll in "${DIRECT[@]}" "${TRANSITIVE[@]}"; do
  if [ -f "$VCPKG_BIN/$dll" ]; then
    cp -f "$VCPKG_BIN/$dll" "$DEST/"
  fi
done

echo "Copied runtime DLLs to $DEST"
