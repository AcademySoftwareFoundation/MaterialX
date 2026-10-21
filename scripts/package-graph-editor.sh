#!/usr/bin/env bash
# Build a relocatable MaterialXGraphEditor zip from an install tree.
#
# Usage: scripts/package-graph-editor.sh [BUILD_DIR] [OUTPUT_ZIP]
#
# The graph editor is installed into a temporary staging prefix, then only the
# runtime directories are copied into the archive.  Headers, static libraries,
# CMake package files, and other development-only content are omitted.

set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
build_dir=${1:-"$repo_root/build-bundle"}
output_zip=${2:-"$repo_root/dist/MaterialXGraphEditor-linux-$(uname -m).zip"}

if [[ ! -f "$build_dir/CMakeCache.txt" ]]; then
    echo "error: '$build_dir' is not a configured CMake build directory" >&2
    exit 1
fi

stage=$(mktemp -d)
package_parent="$stage/package"
package_name="MaterialXGraphEditor-linux-$(uname -m)"
package_dir="$package_parent/$package_name"

cleanup() {
    rm -rf "$stage"
}
trap cleanup EXIT

echo "Installing into a staging prefix..."
cmake --install "$build_dir" --prefix "$stage/prefix" >/dev/null

mkdir -p "$package_dir"
cp -a "$stage/prefix/bin" "$package_dir/"
cp -a "$stage/prefix/resources" "$package_dir/"
cp -a "$stage/prefix/libraries" "$package_dir/"
cp -a "$repo_root/LICENSE" "$repo_root/THIRD-PARTY.md" "$package_dir/"

# MaterialX is normally built statically for this bundle.  If a shared build is
# packaged instead, keep its libraries in ../lib, which is on the executable's
# rpath.
if compgen -G "$stage/prefix/lib/libMaterialX*.so*" >/dev/null; then
    mkdir -p "$package_dir/lib"
    cp -a "$stage/prefix/lib/libMaterialX"*.so* "$package_dir/lib/"
fi

# The Cycles install may contain multiple OpenImageDenoise versions.  Keep the
# one selected by libOpenImageDenoise.so.2; the older 2.3.3 CPU backend has a
# missing matching core library in this package.
find "$package_dir/bin" -maxdepth 1 -name '*OpenImageDenoise*2.3.3*' -delete

# Sanity checks: these are the files whose absence caused the original
# "Could not load font file" and empty-search-path failures.
required=(
    "$package_dir/bin/MaterialXGraphEditor"
    "$package_dir/resources/Fonts/Roboto-Regular.ttf"
    "$package_dir/libraries/targets"
    "$package_dir/bin/shader/stdosl.h"
    "$package_dir/bin/shader/stdcycles.h"
)
for path in "${required[@]}"; do
    if [[ ! -e "$path" ]]; then
        echo "error: required runtime file is missing: $path" >&2
        exit 1
    fi
done

if ! compgen -G "$package_dir/bin/shader/node_*.oso" >/dev/null; then
    echo "error: no compiled Cycles node shaders were installed" >&2
    exit 1
fi

mkdir -p "$(dirname "$output_zip")"
rm -f "$output_zip"
(cd "$package_parent" && zip -y -r "$output_zip" "$package_name" >/dev/null)

echo "Created $output_zip"
du -h "$output_zip"
