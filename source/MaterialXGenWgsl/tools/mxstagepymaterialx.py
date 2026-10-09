#!/usr/bin/env python
"""Copy MaterialX pure-Python tree + built PyMaterialX *.pyd into a single import root."""
import argparse
import glob
import os
import shutil


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", required=True, help="MaterialX repository root")
    parser.add_argument("--pyd-dir", required=True, help="Directory containing PyMaterialX*.pyd")
    parser.add_argument("--stage-root", required=True,
                        help="Output root; MaterialX package will be at <stage-root>/MaterialX")
    parser.add_argument(
        "--ocio-bin",
        default="",
        help="OpenColorIO runtime bin (OpenColorIO_*.dll); registered via os.add_dll_directory on Windows",
    )
    args = parser.parse_args()

    pkg_src = os.path.join(args.repo, "python", "MaterialX")
    pkg_dst = os.path.join(args.stage_root, "MaterialX")
    if os.path.isdir(pkg_dst):
        shutil.rmtree(pkg_dst)
    shutil.copytree(pkg_src, pkg_dst)

    for pyd in glob.glob(os.path.join(args.pyd_dir, "PyMaterialX*.pyd")):
        shutil.copy2(pyd, pkg_dst)

    init_py = os.path.join(pkg_dst, "__init__.py")
    dll_dirs = [os.path.normpath(args.pyd_dir)]
    if args.ocio_bin:
        dll_dirs.append(os.path.normpath(args.ocio_bin))
    with open(init_py, encoding="utf-8") as handle:
        init_src = handle.read()
    insert = (
        "\n# Staged PyMaterialX for OCIO WGSL bake (Windows DLL search paths)\n"
        f"_OCIO_BAKE_DLL_DIRS = {dll_dirs!r}\n"
        "if sys.platform == 'win32' and sys.version_info >= (3, 8):\n"
        "    for _ocio_bake_dll_dir in _OCIO_BAKE_DLL_DIRS:\n"
        "        if os.path.isdir(_ocio_bake_dll_dir):\n"
        "            os.add_dll_directory(_ocio_bake_dll_dir)\n"
    )
    marker = "from .main import *"
    if marker not in init_src:
        raise RuntimeError(f"Unexpected MaterialX __init__.py layout in {init_py}")
    init_src = init_src.replace(marker, insert + "\n" + marker, 1)
    with open(init_py, "w", encoding="utf-8", newline="\n") as handle:
        handle.write(init_src)

    print(f"Staged MaterialX package: {pkg_dst}")


if __name__ == "__main__":
    main()
