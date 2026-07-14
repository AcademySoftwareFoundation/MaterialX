# WGSL Shader Generation

MaterialX includes a native [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator back-end, registered under the `genwgsl` target. `WgslShaderGenerator` derives from `HwShaderGenerator` and emits standalone WGSL vertex and fragment shaders, similar in structure to the MSL and Slang back-ends.

This guide covers the tooling, CMake configuration, and local workflows for working with the WGSL target. For general shader generation concepts, see [Shader Generation](ShaderGeneration.md). For implementation details of the transpiler itself, see [`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).

## Overview

The `genwgsl` uses a **hybrid node library**:

| Library content | Source | Committed to git? |
| --- | --- | --- |
| Most node `.wgsl` files | Transpiled from `genglsl` by `glsl_to_wgsl.py` | No — derived artifact |
| Core `lib/` math and closure helpers | Transpiled from `genglsl/lib/` by `glsl_to_wgsl.py` | No — derived artifact |
| Texture, image, and light nodes | Hand-maintained | Yes |
| `mx_chiang_hair_bsdf` (`EXPECTED_FALLBACK`) | Hand-maintained | Yes |

The GLSL node and lib libraries (`genglsl`, `genglsl/lib/`) are the **single source of truth** for generated WGSL. CI runs the transpiler on every job that generates WGSL to prevent drift between GLSL and WGSL libraries.

The library lives under `libraries/{stdlib,pbrlib,lights}/genwgsl/`, with the target defined in `libraries/targets/genwgsl.mtlx`.

## Prerequisites

| Requirement | Notes |
| --- | --- |
| **Python 3.9+** | Required to run the transpiler |
| **naga-cli** (v29+) | `cargo install naga-cli`, or set the `NAGA` environment variable to the binary path |
| **Rust cargo** (optional) | Only needed if naga is not already installed; CMake can build naga into the build tree |
| **Emscripten 4.0.8** | Required only for JavaScript / WebGPU viewer testing |

## CMake Options

| Option | Default | Description |
| --- | --- | --- |
| `MATERIALX_BUILD_GEN_WGSL` | `OFF` | Build the `MaterialXGenWgsl` library and enable the `genwgsl` shader target |
| `MATERIALX_GENERATE_WGSL_LIBRARY` | `OFF` | Add a `MaterialXGenWgslLibrary` build target that re-transpiles into the build tree as a validation aid |
| `MATERIALX_NAGA_EXECUTABLE` | (auto-detect) | Path to the `naga` CLI |
| `MATERIALX_CARGO_PATH` | (auto-detect) | Path to a Rust cargo home (used to install naga if not found) |

`MATERIALX_BUILD_GEN_WGSL` is **off by default** so ordinary builds do not require Python or naga. Enable it when working on the WGSL target.

`MATERIALX_GENERATE_WGSL_LIBRARY` is independent of `MATERIALX_BUILD_GEN_WGSL` and writes generated files to the build tree only (not the source tree).

## Tooling

### `glsl_to_wgsl.py`

The transpiler at `source/MaterialXGenWgsl/tools/glsl_to_wgsl.py` converts `genglsl` node fragments into `genwgsl` equivalents using [naga](https://github.com/gfx-rs/wgpu/tree/trunk/naga). 

**NOTE:*** It is **not** a general-purpose GLSL-to-WGSL converter. It is scoped to MaterialX shader-node fragments.

**Regenerate the full library in place:**

```sh
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries
```

**Regenerate specific nodes only:**

```sh
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries --only mx_noise3d_float mx_sheen_bsdf
```

A non-zero exit code means an *unexpected* node failed (a regression). Known fallback nodes listed in `EXPECTED_FALLBACK` are tolerated.

### What the transpiler does

`glsl_to_wgsl.py` is **not** a general-purpose GLSL-to-WGSL converter. It transpiles MaterialX **shader-node fragments** — one function per file, referenced by `file=` implementations in `stdlib`, `pbrlib`, and `lights`. naga performs the actual translation; the script wraps it with a pre-processor and post-processor so incomplete node fragments become valid input and the output matches genwgsl library conventions.

**Pipeline (per node function):**

1. **Lib helpers** — transpile `genglsl/lib/*.glsl` first (topological include order, `LIB_PREAMBLE`, overload renaming via inverted `CALL_MAP`)
2. **Pre-process** — wrap the node fragment in a complete GLSL shader naga can parse
3. **Transpile** — `naga --input-kind glsl --shader-stage frag`
4. **Post-process** — clean up naga output and remap helper calls to genwgsl names

#### Input: an incomplete node fragment

A typical genglsl node is not valid standalone GLSL. It has `#include`s, no `main`, and sometimes MaterialX `$`-tokens:

```glsl
#include "lib/mx_noise.glsl"

void mx_noise3d_float(float amplitude, float pivot, vec3 position, out float result)
{
    float value = mx_perlin_noise_float(position);
    result = value * amplitude + pivot;
}
```

The tool cannot feed this directly to naga. Instead it synthesizes a complete translation unit:

- Shared `#define` / `const` / `struct` context from included libs (prototypes only, not full bodies)
- Closure structs (`BSDF`, `surfaceshader`, etc.)
- `$`-token placeholders swapped for legal identifiers (e.g. `$texSamplerSampler2D` → `MTLXTOK_texSamplerSampler2D`, restored after transpile)
- The one node function body being transpiled
- A dummy `main()` entry point (required by naga's GLSL frontend)

#### Post-process: genwgsl conventions

naga's output is correct but verbose (SSA-style parameter shadows, `vec3<f32>` syntax, etc.). The post-processor:

- Collapses single-use temporaries and parameter-copy shadows
- Normalizes types (`vec3<f32>` → `vec3f`, `2f` → `2.0`)
- Remaps overloaded GLSL helper calls via `CALL_MAP` to type-suffixed genwgsl names

WGSL has no function overloading, so the genwgsl `lib/` gives each GLSL overload a distinct name. For the noise example above, `mx_perlin_noise_float(position)` with a `vec3` argument is rewritten to `mx_perlin_noise_float_3d(position)`.

Similarly, GLSL `mx_square` overloads map to `mx_square_f32`, `mx_square_vec2`, or `mx_square_vec3` depending on the argument type.

#### What it handles

| Category | Example | Notes |
| --- | --- | --- |
| Standalone math / utility nodes | `mx_noise3d_float`, `mx_mix_surfaceshader` | `#include` lib helpers; calls remapped via `CALL_MAP` |
| PBR nodes with standard lib signatures | Most BSDF combiners, EDF nodes | Generated when all helper calls resolve |
| Closure / `inout` parameters | `inout BSDF bsdf` | Closure preamble supplies `BSDF` struct; `inout` becomes `ptr<function, BSDF>` |
| Cross-node helper calls | One node calling another node's function | Prototypes collected from all genglsl files |

**Illustrative output** (post-processed fragment for `mx_noise3d_float`):

```wgsl
#include "lib/mx_noise.wgsl"

fn mx_noise3d_float(amplitude: f32, pivot: f32, position: vec3f, result: ptr<function, f32>) {
    let value = mx_perlin_noise_float_3d(position);
    (*result) = value * amplitude + pivot;
}
```

Generated files carry a `// Generated from … do not edit` banner.

#### What it does not handle

| Category | Example | Reason |
| --- | --- | --- |
| Texture / image nodes | `mx_image_color3` | naga's GLSL frontend has no sampler support — auto-skipped by filename pattern (`image`, `hextiled`) |
| Light shaders | `mx_point_light` | Use dynamically generated `LightData` — auto-skipped (`_light$` pattern) |
| Chiang hair BSDF | `mx_chiang_hair_bsdf` | naga limitation on hair scattering helpers — listed in `EXPECTED_FALLBACK`, kept hand-written |
| Unmapped overloads | Any call not in `CALL_MAP` | Node stays hand-written; logged as unsupported |

**Specular environment IBL:** `WgslShaderGenerator` supports FIS, prefilter, and none methods
(`mx_environment_fis.wgsl`, `mx_environment_prefilter.wgsl`, `mx_environment_none.wgsl`). The
prefilter environment bake pass (`hwWriteEnvPrefilter` / `mx_generate_prefilter_env`) remains
GLSL-only today.

**Texture node** (auto-skipped — uses samplers naga cannot parse):

```glsl
#include "lib/$fileTransformUv"

void mx_image_color3($texSamplerSignature, int layer, vec3 defaultval, vec2 texcoord, ...)
{
    vec2 uv = mx_transform_uv(texcoord, uv_scale, uv_offset);
    result = texture($texSamplerSampler2D, uv).rgb;
}
```

The result is a **reduced library**: most nodes and all 22 `lib/` helpers are generated from
genglsl; texture, light, and `mx_chiang_hair_bsdf` remain hand-written. The tool exits non-zero
only when a file *outside* `EXPECTED_FALLBACK` fails unexpectedly (a regression). If a fallback node
starts transpiling cleanly, the tool prints a warning so it can be removed from `EXPECTED_FALLBACK`.

For full transpiler internals see [`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).

### CI

GitHub Actions runs `glsl_to_wgsl.py` on Python-enabled build jobs, the JavaScript job, the Python sdist job, and release archives. A GLSL change that breaks WGSL generation will fail CI even without a local naga install.

## Local Developer Workflows

### After modifying a GLSL node

1. Regenerate the WGSL library:
   ```sh
   python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries
   ```
2. Configure with `-DMATERIALX_BUILD_GEN_WGSL=ON` and rebuild.
3. Run the `[genwgsl]` unit tests:
   ```sh
   ctest -R genwgsl
   ```

### C++ shader-generation testing

This is the fastest path for validating WGSL output without Emscripten:

```sh
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries

cmake -S . -B build -DMATERIALX_BUILD_GEN_WGSL=ON
cmake --build build --config Release
ctest -R genwgsl --test-dir build
```

The `[genwgsl]` tests in `source/MaterialXTest/MaterialXGenWgsl/GenWgsl.cpp` cover syntax, target registration, and shader generation from example materials.

### JavaScript / WebGPU viewer testing

For end-to-end testing in the browser (Three.js WebGPU renderer, TSL bridge), build with both JavaScript and WGSL enabled:

```sh
python source/MaterialXGenWgsl/tools/glsl_to_wgsl.py --libraries libraries --out libraries

cmake -S . -B javascript/build \
  -DMATERIALX_BUILD_JS=ON \
  -DMATERIALX_BUILD_GEN_WGSL=ON \
  -DMATERIALX_EMSDK_PATH=<path-to-emsdk> \
  -G Ninja
cmake --build javascript/build --target install --config Release
```

On Windows, `javascript/build_javascript_win.bat` automates this flow: Emscripten build, npm install, Playwright tests, and a local dev server at `http://localhost:8080`.

The viewer produces two webpack bundles from the same source:

| Page | Backend | Renderer |
| --- | --- | --- |
| `index.html` | WebGL | `THREE.WebGLRenderer` + ESSL (`RawShaderMaterial`) |
| `index-webgpu.html` | WebGPU | `WebGPURenderer` + WGSL (via TSL / `NodeMaterial`) |

Separate bundles are used because the WebGL and WebGPU Three.js entry points are incompatible. A toggle link switches between the two pages.

To build the viewer bundle after WASM is ready:

```sh
cd javascript/MaterialXView
npm install
npm run build
npm run start    # dev server at http://localhost:8080
```

### Build-time validation (optional)

To re-transpile into the build tree on every build (without modifying the source tree):

```sh
cmake -S . -B build \
  -DMATERIALX_BUILD_GEN_WGSL=ON \
  -DMATERIALX_GENERATE_WGSL_LIBRARY=ON
cmake --build build
```

This adds the `MaterialXGenWgslLibrary` target, which surfaces transpile and naga validation failures as build errors.

## Release Artifacts

Generated WGSL node and `lib/` files are not committed to git, but they are included in release archives. The release workflow runs `glsl_to_wgsl.py` before packaging, so `libraries/*/genwgsl/**/*.wgsl` files ship in the archive.

## Related Documentation

- [WGSL Transpiler Fixes](WGSLTranspilerFixes.md) — generation policy and the naga-reconciliation fixes for full-library generation
- [Shader Generation](ShaderGeneration.md) — general shader generation framework
- [`source/MaterialXGenWgsl/README.md`](../../source/MaterialXGenWgsl/README.md) — back-end layout and design
- [`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md) — transpiler internals, `CALL_MAP`, and `EXPECTED_FALLBACK`
- [`javascript/README.md`](../../javascript/README.md) — JavaScript bindings and viewer setup
