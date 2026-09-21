# WGSL Shader Generation

MaterialX includes a [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator, registered as the `genwgsl` target. `WgslShaderGenerator` extends `HwShaderGenerator` and produces standalone WGSL vertex and fragment shaders, similar to the MSL and Slang back-ends.

This guide covers tooling, CMake setup, and local workflows. For general shader generation, see [Shader Generation](ShaderGeneration.md). For transpiler internals, see [`tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).

## Overview

Most of the `genwgsl` library is **generated from `genglsl`** by `mxgenwgsl.py`. A few files are hand-written:

| Library content                         | Source                                      |
| --------------------------------------- | ------------------------------------------- |
| Node `.wgsl` files and `lib/` helpers   | Generated from `genglsl` by `mxgenwgsl.py`  |
| Light shaders (3)                       | Hand-written (`LightData` struct)           |
| `mx_math_platform.wgsl`                 | Hand-written (naga platform gaps)           |

The GLSL libraries (`genglsl`, `genglsl/lib/`) are the **single source of truth**. CI runs the transpiler on every WGSL build to keep GLSL and WGSL in sync.

### How `$`-tokens flow through the pipeline

MaterialX `$`-tokens (e.g. `$envRadiance`, `$texSamplerSampler2D`) are resolved in **two stages**:

1. **Build time (`mxgenwgsl.py`)** — `$`-tokens are replaced with placeholder identifiers (`MTLXTOK_*`) so naga can parse the GLSL. After transpile, the placeholders are restored back to `$`-tokens in the generated `.wgsl`.
2. **Runtime (`WgslShaderGenerator`)** — `_tokenSubstitutions` replaces `$`-tokens with concrete WGSL bindings (split texture/sampler pairs, environment uniforms, etc.).

Both stages must preserve tokens end-to-end. Hand-written files in `skip_transpile.txt` cover cases naga cannot handle.

**Adding or changing a token:** The token tables (`TOKEN_EXPANSIONS`, `TOKEN_RESTORE_RULES`) live in [`mxgenwgsl.py`](../../source/MaterialXGenWgsl/tools/mxgenwgsl.py). See the checklist in [`tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md#when-you-change-hwconstants) for the full steps when editing `HwConstants.cpp`.

The library lives under `libraries/{stdlib,pbrlib,lights}/genwgsl/`, with the target defined in `libraries/targets/genwgsl.mtlx`.

## Prerequisites

| Requirement               | Notes                                                                                  |
| ------------------------- | -------------------------------------------------------------------------------------- |
| **Python 3.9+**           | Runs the transpiler                                                                    |
| **naga-cli** (v30.0.0)    | `cargo install naga-cli --version 30.0.0`, or set `NAGA` env var to the binary path    |
| **Rust cargo** (optional) | Only needed if naga is not installed; CMake can install it into the build tree          |
| **Emscripten 4.0.8**      | Only needed for JavaScript / WebGPU viewer testing                                     |

## CMake Options

| Option                            | Default       | Description                                                                                                     |
| --------------------------------- | ------------- | --------------------------------------------------------------------------------------------------------------- |
| `MATERIALX_BUILD_GEN_WGSL`        | `OFF`         | Build `MaterialXGenWgsl`, enable `genwgsl`, and run `mxgenwgsl.py` in-place under `libraries/` on every build  |
| `MATERIALX_GENERATE_WGSL_LIBRARY` | `OFF`         | Without `BUILD_GEN_WGSL`, add a target that re-transpiles into the build tree for validation only               |
| `MATERIALX_NAGA_EXECUTABLE`       | (auto-detect) | Path to the `naga` CLI                                                                                          |
| `MATERIALX_NAGA_VERSION`          | `30.0.0`      | Pinned `naga-cli` version installed by CMake via cargo (matches CI)                                             |
| `MATERIALX_CARGO_PATH`            | (auto-detect) | Rust cargo home (used to install naga if not found)                                                             |

`MATERIALX_BUILD_GEN_WGSL` is **off by default** for standalone C++ builds so they don't need Python or naga. The JavaScript build scripts (`build_javascript_win.bat`, CI) pass it as ON automatically. Enable it manually when working on the WGSL target outside the JS build.

`MATERIALX_GENERATE_WGSL_LIBRARY` writes generated files to `${CMAKE_BINARY_DIR}/genwgsl_generated` (not the source tree) — useful for validation without modifying `libraries/`.

## Tooling

### `mxgenwgsl.py`

The transpiler at `source/MaterialXGenWgsl/tools/mxgenwgsl.py` converts `genglsl` node fragments into `genwgsl` equivalents using [naga](https://github.com/gfx-rs/wgpu/tree/trunk/naga).

**Note:** This is **not** a general-purpose GLSL-to-WGSL converter. It only handles MaterialX shader-node fragments.

**Regenerate the full library:**

```sh
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries
```

**Regenerate specific nodes:**

```sh
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries --only mx_noise3d_float mx_sheen_bsdf
```

A non-zero exit code means an unexpected node failed (a regression). Nodes listed in `skip_transpile.txt` are skipped entirely.

### What the transpiler does

naga does the core GLSL → WGSL translation. The script adds pre-processing and post-processing because genglsl sources are not complete shaders — they use `#include`, `$`-tokens, and have no `main()` entry point.

**Pipeline (per node):**

1. **Lib helpers** — transpile `genglsl/lib/*.glsl` first (topological include order, overload renaming via `mangle()`)
2. **Pre-process** — wrap the node fragment in a complete GLSL shader naga can parse
3. **Transpile** — `naga --input-kind glsl --shader-stage frag`
4. **Post-process** — clean up naga output and remap overloaded calls to their `genwgsl` names

#### Input: an incomplete node fragment

A typical genglsl node is not valid standalone GLSL:

```glsl
#include "lib/mx_noise.glsl"

void mx_noise3d_float(float amplitude, float pivot, vec3 position, out float result)
{
    float value = mx_perlin_noise_float(position);
    result = value * amplitude + pivot;
}
```

The tool builds a complete GLSL shader around it:

- `#define`/`const`/`struct` context from included libs (prototypes only, not full bodies)
- Closure structs (`BSDF`, `surfaceshader`, etc.)
- `$`-tokens replaced with legal identifiers (e.g. `$texSamplerSampler2D` → `MTLXTOK_texSamplerSampler2D`, restored after transpile)
- The node function body
- A dummy `main()` entry point (required by naga)

#### Post-processing

naga's output is correct but verbose. The post-processor:

- Collapses single-use temporaries and parameter-copy shadows
- Normalizes types (`vec3<f32>` → `vec3f`, `2f` → `2.0`)
- Remaps overloaded GLSL calls to type-suffixed WGSL names via `mangle()`
- Re-attaches GLSL comments that naga discards

WGSL has no function overloading, so each GLSL overload gets a unique name. For example, `mx_perlin_noise_float(vec3)` becomes `mx_perlin_noise_float_3d(position)`, and `mx_square(float)` becomes `mx_square_f32`.

#### What it handles

| Category                               | Example                                    | Notes                                                      |
| -------------------------------------- | ------------------------------------------ | ---------------------------------------------------------- |
| Math / utility nodes                   | `mx_noise3d_float`, `mx_mix_surfaceshader` | Lib helpers included; calls remapped via `mangle()`        |
| PBR nodes                              | Most BSDF combiners, EDF nodes             | Generated when all helper calls resolve                    |
| Closure / `inout` parameters           | `inout BSDF bsdf`                          | `BSDF` struct provided; `inout` → `ptr<function, BSDF>`   |
| Cross-node calls                       | One node calling another node's function   | Prototypes collected from all genglsl files                |

**Example output** (`mx_noise3d_float`):

```wgsl
#include "lib/mx_noise.wgsl"

fn mx_noise3d_float(amplitude: f32, pivot: f32, position: vec3f, result: ptr<function, f32>) {
    let value = mx_perlin_noise_float_3d(position);
    (*result) = value * amplitude + pivot;
}
```

Generated files carry a `// Generated from … do not edit` banner.

#### What it does not handle

| Category              | Example              | Reason                                                                     |
| --------------------- | -------------------- | -------------------------------------------------------------------------- |
| Texture / image nodes | `mx_image_color3`    | naga cannot parse GLSL sampler types — auto-skipped                        |
| Light shaders         | `mx_point_light`     | Need the dynamic `LightData` struct — auto-skipped                         |
| Unmapped overloads    | Some BSDF helpers    | `mangle()` returns `None` — node stays hand-written                        |

**Specular environment IBL:** `WgslShaderGenerator` supports FIS, prefilter, and none methods (`mx_environment_fis.wgsl`, `mx_environment_prefilter.wgsl`, `mx_environment_none.wgsl`). Bake passes and shadow mapping are deferred — see [Deferred Features](#deferred-features).

The result is a **reduced library**: most nodes and all 22 `lib/` helpers are generated from genglsl; texture, light, and a few edge-case nodes remain hand-written. A non-zero exit only means something *unexpected* broke.

For full transpiler internals see [`tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).

### CI

GitHub Actions installs Python, naga, and the transpiler dependencies, then relies on CMake (`-DMATERIALX_BUILD_GEN_WGSL=ON`) to run `mxgenwgsl.py` during `cmake --build`. A GLSL change that breaks WGSL generation fails CI even without a local naga install.

## Local Developer Workflows

### After modifying a GLSL node

1. Configure with `-DMATERIALX_BUILD_GEN_WGSL=ON` and rebuild (CMake runs the transpiler automatically).
2. Run the `[genwgsl]` unit tests:
   ```sh
   ctest -R genwgsl
   ```

To regenerate manually without a full build:

```sh
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries
```

### C++ shader-generation testing

The fastest way to validate WGSL output without Emscripten:

```sh
cmake -S . -B build -DMATERIALX_BUILD_GEN_WGSL=ON
cmake --build build --config Release
ctest -R genwgsl --test-dir build
```

The `[genwgsl]` tests in `GenWgsl.cpp` cover syntax, target registration, single-material generation, and a full `WgslShaderGeneratorTester` run over TestSuite and Examples materials.

### JavaScript / WebGPU viewer testing

For end-to-end browser testing (Three.js WebGPU renderer, TSL bridge):

```sh
cmake -S . -B javascript/build \
  -DMATERIALX_BUILD_JS=ON \
  -DMATERIALX_BUILD_GEN_WGSL=ON \
  -DMATERIALX_EMSDK_PATH=<path-to-emsdk> \
  -G Ninja
cmake --build javascript/build --target install --config Release
```

On Windows, `javascript/build_javascript_win.bat` automates this: Emscripten build, npm install, Playwright tests, and a dev server at `http://localhost:8080`.

The viewer produces two bundles:

| Page                | Backend | Renderer                                           |
| ------------------- | ------- | -------------------------------------------------- |
| `index.html`        | WebGL   | `THREE.WebGLRenderer` + ESSL (`RawShaderMaterial`) |
| `index-webgpu.html` | WebGPU  | `WebGPURenderer` + WGSL (via TSL / `NodeMaterial`) |

To build the viewer after WASM is ready:

```sh
cd javascript/MaterialXView
npm install
npm run build
npm run start    # dev server at http://localhost:8080
```

### Build-tree validation (optional)

To re-transpile into the build tree without modifying `libraries/`:

```sh
cmake -S . -B build -DMATERIALX_GENERATE_WGSL_LIBRARY=ON
cmake --build build --target MaterialXGenWgslLibrary
```

Output goes to `${CMAKE_BINARY_DIR}/genwgsl_generated`.

## Release Artifacts

Generated `.wgsl` files are not committed to git but are included in release archives. The release workflow runs `mxgenwgsl.py` before packaging so all `libraries/*/genwgsl/**/*.wgsl` files ship alongside hand-written ones.

## Source Control

Generated `.wgsl` files are excluded via `.gitignore`. Only hand-written files listed in `skip_transpile.txt` are committed. When adding a new hand-written `.wgsl` to `skip_transpile.txt`, also add the matching `!` negation to `.gitignore`.

## WGSL Validation

CI validates generated WGSL at three levels:

1. **Transpile-time** — `mxgenwgsl.py` runs naga for each node; any naga error is fatal.
2. **Full-shader validation** — `generateshader.py --target wgsl --validator naga` generates complete shaders from example materials and validates each with naga.
3. **Generator coverage** — the `[genwgsl]` C++ tests run `WgslShaderGeneratorTester` over TestSuite and Examples materials.

## Related Documentation

- [Shader Generation](ShaderGeneration.md) — general shader generation framework
- [`source/MaterialXGenWgsl/README.md`](../../source/MaterialXGenWgsl/README.md) — back-end layout and design
- [`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md) — transpiler internals, overload naming, and `skip_transpile.txt`
- [`javascript/README.md`](../../javascript/README.md) — JavaScript bindings and viewer setup
