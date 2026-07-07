# MaterialXGenWgsl

A native [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator back-end for
MaterialX, registered under the `genwgsl` target. `WgslShaderGenerator` derives directly from
`HwShaderGenerator` (not the GLSL hierarchy) and emits standalone WGSL vertex + fragment shaders,
mirroring the structure of the `MaterialXGenMsl` / `MaterialXGenSlang` back-ends. It is gated behind
the `MATERIALX_BUILD_GEN_WGSL` CMake option (**OFF by default**: its node library is transpiled from
genglsl and is not committed, so enabling the target requires that library to be generated first —
see below).

## What makes this back-end unusual

Every other shader-gen back-end ships a fully hand-written node library. `genwgsl` does **not** —
its node library is a *hybrid*, and that is the main thing to understand before editing it:

* **Most node `.wgsl` files are machine-generated** from their `genglsl` originals by the offline
  transpiler in [`tools/`](tools/README.md). They are **not committed** — they are a derived build
  artifact, transpiled from genglsl (the single source of truth) by CI, so drift between the GLSL and
  WGSL libraries is impossible by construction. The generated files carry a
  `// Generated from … do not edit` banner; do not hand-edit them, and do not commit them.
* **A small number of nodes are hand-written** and intentionally *not* generated, because WGSL has
  no function overloading and a few `genwgsl/lib/` helpers were hand-adapted away from their GLSL
  signatures. These are listed as `EXPECTED_FALLBACK` in `tools/glsl_to_wgsl.py` (currently the
  `conductor` / `dielectric` / `generalized_schlick` BSDFs, `subsurface`, and `chiang_hair`).
* **Texture / image and light nodes are hand-written** too — the transpiler skips them (naga's GLSL
  front-end has no sampler support, and light shaders use the dynamically generated `LightData`).
* **Core `lib/` math and closure helpers are hand-maintained** (out of scope for the transpiler).

The library lives in `libraries/{stdlib,pbrlib,lights}/genwgsl/` with the target defined in
`libraries/targets/genwgsl.mtlx`; node implementations are wired up via `*_genwgsl_impl.mtlx`.

## Generating the library

Because the generated node files are not committed, you must produce them before building the WGSL
target. CI does this automatically (installing the [`naga`](https://github.com/gfx-rs/wgpu/tree/trunk/naga)
CLI and running the transpiler) on the jobs that build or ship WGSL — the web viewer, the Python
sdist/wheels, and the tagged-release archives. Locally, populate the library in place with:

```
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries
```

then configure with `-DMATERIALX_BUILD_GEN_WGSL=ON`. The transpiler exits non-zero on an unexpected
failure (a regression or a new naga validation error), while tolerating the known `EXPECTED_FALLBACK`
nodes — so CI running it doubles as validation that a change hasn't broken the WGSL target. See
[`tools/README.md`](tools/README.md) for the tool, its overload mapping table, and its lib-arity
self-validation.

The separate `-DMATERIALX_GENERATE_WGSL_LIBRARY=ON` option adds a `MaterialXGenWgslLibrary` target
that re-transpiles into the *build tree* (not the source tree) as a local validation aid; it is
decoupled from `MATERIALX_BUILD_GEN_WGSL`.

## Layout

| Path | Contents |
| --- | --- |
| `WgslShaderGenerator.{h,cpp}` | The `genwgsl` shader generator (vertex + fragment WGSL). |
| `WgslSyntax.{h,cpp}` | WGSL type names and syntactic rules. |
| `WgslResourceBindingContext.{h,cpp}` | `@group`/`@binding` uniform, split texture/sampler, and structured light-data bindings. |
| `Nodes/` | C++ node implementations that emit WGSL dynamically. |
| `tools/` | The offline `genglsl`→`genwgsl` library transpiler (see its README). |
