# WGSL Transpiler: Full-Library Generation Fixes

This note summarizes the changes that make `glsl_to_wgsl.py` generate the **entire** `genwgsl` node
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
`source/MaterialXGenWgsl/tools/glsl_to_wgsl.py`.

| Area | Symptom before fix | Fix |
| --- | --- | --- |
| **Struct parsing** (`transpile_glsl_structs`) | GLSL comments inside a struct (e.g. `// Fresnel model`) were parsed as fields (`Fresnel: //,`) and array fields (`vec2 c[3]`) became `c[3]: vec2f` | Strip line/block comments; rewrite array fields to `array<T, N>`; skip types provided centrally by `LIB_STRUCT_PREAMBLE` (`LIB_PREAMBLE_STRUCTS`) so `FresnelData`/`ClosureData` aren't redeclared |
| **`surfaceshader`** | Redeclared — emitted both by `WgslShaderGenerator::emitTypeDefinitions` and by the lib preamble | Removed `surfaceshader` from `LIB_STRUCT_PREAMBLE` (the generator owns it; the `material = surfaceshader` alias still resolves) |
| **File-scope consts** (`transpile_glsl_consts`) | Function-local `const int SAMPLE_COUNT` was hoisted to module scope → duplicate / redeclared consts across microfacet libs | Blank out function bodies before matching, so only true file-scope consts are emitted |
| **Const references** (`resolve_const_refs`) | naga suffixes references to a digit-ending const (`FUJII_CONSTANT_1_`) or a name it sees twice in its input (`FRESNEL_MODEL_SCHLICK_1`), leaving them unresolved | Rewrite `NAME_` / `NAME_<N>` references back to the declared const name (keyed only on known const names, so numbered locals are untouched) |
| **`FresnelData` fields** (`LIB_FIELD_RENAMES`) | naga renders digit-ending fields `F0`/`F82`/`F90` as `F0_`/`F82_`/`F90_`, mismatching the struct | Strip the trailing `_` from those field accesses |
| **`mx_noise` overloads** (`CALL_MAP`) | WGSL has no overloading, so `mx_bilerp`, `mx_hash_int`, `mx_cell_noise_vec3`, etc. collided | Added type-suffixed `CALL_MAP` entries (e.g. `mx_hash_int_i2`, `mx_cell_noise_vec3_vec2`) matching the hand-written naming |
| **Overload resolution** (`extract_transpiled_fn`, `remap_calls_by_naga_sig`) | For overloaded siblings (`mx_ggx_dir_albedo` vec3/scalar/FresnelData) the wrong body was extracted and calls resolved to the wrong overload | Extract by parameter **signature** when a name is ambiguous; remap calls using the **actual per-module signatures** naga assigned, instead of assuming a sorted index order |
| **`out`/`inout` params** (`naga_proto_params`) | Stripping `out`/`inout` from prototypes made callers pass a dereferenced value (`mx_normalmap_vector2(..., (*result))`) instead of the pointer | Keep `out`/`inout` in prototypes (only strip the neutral `const`/`in`) so naga passes `ptr<function, T>` |

### The recurring theme

naga is deterministic but rewrites identifiers to keep its output valid: it appends `_` to any name
ending in a digit, `_N` to colliding names, and renders `out`/`inout` params as `ptr<function, T>`.
Wherever the transpiler re-emits a symbol itself (a hand-written struct/const, or a prototype), the
generated references must be reconciled back to that symbol. The fixes above are all instances of
that reconciliation.

## Build / repo workflow

Generated `.wgsl` files carry a `// Generated from … glsl_to_wgsl.py` banner and are **not
committed** — they are regenerated from `genglsl` (the single source of truth) in CI and locally.
Because `MATERIALX_BUILD_GEN_WGSL` now defaults **OFF**, a clean local build of the web viewer is:

```sh
# 1. regenerate the WGSL library (genglsl -> genwgsl)
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries

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
