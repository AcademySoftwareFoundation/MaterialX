#!/usr/bin/env python3
#
# Copyright Contributors to the MaterialX Project
# SPDX-License-Identifier: Apache-2.0
#
"""
Transpile MaterialX genglsl *shader-node* library fragments to genwgsl (WGSL).

The genglsl node fragments (e.g. libraries/pbrlib/genglsl/mx_dielectric_bsdf.glsl) are not
complete shaders: they use `#include`, `$`-tokens, and define functions with no entry point.
This tool turns each node function into a complete GLSL translation unit, runs `naga`
(GLSL -> WGSL), then strips scaffolding and cleans up naga's SSA-style output into a readable
genwgsl fragment that mirrors the hand-written convention.

Strategy (key to readable output): transpile ONE node function at a time, supplying the
included lib context as *struct definitions + function prototypes + #defines* rather than full
bodies. With a single real function body in the module, naga keeps clean parameter names and we
avoid both name-collision suffixes and `$`-tokens that live inside lib bodies.

Re-running keeps genwgsl in sync with genglsl; diffing the output against committed files
surfaces drift.

Scope: standalone node `.glsl` files referenced by `file=` impls in stdlib/pbrlib/lights.
Out of scope: core `lib/` helpers (hand-maintained) and image/texture nodes (naga's GLSL
frontend has no sampler support) -- the latter are auto-skipped.

Usage:
  python glsl_to_wgsl.py --libraries libraries --out build/genwgsl_generated
  python glsl_to_wgsl.py --libraries libraries --only mx_conductor_bsdf mx_normalmap
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

# Nodes that are intentionally NOT generated and remain hand-written, by file stem. These call lib
# helpers the genwgsl lib hand-adapted (FresnelData-taking ggx helpers, mx_subsurface_scattering_approx's
# extra `curvature` param) or hit a naga limitation (chiang hair). They are EXPECTED to fail transpile,
# so they don't constitute a build error -- only an *unexpected* failure (a previously-generable node
# that broke, e.g. after a genglsl change) sets a non-zero exit. Keep this in sync with the committed
# hand-written .wgsl files: if one starts transpiling cleanly, the tool warns so it can be removed here.
EXPECTED_FALLBACK = {
    "mx_chiang_hair_bsdf",
    "mx_conductor_bsdf",
    "mx_dielectric_bsdf",
    "mx_generalized_schlick_bsdf",
    "mx_subsurface_bsdf",
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

    ("mx_matrix_mul", ("vec2", "mat2")): "mx_matrix_mul_vec2_mat2",
    ("mx_matrix_mul", ("vec3", "mat3")): "mx_matrix_mul_vec3_mat3",
    ("mx_matrix_mul", ("vec4", "mat4")): "mx_matrix_mul_vec4_mat4",
    ("mx_matrix_mul", ("mat2", "vec2")): "mx_matrix_mul_mat2_vec2",
    ("mx_matrix_mul", ("mat3", "vec3")): "mx_matrix_mul_mat3_vec3",
    ("mx_matrix_mul", ("mat4", "vec4")): "mx_matrix_mul_mat4_vec4",
    ("mx_matrix_mul", ("mat2", "mat2")): "mx_matrix_mul_mat2_mat2",
    ("mx_matrix_mul", ("mat3", "mat3")): "mx_matrix_mul_mat3_mat3",
    ("mx_matrix_mul", ("mat4", "mat4")): "mx_matrix_mul_mat4_mat4",

    # vec3/vec3-F90 directional albedo maps cleanly; the float and FresnelData overloads were
    # adapted in the genwgsl lib (caller precomputes), so nodes using them stay hand-written.
    ("mx_ggx_dir_albedo", ("float", "float", "vec3", "vec3")): "mx_ggx_dir_albedo",
    ("mx_ggx_dir_albedo", ("float", "float", "float", "float")): None,
    ("mx_ggx_dir_albedo", ("float", "float", "FresnelData")): None,
    # Single GLSL signature, but the genwgsl lib takes a precomputed Fss (vec3f), not FresnelData.
    ("mx_ggx_energy_compensation", ("float", "float", "FresnelData")): None,
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


def build_context(libroot, libs):
    """Build the naga parse context. Returns (base, protos, overloaded):
      base       - CLOSURE_PREAMBLE + dedup'd #defines/consts/struct-defs from every genglsl lib.
      protos     - {(name, types): "prototype;"} for every function defined in any genglsl lib OR
                   node file, so helpers AND cross-node calls (e.g. mx_burn_color3 -> mx_burn_float)
                   and generator-supplied helpers (mx_environment_radiance) all resolve.
      overloaded - the set of names appearing with more than one signature (informational; the
                   actual overload remapping is driven by CALL_MAP in remap_calls).
    Per node we emit `base` + every prototype except the one being defined."""
    defines, consts, structs, protos = {}, {}, {}, {}
    # Scan lib/ helpers and the node files themselves (the latter for cross-node prototypes).
    sources = []
    for lib in libs:
        gldir = libroot / lib / "genglsl"
        if gldir.is_dir():
            sources += sorted((gldir / "lib").glob("*.glsl"))
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
            # Dedupe by name + parameter *types* (GLSL overload identity, ignoring the
            # `const`/`in` qualifiers naga also ignores), so the same helper defined in
            # several lib files collapses to one prototype while real overloads survive.
            types = []
            for p in fn["params"].split(","):
                p = re.sub(r"^\s*(const|in)\s+", "", p.strip())  # drop leading qualifier
                if p:
                    # Strip the parameter NAME (and any `[N]` array suffix) off the end, leaving the
                    # bare type; collapse internal whitespace. e.g. "vec3 F0" -> "vec3".
                    types.append(re.sub(r"\s+", " ", re.sub(r"\s+[A-Za-z_]\w*\s*(\[\d*\])?$", "", p)).strip())
            protos.setdefault((fn["name"], tuple(types)), f"{fn['ret']} {fn['name']}({fn['params']});")
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


def build_wgsl_lib_symbols(libroot, libs):
    """Parse the hand-written genwgsl `lib/` files into {function name: parameter count}.

    The genwgsl lib has been hand-adapted in places that diverge from genglsl beyond overloading
    (e.g. mx_subsurface_scattering_approx grew a `curvature` parameter, mx_ggx_energy_compensation
    takes a precomputed Fss). A transpiled node faithfully follows the GLSL call, so we validate
    every helper call against the real lib arity and fall the node back to hand-written on a
    mismatch -- this auto-detects such divergences instead of discovering them at naga link time."""
    symbols = {}
    for lib in libs:
        wdir = libroot / lib / "genwgsl" / "lib"
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


def top_level_fns(wgsl):
    """Yield (name, text) for each top-level `fn` in a WGSL module."""
    for m in re.finditer(r"\bfn\s+([A-Za-z_]\w*)\s*\(", wgsl):
        brace = wgsl.index("{", m.start())
        yield m.group(1), wgsl[m.start():_match_brace(wgsl, brace)]


# ---------------------------------------------------------------------------- driver

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
    # Build the two reference tables once, up front: the GLSL parse context (shared by every node)
    # and the genwgsl lib's real signatures (for the arity check). `_overloaded` is unused here --
    # remap_calls drives off CALL_MAP -- but build_context returns it for callers that want it.
    base, protos, _overloaded = build_context(libroot, libs)
    lib_symbols = build_wgsl_lib_symbols(libroot, libs)
    ok = skip = 0
    expected, unexpected = [], []   # node stems that failed transpile, by category
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
                expected.append(base_name)   # known hand-written fallback -- not a build error
            else:
                unexpected.append(base_name)  # regression: should have generated, but didn't

    print(f"\nDone: {ok} transpiled, {skip} skipped, "
          f"{len(expected)} expected fallbacks, {len(unexpected)} unexpected failures.")
    if unexpected:
        # Surface as an error so a CMake build step fails (see MATERIALX_GENERATE_WGSL_LIBRARY).
        print("ERROR: nodes that should transpile failed: " + ", ".join(sorted(unexpected)))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
