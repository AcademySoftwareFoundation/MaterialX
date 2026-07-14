#!/usr/bin/env python3
#
# Copyright Contributors to the MaterialX Project
# SPDX-License-Identifier: Apache-2.0
#
"""
Transpile MaterialX genglsl node fragments and genglsl/lib helpers to genwgsl (WGSL).

genglsl sources are not complete shaders: they use `#include`, `$`-tokens, and define functions
with no entry point. This tool wraps each target in a naga-parseable GLSL fragment shader, runs
`naga` (GLSL -> WGSL), then post-processes naga's SSA-style output into readable genwgsl fragments.

Generated node and lib `.wgsl` files are derived artifacts (not committed; CI regenerates them
before build). Re-running keeps genwgsl in sync with genglsl.

Node transpilation (per mx_*.glsl function):
  - ONE function body per naga call; lib context is *prototypes + #defines + structs*, not full
    lib bodies — keeps parameter names clean and avoids `$`-tokens inside lib implementations.
  - Skipped: image/hextiled texture nodes and light shaders (naga sampler / LightData limits).
  - Expected fallback: mx_chiang_hair_bsdf (naga limitation on hair scattering helpers).

Lib transpilation (`transpile_libs`, runs before nodes):
  - Each genglsl/lib/*.glsl becomes genwgsl/lib/*.wgsl in topological #include order (22 files).
  - Default: per-function transpile with full bodies of #included lib deps inlined, plus
    LIB_PREAMBLE (generator-option pins), texture stubs, and CALL_MAP overload renaming.
  - Sibling-body path for mx_microfacet_specular, mx_microfacet_sheen, mx_flake, mx_noise
    (heavy intra-file call graphs: transitive in-file callees + overload-aware topo sort).
  - Special case: mx_closure_type (static struct preamble + transpiled makeClosureData).
  - Lib transpile failures are hard errors (no hand-port fallback).

Usage:
  python glsl_to_wgsl.py --libraries libraries --out libraries
  python glsl_to_wgsl.py --libraries libraries --out build/genwgsl_generated
  python glsl_to_wgsl.py --libraries libraries --only mx_conductor_bsdf mx_math
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

# The naga CLI (GLSL->WGSL). Resolved in main() as: --naga arg, then $NAGA, then a `naga` on PATH.
# Install with `cargo install naga-cli` (see https://github.com/gfx-rs/wgpu/tree/trunk/naga).
NAGA = os.environ.get("NAGA") or "naga"


def resolve_naga(explicit=None):
    """Return a runnable naga command: explicit --naga, else $NAGA, else `naga` from PATH."""
    return explicit or os.environ.get("NAGA") or shutil.which("naga") or "naga"


def naga_version(naga):
    """Return naga's version string if it runs, else None (used to fail early with guidance)."""
    try:
        r = subprocess.run([naga, "--version"], capture_output=True, text=True)
        return r.stdout.strip() if r.returncode == 0 else None
    except OSError:
        return None


def discover_libs(libroot):
    """Library folders to process: every immediate subdirectory of --libraries that has a `genglsl`
    node directory. Today that is stdlib/pbrlib/lights (bxdf is nodegraph-based, nprlib/targets have
    no genglsl node fragments), but auto-discovering means new or downstream libraries are picked up
    without editing this tool. Sorted for stable output ordering."""
    libroot = Path(libroot)
    if not libroot.is_dir():
        return []
    return sorted(p.name for p in libroot.iterdir() if (p / "genglsl").is_dir())


SKIP_PATTERNS = [re.compile(p) for p in (
    r"image",      # texture nodes: naga's GLSL frontend has no sampler support
    r"hextiled",   # texture nodes
    r"_light$",    # light shaders take the dynamically-generated LightData struct
)]

# Nodes that are intentionally NOT generated and remain hand-written, by file stem. These hit a
# naga limitation (chiang hair). They are EXPECTED to fail transpile, so they don't constitute a
# build error -- only an *unexpected* failure (a previously-generable node that broke, e.g. after a
# genglsl change) sets a non-zero exit. Keep this in sync with the committed hand-written .wgsl
# files: if one starts transpiling cleanly, the tool warns so it can be removed here.
EXPECTED_FALLBACK = {
    "mx_chiang_hair_bsdf",
}

# genwgsl/lib helpers kept hand-written rather than transpiled, for two reasons:
#   * whole library dirs the project maintains by hand (HANDWRITTEN_LIB_DIRS) -- their lib .wgsl are
#     left untouched; nodes in those dirs are still generated;
#   * individual helpers whose bindings naga's GLSL frontend cannot express: environment
#     radiance/prefilter and shadow-map lookups sample generator-substituted texture+sampler
#     `$`-tokens, which naga rejects (same sampler limitation that skips image/light nodes). The
#     transpiler stubs the texture ops to parse, but the stub calls would leak into the output.
HANDWRITTEN_LIB_DIRS = {"stdlib"}
LIB_KEEP_HANDWRITTEN = {
    "mx_environment_fis", "mx_environment_none", "mx_environment_prefilter",
    "mx_generate_albedo_table", "mx_generate_prefilter_env", "mx_shadow", "mx_shadow_platform",
}

TYPE_FIXUPS = [
    (re.compile(r"\bvec([234])<f32>"), r"vec\1f"),
    (re.compile(r"\bvec([234])<i32>"), r"vec\1i"),
    (re.compile(r"\bvec([234])<u32>"), r"vec\1u"),
    (re.compile(r"\bmat([234])x\1<f32>"), r"mat\1x\1f"),
]

# WGSL has no function overloading, so the hand-written genwgsl lib gives each GLSL overload a
# distinct, type-suffixed name. This table maps a GLSL helper call -- keyed by (name, parameter
# types) exactly as build_context extracts them -- to the genwgsl function it resolves to.
# naga disambiguates overloaded calls with a deterministic `_N` suffix in prototype-declaration
# order (verified: stubs are kept and indexed by declaration order even when unused), so the
# remap pass below can rewrite each call to its genwgsl name.
#   * value None  -> intentionally unsupported (the genwgsl lib adapted this signature, e.g. the
#                    FresnelData-taking BSDF helpers); a node calling it stays hand-written.
#   * missing key -> an unmapped overload; treated like None (node stays hand-written) and logged.
CALL_MAP = {
    ("mx_square", ("float",)): "mx_square_f32",
    ("mx_square", ("vec2",)): "mx_square_vec2",
    ("mx_square", ("vec3",)): "mx_square_vec3",

    ("mx_f0_to_ior", ("float",)): "mx_f0_to_ior",
    ("mx_f0_to_ior", ("vec3",)): "mx_f0_to_ior_vec3",

    ("mx_fresnel_schlick", ("float", "float")): "mx_fresnel_schlick_f32",
    ("mx_fresnel_schlick", ("float", "vec3")): "mx_fresnel_schlick_vec3",
    ("mx_fresnel_schlick", ("float", "float", "float")): "mx_fresnel_schlick_f32_f90",
    ("mx_fresnel_schlick", ("float", "vec3", "vec3")): "mx_fresnel_schlick_vec3_f90",
    ("mx_fresnel_schlick", ("float", "float", "float", "float")): "mx_fresnel_schlick_f32_exp",
    ("mx_fresnel_schlick", ("float", "vec3", "vec3", "float")): "mx_fresnel_schlick_vec3_exp",

    ("mx_perlin_noise_float", ("vec2",)): "mx_perlin_noise_float_2d",
    ("mx_perlin_noise_float", ("vec3",)): "mx_perlin_noise_float_3d",
    ("mx_perlin_noise_vec3", ("vec2",)): "mx_perlin_noise_vec3_2d",
    ("mx_perlin_noise_vec3", ("vec3",)): "mx_perlin_noise_vec3_3d",

    ("mx_cell_noise_float", ("float",)): "mx_cell_noise_float_f32",
    ("mx_cell_noise_float", ("vec2",)): "mx_cell_noise_float_vec2",
    ("mx_cell_noise_float", ("vec3",)): "mx_cell_noise_float_vec3",
    ("mx_cell_noise_float", ("vec4",)): "mx_cell_noise_float_vec4",

    ("mx_worley_noise_float", ("vec2", "float", "int", "int")): "mx_worley_noise_float_2d",
    ("mx_worley_noise_float", ("vec3", "float", "int", "int")): "mx_worley_noise_float_3d",
    ("mx_worley_noise_vec2", ("vec2", "float", "int", "int")): "mx_worley_noise_vec2_2d",
    ("mx_worley_noise_vec2", ("vec3", "float", "int", "int")): "mx_worley_noise_vec2_3d",
    ("mx_worley_noise_vec3", ("vec2", "float", "int", "int")): "mx_worley_noise_vec3_2d",
    ("mx_worley_noise_vec3", ("vec3", "float", "int", "int")): "mx_worley_noise_vec3_3d",

    # Remaining mx_noise.glsl overloaded helpers -> type-suffixed genwgsl names (WGSL has no
    # overloading). Names mirror the hand-written genwgsl lib so cross-file callers still resolve.
    ("mx_bilerp", ("float", "float", "float", "float", "float", "float")): "mx_bilerp_f32",
    ("mx_bilerp", ("vec3", "vec3", "vec3", "vec3", "float", "float")): "mx_bilerp_vec3",
    ("mx_trilerp", ("float",) * 11): "mx_trilerp_f32",
    ("mx_trilerp", ("vec3",) * 8 + ("float", "float", "float")): "mx_trilerp_vec3",

    ("mx_gradient_float", ("uint", "float", "float")): "mx_gradient_float_2d",
    ("mx_gradient_float", ("uint", "float", "float", "float")): "mx_gradient_float_3d",
    ("mx_gradient_vec3", ("uvec3", "float", "float")): "mx_gradient_vec3_2d",
    ("mx_gradient_vec3", ("uvec3", "float", "float", "float")): "mx_gradient_vec3_3d",
    ("mx_gradient_scale2d", ("float",)): "mx_gradient_scale2d_f32",
    ("mx_gradient_scale2d", ("vec3",)): "mx_gradient_scale2d_vec3",
    ("mx_gradient_scale3d", ("float",)): "mx_gradient_scale3d_f32",
    ("mx_gradient_scale3d", ("vec3",)): "mx_gradient_scale3d_vec3",

    ("mx_hash_int", ("int",)): "mx_hash_int_i1",
    ("mx_hash_int", ("int", "int")): "mx_hash_int_i2",
    ("mx_hash_int", ("int", "int", "int")): "mx_hash_int_i3",
    ("mx_hash_int", ("int", "int", "int", "int")): "mx_hash_int_i4",
    ("mx_hash_int", ("int", "int", "int", "int", "int")): "mx_hash_int_i5",
    ("mx_hash_vec3", ("int", "int")): "mx_hash_vec3_i2",
    ("mx_hash_vec3", ("int", "int", "int")): "mx_hash_vec3_i3",

    ("mx_cell_noise_vec3", ("float",)): "mx_cell_noise_vec3_f32",
    ("mx_cell_noise_vec3", ("vec2",)): "mx_cell_noise_vec3_vec2",
    ("mx_cell_noise_vec3", ("vec3",)): "mx_cell_noise_vec3_vec3",
    ("mx_cell_noise_vec3", ("vec4",)): "mx_cell_noise_vec3_vec4",

    ("mx_worley_cell_position", ("int", "int", "int", "int", "float")): "mx_worley_cell_position_2d",
    ("mx_worley_cell_position", ("int", "int", "int", "int", "int", "int", "float")): "mx_worley_cell_position_3d",
    ("mx_worley_distance", ("vec2", "int", "int", "int", "int", "float", "int")): "mx_worley_distance_2d",
    ("mx_worley_distance", ("vec3", "int", "int", "int", "int", "int", "int", "float", "int")): "mx_worley_distance_3d",

    ("mx_matrix_mul", ("vec2", "mat2")): "mx_matrix_mul_vec2_mat2",
    ("mx_matrix_mul", ("vec3", "mat3")): "mx_matrix_mul_vec3_mat3",
    ("mx_matrix_mul", ("vec4", "mat4")): "mx_matrix_mul_vec4_mat4",
    ("mx_matrix_mul", ("mat2", "vec2")): "mx_matrix_mul_mat2_vec2",
    ("mx_matrix_mul", ("mat3", "vec3")): "mx_matrix_mul_mat3_vec3",
    ("mx_matrix_mul", ("mat4", "vec4")): "mx_matrix_mul_mat4_vec4",
    ("mx_matrix_mul", ("mat2", "mat2")): "mx_matrix_mul_mat2_mat2",
    ("mx_matrix_mul", ("mat3", "mat3")): "mx_matrix_mul_mat3_mat3",
    ("mx_matrix_mul", ("mat4", "mat4")): "mx_matrix_mul_mat4_mat4",

    # vec3/vec3-F90 directional albedo maps cleanly; float overload delegates to vec3.
    ("mx_ggx_dir_albedo", ("float", "float", "vec3", "vec3")): "mx_ggx_dir_albedo",
    ("mx_ggx_dir_albedo", ("float", "float", "float", "float")): "mx_ggx_dir_albedo_scalar",
    ("mx_ggx_dir_albedo", ("float", "float", "FresnelData")): "mx_ggx_dir_albedo_fresnel",
    ("mx_ggx_energy_compensation", ("float", "float", "FresnelData")): "mx_ggx_energy_compensation",
}

# ---------------------------------------------------------------------------- lib transpilation tables

# Lib files where per-function transpile without in-file siblings fails because helpers call
# each other in the same file. These use transpile_lib_file_siblings (transitive in-file
# callee bodies + overload-aware topo sort) instead of one-function-at-a-time.
LIB_USE_SIBLING_BODIES = {
    "mx_microfacet_specular", "mx_microfacet_sheen", "mx_flake", "mx_noise",
}

# Pin preprocessor branches to match current genwgsl behavior. Method 0 = analytic directional
# albedo (table/MC paths in microfacet libs are excluded from the WGSL port).
LIB_PREAMBLE = """
#define DIRECTIONAL_ALBEDO_METHOD 0
#define AIRY_FRESNEL_ITERATIONS 2
"""

# naga's GLSL frontend rejects `uniform sampler2D` and real texture ops in lib fragments.
# These stubs let the shader parse; only the function bodies we extract matter for output.
TEXTURE_STUB_PREAMBLE = """
vec3 mtlx_tex_lookup_rgb(vec2 uv, float lod) { return vec3(0.0); }
vec2 mtlx_tex_lookup_rg(vec2 uv) { return vec2(0.0); }
float mtlx_tex_lookup_b(vec2 uv) { return 0.0; }
float mtlx_tex_size_x() { return 256.0; }
"""

# MaterialX $-tokens are not valid GLSL identifiers. Lib files use them for sampler uniforms,
# environment tables, and the closure constructor macro — expand to literals/stubs before naga.
LIB_TOKEN_FIXUPS = [
    (re.compile(r"\$texSamplerSignature\b"), "sampler2D mtlx_tex_sampler"),
    (re.compile(r"\$texSamplerSampler2D\b"), "mtlx_tex_sampler"),
    (re.compile(r"\$albedoTable\b"), "mtlx_albedo_table"),
    (re.compile(r"\$albedoTableSize\b"), "vec2(256.0)"),
    (re.compile(r"\$envRadianceSamples\b"), "16"),
    (re.compile(r"\$envRadianceMips\b"), "8"),
    (re.compile(r"\$envMatrix\b"), "mat4(1.0)"),
    (re.compile(r"\$envRadiance\b"), "mtlx_env_radiance_stub"),
    (re.compile(r"\$envIrradiance\b"), "mtlx_env_irradiance_stub"),
    (re.compile(r"\$envLightIntensity\b"), "vec3(1.0)"),
    (re.compile(r"\$envPrefilterMip\b"), "0.0"),
    (re.compile(r"\$envRadianceSampler2D\b"), "mtlx_tex_sampler"),
    (re.compile(r"\$closureDataConstructor\b"),
     "ClosureData(closureType, L, V, N, P, occlusion)"),
    (re.compile(r"\$refractionTwoSided\b"), "false"),
]

# GLSL FresnelData uses tf_* field names; genwgsl lib uses thinfilm_* (hand-port convention).
LIB_FIELD_RENAMES = [
    (re.compile(r"\btf_thickness\b"), "thinfilm_thickness"),
    (re.compile(r"\btf_ior\b"), "thinfilm_ior"),
    # naga's WGSL backend appends `_` to any identifier ending in a digit (F0 -> F0_). The genwgsl
    # FresnelData (LIB_STRUCT_PREAMBLE) keeps the digit-suffixed field names, so strip naga's
    # trailing underscore back off these field accesses to match the struct definition.
    (re.compile(r"\bF0_\b"), "F0"),
    (re.compile(r"\bF82_\b"), "F82"),
    (re.compile(r"\bF90_\b"), "F90"),
]

# mx_closure_type.glsl only defines ClosureData + makeClosureData; the genwgsl file must also
# expose BSDF/VDF/FresnelData/EDF/material that nodes include. Prepended verbatim (not from naga).
# NOTE: surfaceshader is intentionally NOT here -- WgslShaderGenerator::emitTypeDefinitions emits it
# (and displacement/lightshader) *before* including this file, so the `material = surfaceshader`
# alias below resolves. Re-declaring it here would redeclare the struct in the assembled shader.
LIB_STRUCT_PREAMBLE = """
struct ClosureData {
    closureType: i32,
    L: vec3f,
    V: vec3f,
    N: vec3f,
    P: vec3f,
    occlusion: f32,
}

struct BSDF {
    response: vec3f,
    throughput: vec3f,
}

struct VDF {
    response: vec3f,
    throughput: vec3f,
}

alias EDF = vec3f;
alias material = surfaceshader;

struct FresnelData {
    model: i32,
    airy: bool,
    ior: vec3f,
    extinction: vec3f,
    F0: vec3f,
    F82: vec3f,
    F90: vec3f,
    exponent: f32,
    thinfilm_thickness: f32,
    thinfilm_ior: f32,
    refraction: bool,
}
"""

# Struct types emitted centrally by LIB_STRUCT_PREAMBLE (into mx_closure_type.wgsl). Other lib
# files that also declare them in GLSL (e.g. FresnelData in mx_microfacet_specular.glsl) must NOT
# re-emit a definition, or the assembled shader has duplicate/conflicting structs -- and the
# preamble's hand-port field names (thinfilm_* vs. the GLSL tf_*) would diverge from the bodies.
LIB_PREAMBLE_STRUCTS = {"surfaceshader", "ClosureData", "BSDF", "VDF", "FresnelData"}

GLSL_TO_WGSL_TYPE = {
    # Used by transpile_glsl_structs/consts when emitting WGSL struct/const preamble for lib files.
    "float": "f32", "int": "i32", "bool": "bool",
    "vec2": "vec2f", "vec3": "vec3f", "vec4": "vec4f",
    "mat2": "mat2x2f", "mat3": "mat3x3f", "mat4": "mat4x4f",
    "mat3x3": "mat3x3f", "mat2x2": "mat2x2f", "mat4x4": "mat4x4f",
}


# ---------------------------------------------------------------------------- parsing

# These two scanners do depth-aware bracket matching so we can carve functions and argument lists
# out of source text without a full parser. They are the foundation the rest of the parsing relies
# on -- naga gives us no AST, so everything here is string surgery over balanced brackets.

def _match_brace(text, i):
    """Given index of an opening '{', return index just past the matching '}'."""
    depth = 0
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return len(text)


def _match_paren(text, i):
    """Given index of an opening '(', return the index OF the matching ')' (-1 if unbalanced).

    Note the asymmetry with _match_brace: this returns the closing paren's own index (callers slice
    text[open+1:close] to get the contents), whereas _match_brace returns one past the '}'."""
    depth = 0
    while i < len(text):
        if text[i] == "(":
            depth += 1
        elif text[i] == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


GLSL_KEYWORDS = {"if", "else", "for", "while", "switch", "do", "return", "case", "default"}


def parse_functions(text):
    """Return [{ret, name, params, full}] for every top-level function *definition*.

    Heuristic, not a real parser: we match the `<returnType> <name>(` shape, then confirm it is a
    definition (not a call or a prototype) by checking that a `{` follows the closing paren. The
    keyword filter rejects control-flow that looks like `<word> <word>(` (e.g. `else if (`), and the
    `{`-lookahead rejects prototypes ending in `;`. `full` is the entire definition text (signature
    through matching `}`) so callers can re-emit or wrap it."""
    fns = []
    for m in re.finditer(r"\b([A-Za-z_]\w*)\s+([A-Za-z_]\w*)\s*\(", text):
        if m.group(1) in GLSL_KEYWORDS or m.group(2) in GLSL_KEYWORDS:
            continue  # control-flow construct (e.g. `else if (...) {`), not a definition
        open_p = text.index("(", m.start(2))
        close_p = _match_paren(text, open_p)
        if close_p < 0:
            continue
        # Skip whitespace after `)`; a definition has `{` here, a call/prototype has `;` or `,`.
        j = close_p + 1
        while j < len(text) and text[j].isspace():
            j += 1
        if j >= len(text) or text[j] != "{":
            continue  # a call/prototype, not a definition
        end = _match_brace(text, j)
        fns.append({"ret": m.group(1), "name": m.group(2),
                    "params": text[open_p + 1:close_p].strip(), "full": text[m.start(1):end]})
    return fns


def fn_param_types(params_str):
    """Extract bare GLSL parameter types from a parameter list string."""
    types = []
    for p in params_str.split(","):
        p = re.sub(r"^\s*(const|in|out|inout)\s+", "", p.strip())
        if p:
            types.append(re.sub(r"\s+", " ",
                                re.sub(r"\s+[A-Za-z_]\w*\s*(\[\d*\])?$", "", p)).strip())
    return tuple(types)


def expand_lib_tokens(text):
    """Replace MaterialX $-tokens and texture/sampler calls with naga-parseable GLSL.

    Node transpile uses per-function MTLXTOK_* sentinels; lib transpile uses fixed substitutions
    from LIB_TOKEN_FIXUPS plus regex rewrites below so included lib bodies compile as a unit."""
    for rx, repl in LIB_TOKEN_FIXUPS:
        text = rx.sub(repl, text)
    # Replace sampler types and texture ops with stubs naga accepts.
    text = re.sub(r"\bsampler2D\s+mtlx_\w+\b", "int mtlx_sampler_stub", text)
    text = re.sub(r"textureLod\s*\(\s*mtlx_tex_sampler\s*,\s*([^,]+),\s*([^)]+)\)",
                  r"mtlx_tex_lookup_rgb(\1, \2)", text)
    text = re.sub(r"texture\s*\(\s*mtlx_tex_sampler\s*,\s*([^)]+)\)\.xy",
                  r"mtlx_tex_lookup_rg(\1)", text)
    text = re.sub(r"texture\s*\(\s*mtlx_albedo_table\s*,\s*([^)]+)\)\.rg",
                  r"mtlx_tex_lookup_rg(\1)", text)
    text = re.sub(r"texture\s*\(\s*mtlx_albedo_table\s*,\s*([^)]+)\)\.b",
                  r"mtlx_tex_lookup_b(\1)", text)
    text = re.sub(r"textureSize\s*\(\s*mtlx_tex_sampler\s*,\s*0\s*\)",
                  r"vec2(256.0)", text)
    text = re.sub(r"textureSize\s*\(\s*mtlx_albedo_table\s*,\s*0\s*\)\.x",
                  r"mtlx_tex_size_x()", text)
    text = re.sub(r"textureSize\s*\(\s*mtlx_env_radiance_stub\s*,\s*0\s*\)\.x",
                  r"mtlx_tex_size_x()", text)
    text = re.sub(r"texture\s*\(\s*mtlx_env_radiance_stub\s*,\s*([^)]+)\)\.rgb",
                  r"mtlx_tex_lookup_rgb(\1, 0.0)", text)
    text = re.sub(r"texture\s*\(\s*mtlx_env_irradiance_stub\s*,\s*([^)]+)\)\.rgb",
                  r"mtlx_tex_lookup_rgb(\1, 0.0)", text)
    text = re.sub(r"mx_latlong_map_lookup\s*\(([^,]+),\s*([^,]+),\s*([^,]+),\s*mtlx_env_radiance_stub\s*\)",
                  r"mtlx_tex_lookup_rgb(vec2(0.0), \3)", text)
    text = re.sub(r"mx_latlong_map_lookup\s*\(([^,]+),\s*([^,]+),\s*([^,]+),\s*mtlx_env_irradiance_stub\s*\)",
                  r"mtlx_tex_lookup_rgb(vec2(0.0), \3)", text)
    return text


def wgsl_fn_name(glsl_name, types):
    """Return the genwgsl function name for a GLSL overload.

    CALL_MAP is keyed by (name, param_types). For lib *output* we use this to emit type-suffixed
    definitions (e.g. mx_square_f32); for node *calls* remap_calls() applies the same map."""
    target = CALL_MAP.get((glsl_name, types))
    if target:
        return target
    return glsl_name


def apply_field_renames(text):
    """Apply LIB_FIELD_RENAMES to struct bodies and field accesses in transpiled WGSL."""
    for rx, repl in LIB_FIELD_RENAMES:
        text = rx.sub(repl, text)
    return text


def resolve_const_refs(text, const_names):
    """Rewrite naga-suffixed references to module-scope consts back to their declared name.

    naga's WGSL backend suffixes an identifier reference with `_` when the name ends in a digit
    (guarding its own `name_N` SSA scheme) and with `_<N>` when the name collides with another
    declaration in naga's input. Both bite our file-scope consts: build_context puts every lib
    const in naga's parse context, and the sibling-body path *also* emits the file's preamble into
    the wrapped fragment, so naga sees `FRESNEL_MODEL_SCHLICK` twice and renames body references to
    `FRESNEL_MODEL_SCHLICK_1`; a digit-ending const like FUJII_CONSTANT_1 becomes FUJII_CONSTANT_1_.
    The declarations we emit keep the clean GLSL name, so map `NAME_` / `NAME_<N>` back to `NAME`.
    Longest name first so a shorter const can't capture a longer one's suffix; a suffixed token that
    is itself a declared const (e.g. FUJII_CONSTANT_1) is left untouched. Keyed only on known const
    names, so naga's numbered *locals* (e.g. F0_1 = F0) are not affected."""
    names = set(n for n in const_names if n)
    for nm in sorted(names, key=len, reverse=True):
        text = re.sub(r"\b" + re.escape(nm) + r"_\d*\b",
                      lambda m: m.group(0) if m.group(0) in names else nm, text)
    return text


def const_names_from_wgsl(consts):
    """Names of `const NAME: T = ...` declarations (WGSL form, from transpile_glsl_consts)."""
    return [m.group(1) for c in consts for m in [re.match(r"const (\w+):", c)] if m]


def glsl_const_names(glsl_text):
    """Names of file-scope `const TYPE NAME = ...` declarations in GLSL context text (e.g. base)."""
    return re.findall(r"\bconst\s+[\w<>]+\s+([A-Za-z_]\w*)\s*=", glsl_text)


def glsl_type_to_wgsl(typename):
    return GLSL_TO_WGSL_TYPE.get(typename, typename)


def transpile_glsl_structs(text):
    """Convert top-level GLSL struct definitions to WGSL (emitted above lib fn bodies).

    Skips types provided centrally by LIB_STRUCT_PREAMBLE (see LIB_PREAMBLE_STRUCTS). Strips GLSL
    comments before splitting fields -- otherwise a comment word is read as a `type name` pair (e.g.
    `// Fresnel model` -> `Fresnel: //,`) and the real following field is dropped. Rewrites GLSL
    array fields to WGSL (`vec2 coords[3]` -> `coords: array<vec2f, 3>`)."""
    out = []
    for m in re.finditer(r"\bstruct\s+(\w+)\s*\{([^}]*)\}\s*;?", text):
        name = m.group(1)
        if name in LIB_PREAMBLE_STRUCTS:
            continue
        body_src = re.sub(r"/\*.*?\*/", "", m.group(2), flags=re.S)  # block comments
        body_src = re.sub(r"//[^\n]*", "", body_src)                 # line comments
        fields = []
        for part in body_src.split(";"):
            part = part.strip()
            if not part:
                continue
            tokens = part.split()
            if len(tokens) < 2:
                continue
            ftype, fname = tokens[0], tokens[1]
            wtype = glsl_type_to_wgsl(ftype)
            arr = re.match(r"(\w+)\[(\d+)\]$", fname)  # GLSL array field `name[N]`
            if arr:
                fname, wtype = arr.group(1), f"array<{wtype}, {arr.group(2)}>"
            fields.append(f"    {fname}: {wtype},")
        if fields:
            body = "\n".join(fields).rstrip(",")
            out.append(f"struct {name} {{\n{body}\n}}")
    return out


def transpile_glsl_consts(text):
    """Convert file-scope GLSL `const T name = ...` to WGSL `const name: T = ...` syntax.

    Only *top-level* consts are emitted. Function-local consts (e.g. the `const int SAMPLE_COUNT`
    inside several directional-albedo helpers) are handled by naga inside the transpiled bodies;
    hoisting them here would produce duplicate module-scope consts -- and since the same name
    recurs across microfacet libs, redeclaration errors once several are included together. Blank
    out function bodies first so their indented consts aren't matched by the file-scope regex."""
    scope = text
    for fn in parse_functions(text):
        scope = scope.replace(fn["full"], "")
    out = []
    for m in re.finditer(r"^\s*const\s+(\w+)\s+(\w+)\s*=\s*([^;]+);", scope, re.M):
        out.append(f"const {m.group(2)}: {glsl_type_to_wgsl(m.group(1))} = {m.group(3)};")
    return out


def lib_includes(text):
    """Return #include paths from a lib file (lib/... only)."""
    return [inc for inc in re.findall(r'#include\s+"([^"]+)"', text)
            if inc.startswith("lib/")]


def topo_sort_lib_files(lib_files):
    """Topological sort of lib/*.glsl by `#include \"lib/...\"` dependencies.

    Ensures mx_microfacet.glsl is processed before mx_microfacet_specular.glsl, etc., so
    generated genwgsl/lib/*.wgsl includes resolve when nodes are transpiled later."""
    stems = {f.stem: f for f in lib_files}
    deps = {}
    for f in lib_files:
        txt = f.read_text(encoding="utf-8")
        deps[f.stem] = {Path(inc).stem for inc in lib_includes(txt) if Path(inc).stem in stems}
    order, seen = [], set()

    def visit(stem):
        if stem in seen:
            return
        seen.add(stem)
        for d in sorted(deps.get(stem, ())):
            visit(d)
        order.append(stems[stem])

    for stem in sorted(stems):
        visit(stem)
    return order


def postprocess_wgsl_fn(text, fn_name, protos, lib_symbols=None, remap=True):
    """Shared post-processing for one transpiled WGSL function (lib or node).

    Runs cleanup, type normalization, CALL_MAP remapping, and optional lib arity checks.
    Returns (text, []) on success or (None, unsupported_calls) when remap/arity fails."""
    text = cleanup_function(text)
    for rx, repl in TYPE_FIXUPS:
        text = rx.sub(repl, text)
    text = re.sub(r"\b(\d+\.\d+(?:[eE][-+]?\d+)?)f\b", r"\1", text)
    text = re.sub(r"(?<![\w.])(\d+)f\b", r"\1.0", text)
    all_names = {nm for (nm, _t) in protos}
    for nm in all_names:
        text = re.sub(r"\b" + re.escape(nm) + r"_\b", nm, text)
    text = apply_field_renames(text)
    if remap:
        text, unsupported = remap_calls(text, fn_name, protos)
        if lib_symbols is not None:
            unsupported += check_lib_arity(text, lib_symbols)
        if unsupported:
            return None, unsupported
    return text, []


def naga_transpile_glsl(glsl_src, debug_path=None):
    """Run naga on a complete GLSL fragment shader; return (WGSL text, None) or (None, error).

    When MTLX_DEBUG is set, callers may pass debug_path to persist the failing GLSL input."""
    with tempfile.TemporaryDirectory() as td:
        frag = Path(td) / "in.frag"
        wtmp = Path(td) / "out.wgsl"
        frag.write_text(glsl_src, encoding="utf-8")
        r = subprocess.run([NAGA, "--input-kind", "glsl", "--shader-stage", "frag",
                            str(frag), str(wtmp)], capture_output=True, text=True)
        if r.returncode != 0:
            if debug_path:
                debug_path.parent.mkdir(parents=True, exist_ok=True)
                debug_path.write_text(glsl_src, encoding="utf-8")
            err = next((l for l in r.stderr.splitlines() if "error" in l.lower()), "naga error")
            return None, err.strip()
        return wtmp.read_text(encoding="utf-8"), None


# naga renders GLSL scalar/vector/matrix types in this canonical form; used to match a target
# overload's GLSL parameter types against a naga output function's signature.
GLSL_TO_NAGA_TYPE = {
    "float": "f32", "int": "i32", "uint": "u32", "bool": "bool",
    "vec2": "vec2<f32>", "vec3": "vec3<f32>", "vec4": "vec4<f32>",
    "ivec2": "vec2<i32>", "ivec3": "vec3<i32>", "ivec4": "vec4<i32>",
    "uvec2": "vec2<u32>", "uvec3": "vec3<u32>", "uvec4": "vec4<u32>",
    "mat2": "mat2x2<f32>", "mat3": "mat3x3<f32>", "mat4": "mat4x4<f32>",
}


def _wgsl_sig_types(fn_text):
    """Parameter types from a (raw naga) WGSL fn signature, e.g. ('f32', 'vec3<f32>')."""
    sig = re.match(r"fn\s+\w+\s*\(([^)]*)\)", fn_text, re.S)
    if not sig:
        return ()
    out = []
    for p in sig.group(1).split(","):
        m = re.match(r"[A-Za-z_]\w*\s*:\s*(.+)", p.strip(), re.S)
        if m:
            out.append(re.sub(r"\s+", "", m.group(1)))
    return tuple(out)


def extract_transpiled_fn(wgsl, glsl_name, wgsl_name, glsl_types=None):
    """Pull one non-stub function from naga output and rename to the target genwgsl name.

    Per-function lib/node transpile feeds naga a module with one real body; this extracts that
    body and renames it (e.g. mx_square -> mx_square_f32) via wgsl_fn_name. When several same-named
    overloads appear in one naga module (sibling path, e.g. mx_ggx_dir_albedo's vec3, scalar, and
    FresnelData forms), `glsl_types` disambiguates by matching the parameter signature; with a single
    candidate the base-name match is used directly (signature match is skipped, since naga rewrites
    `out` params to pointers and would otherwise not compare equal)."""
    cands = [(name, t) for name, t in top_level_fns(wgsl)
             if fn_base(name) == glsl_name and "{\n}" not in t and t.count("\n") > 1]
    if not cands:
        return None
    chosen = cands[0]
    if len(cands) > 1 and glsl_types is not None:
        want = tuple(GLSL_TO_NAGA_TYPE.get(t, t) for t in glsl_types)
        chosen = next(((n, t) for n, t in cands if _wgsl_sig_types(t) == want), cands[0])
    name, t = chosen
    return re.sub(r"(\bfn\s+)" + re.escape(name) + r"\b", r"\1" + wgsl_name, t)


# Closure/shader types the generator emits (GlslShaderGenerator::emitTypeDefinitions), not the
# lib files -- supply them so node BSDF/EDF signatures parse.
CLOSURE_PREAMBLE = """
struct BSDF { vec3 response; vec3 throughput; };
struct VDF { vec3 response; vec3 throughput; };
#define EDF vec3
struct surfaceshader { vec3 color; vec3 transparency; };
struct volumeshader { vec3 color; vec3 transparency; };
struct displacementshader { vec3 offset; float scale; };
struct lightshader { vec3 intensity; vec3 direction; };
#define material surfaceshader
"""


def naga_proto_params(params_str):
    """Normalize GLSL prototype params for naga: strip the semantically-neutral `const`/`in`
    qualifiers, KEEP `out`/`inout`, and drop sampler params naga's prototype parser rejects.

    Keeping `out`/`inout` is essential: naga renders those params as `ptr<function, T>`, and when it
    transpiles a *caller* it must see the callee's prototype as by-pointer so it passes the pointer
    (e.g. `mx_normalmap_vector2(..., result)`) rather than dereferencing it (`(*result)`), which
    would mismatch the transpiled definition's pointer parameter. naga accepts `out`/`inout` in a
    forward declaration."""
    parts = []
    for p in params_str.split(","):
        p = p.strip()
        p = re.sub(r"^const\s+", "", p)   # const: no call-site effect
        p = re.sub(r"^in\s+", "", p)      # `in` is the default (by value); `\s+` guards `int`/`inout`
        if not p or "sampler2D" in p:
            continue
        parts.append(p)
    return ", ".join(parts)


def build_context(libroot, libs, nodes=True):
    """Build the naga parse context. Returns (base, protos, overloaded):
      base       - CLOSURE_PREAMBLE + dedup'd #defines/consts/struct-defs from every genglsl lib.
      protos     - {(name, types): "prototype;"} for every function defined in any genglsl lib OR
                   node file, so helpers AND cross-node calls (e.g. mx_burn_color3 -> mx_burn_float)
                   and generator-supplied helpers (mx_environment_radiance) all resolve.
      overloaded - the set of names appearing with more than one signature (informational; the
                   actual overload remapping is driven by CALL_MAP in remap_calls).
    Per node we emit `base` + every prototype except the one being defined.

    Pass nodes=False for lib transpilation: skip mx_*.glsl node files (their `inout` protos break
    naga) and rely on inlined lib bodies + LIB_PREAMBLE instead of CLOSURE_PREAMBLE duplication."""
    defines, consts, structs, protos = {}, {}, {}, {}
    # Scan lib/ helpers and the node files themselves (the latter for cross-node prototypes).
    sources = []
    for lib in libs:
        gldir = libroot / lib / "genglsl"
        if gldir.is_dir():
            sources += sorted((gldir / "lib").glob("*.glsl"))
            if nodes:
                # Node files too (for cross-node prototypes), minus the skipped ones (light shaders
                # reference the dynamic LightData struct; texture nodes carry $-tokens).
                sources += [f for f in sorted(gldir.glob("mx_*.glsl"))
                            if not any(p.search(f.stem) for p in SKIP_PATTERNS)]
    # Collect #defines, consts, struct definitions, and function prototypes across all sources.
    # `setdefault` keeps the first occurrence, so a symbol defined in several files is deduped.
    for f in sources:
        txt = f.read_text(encoding="utf-8")
        for l in txt.splitlines():
            m = re.match(r"#define\s+(\w+)", l.lstrip())
            if m:
                defines.setdefault(m.group(1), l.strip())
        for m in re.finditer(r"^\s*const\s+[\w<>]+\s+(\w+)\s*=[^;{]*;", txt, re.M):
            consts.setdefault(m.group(1), m.group(0).strip())
        for m in re.finditer(r"\bstruct\s+(\w+)\s*\{[^}]*\}\s*;?", txt):
            structs.setdefault(m.group(1), m.group(0))
        for fn in parse_functions(txt):
            types = fn_param_types(fn["params"])
            protos.setdefault((fn["name"], types),
                              f"{fn['ret']} {fn['name']}({naga_proto_params(fn['params'])});")
    base_items = list(defines.values()) + list(consts.values()) + list(structs.values())
    base = CLOSURE_PREAMBLE + "\n".join(s for s in base_items if "$" not in s)
    protos = {k: v for k, v in protos.items() if "$" not in v}
    # Names with more than one signature are GLSL-overloaded. WGSL has no overloading and the
    # hand-written genwgsl lib renames/consolidates these (e.g. mx_square_f32, a single
    # mx_ggx_dir_albedo), so the util cannot guarantee a transpiled call resolves against the lib.
    proto_names = [nm for (nm, _t) in protos]
    overloaded = {nm for nm in proto_names if proto_names.count(nm) > 1}
    return base, protos, overloaded


# ---------------------------------------------------------------------------- cleanup

# naga emits SSA-style output: function parameters are immutable in its IR, so it copies each into a
# mutable local shadow, and it spills every subexpression into a numbered `let _eN`. Faithful but
# unreadable. The two passes below undo that churn with conservative, single-assignment-safe rewrites
# (anything ambiguous is left as-is -- correct but verbose), so the committed WGSL reads like the
# hand-written files.

def collapse_param_copies(body, params):
    """Fold naga's read-only param shadows: `var p_1: T; p_1 = p;` -> use `p` directly.

    Only safe when the shadow is assigned exactly once (the `p_1 = p;` init). If naga reassigns the
    shadow later, the single-assignment test fails and we leave it alone."""
    for p in params:
        for m in re.finditer(r"\bvar\s+(" + re.escape(p) + r"_\d+)\s*:\s*[\w<>]+\s*;", body):
            shadow = m.group(1)
            if len(re.findall(r"\b" + re.escape(shadow) + r"\s*=", body)) == 1 and \
               re.search(r"\b" + re.escape(shadow) + r"\s*=\s*" + re.escape(p) + r"\s*;", body):
                body = re.sub(r"[ \t]*\bvar\s+" + re.escape(shadow) + r"\s*:\s*[\w<>]+\s*;\n?", "", body)
                body = re.sub(r"[ \t]*\b" + re.escape(shadow) + r"\s*=\s*" + re.escape(p) + r"\s*;\n?", "", body)
                body = re.sub(r"\b" + re.escape(shadow) + r"\b", p, body)
    return body


def inline_single_use_temps(body):
    """Inline naga's single-use `let _eN = expr;` temporaries.

    Iterates to a fixpoint: each pass inlines one temp used exactly once (the `- 1` discounts the
    definition itself from the match count), then restarts because inlining can make another temp
    single-use. The expr is wrapped in parens when it contains an operator, so precedence is
    preserved at the splice site."""
    changed = True
    while changed:
        changed = False
        for m in re.finditer(r"[ \t]*let\s+(_e\d+)\s*=\s*(.+?);\n", body):
            name, expr = m.group(1), m.group(2)
            if len(re.findall(r"\b" + re.escape(name) + r"\b", body)) - 1 == 1:
                repl = "(" + expr + ")" if re.search(r"[-+*/ ]", expr.strip()) else expr
                body = body[:m.start()] + body[m.end():]
                body = re.sub(r"\b" + re.escape(name) + r"\b", lambda _m: repl, body, count=1)
                changed = True
                break  # body changed; restart the scan from the top
    return body


def cleanup_function(fn_text):
    """Run both readability passes over one WGSL function, in dependency order (param shadows first,
    then temps, since collapsing a shadow can leave a temp single-use)."""
    sig = re.match(r"fn\s+\w+\s*\(([^)]*)\)", fn_text, re.S)
    params = re.findall(r"(\w+)\s*:", sig.group(1)) if sig else []
    return inline_single_use_temps(collapse_param_copies(fn_text, params))


def fn_base(name):
    """naga may append `_`/`_N` to disambiguate; strip it to recover the source name."""
    return re.sub(r"_\d*$", "", name)


def _count_items(s, brackets="()[]<>"):
    """Count top-level comma-separated items in `s` (0 if blank), ignoring commas nested inside
    the given bracket pairs."""
    s = s.strip()
    if not s:
        return 0
    opens = brackets[0::2]
    closes = brackets[1::2]
    depth = 0
    n = 1
    for ch in s:
        if ch in opens:
            depth += 1
        elif ch in closes:
            depth = max(0, depth - 1)
        elif ch == "," and depth == 0:
            n += 1
    return n


def build_wgsl_lib_symbols(libroot, libs, outroot=None):
    """Parse genwgsl `lib/` files into {function name: parameter count}.

    Reads from `outroot` when set (post-generation), else `libroot` (legacy hand-written tree).
    """
    symbols = {}
    root = outroot if outroot is not None else libroot
    for lib in libs:
        wdir = root / lib / "genwgsl" / "lib"
        if not wdir.is_dir():
            continue
        for f in sorted(wdir.glob("*.wgsl")):
            txt = f.read_text(encoding="utf-8")
            for m in re.finditer(r"\bfn\s+([A-Za-z_]\w*)\s*\(", txt):
                op = txt.index("(", m.start())
                cp = _match_paren(txt, op)
                if cp >= 0:
                    symbols[m.group(1)] = _count_items(txt[op + 1:cp])
    return symbols


def check_lib_arity(text, lib_symbols):
    """Return a list of calls whose arg count disagrees with the genwgsl lib's parameter count."""
    bad = []
    for m in re.finditer(r"\b(mx_\w+)\s*\(", text):
        name = m.group(1)
        if name not in lib_symbols:
            continue  # cross-node helper, builtin, or the node's own fn -- not a lib symbol
        op = text.index("(", m.start())
        cp = _match_paren(text, op)
        if cp < 0:
            continue
        nargs = _count_items(text[op + 1:cp], "()[]")  # call args: no generics, keep '<'/'>' literal
        if nargs != lib_symbols[name]:
            bad.append(f"{name} (call has {nargs}, lib expects {lib_symbols[name]})")
    return bad


def remap_calls(text, node_fn_name, protos):
    """Rewrite overloaded/diverged helper calls to their genwgsl names via CALL_MAP.

    naga numbers overload stubs (`mx_foo`, `mx_foo_1`, ...) by prototype-declaration order, and
    proto_block is emitted sorted by (name, types), so the i-th call name corresponds to the i-th
    entry of the base's sorted type list. Returns (text, unsupported) where unsupported lists any
    call whose CALL_MAP entry is None/missing -- the caller fails the node so it stays hand-written.
    """
    by_base = {}
    for (nm, types) in protos:
        by_base.setdefault(nm, []).append(types)
    for nm in by_base:
        by_base[nm] = sorted(by_base[nm])

    unsupported = []
    for base in sorted({b for (b, _t) in CALL_MAP}, key=len, reverse=True):
        if base == node_fn_name or base not in by_base:
            continue
        for i, types in enumerate(by_base[base]):
            callname = base if i == 0 else f"{base}_{i}"
            if not re.search(r"\b" + re.escape(callname) + r"\s*\(", text):
                continue
            target = CALL_MAP.get((base, types))
            if not target:  # None (adapted) or missing (unmapped overload) -> keep hand-written
                unsupported.append(callname + "(" + ", ".join(types) + ")")
                continue
            text = re.sub(r"\b" + re.escape(callname) + r"\s*\(", target + "(", text)
    return text, unsupported


# naga's canonical type spelling -> GLSL type used to key CALL_MAP (inverse of GLSL_TO_NAGA_TYPE).
NAGA_TO_GLSL_TYPE = {v: k for k, v in GLSL_TO_NAGA_TYPE.items()}


def remap_calls_by_naga_sig(text, wgsl_module, self_base):
    """Remap overloaded helper calls using the signatures naga actually assigned in wgsl_module.

    The sibling-body path defines several overloads of a name in one module, and naga numbers the
    stubs (`mx_foo`, `mx_foo_1`, ...) in an order that depends on its internal processing -- not the
    sorted-proto order remap_calls assumes. Rather than guess the index, read each naga function's
    real parameter signature straight from the module, recover its GLSL types, and resolve
    (base, types) through CALL_MAP. Longest names first so `mx_foo_1` isn't clobbered by `mx_foo`.
    `self_base` is the target's own name (skip, so a recursive call isn't misrouted). Returns
    (text, unsupported) where unsupported lists calls whose CALL_MAP entry is None/missing."""
    call_bases = {b for (b, _t) in CALL_MAP}
    sigs = {}
    for name, t in top_level_fns(wgsl_module):
        wt = _wgsl_sig_types(t)
        sigs[name] = tuple(NAGA_TO_GLSL_TYPE.get(x, x) for x in wt)
    unsupported = []
    for name in sorted(sigs, key=len, reverse=True):
        base = fn_base(name)
        if base == self_base or base not in call_bases:
            continue
        if not re.search(r"\b" + re.escape(name) + r"\s*\(", text):
            continue
        key = (base, sigs[name])
        if key not in CALL_MAP:
            continue  # this exact overload isn't a CALL_MAP entry (e.g. a same-base local helper)
        target = CALL_MAP[key]
        if not target:  # explicit None -> adapted signature, keep hand-written
            unsupported.append(name + "(" + ", ".join(sigs[name]) + ")")
            continue
        text = re.sub(r"\b" + re.escape(name) + r"\s*\(", target + "(", text)
    return text, unsupported


def top_level_fns(wgsl):
    """Yield (name, text) for each top-level `fn` in a WGSL module."""
    for m in re.finditer(r"\bfn\s+([A-Za-z_]\w*)\s*\(", wgsl):
        brace = wgsl.index("{", m.start())
        yield m.group(1), wgsl[m.start():_match_brace(wgsl, brace)]


# ---------------------------------------------------------------------------- lib driver
#
# transpile_libs() walks genglsl/lib/*.glsl in include order and writes genwgsl/lib/*.wgsl.
# Strategy mirrors nodes (wrap -> naga -> cleanup) but emits full helper libraries:
#   - default: one naga invocation per function, with #included deps inlined as bodies
#   - LIB_USE_SIBLING_BODIES: transitive in-file callee bodies (overload-aware topo sort)
#   - mx_closure_type: static struct preamble + transpiled makeClosureData only

def lib_needs_samplers(src):
    """True if this lib references samplers or $-token texture uniforms (needs TEXTURE_STUB_PREAMBLE)."""
    return bool(re.search(r"\btexture\w*\s*\(|\$texSampler|\$albedoTable|\$envRadiance|\$envIrradiance",
                          src))


def _extract_mx_calls(fn_body, known_names):
    """Return mx_* callees invoked in fn_body (body only, not the signature)."""
    if "{" not in fn_body:
        return set()
    body = fn_body[fn_body.index("{") + 1:fn_body.rfind("}")]
    return {m.group(1) for m in re.finditer(r"\b(mx_\w+)\s*\(", body) if m.group(1) in known_names}


def sort_functions_topo(fns):
    """Reorder function definitions so callees precede callers (naga requirement).

    Overload-aware: each (name, param-types) pair is a distinct node so naga receives every
    callee body, not just the first overload sharing a name."""
    if len(fns) <= 1:
        return fns

    def fn_key(fn):
        return (fn["name"], fn_param_types(fn["params"]))

    by_name = {}
    key_to_fn = {}
    for fn in fns:
        k = fn_key(fn)
        by_name.setdefault(fn["name"], []).append(fn)
        key_to_fn[k] = fn

    local_names = set(by_name.keys())
    deps = {}
    for fn in fns:
        k = fn_key(fn)
        dep_keys = []
        for callee in _extract_mx_calls(fn["full"], local_names):
            for callee_fn in by_name.get(callee, []):
                ck = fn_key(callee_fn)
                if ck in key_to_fn:
                    dep_keys.append(ck)
        deps[k] = dep_keys

    order_keys, temp, perm = [], set(), set()

    def visit(k):
        if k in perm:
            return
        if k in temp:
            return
        temp.add(k)
        for dk in deps.get(k, ()):
            visit(dk)
        temp.remove(k)
        perm.add(k)
        order_keys.append(k)

    for fn in fns:
        visit(fn_key(fn))

    return [key_to_fn[k] for k in order_keys]


def rebuild_fn_body(fns):
    """Concatenate function definitions in topo order for a single naga translation unit."""
    return "\n\n".join(fn["full"] for fn in fns)


def expand_lib_includes(text, gllib_dir, seen=None):
    """Inline `#include \"lib/...\"` bodies so naga sees full dependency definitions.

    Recursive with `seen` to break include cycles. Only `lib/` includes are expanded (not
    cross-library paths). Used for per-function dep context and whole-file transpile."""
    seen = seen or set()

    def replacer(match):
        inc_path = match.group(1)
        if not inc_path.startswith("lib/"):
            return match.group(0)
        stem = Path(inc_path).stem
        if stem in seen:
            return ""
        seen.add(stem)
        inc_file = gllib_dir / (stem + ".glsl")
        if not inc_file.is_file():
            return ""
        inc_text = expand_lib_tokens(inc_file.read_text(encoding="utf-8"))
        inc_text = expand_lib_includes(inc_text, gllib_dir, seen)
        inc_text = re.sub(r'#include\s+"[^"]+"\s*\n', "", inc_text)
        return inc_text + "\n"

    return re.sub(r'#include\s+"([^"]+)"\s*\n', replacer, text)


def _strip_wgsl_stubs(wgsl):
    """Remove empty stub functions naga emits for unused GLSL prototypes."""
    out = []
    for name, text in top_level_fns(wgsl):
        if "{\n}" in text and text.count("\n") <= 2:
            continue
        out.append(text)
    return "\n\n".join(out)


def _match_glsl_to_wgsl_fns(glsl_fns, wgsl):
    """Pair GLSL function definitions with naga output by name and overload order.

    Whole-file transpile emits every function in one module; this maps each GLSL definition to
    its naga counterpart (mx_foo, mx_foo_1, ...) and renames to the genwgsl overload name."""
    wgsl_by_base = {}
    for name, text in top_level_fns(wgsl):
        if "{\n}" in text and text.count("\n") <= 2:
            continue
        wgsl_by_base.setdefault(fn_base(name), []).append((name, text))

    outputs = []
    seen = {}
    for fn in glsl_fns:
        i = seen.get(fn["name"], 0)
        seen[fn["name"]] = i + 1
        wgsl_list = wgsl_by_base.get(fn["name"], [])
        if i >= len(wgsl_list):
            return None, fn["name"]
        naga_name, text = wgsl_list[i]
        out_name = wgsl_fn_name(fn["name"], fn_param_types(fn["params"]))
        text = re.sub(r"(\bfn\s+)" + re.escape(naga_name) + r"\b", r"\1" + out_name, text)
        outputs.append((fn["name"], text))
    return outputs, None


def _mx_calls_in_text(text):
    """Return mx_* names invoked in text (function bodies and preamble, not signatures)."""
    calls = set()
    fns = parse_functions(text)
    if fns:
        preamble = text[:text.index(fns[0]["full"])]
        calls |= {m.group(1) for m in re.finditer(r"\b(mx_\w+)\s*\(", preamble)}
        for fn in fns:
            body = fn["full"][fn["full"].index("{") + 1:fn["full"].rfind("}")]
            calls |= {m.group(1) for m in re.finditer(r"\b(mx_\w+)\s*\(", body)}
    else:
        calls |= {m.group(1) for m in re.finditer(r"\b(mx_\w+)\s*\(", text)}
    return calls


def proto_block_for_body(body_src, protos, defined_sigs):
    """Emit only prototypes for mx_* helpers referenced in body_src and not already defined."""
    called = _mx_calls_in_text(body_src)
    lines = []
    for (nm, t), p in sorted(protos.items()):
        if (nm, t) in defined_sigs:
            continue
        if nm in called:
            lines.append(p)
    return "\n".join(lines)


def sibling_callee_closure(target_fn, local_fns):
    """Local functions transitively called by target_fn (all overloads of each called name)."""
    by_name = {}
    for fn in local_fns:
        by_name.setdefault(fn["name"], []).append(fn)
    local_names = set(by_name.keys())

    def fn_key(fn):
        return (fn["name"], fn_param_types(fn["params"]))

    needed = set()
    stack = [target_fn]
    while stack:
        fn = stack.pop()
        key = fn_key(fn)
        if key in needed:
            continue
        needed.add(key)
        for callee in _extract_mx_calls(fn["full"], local_names):
            for callee_fn in by_name.get(callee, []):
                if fn_key(callee_fn) not in needed:
                    stack.append(callee_fn)

    closure = [fn for fn in local_fns if fn_key(fn) in needed]
    return sort_functions_topo(closure)


def file_preamble_before_fns(src):
    """Struct/const text before the first function definition in a lib fragment."""
    stripped = re.sub(r'#include\s+"[^"]+"\s*\n', "", src)
    fns = parse_functions(stripped)
    if not fns:
        return ""
    return stripped[:stripped.index(fns[0]["full"])]


def apply_lib_wgsl_patches(stem, text):
    """Stem-specific fixes for transpiled lib output (generator/runtime conventions)."""
    if stem == "mx_microfacet_specular":
        # Generator emits split env texture + sampler; match hand-port latlong signature.
        text = re.sub(
            r"fn mx_latlong_map_lookup\([^)]+\)[^{]*\{[^}]*\}",
            """fn mx_latlong_map_lookup(dir: vec3f, transform: mat4x4f, lod: f32, envTex: texture_2d<f32>, envSampler: sampler) -> vec3f {
    let envDir = normalize((transform * vec4f(dir, 0.0)).xyz);
    let uv = mx_latlong_projection(envDir);
    return textureSampleLevel(envTex, envSampler, uv, lod).rgb;
}""",
            text,
            count=1,
            flags=re.S,
        )
    return text


def transpile_lib_file_siblings(glsl_path, out_path, base, protos, lib_name, src, raw, header,
                                sampler_preamble, gllib_dir):
    """Per-function transpile with transitive in-file callee bodies (naga ordering fix)."""
    stem = glsl_path.stem
    stripped = re.sub(r'#include\s+"[^"]+"\s*\n', "", src)
    local_fns = parse_functions(stripped)
    if not local_fns:
        return False

    file_preamble = file_preamble_before_fns(src)
    inc_lines = "\n".join(
        f'#include "{i}"\n' for i in re.findall(r'#include\s+"([^"]+)"', raw) if i.startswith("lib/"))
    dep_src = expand_lib_includes(inc_lines, gllib_dir) if inc_lines else ""
    dep_src = re.sub(r'#include\s+"[^"]+"\s*\n', "", dep_src)
    for f in local_fns:
        dep_src = dep_src.replace(f["full"], "")

    dep_fns = parse_functions(dep_src) if dep_src else []
    dep_preamble = ""
    if dep_fns:
        dep_preamble = dep_src[:dep_src.index(dep_fns[0]["full"])]
    dep_names = {fn["name"] for fn in dep_fns}

    def fn_key(fn):
        return (fn["name"], fn_param_types(fn["params"]))

    outputs = []
    for target_fn in local_fns:
        types = fn_param_types(target_fn["params"])
        out_name = wgsl_fn_name(target_fn["name"], types)
        siblings = sibling_callee_closure(target_fn, local_fns)

        # Pull in dep-file helpers transitively called from the sibling closure.
        needed_dep = []
        needed_dep_keys = set()
        stack = list(siblings)
        while stack:
            fn = stack.pop()
            for callee in _extract_mx_calls(fn["full"], dep_names):
                for dep_fn in dep_fns:
                    if dep_fn["name"] == callee:
                        k = fn_key(dep_fn)
                        if k not in needed_dep_keys:
                            needed_dep_keys.add(k)
                            needed_dep.append(dep_fn)
                            stack.append(dep_fn)

        all_body_fns = sort_functions_topo(needed_dep + siblings)
        callees = [fn for fn in all_body_fns if fn_key(fn) != fn_key(target_fn)]
        callees = sort_functions_topo(callees)
        ordered = callees + [target_fn]
        sibling_body = rebuild_fn_body(ordered)
        sibling_sigs = {fn_key(fn) for fn in ordered}
        defined_sigs = sibling_sigs
        combined = file_preamble + dep_preamble + sibling_body
        proto_block = proto_block_for_body(combined, protos, defined_sigs)
        glsl = ("#version 450\n" + LIB_PREAMBLE + sampler_preamble + base + "\n" +
                file_preamble + dep_preamble + "\n" + proto_block + "\n" + sibling_body +
                "\nlayout(location=0) out vec4 mtlx_o;\nvoid main() { mtlx_o = vec4(0.0); }\n")
        wgsl, err = naga_transpile_glsl(
            glsl, out_path.parent / f"_debug_lib_{stem}_{target_fn['name']}.frag"
            if os.environ.get("MTLX_DEBUG") else None)
        if wgsl is None:
            print(f"  FAIL lib/{glsl_path.name}::{target_fn['name']}: {err}")
            return False
        text = extract_transpiled_fn(wgsl, target_fn["name"], out_name, glsl_types=types)
        if text is None:
            print(f"  FAIL lib/{glsl_path.name}::{target_fn['name']}: not found in naga output")
            return False
        # Overloaded calls: resolve by naga's actual per-module signatures (its stub numbering here
        # is topological, not the sorted order remap_calls assumes), then run the shared cleanup.
        text, unsupported = remap_calls_by_naga_sig(text, wgsl, target_fn["name"])
        if unsupported:
            print(f"  FAIL lib/{glsl_path.name}::{target_fn['name']}: unmapped call(s) {unsupported}")
            return False
        text, _ = postprocess_wgsl_fn(text, target_fn["name"], protos, remap=False)
        outputs.append(text)

    structs = transpile_glsl_structs(stripped)
    consts = transpile_glsl_consts(stripped)
    preamble = ("\n\n".join(structs + consts) + "\n\n") if structs or consts else ""
    src_rel = f"libraries/{lib_name}/genglsl/lib/{glsl_path.name}"
    banner = (f"// Generated from {src_rel} by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.\n"
              f"// Do not edit -- re-run the transpiler to regenerate "
              f"(see source/MaterialXGenWgsl/README.md).\n\n")
    all_consts = glsl_const_names(base) + const_names_from_wgsl(consts)
    body = resolve_const_refs("\n\n".join(outputs), all_consts)
    body = apply_lib_wgsl_patches(stem, body)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(banner + header + preamble + body + "\n", encoding="utf-8")
    print(f"  OK   lib/{glsl_path.name} -> {out_path}")
    return True


def transpile_lib_file(glsl_path, out_path, base, protos, lib_name):
    """Transpile one genglsl/lib/*.glsl file to genwgsl/lib/*.wgsl.

    Dispatches to _transpile_closure_type_lib, transpile_lib_file_siblings, or per-function
    transpile (default). Output: generated banner + #include lines + structs/consts + fn bodies."""
    raw = glsl_path.read_text(encoding="utf-8")
    src = expand_lib_tokens(raw)
    stem = glsl_path.stem
    sampler_preamble = TEXTURE_STUB_PREAMBLE if lib_needs_samplers(raw) else ""

    if stem == "mx_closure_type":
        return _transpile_closure_type_lib(glsl_path, out_path, base, protos, lib_name)

    includes = [f'#include "{inc.replace(".glsl", ".wgsl")}"'
                for inc in re.findall(r'#include\s+"([^"]+)"', raw)]
    header = ("\n".join(includes) + "\n\n") if includes else ""
    gllib_dir = glsl_path.parent

    if stem in LIB_USE_SIBLING_BODIES:
        return transpile_lib_file_siblings(glsl_path, out_path, base, protos, lib_name, src, raw,
                                           header, sampler_preamble, gllib_dir)

    local_fns = parse_functions(re.sub(r'#include\s+"[^"]+"\s*\n', "", src))
    if not local_fns:
        print(f"  SKIP lib/{glsl_path.name}: no functions found")
        return False

    local_sigs = {(fn["name"], fn_param_types(fn["params"])) for fn in local_fns}
    local_names = {fn["name"] for fn in local_fns}
    inc_lines = "\n".join(
        f'#include "{i}"\n' for i in re.findall(r'#include\s+"([^"]+)"', raw) if i.startswith("lib/"))
    dep_src = expand_lib_includes(inc_lines, gllib_dir) if inc_lines else ""
    dep_src = re.sub(r'#include\s+"[^"]+"\s*\n', "", dep_src)
    # Remove local function bodies from dep_src — only the target fn body is transpiled per pass.
    for f in local_fns:
        dep_src = dep_src.replace(f["full"], "")

    outputs = []
    for fn in local_fns:
        types = fn_param_types(fn["params"])
        out_name = wgsl_fn_name(fn["name"], types)
        proto_block = "\n".join(
            p for (nm, t), p in sorted(protos.items())
            if not (nm == fn["name"] and t == types))
        glsl = ("#version 450\n" + LIB_PREAMBLE + sampler_preamble + base + "\n" +
                dep_src + "\n" + proto_block + "\n" + fn["full"] +
                "\nlayout(location=0) out vec4 mtlx_o;\nvoid main() { mtlx_o = vec4(0.0); }\n")
        wgsl, err = naga_transpile_glsl(
            glsl, out_path.parent / f"_debug_lib_{stem}_{fn['name']}.frag"
            if os.environ.get("MTLX_DEBUG") else None)
        if wgsl is None:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: {err}")
            return False
        text = extract_transpiled_fn(wgsl, fn["name"], out_name)
        if text is None:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: not found in naga output")
            return False
        text, unsupported = postprocess_wgsl_fn(text, fn["name"], protos, remap=True)
        if unsupported:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: unmapped call(s) {unsupported}")
            return False
        outputs.append(text)

    structs = transpile_glsl_structs(re.sub(r'#include\s+"[^"]+"\s*\n', "", src))
    consts = transpile_glsl_consts(re.sub(r'#include\s+"[^"]+"\s*\n', "", src))
    preamble_parts = structs + consts
    preamble = ("\n\n".join(preamble_parts) + "\n\n") if preamble_parts else ""

    src_rel = f"libraries/{lib_name}/genglsl/lib/{glsl_path.name}"
    banner = (f"// Generated from {src_rel} by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.\n"
              f"// Do not edit -- re-run the transpiler to regenerate "
              f"(see source/MaterialXGenWgsl/README.md).\n\n")
    all_consts = glsl_const_names(base) + const_names_from_wgsl(consts)
    body = resolve_const_refs("\n\n".join(outputs), all_consts)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(banner + header + preamble + body + "\n", encoding="utf-8")
    print(f"  OK   lib/{glsl_path.name} -> {out_path}")
    return True


def _transpile_closure_type_lib(glsl_path, out_path, base, protos, lib_name):
    """Special handler for mx_closure_type: prepend shared structs, transpile makeClosureData.

    GLSL only defines ClosureData + constructor macro; genwgsl nodes expect BSDF/VDF/FresnelData
    types from this file. Struct block is static (LIB_STRUCT_PREAMBLE); functions come from naga."""
    src = expand_lib_tokens(glsl_path.read_text(encoding="utf-8"))
    outputs = []
    for fn in parse_functions(src):
        types = fn_param_types(fn["params"])
        out_name = wgsl_fn_name(fn["name"], types)
        proto_block = "\n".join(
            p for (nm, t), p in sorted(protos.items())
            if not (nm == fn["name"] and t == types))
        glsl = ("#version 450\n" + LIB_PREAMBLE + base + "\n" + proto_block +
                "\n" + fn["full"] +
                "\nlayout(location=0) out vec4 mtlx_o;\nvoid main() { mtlx_o = vec4(0.0); }\n")
        wgsl, err = naga_transpile_glsl(glsl)
        if wgsl is None:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: {err}")
            return False
        text = extract_transpiled_fn(wgsl, fn["name"], out_name)
        if text is None:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: not found in naga output")
            return False
        text, unsupported = postprocess_wgsl_fn(text, fn["name"], protos, remap=False)
        if unsupported:
            print(f"  FAIL lib/{glsl_path.name}::{fn['name']}: {unsupported}")
            return False
        outputs.append(apply_field_renames(text))

    src_rel = f"libraries/{lib_name}/genglsl/lib/{glsl_path.name}"
    banner = (f"// Generated from {src_rel} by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.\n"
              f"// Do not edit -- re-run the transpiler to regenerate "
              f"(see source/MaterialXGenWgsl/README.md).\n\n")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(banner + LIB_STRUCT_PREAMBLE.strip() + "\n\n" + "\n\n".join(outputs) + "\n",
                        encoding="utf-8")
    print(f"  OK   lib/{glsl_path.name} -> {out_path}")
    return True


def transpile_libs(libroot, outroot, libs, base, protos, only=None):
    """Transpile all genglsl/lib/*.glsl helpers to genwgsl/lib/*.wgsl.

    Runs before node transpilation in main(). Returns a list of stems that failed.
    Uses build_context(nodes=False) for lib-only prototypes."""
    ok = failed = 0
    unexpected = []
    # Lib-only context: no node protos (inout) and no duplicate closure preamble.
    lib_base, lib_protos, _ = build_context(libroot, libs, nodes=False)
    print("\nTranspiling genglsl/lib/ helpers...")
    for lib in libs:
        if lib in HANDWRITTEN_LIB_DIRS:
            print(f"  KEEP {lib}/genwgsl/lib (hand-written by project choice; not transpiled)")
            continue
        gllib = libroot / lib / "genglsl" / "lib"
        if not gllib.is_dir():
            continue
        lib_files = topo_sort_lib_files(sorted(gllib.glob("*.glsl")))
        for glsl in lib_files:
            if glsl.stem in LIB_KEEP_HANDWRITTEN:
                print(f"  KEEP lib/{glsl.name} (hand-written; naga cannot express its sampler bindings)")
                continue
            if only and glsl.stem not in only:
                continue
            out = outroot / lib / "genwgsl" / "lib" / (glsl.stem + ".wgsl")
            if transpile_lib_file(glsl, out, lib_base, lib_protos, lib):
                ok += 1
            else:
                failed += 1
                unexpected.append(glsl.stem)
    print(f"Lib done: {ok} transpiled, {failed} failures.")
    return unexpected


# ---------------------------------------------------------------------------- node driver

def transpile(node_path, out_path, base, protos, lib_symbols):
    node_src = node_path.read_text(encoding="utf-8")
    node_fns = parse_functions(node_src)
    if not node_fns:
        print(f"  SKIP {node_path.name}: no functions found")
        return False

    all_names = {nm for (nm, _t) in protos}

    outputs = []
    for fn in node_fns:
        # Supply every known prototype except the function being defined here (siblings and
        # cross-node helpers are all in `protos`, keyed by name+types).
        # Emit sorted by (name, types) so each overloaded base's prototypes are declared in the
        # same order remap_calls indexes them (naga numbers overload stubs by declaration order).
        proto_block = "\n".join(p for (nm, _t), p in sorted(protos.items()) if nm != fn["name"])
        body = fn["full"]

        # MaterialX `$`-tokens (e.g. $blur, $albedoTable) aren't valid GLSL identifiers, so naga
        # can't parse them. Swap each `$tok` for a legal placeholder `MTLXTOK_tok` before transpiling
        # and restore it afterwards; token_map records the reverse mapping for this function.
        token_map = {}

        def sentinel(m):
            s = "MTLXTOK_" + m.group(1)
            token_map[s] = m.group(0)
            return s
        body = re.sub(r"\$([A-Za-z_]\w*)", sentinel, body)

        # Assemble a complete, naga-parseable fragment shader: version + shared context (structs,
        # #defines, prototypes) + this one real function body + a do-nothing entry point. naga's GLSL
        # frontend requires a staged shader with a main(); it keeps non-entry functions in the output,
        # which is exactly the one body we want back.
        glsl = ("#version 450\n" + base + "\n" + proto_block + "\n" + body +
                "\nlayout(location=0) out vec4 mtlx_o;\nvoid main() { mtlx_o = vec4(0.0); }\n")

        with tempfile.TemporaryDirectory() as td:
            frag = Path(td) / "in.frag"
            wtmp = Path(td) / "out.wgsl"
            frag.write_text(glsl, encoding="utf-8")
            r = subprocess.run([NAGA, "--input-kind", "glsl", "--shader-stage", "frag",
                                str(frag), str(wtmp)], capture_output=True, text=True)
            if r.returncode != 0:
                err = next((l for l in r.stderr.splitlines() if "error" in l.lower()), "naga error")
                print(f"  FAIL {node_path.name}::{fn['name']}: {err.strip()}")
                if os.environ.get("MTLX_DEBUG"):
                    dbg = out_path.parent / f"_debug_{fn['name']}.frag"
                    dbg.parent.mkdir(parents=True, exist_ok=True)
                    dbg.write_text(glsl, encoding="utf-8")
                    print(f"       wrote {dbg}")
                return False
            wgsl = wtmp.read_text(encoding="utf-8")

        # Pull our one real function back out of naga's module. The context prototypes become empty
        # stub functions in the output, so match by source name (fn_base strips any naga `_N` suffix)
        # AND require a non-trivial body: `"{\n}"` excludes the empty stubs and the `> 1` line count
        # guards against a one-liner stub slipping through. Then rename the suffixed fn back to clean.
        text = None
        for name, t in top_level_fns(wgsl):
            if fn_base(name) == fn["name"] and "{\n}" not in t and t.count("\n") > 1:
                text = re.sub(r"(\bfn\s+)" + re.escape(name) + r"\b", r"\1" + fn["name"], t)
                break
        if text is None:
            print(f"  FAIL {node_path.name}::{fn['name']}: not found in naga output")
            return False

        text = cleanup_function(text)
        for rx, repl in TYPE_FIXUPS:
            text = rx.sub(repl, text)
        # Float literal suffixes: drop `f` from decimals first (so the int rule can't bite a
        # decimal's fractional digits, e.g. 0.00000001f -> 0.00000001), then pad bare ints (2f -> 2.0).
        text = re.sub(r"\b(\d+\.\d+(?:[eE][-+]?\d+)?)f\b", r"\1", text)
        text = re.sub(r"(?<![\w.])(\d+)f\b", r"\1.0", text)
        # naga suffixes calls to overloaded/colliding functions (mx_foo_); restore known names.
        for nm in all_names:
            text = re.sub(r"\b" + re.escape(nm) + r"_\b", nm, text)
        for s, tok in token_map.items():
            text = text.replace(s, tok)

        # Rewrite overloaded/diverged helper calls to their genwgsl names. Anything the table marks
        # unsupported (adapted signatures, e.g. the FresnelData BSDF helpers) fails the node so it
        # falls back to the hand-written .wgsl -- yielding a reduced (generated + hand-written) lib.
        text, unsupported = remap_calls(text, fn["name"], protos)
        # Also reject calls whose arity disagrees with the real genwgsl lib (adapted signatures).
        unsupported += check_lib_arity(text, lib_symbols)
        if unsupported:
            print(f"  FAIL {node_path.name}::{fn['name']}: calls adapted/unmapped helper(s) "
                  f"{unsupported} (kept hand-written)")
            return False

        outputs.append(text)

    # Re-emit the node's own #include lines with the .glsl->.wgsl extension swap, so the generated
    # fragment pulls in the same lib helpers (now their WGSL versions) the GLSL source did. A node
    # file may define several functions (e.g. a vector2 + float variant); join them in source order.
    includes = [f'#include "{inc.replace(".glsl", ".wgsl")}"'
                for inc in re.findall(r'#include\s+"([^"]+)"', node_src)]
    header = ("\n".join(includes) + "\n\n") if includes else ""

    # Banner marking the file as generated, so it is visually distinct from the hand-written
    # genwgsl nodes it sits next to. Names the genglsl source (lib-relative, so the line is stable
    # regardless of where --libraries points) and the tool. No timestamp: re-running must produce
    # byte-identical output so `git diff` cleanly surfaces drift.
    src_rel = f"libraries/{node_path.parent.parent.name}/genglsl/{node_path.name}"
    banner = (f"// Generated from {src_rel} by source/MaterialXGenWgsl/tools/glsl_to_wgsl.py.\n"
              f"// Do not edit -- re-run the transpiler to regenerate "
              f"(see source/MaterialXGenWgsl/README.md).\n\n")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(banner + header + "\n\n".join(outputs) + "\n", encoding="utf-8")
    print(f"  OK   {node_path.name} -> {out_path}")
    return True


def main():
    ap = argparse.ArgumentParser(description="Transpile genglsl node fragments to genwgsl.")
    ap.add_argument("--libraries", default="libraries")
    ap.add_argument("--out", default="build/genwgsl_generated")
    ap.add_argument("--only", nargs="*")
    ap.add_argument("--naga", help="Path to the naga CLI (overrides the NAGA env var / PATH lookup).")
    args = ap.parse_args()

    # Resolve and preflight naga so a missing tool fails once, with guidance, instead of per node.
    global NAGA
    NAGA = resolve_naga(args.naga)
    version = naga_version(NAGA)
    if not version:
        print(f"ERROR: naga CLI not found or not runnable (tried '{NAGA}').\n"
              "       Install it with `cargo install naga-cli`, or pass --naga <path> / set the\n"
              "       NAGA environment variable. See https://github.com/gfx-rs/wgpu/tree/trunk/naga.",
              file=sys.stderr)
        return 2
    print(f"Using naga: {NAGA} ({version})")

    libroot, outroot = Path(args.libraries), Path(args.out)
    libs = discover_libs(libroot)
    base, protos, _overloaded = build_context(libroot, libs)

    # Lib helpers first: nodes #include genwgsl/lib/*.wgsl and check_lib_arity reads fresh output.
    lib_unexpected = transpile_libs(libroot, outroot, libs, base, protos, only=args.only)
    lib_symbols = build_wgsl_lib_symbols(libroot, libs, outroot=outroot)

    ok = skip = 0
    node_expected, node_unexpected = [], []
    print("\nTranspiling genglsl node fragments...")
    for lib in libs:
        gldir = libroot / lib / "genglsl"
        if not gldir.is_dir():
            continue
        for glsl in sorted(gldir.glob("mx_*.glsl")):
            base_name = glsl.stem
            if args.only and base_name not in args.only:
                continue
            if any(p.search(base_name) for p in SKIP_PATTERNS):
                print(f"  SKIP {glsl.name}: texture/sampler node (unsupported by naga)")
                skip += 1
                continue
            out = outroot / lib / "genwgsl" / (base_name + ".wgsl")
            if transpile(glsl, out, base, protos, lib_symbols):
                ok += 1
                if base_name in EXPECTED_FALLBACK:
                    # A node we expected to stay hand-written now transpiles cleanly -- the lib
                    # divergence it depended on was probably resolved. Not fatal, but worth flagging.
                    print(f"  WARN {glsl.name}: now transpiles cleanly; remove it from "
                          f"EXPECTED_FALLBACK and commit the generated version.")
            elif base_name in EXPECTED_FALLBACK:
                node_expected.append(base_name)
            else:
                node_unexpected.append(base_name)

    print(f"\nDone: {ok} nodes transpiled, {skip} skipped, "
          f"{len(node_expected)} expected fallbacks, {len(node_unexpected)} unexpected failures.")
    all_unexpected = lib_unexpected + node_unexpected
    if all_unexpected:
        print("ERROR: files that should transpile failed: " + ", ".join(sorted(set(all_unexpected))))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
