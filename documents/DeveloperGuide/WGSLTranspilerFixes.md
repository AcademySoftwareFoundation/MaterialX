# WGSL Transpiler: Full-Library Generation Fixes

This note summarizes the changes that make `mxgenwgsl.py` generate the **entire** `genwgsl` node
and lib library from `genglsl` (previously many files were hand-written). It covers (1) the
generation policy — what is generated vs. kept hand-written — and (2) the transpiler correctness
fixes needed to reconcile naga's WGSL output with the genwgsl library conventions.

For the general workflow and tooling see [WGSLShaderGeneration.md](WGSLShaderGeneration.md) and
[`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).

## Generation policy

| Scope | Handling | Mechanism |
| --- | --- | --- |
| `pbrlib/genwgsl/` nodes + most `pbrlib/genwgsl/lib` helpers | **Generated** | transpiled from `genglsl` |
| `stdlib/genwgsl/lib` helpers | **Hand-written** (project choice) | `HANDWRITTEN_LIB_DIRS` |
| Environment / shadow / bake lib helpers | **Hand-written** | `LIB_KEEP_HANDWRITTEN` |
| Image / hextile / light nodes | **Hand-written** (auto-skipped) | `SKIP_PATTERNS` |
| `mx_chiang_hair_bsdf` | **Hand-written** | `EXPECTED_FALLBACK` |

**Why some lib helpers stay hand-written.** The environment (`mx_environment_fis/none/prefilter`),
shadow (`mx_shadow`, `mx_shadow_platform`), and bake (`mx_generate_albedo_table`,
`mx_generate_prefilter_env`) helpers sample textures through generator-substituted `$`-token
**texture + sampler** bindings. naga's GLSL frontend cannot parse sampler types, so the transpiler
can only stub those calls — the stubs would leak into the output as unresolved references. These
files are therefore listed in `LIB_KEEP_HANDWRITTEN` and maintained by hand (the same limitation
already skips image/light *nodes*). `mx_environment_prefilter.wgsl` and
`mx_generate_prefilter_env.wgsl` were hand-authored to match the `mx_environment_fis.wgsl`
conventions.

The four BSDF nodes `mx_conductor/dielectric/generalized_schlick/subsurface_bsdf` now transpile
cleanly and are generated; only `mx_chiang_hair_bsdf` remains an `EXPECTED_FALLBACK`.

## Transpiler correctness fixes

naga produces valid but SSA-style WGSL and deterministically rewrites identifiers. The following
post-processing was added/fixed so the generated output matches the genwgsl library and the
`WgslShaderGenerator` runtime template. All changes are in
`source/MaterialXGenWgsl/tools/mxgenwgsl.py`.

| Area | Symptom before fix | Fix |
| --- | --- | --- |
| **Struct parsing** (`transpile_glsl_structs`) | GLSL comments inside a struct (e.g. `// Fresnel model`) were parsed as fields (`Fresnel: //,`) and array fields (`vec2 c[3]`) became `c[3]: vec2f` | Strip line/block comments; rewrite array fields to `array<T, N>`; skip types provided centrally by `wgsl_closure_preamble()` (`LIB_PREAMBLE_STRUCTS`) so `FresnelData`/`ClosureData` aren't redeclared |
| **`surfaceshader`** | Redeclared — emitted both by `WgslShaderGenerator::emitTypeDefinitions` and by the lib preamble | `surfaceshader` is parsed from `GlslSyntax.cpp` for node context only; the WGSL lib preamble omits it (the generator owns it; the `material = surfaceshader` alias still resolves) |
| **File-scope consts** (`transpile_glsl_consts`) | Function-local `const int SAMPLE_COUNT` was hoisted to module scope → duplicate / redeclared consts across microfacet libs | Blank out function bodies before matching, so only true file-scope consts are emitted |
| **naga identifier reconciliation** (`denaga_name`) | naga suffixes references to a digit-ending const (`FUJII_CONSTANT_1_`), a name it sees twice (`FRESNEL_MODEL_SCHLICK_1`), the digit-ending fields `F0`/`F82`/`F90` (`F0_`), and prototype names — leaving them mismatched | One Namer-grounded helper (`proc/namer.rs`) maps `NAME_` / `NAME_<N>` references back to the clean declaration name; used by `resolve_const_refs`, `apply_field_renames` (digit fields from the derived preamble), and the prototype-underscore strip. Keyed only on known names, so numbered locals are untouched |
| **`FresnelData` fields** | GLSL used `tf_*` while the hand-ported WGSL lib used `thinfilm_*`, requiring a transpiler rename | Renamed `tf_*` → `thinfilm_*` in `mx_microfacet_specular.glsl` so GLSL and emitted WGSL share one field layout; removed `LIB_FIELD_RENAMES` |
| **Overloaded helpers** (`mangle`) | WGSL has no overloading, so `mx_bilerp`, `mx_hash_int`, `mx_cell_noise_vec3`, `mx_fresnel_schlick`, etc. collided | `mangle(name, types, overloaded)` derives a type-suffixed name from a per-family `SUFFIX_SCHEME` (default: first parameter type) plus a small `EXCEPTIONS` table for irregular/semantic names; the overload *keys* come from scanning genglsl, not a hand-listed table |
| **Overload resolution** (`extract_transpiled_fn`, `remap_calls_by_naga_sig`) | For overloaded siblings (`mx_ggx_dir_albedo` vec3/scalar/FresnelData) the wrong body was extracted and calls resolved to the wrong overload | Extract by parameter **signature** when a name is ambiguous; remap calls using the **actual per-module signatures** naga assigned, instead of assuming a sorted index order |
| **`out`/`inout` params** (`naga_proto_params`) | Stripping `out`/`inout` from prototypes made callers pass a dereferenced value (`mx_normalmap_vector2(..., (*result))`) instead of the pointer | Keep `out`/`inout` in prototypes (only strip the neutral `const`/`in`) so naga passes `ptr<function, T>` |
| **Comment preservation** (`parse_functions`, `inject_inline_comments`) | naga discards all GLSL comments, so doc comments and references were lost | Re-attach from genglsl: function `lead` (doc) blocks and file-level top comments verbatim; inline `//` body comments best-effort via stable-token anchors (whole-line insertion, unmatched dropped, so output is never corrupted). Struct/const comments stay stripped |
| **Scan-driven validation** (`validate_overload_coverage` / `_token_coverage` / `_lib_names`) | A new genglsl overload or `$`-token could silently mis-emit a name or fail naga with an obscure error | Preflight hard-errors when an overload has no `mangle()` mapping, a `$`-token in a transpiled lib source is undeclared in `HwConstants.cpp` and unstubbed in `NAGA_LIB_STUBS`, or a `mangle()` name is absent from the real genwgsl lib |
| **Derived closure preambles** (`wgsl_closure_preamble` / `glsl_closure_preamble`) | `CLOSURE_PREAMBLE`, `LIB_STRUCT_PREAMBLE`, and `LIB_TOKEN_FIXUPS` duplicated C++/GLSL sources | Emit closure structs from `genglsl/lib` + `GlslSyntax.cpp`; validate token names from `HwConstants.cpp`; keep only naga-specific stub values in `NAGA_LIB_STUBS` |

### The recurring theme

naga is deterministic but rewrites identifiers to keep its output valid: its `Namer`
(`proc/namer.rs`) appends `_` to any name ending in a digit or a keyword/builtin, `_N` to colliding
names, and renders `out`/`inout` params as `ptr<function, T>`. Wherever the transpiler re-emits a
symbol itself (a hand-written struct/const, or a prototype), the generated references must be
reconciled back to that symbol — now via the single Namer-grounded `denaga_name()` helper. The fixes
above are all instances of that reconciliation.

## Build / repo workflow

Generated `.wgsl` files carry a `// Generated from … mxgenwgsl.py` banner and are **not
committed** — they are regenerated from `genglsl` (the single source of truth) in CI and locally.
Because `MATERIALX_BUILD_GEN_WGSL` now defaults **OFF**, a clean local build of the web viewer is:

```sh
# 1. regenerate the WGSL library (genglsl -> genwgsl)
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries

# 2. configure with the WGSL backend enabled
cmake -S . -B javascript/build \
  -DMATERIALX_BUILD_JS=ON -DMATERIALX_BUILD_GEN_WGSL=ON \
  -DMATERIALX_EMSDK_PATH=<path-to-emsdk> -G Ninja

# 3. build the WASM modules (libraries are embedded into JsMaterialXGenShader.data)
cmake --build javascript/build --target JsMaterialXCore JsMaterialXGenShader

# 4. run the viewer
cd javascript/MaterialXView && npm install && npm run start   # http://localhost:8080
```

Re-running the transpiler must produce byte-identical output, so a `git diff` after regeneration
cleanly surfaces any GLSL/WGSL drift.
