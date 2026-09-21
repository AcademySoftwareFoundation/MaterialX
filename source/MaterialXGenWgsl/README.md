# MaterialXGenWgsl

A native [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator back-end for
MaterialX, registered under the `genwgsl` target. `WgslShaderGenerator` derives directly from
`HwShaderGenerator` (not the GLSL hierarchy) and emits standalone WGSL vertex + fragment shaders,
mirroring the structure of the `MaterialXGenMsl` / `MaterialXGenSlang` back-ends. It is gated behind
the `MATERIALX_BUILD_GEN_WGSL` CMake option (**OFF by default**: its node library is transpiled from
genglsl and is not committed, so enabling the target requires that library to be generated first —
see below).

## Node library

Every other shader-gen back-end ships a fully hand-written node library. `genwgsl` does **not** —
its node library is a *hybrid*:

* **Most node `.wgsl` files are machine-generated** from their `genglsl` originals by the offline
  transpiler in [`tools/`](tools/README.md). They are **not committed** — they are a derived build
  artifact, transpiled from genglsl (the single source of truth) by CI, so drift between the GLSL and
  WGSL libraries is impossible by construction. Generated files carry a `// @mxgenwgsl …` marker
  that identifies machine-generated files so developers know not to hand-edit them; it is not stripped
  at runtime.
* **`lib/` helper files are machine-generated** from `genglsl/lib/` by the same transpiler (run
  before node transpilation). All 22 `genglsl/lib` files transpile via naga (including prefilter
  environment helpers). They are also **not committed**.
* **Only light nodes are hand-written** — they use the dynamically generated `LightData` struct
  that the fragment context cannot supply to the transpiler.
  See [`skip_transpile.txt`](tools/skip_transpile.txt) for the full list.

The library lives in `libraries/{stdlib,pbrlib,lights}/genwgsl/` with the target defined in
`libraries/targets/genwgsl.mtlx`; node implementations are wired up via `*_genwgsl_impl.mtlx`.

## Generating the library

Because the generated node files are not committed, you must produce them before building the WGSL
target. CI installs naga and the transpiler's Python dependencies, then runs generation through
CMake (`MaterialXGenWgslLibrary`) on jobs that build or ship WGSL. Locally, either configure with
`-DMATERIALX_BUILD_GEN_WGSL=ON` and build (CMake transpiles in-place automatically), or populate the
library manually:

```
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries
```

The transpiler exits non-zero on any lib or node failure outside `skip_transpile.txt` — so CI running
it doubles as validation that a change hasn't broken the WGSL target. See
[`tools/README.md`](tools/README.md) for the tool, its overload mapping table, and its lib-arity
self-validation.

The separate `-DMATERIALX_GENERATE_WGSL_LIBRARY=ON` option (without `MATERIALX_BUILD_GEN_WGSL`) adds
a `MaterialXGenWgslLibrary` target that re-transpiles into the *build tree* as a local validation
aid.

## Binding model

`WgslResourceBindingContext` emits resource bindings in the generated WGSL:

* **Textures and samplers** are individual `@group/@binding` declarations (one `texture_2d<f32>` +
  one `sampler` per FILENAME uniform).
* **Value uniforms** (scalars, vectors, matrices) are packed into a **single struct UBO** per
  uniform block (`PublicUniforms`, `PrivateUniforms`). This keeps the total binding count within
  WebGPU's default `maxUniformBuffersPerShaderStage` limit of 12. `const` alias lines let the
  shader body reference uniforms by their bare names (e.g. `base_color` instead of
  `u_pub.base_color`).
* **Light data** is bound as a `var<storage, read>` structured array (the `uniform` address space
  requires 16-byte-aligned strides, which small structs like `LightData` don't satisfy).

## Texture calls

The transpiler preserves texture LOD and gradient arguments through the naga round-trip using
distinct placeholder functions (`mtlx_tex_lookup_level_*` for `textureLod`,
`mtlx_tex_lookup_grad_*` for `textureGrad`). These are restored to their WGSL equivalents
(`textureSampleLevel`, `textureSampleGrad`) after transpilation, so LOD bias and explicit
gradient parameters are not lost.

## Layout

| Path | Contents |
| --- | --- |
| `WgslShaderGenerator.{h,cpp}` | The `genwgsl` shader generator (vertex + fragment WGSL). |
| `WgslSyntax.{h,cpp}` | WGSL type names and syntactic rules. |
| `WgslResourceBindingContext.{h,cpp}` | `@group`/`@binding` struct-packed uniforms, split texture/sampler, and structured light-data bindings. |
| `Nodes/` | C++ node implementations that emit WGSL dynamically. |
| `tools/` | The offline `genglsl`→`genwgsl` library transpiler (see its README). |
