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

The library lives in `libraries/{stdlib,pbrlib,lights,nprlib}/genwgsl/` with the target defined in
`libraries/targets/genwgsl.mtlx`; node implementations are wired up via `*_genwgsl_impl.mtlx`.
When OpenColorIO is enabled, pre-baked OCIO GPU functions live under `stdlib/genwgsl/ocio/`.

## Generating the library

Because the generated node files are not committed, you must produce them before building the WGSL
target. CI installs naga and the transpiler's Python dependencies, then runs generation through
CMake (`MaterialXGenWgslLibrary`) on jobs that build or ship WGSL. Locally, either configure with
`-DMATERIALX_BUILD_GEN_WGSL=ON` and build (CMake transpiles in-place automatically), or populate the
library manually:

```
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries
```

A full run also checks that every genglsl node and lib file has a genwgsl counterpart (generated,
or hand-written and listed in `skip_transpile.txt`), and fails otherwise.

If Python or naga is missing when you configure with `MATERIALX_BUILD_GEN_WGSL=ON`, CMake uses the
existing in-place library if it is complete by that same rule, with a warning that it can't be
verified as current; otherwise configure fails. With `MATERIALX_BUILD_OCIO` on, missing baked OCIO
fragments only produce a warning, since they come from a separate optional target.

The Python sdist ships the generated library, so wheel builds don't need naga. Wheel builds never
install naga through cargo; they use the shipped library as is.

The transpiler exits non-zero on any lib or node failure outside `skip_transpile.txt` — so CI running
it doubles as validation that a change hasn't broken the WGSL target. See
[`tools/README.md`](tools/README.md) for the tool, its overload mapping table, and its lib-arity
self-validation.

**Other tools** (same folder):

| Script | Role |
| --- | --- |
| [`mxvalidategenwgsl.py`](tools/mxvalidategenwgsl.py) | Full-shader naga validation + optional MaterialXView TSL parity |
| [`mxvalidategenwgsl_viewer.mjs`](tools/mxvalidategenwgsl_viewer.mjs) | Node helper used by the validator |
| [`mxstagepymaterialx.py`](tools/mxstagepymaterialx.py) | CMake-only staging for OCIO bake (`MaterialXGenWgslOcioLibrary`) |
| [`test_mxgenwgsl.py`](tools/test_mxgenwgsl.py) / [`test_mxwgsltidy.py`](tools/test_mxwgsltidy.py) | Fast offline tests (no naga required for most checks) |

Developer guide: [`documents/DeveloperGuide/WGSLShaderGeneration.md`](../../documents/DeveloperGuide/WGSLShaderGeneration.md).

The separate `-DMATERIALX_GENERATE_WGSL_LIBRARY=ON` option (without `MATERIALX_BUILD_GEN_WGSL`) adds
a `MaterialXGenWgslLibrary` target that re-transpiles into the *build tree* as a local validation
aid.

## Binding model

`WgslResourceBindingContext` emits resource bindings in the generated WGSL:

* **Textures and samplers** are individual `@group/@binding` declarations (one `texture_2d<f32>` +
  one `sampler` per FILENAME uniform).
* **Value uniforms** (scalars, vectors, matrices) are packed into a **single struct UBO** per
  uniform block (`PublicUniforms`, `PrivateUniforms`). This keeps the total binding count within
  WebGPU's default `maxUniformBuffersPerShaderStage` limit of 12. The shader body references
  members through the instance (e.g. `u_pub.base_color`); reflection reports bare names
  (`base_color`), as for other hardware targets. A block that contains an array uniform
  (`floatarray`, `integerarray`, ...) is bound as `var<storage, read>` instead of
  `var<uniform>`, because the `uniform` address space requires 16-byte array strides. Hosts must
  create a read-only storage buffer binding for such blocks.
* **Light data** is bound as a `var<storage, read>` structured array (the `uniform` address space
  requires 16-byte-aligned strides, which small structs like `LightData` don't satisfy).

## Shadow maps

`u_shadowMatrix` must follow WebGPU clip-space conventions, the same projection used to render
the shadow map with `hwWriteDepthMoments`: NDC Z in [0, 1], not OpenGL's [-1, 1].
`mx_shadow_occlusion` maps NDC XY to texture UV with V pointing down and compares NDC Z directly
against the stored depth moments, so a host must not reuse an OpenGL-convention shadow matrix
from its GLSL path.

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
