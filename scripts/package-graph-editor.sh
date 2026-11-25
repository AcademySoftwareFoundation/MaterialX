#!/usr/bin/env bash
# Build a relocatable MaterialXGraphEditor archive from an install tree.
#
# Usage: scripts/package-graph-editor.sh [BUILD_DIR] [OUTPUT_ARCHIVE]
#
# OUTPUT_ARCHIVE may end in .zip, .tar.gz, .tgz, or .tar.xz.  The graph editor
# is installed into a temporary staging prefix, then only the runtime
# directories are copied into the archive.  Headers, static libraries, CMake
# package files, and other development-only content are omitted.

set -euo pipefail

repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
build_dir=${1:-"$repo_root/build-bundle"}
output_archive=${2:-"$repo_root/dist/MaterialXGraphEditor-linux-$(uname -m).zip"}

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
# Runtime data installed by dependencies (for example OSL's standard shader
# headers under share/OSL/shaders) lives beside the executable's parent.
if [[ -d "$stage/prefix/share" ]]; then
    cp -a "$stage/prefix/share" "$package_dir/"
fi
cp -a "$repo_root/LICENSE" "$repo_root/THIRD-PARTY.md" "$package_dir/"

# MaterialX is normally built statically for this bundle.  If a shared build is
# packaged instead, keep its libraries in ../lib, which is on the executable's
# rpath.
if compgen -G "$stage/prefix/lib/libMaterialX*.so*" >/dev/null; then
    mkdir -p "$package_dir/lib"
    cp -a "$stage/prefix/lib/libMaterialX"*.so* "$package_dir/lib/"
fi

# Optionally strip release binaries and bundled shared libraries.  This keeps
# the archive small but is opt-in so that debug builds can be packaged as-is.
if [[ "${MATERIALX_STRIP_RUNTIME:-0}" != "0" ]] && command -v strip >/dev/null 2>&1; then
    find "$package_dir/bin" -maxdepth 1 -type f \
        \( -name '*.so*' -o -name 'MaterialXGraphEditor' \) \
        -exec strip --strip-unneeded {} + 2>/dev/null || true
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
)
for path in "${required[@]}"; do
    if [[ ! -e "$path" ]]; then
        echo "error: required runtime file is missing: $path" >&2
        exit 1
    fi
done

# The Cycles backend packages its OSL shaders beside the executable.
if [[ -d "$package_dir/bin/shader" ]]; then
    for path in "$package_dir/bin/shader/stdosl.h" "$package_dir/bin/shader/stdcycles.h"; do
        if [[ ! -e "$path" ]]; then
            echo "error: required Cycles shader file is missing: $path" >&2
            exit 1
        fi
    done
    if ! compgen -G "$package_dir/bin/shader/node_*.oso" >/dev/null; then
        echo "error: no compiled Cycles node shaders were installed" >&2
        exit 1
    fi
fi

# The OSL testrender backend packages the standard OSL shader headers.
if [[ -d "$package_dir/share/OSL/shaders" ]]; then
    if [[ ! -e "$package_dir/share/OSL/shaders/stdosl.h" ]]; then
        echo "error: required OSL shader file is missing: share/OSL/shaders/stdosl.h" >&2
        exit 1
    fi
fi

# Verify that the executable and every bundled library resolve without relying
# on the build machine's LD_LIBRARY_PATH.  This is the core relocatability
# check for the release archive.
if command -v ldd >/dev/null 2>&1; then
    if unresolved=$(ldd "$package_dir/bin/MaterialXGraphEditor" 2>&1 | grep "not found"); then
        echo "error: unresolved runtime dependencies after bundling:" >&2
        echo "$unresolved" >&2
        exit 1
    fi
fi

mkdir -p "$(dirname "$output_archive")"
rm -f "$output_archive"
case "$output_archive" in
    *.tar.gz|*.tgz)
        tar -C "$package_parent" -czf "$output_archive" "$package_name" ;;
    *.tar.xz)
        tar -C "$package_parent" -cJf "$output_archive" "$package_name" ;;
    *)
        (cd "$package_parent" && zip -y -r "$output_archive" "$package_name" >/dev/null) ;;
esac

echo "Created $output_archive"
du -h "$output_archive"
