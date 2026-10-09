#!/usr/bin/env python
"""Validate WGSL shader generation by generating and naga-compiling each
renderable element in a set of MaterialX documents.

Usage (after building with MATERIALX_BUILD_GEN_WGSL=ON):
    python mxvalidategenwgsl.py --naga path/to/naga
    python mxvalidategenwgsl.py --input resources/Materials/Examples/StandardSurface \
        --naga path/to/naga --preset default prefilter shadow

By default also runs MaterialXView TSL-portable conversion + naga (same path as WebGPU).
Disable with --no-viewer-parity. Requires Node.js on PATH.
"""
import argparse
import os
import subprocess
import sys
import tempfile

import MaterialX as mx
import MaterialX.PyMaterialXGenShader as mx_gen_shader

try:
    import MaterialX.PyMaterialXGenWgsl as mx_gen_wgsl
except ImportError:
    sys.exit("WGSL generator unavailable (MATERIALX_BUILD_GEN_WGSL was disabled).")


# Skip files/nodedefs that the C++ tester also skips.
SKIP_NODEDEFS = {
    "ND_displacement_float",
    "ND_displacement_vector3",
    "ND_lightcompoundtest",
}

# GenOptions presets keyed by name.
GENOPTIONS_PRESETS = {
    "default": {},
    "prefilter": {
        "hwSpecularEnvironmentMethod": mx_gen_shader.SPECULAR_ENVIRONMENT_PREFILTER,
    },
    "shadow": {
        "hwShadowMap": True,
    },
    "ao": {
        "hwAmbientOcclusion": True,
    },
    "shadow_ao": {
        "hwShadowMap": True,
        "hwAmbientOcclusion": True,
    },
}

_TOOLS_DIR = os.path.dirname(os.path.abspath(__file__))
_VIEWER_PARITY_SCRIPT = os.path.join(_TOOLS_DIR, "mxvalidategenwgsl_viewer.mjs")

# Same document roots as WgslShaderGeneratorTester (GenWgsl.cpp).
_DEFAULT_INPUT_DIRS = (
    "resources/Materials/TestSuite",
    "resources/Materials/Examples",
)


def _ocio_cms_available(shadergen, stdlib):
    """True when OCIO CMS can be loaded (matches MATERIALX_BUILD_OCIO builds)."""
    if not hasattr(mx_gen_shader, "OcioColorManagementSystem"):
        return False
    for config in ("ocio://cg-config-latest", "ocio://studio-config-latest"):
        try:
            cms = mx_gen_shader.OcioColorManagementSystem.createFromBuiltinConfig(
                config, shadergen.getTarget())
            cms.loadLibrary(stdlib)
            return True
        except Exception:
            continue
    return False


def _skip_files_for_corpus(shadergen, stdlib):
    """Align with GenShaderUtil / GenWgsl.h OCIO gating."""
    skip = set()
    if not _ocio_cms_available(shadergen, stdlib):
        skip.add("ocio_color_management.mtlx")
    return skip


def _viewer_parity_validate(pixel_path, vertex_path, naga, node_exe):
    """Run TSL-portable conversion + naga; return error string or empty."""
    if not os.path.isfile(_VIEWER_PARITY_SCRIPT):
        return "viewer parity script missing: " + _VIEWER_PARITY_SCRIPT
    cmd = [node_exe, _VIEWER_PARITY_SCRIPT, "--pixel", pixel_path, "--naga", naga]
    if vertex_path and os.path.isfile(vertex_path):
        cmd.extend(["--vertex", vertex_path])
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        return ""
    except subprocess.CalledProcessError as exc:
        out = exc.output.decode("utf-8", errors="replace")
        if "Could not parse WGSL" in out:
            return out[out.find("Could not parse WGSL"):].strip()
        if "error:" in out:
            idx = out.rfind("error:")
            return out[max(0, idx - 120):].strip()
        return out.strip()


def _collect_mtlx(root, skip_files=None):
    """Recursively collect .mtlx files under *root*."""
    skip_files = skip_files or set()
    paths = []
    root = os.path.abspath(root)
    if os.path.isfile(root):
        if root.endswith(".mtlx") and os.path.basename(root) not in skip_files:
            paths.append(root)
    else:
        for dirpath, _dirs, files in os.walk(root):
            for f in sorted(files):
                if f.endswith(".mtlx") and f not in skip_files:
                    paths.append(os.path.join(dirpath, f))
    return paths


def _load_documents_style_dir(root_dir, search_path, skip_files):
    """Load .mtlx like MaterialXFormat::loadDocuments (one level of subfolders)."""
    pairs = []
    errors = []
    skip_files = set(skip_files)
    root = mx.FilePath(os.path.abspath(root_dir))
    if not root.isDirectory():
        return pairs, errors
    for subdir in root.getSubDirectories():
        for filename in subdir.getFilesInDirectory("mtlx"):
            if str(filename) in skip_files:
                continue
            file_path = subdir / filename
            doc = mx.createDocument()
            read_path = mx.FileSearchPath(search_path.asString())
            read_path.append(subdir)
            try:
                mx.readFromXmlFile(doc, file_path, read_path)
                pairs.append((file_path.asString(), doc))
            except mx.Exception as exc:
                errors.append(f"Failed to load: {file_path} — {exc}")
    return pairs, errors


def _load_tester_default_inputs(repo_root, search_path, skip_files):
    """TestSuite + Examples, matching WgslShaderGeneratorTester."""
    pairs = []
    errors = []
    for rel in _DEFAULT_INPUT_DIRS:
        root = os.path.join(repo_root, rel)
        if not os.path.isdir(root):
            continue
        sub_pairs, sub_errors = _load_documents_style_dir(root, search_path, skip_files)
        pairs.extend(sub_pairs)
        errors.extend(sub_errors)
    return pairs, errors


def _uses_documents_style_layout(abs_input, repo_root):
    """True when *abs_input* is TestSuite/Examples (C++ loadDocuments layout)."""
    try:
        rel = os.path.relpath(abs_input, repo_root).replace("\\", "/")
    except ValueError:
        return False
    return rel in _DEFAULT_INPUT_DIRS


def _naga_validate(path, stage_flag, naga):
    """Run naga on *path*; return error string or empty on success."""
    cmd = [naga, "--input-kind", "wgsl", "--shader-stage", stage_flag, path]
    try:
        subprocess.check_output(cmd, stderr=subprocess.STDOUT)
        return ""
    except subprocess.CalledProcessError as exc:
        return exc.output.decode("utf-8", errors="replace")


def _apply_genoptions(context, preset_name):
    opts = context.getOptions()
    preset = GENOPTIONS_PRESETS.get(preset_name, {})
    for key, val in preset.items():
        setattr(opts, key, val)


def _apply_tester_defaults(context):
    """Match HwShaderGenerator / ShaderGeneratorTester GenOptions defaults."""
    opts = context.getOptions()
    opts.premultipliedBsdfAdd = True
    if not opts.targetDistanceUnit:
        opts.targetDistanceUnit = "meter"
    if not opts.targetColorSpaceOverride:
        opts.targetColorSpaceOverride = "lin_rec709_scene"


def _find_lights(doc):
    lights = []
    for node in doc.getNodes():
        if node.getType() == mx.LIGHT_SHADER_TYPE_STRING:
            lights.append(node)
    return lights


def _register_lights(dep_lib, lights, context):
    mx_gen_shader.HwShaderGenerator.unbindLightShaders(context)
    if not lights:
        context.getOptions().hwMaxActiveLightSources = 0
        return

    id_map = {}
    next_id = 1
    for node in lights:
        nodedef = node.getNodeDef()
        if nodedef:
            name = nodedef.getName()
            if name not in id_map:
                id_map[name] = next_id
                next_id += 1

    for name, light_id in id_map.items():
        nodedef = dep_lib.getNodeDef(name)
        if nodedef:
            mx_gen_shader.HwShaderGenerator.bindLightShader(nodedef, light_id, context)

    context.getOptions().hwMaxActiveLightSources = len(lights)


def _prepare_document_context(shadergen, stdlib, doc, context):
    shadergen.registerShaderMetadata(doc, context)
    shadergen.registerTypeDefs(doc)
    lights = _find_lights(stdlib)
    _register_lights(stdlib, lights, context)


def _create_color_management(shadergen, stdlib):
    """Prefer OCIO CMS when available (required for ocio_color_management.mtlx)."""
    if hasattr(mx_gen_shader, "OcioColorManagementSystem"):
        for config in ("ocio://cg-config-latest", "ocio://studio-config-latest"):
            try:
                cms = mx_gen_shader.OcioColorManagementSystem.createFromBuiltinConfig(
                    config, shadergen.getTarget())
                cms.loadLibrary(stdlib)
                return cms
            except Exception:
                continue
    cms = mx_gen_shader.DefaultColorManagementSystem.create(shadergen.getTarget())
    cms.loadLibrary(stdlib)
    return cms


def _setup_generator(stdlib):
    """Create and configure a WgslShaderGenerator with CMS and units."""
    shadergen = mx_gen_wgsl.WgslShaderGenerator.create()

    cms = _create_color_management(shadergen, stdlib)
    shadergen.setColorManagementSystem(cms)

    unitsystem = mx_gen_shader.UnitSystem.create(shadergen.getTarget())
    registry = mx.UnitConverterRegistry.create()
    for unit_type in ("distance", "angle"):
        td = stdlib.getUnitTypeDef(unit_type)
        if td:
            registry.addUnitConverter(td, mx.LinearUnitConverter.create(td))
    unitsystem.loadLibrary(stdlib)
    unitsystem.setUnitConverterRegistry(registry)
    shadergen.setUnitSystem(unitsystem)

    return shadergen


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", action="append",
                        help="Path to an .mtlx file or folder. "
                        "TestSuite/Examples use the same layout as WgslShaderGeneratorTester; "
                        "other folders are scanned recursively. "
                        f"Default (no --input): {', '.join(_DEFAULT_INPUT_DIRS)}.")
    parser.add_argument("--naga", default=os.environ.get("NAGA", "naga"),
                        help="Path to the naga CLI executable.")
    parser.add_argument("--preset", default=["default"], nargs="+",
                        choices=list(GENOPTIONS_PRESETS.keys()),
                        help="GenOptions preset(s) to test.")
    parser.add_argument("--output", default=None,
                        help="Directory for generated shaders (uses temp dir if omitted).")
    parser.add_argument(
        "--viewer-parity",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="After full-module naga, run MaterialXView TSL-portable conversion + naga (default: on).",
    )
    parser.add_argument(
        "--node",
        default=os.environ.get("NODE", "node"),
        help="Node.js executable for --viewer-parity (default: node or NODE env).",
    )
    opts = parser.parse_args()

    # Load standard libraries.
    stdlib = mx.createDocument()
    repo_root = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", ".."))
    search_path = mx.FileSearchPath(repo_root)
    search_path.append(mx.getDefaultDataSearchPath())
    mx.loadLibraries(mx.getDefaultDataLibraryFolders(), search_path, stdlib)

    # Also load test light rigs.
    light_dir = search_path.find("resources/Materials/TestSuite/lights")
    if light_dir:
        for rig in ("light_compound_test.mtlx", "light_rig_test_1.mtlx"):
            rig_path = os.path.join(str(light_dir), rig)
            if os.path.isfile(rig_path):
                mx.readFromXmlFile(stdlib, rig_path, search_path)

    shadergen = _setup_generator(stdlib)
    shadergen.registerShaderMetadata(stdlib, mx_gen_shader.GenContext(shadergen))
    skip_files = _skip_files_for_corpus(shadergen, stdlib)

    document_pairs = []
    load_errors = []
    input_paths = opts.input
    if not input_paths:
        pairs, load_errors = _load_tester_default_inputs(
            repo_root, search_path, skip_files)
        document_pairs.extend(pairs)
    else:
        for inp in input_paths:
            abs_inp = os.path.abspath(inp)
            if os.path.isfile(abs_inp):
                doc = mx.createDocument()
                try:
                    mx.readFromXmlFile(doc, abs_inp, search_path)
                    document_pairs.append((abs_inp, doc))
                except mx.ExceptionFileMissing as exc:
                    print(f"  SKIP (missing): {abs_inp} — {exc}")
                continue
            if _uses_documents_style_layout(abs_inp, repo_root):
                pairs, errs = _load_documents_style_dir(
                    abs_inp, search_path, skip_files)
                document_pairs.extend(pairs)
                load_errors.extend(errs)
                continue
            for mtlx_path in _collect_mtlx(abs_inp, skip_files):
                doc = mx.createDocument()
                try:
                    mx.readFromXmlFile(doc, mtlx_path, search_path)
                except mx.ExceptionFileMissing as exc:
                    print(f"  SKIP (missing): {mtlx_path} — {exc}")
                    continue
                document_pairs.append((mtlx_path, doc))

    for err in load_errors:
        print(f"  WARN (load): {err}")

    if not document_pairs:
        sys.exit("No documents to validate.")

    use_temp = opts.output is None
    out_dir = opts.output or tempfile.mkdtemp(prefix="wgsl_validate_")
    os.makedirs(out_dir, exist_ok=True)

    total = 0
    failures = []

    for preset_name in opts.preset:
        print(f"=== GenOptions preset: {preset_name} ===")
        for mtlx_path, doc in document_pairs:
            try:
                doc.setDataLibrary(stdlib)
            except mx.Exception as exc:
                print(f"  SKIP (library): {mtlx_path} — {exc}")
                continue

            valid, msg = doc.validate()
            if not valid:
                print(f"  WARN (validation): {mtlx_path}\n    {msg}")

            renderables = mx_gen_shader.findRenderableElements(doc)
            if not renderables:
                continue

            context = mx_gen_shader.GenContext(shadergen)
            context.registerSourceCodeSearchPath(search_path)
            context.registerSourceCodeSearchPath(
                mx.FileSearchPath(os.path.dirname(os.path.abspath(mtlx_path))))
            _apply_genoptions(context, preset_name)
            _apply_tester_defaults(context)
            _prepare_document_context(shadergen, stdlib, doc, context)

            for elem in renderables:
                elem_name = elem.getNamePath() or elem.getName()
                node_def = elem.getNodeDef() if hasattr(elem, "getNodeDef") else None
                if node_def and node_def.getName() in SKIP_NODEDEFS:
                    continue

                doc_stem = os.path.splitext(os.path.basename(mtlx_path))[0]
                safe_name = mx.createValidName(f"{doc_stem}_{elem_name}")
                try:
                    shader = shadergen.generate(safe_name, elem, context)
                except Exception as exc:
                    failures.append((preset_name, mtlx_path, elem_name, f"generate: {exc}"))
                    total += 1
                    continue

                if shader is None:
                    failures.append((preset_name, mtlx_path, elem_name, "generate returned None"))
                    total += 1
                    continue

                stage_paths = {}
                for stage_name, stage_flag, ext in [
                    (mx_gen_shader.PIXEL_STAGE, "frag", "frag"),
                    (mx_gen_shader.VERTEX_STAGE, "vert", "vert"),
                ]:
                    src = shader.getSourceCode(stage_name)
                    if not src:
                        continue
                    fname = f"{safe_name}.{preset_name}.wgsl.{ext}"
                    out_path = os.path.join(out_dir, fname)
                    with open(out_path, "w", encoding="utf-8") as f:
                        f.write(src)
                    stage_paths[ext] = out_path
                    err = _naga_validate(out_path, stage_flag, opts.naga)
                    total += 1
                    if err:
                        failures.append((preset_name, mtlx_path, elem_name,
                                         f"naga ({stage_flag}): {err[:300]}"))

                if opts.viewer_parity and stage_paths.get("frag"):
                    err = _viewer_parity_validate(
                        stage_paths["frag"],
                        stage_paths.get("vert"),
                        opts.naga,
                        opts.node,
                    )
                    total += 1
                    if err:
                        failures.append((preset_name, mtlx_path, elem_name,
                                         f"viewer-parity: {err[:500]}"))

    def _safe_print(text):
        try:
            print(text)
        except UnicodeEncodeError:
            print(text.encode("ascii", errors="replace").decode("ascii"))

    _safe_print(f"\n{'='*60}")
    print(f"Total stages validated: {total}")
    print(f"Failures: {len(failures)}")
    if failures:
        for preset, path, elem, msg in failures:
            _safe_print(f"  FAIL [{preset}] {os.path.basename(path)}::{elem}")
            _safe_print(f"       {msg}")
        sys.exit(1)
    else:
        msg = "All stages passed naga validation."
        if opts.viewer_parity:
            msg += " (includes MaterialXView TSL-portable pixel parity)"
        print(msg)


if __name__ == "__main__":
    main()
