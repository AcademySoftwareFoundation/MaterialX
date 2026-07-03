# MaterialXGenWgsl

A native [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator back-end for
MaterialX, registered under the `genwgsl` target. `WgslShaderGenerator` derives directly from
`HwShaderGenerator` (not the GLSL hierarchy) and emits standalone WGSL vertex + fragment shaders,
mirroring the structure of the `MaterialXGenMsl` / `MaterialXGenSlang` back-ends. It is gated behind
the `MATERIALX_BUILD_GEN_WGSL` CMake option (ON by default).

## What makes this back-end unusual

Every other shader-gen back-end ships a fully hand-written node library. `genwgsl` does **not** —
its node library is a *hybrid*, and that is the main thing to understand before editing it:

* **Most node `.wgsl` files are machine-generated** from their `genglsl` originals by the offline
  transpiler in [`tools/`](tools/README.md). They are committed (the generator does not run at
  build time by default) but carry a `// Generated from … do not edit` banner to distinguish them
  from the hand-written files alongside — do not hand-edit them; regenerate.
* **A small number of nodes are hand-written** and intentionally *not* generated, because WGSL has
  no function overloading and a few `genwgsl/lib/` helpers were hand-adapted away from their GLSL
  signatures. These are listed as `EXPECTED_FALLBACK` in `tools/glsl_to_wgsl.py` (currently the
  `conductor` / `dielectric` / `generalized_schlick` BSDFs, `subsurface`, and `chiang_hair`).
* **Texture / image and light nodes are hand-written** too — the transpiler skips them (naga's GLSL
  front-end has no sampler support, and light shaders use the dynamically generated `LightData`).
* **Core `lib/` math and closure helpers are hand-maintained** (out of scope for the transpiler).

The library lives in `libraries/{stdlib,pbrlib,lights}/genwgsl/` with the target defined in
`libraries/targets/genwgsl.mtlx`; node implementations are wired up via `*_genwgsl_impl.mtlx`.

## Keeping the generated library in sync with genglsl

The generated `.wgsl` files can drift as `genglsl` improves. The transpiler regenerates them and a
diff surfaces the drift — see [`tools/README.md`](tools/README.md) for the tool, its overload
mapping table, and its lib-arity self-validation.

For CI / local validation, configure with `-DMATERIALX_GENERATE_WGSL_LIBRARY=ON` (requires Python
and the [`naga`](https://github.com/gfx-rs/wgpu/tree/trunk/naga) CLI). This adds a
`MaterialXGenWgslLibrary` build target that re-transpiles the library into the build tree on every
build and **fails the build** if a node that should generate cleanly does not (a regression or a new
naga validation error), while tolerating the known `EXPECTED_FALLBACK` nodes. The committed `.wgsl`
files remain the source of truth; the build-tree output is for validation only.

## Layout

| Path | Contents |
| --- | --- |
| `WgslShaderGenerator.{h,cpp}` | The `genwgsl` shader generator (vertex + fragment WGSL). |
| `WgslSyntax.{h,cpp}` | WGSL type names and syntactic rules. |
| `WgslResourceBindingContext.{h,cpp}` | `@group`/`@binding` uniform, split texture/sampler, and structured light-data bindings. |
| `Nodes/` | C++ node implementations that emit WGSL dynamically. |
| `tools/` | The offline `genglsl`→`genwgsl` library transpiler (see its README). |
