# WGSL Shader Generation

MaterialX includes a [WGSL](https://www.w3.org/TR/WGSL/) (WebGPU Shading Language) shader generator for the `genwgsl` target. `WgslShaderGenerator` extends `HwShaderGenerator` and produces WGSL vertex and fragment shaders, like the MSL and Slang back-ends.

This guide covers setup, local workflows, and validation. For the general framework, see [Shader Generation](ShaderGeneration.md). For transpiler internals, see [`tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md).



## WGSL Data Library

The `genwgsl` library lives in `libraries/{stdlib,pbrlib,lights,nprlib}/genwgsl/`, and the target is defined in `libraries/targets/genwgsl.mtlx`.
Most `.wgsl` files are generated from `genglsl` by `mxgenwgsl.py` and are not committed. Only the files listed in [`skip_transpile.txt`](../../source/MaterialXGenWgsl/tools/skip_transpile.txt) are hand-written:

|Hand-written file|Reason|
|-|-|
|Light shaders (3)|Use the dynamically generated `LightData` struct|
|`stdlib/.../mx_math_platform.wgsl`|Math helpers naga cannot transpile (`mx_mod`, `mx_isinf`)|
|`pbrlib/.../mx_shadow_platform.wgsl`|Split texture/sampler shadow lookup; no `gl_FragCoord` in WGSL|

Note that the GLSL library (`genglsl`) is the single source of truth.

### MaterialX Shader tokens

MaterialX tokens (e.g. `$envRadiance`, `$texSamplerSampler2D`) are handled in two steps:

1. **Generation (`mxgenwgsl.py`)**: tokens are swapped for `MTLXTOK_*` placeholders so naga can parse the GLSL, then restored in the generated `.wgsl`. This allows the `.wgsl` to have same token as `.glsl`.
2. **Shader generation (`WgslShaderGenerator`)**: tokens are replaced with real WGSL bindings, such as split texture/sampler pairs and environment uniforms.

The token tables (`TOKEN_EXPANSIONS`, `TOKEN_RESTORE_RULES`) are in [`mxgenwgsl.py`](../../source/MaterialXGenWgsl/tools/mxgenwgsl.py). If you edit `HwConstants.cpp`, please update the mapping using [this readme](../../source/MaterialXGenWgsl/tools/README.md#when-you-change-hwconstants).



## Prerequisites

|Requirement|Notes|
|-|-|
|Python 3.9+|Runs the transpiler. 3.10+ also enables the optional tidy pass.|
|naga-cli 30.0.0|`cargo install naga-cli --version 30.0.0`, or set the `NAGA` env var.|
|Rust cargo (optional)|Lets CMake install naga into the build tree if it is not found.|
|Emscripten 4.0.8|Only for the JavaScript build and WebGPU viewer.|

## CMake Options

|Option|Default|Description|
|-|-|-|
|`MATERIALX_BUILD_GEN_WGSL`|`OFF`|Build `MaterialXGenWgsl` and generate the library in `libraries/` on each build.|
|`MATERIALX_GENERATE_WGSL_LIBRARY`|`OFF`|Without `BUILD_GEN_WGSL`: generate into `<build>/genwgsl_generated` for validation only.|
|`MATERIALX_NAGA_EXECUTABLE`|auto-detect|Path to the `naga` CLI.|
|`MATERIALX_NAGA_VERSION`|`30.0.0`|naga-cli version CMake installs via cargo (matches CI).|
|`MATERIALX_CARGO_PATH`|auto-detect|Cargo home used to install naga.|

`MATERIALX_BUILD_GEN_WGSL` is off by default, so plain C++ builds don't need Python or naga. The JavaScript build (`build_javascript_win.bat` and CI) turns it on.

## Local Workflows

### After changing a GLSL node

Build with `MATERIALX_BUILD_GEN_WGSL=ON` (this regenerates the library), then run the WGSL tests:

```sh
cmake -S . -B build -DMATERIALX_BUILD_GEN_WGSL=ON
cmake --build build --config Release
ctest -R WGSL -C Release --test-dir build
```

The tests in `GenWgsl.cpp` check syntax, target registration, single-material generation, and generation for all TestSuite and Examples materials.

To regenerate without building (all nodes, or just some):

```sh
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --libraries libraries --out libraries --only mx_noise3d_float mx_sheen_bsdf
```

To delete generated files and return to a hand-written-only tree, use `--clean`. A non-zero exit code means a node that should transpile failed.

### WebGPU viewer

```sh
cmake -S . -B javascript/build -G Ninja \
  -DMATERIALX_BUILD_JS=ON \
  -DMATERIALX_BUILD_GEN_WGSL=ON \
  -DMATERIALX_EMSDK_PATH=<path-to-emsdk>
cmake --build javascript/build --target install --config Release

cd javascript/MaterialXView
npm install
npm run build
npm run start    # http://localhost:8080
```

On Windows, `javascript/build_javascript_win.bat` runs all of these steps plus the Playwright tests.

The viewer has two pages:

|Page|Backend|Renderer|
|-|-|-|
|`index.html`|WebGL|`THREE.WebGLRenderer` + ESSL (`RawShaderMaterial`)|
|`index-webgpu.html`|WebGPU|`WebGPURenderer` + WGSL (via TSL / `NodeMaterial`)|

### WebGPU smoke tests

The smoke tests load a few materials in `index-webgpu.html` on the shaderball and check that each one renders. A test fails if:

- Chromium cannot create a WebGPU device, or the viewer falls back to WebGL2
- the page logs a console error
- the center of the shaderball is mostly magenta (shader error) or black (nothing rendered)

The tests need a GPU with WebGPU support, so they don't run in CI or as part of `npm run test:browser`. Run them yourself after changing the generator, the `genwgsl` library, or the WebGPU viewer.

First build the WASM and the viewer (see above). The tests serve `MaterialXView/dist` directly, so you don't need a running server. If `dist/` is older than `javascript/build/bin`, the tests fail and ask you to run `npm run build` again.

```sh
cd javascript/MaterialXTest
npm install
npx playwright install chromium
npm run test:webgpu-smoke
```

Screenshots are saved to `javascript/MaterialXTest/_build/webgpu-smoke/<case>.png`, so you can check them by eye.

To run from another entry point, such as `npx playwright test`, UI mode, or the VS Code extension, set `MX_WEBGPU_SMOKE=1`. Without it, the `webgpu-smoke` project contains no tests.

To add a material, add an entry to `WEBGPU_SMOKE_CASES` in `javascript/MaterialXTest/browser/webgpuSmokeCases.mjs`.

### Generate into the build tree

To check generation without touching `libraries/`:

```sh
cmake -S . -B build -DMATERIALX_GENERATE_WGSL_LIBRARY=ON
cmake --build build --target MaterialXGenWgslLibrary
```

## Validation

CI checks WGSL at three levels:

1. **Generation**: `mxgenwgsl.py` runs naga on every node. Any naga error fails the build.
2. **Generator tests**: the C++ WGSL tests generate every TestSuite and Examples material.
3. **Naga validation job**: `mxvalidategenwgsl.py` validates TestSuite and Examples, plus StandardSurface across a matrix of GenOptions (prefilter, shadow, AO, and more).

To run the validator locally (from `python/`, after building with `MATERIALX_BUILD_GEN_WGSL=ON`):

```sh
python ../source/MaterialXGenWgsl/tools/mxvalidategenwgsl.py --naga path/to/naga --preset default

python ../source/MaterialXGenWgsl/tools/mxvalidategenwgsl.py \
  --input ../resources/Materials/Examples/StandardSurface \
  --naga path/to/naga --preset default prefilter shadow ao shadow_ao
```

By default, the validator also checks **viewer parity**: it converts each pixel shader with `mxtsladapter.mjs` (the same path the WebGPU viewer uses) and validates it again. This catches errors that only show up in the browser. It needs Node.js; turn it off with `--no-viewer-parity`.

For fast checks without a C++ build, run `test_mxgenwgsl.py` and `test_mxwgsltidy.py`.

## How the Transpiler Works

`mxgenwgsl.py` uses [naga](https://github.com/gfx-rs/wgpu/tree/trunk/naga) to convert `genglsl` files to WGSL. It only handles MaterialX node files. It is not a general GLSL-to-WGSL converter.

genglsl files are not complete shaders: they use `#include` and `$`-tokens and have no `main()`. For each file, the tool:

1. **Transpiles `lib/` helpers first**, in include order.
2. **Wraps the node** in a complete GLSL shader: prototypes from included libs, closure structs (`BSDF`, etc.), placeholder tokens, and a dummy `main()`.
3. **Runs naga**: `naga --input-kind glsl --shader-stage frag`.
4. **Tidies the output** (`mxwgsltidy.py`): removes extra temporaries, shortens types (`vec3<f32>` → `vec3f`), restores comments, and renames overloaded calls.

WGSL has no function overloading, so each GLSL overload gets a type-suffixed name. For example, `mx_perlin_noise_float(vec3)` becomes `mx_perlin_noise_float_3d`.

Input (`genglsl/mx_noise3d_float.glsl`):

```glsl
#include "lib/mx_noise.glsl"

void mx_noise3d_float(float amplitude, float pivot, vec3 position, out float result)
{
    float value = mx_perlin_noise_float(position);
    result = value * amplitude + pivot;
}
```

Output (`genwgsl/mx_noise3d_float.wgsl`):

```wgsl
#include "lib/mx_noise.wgsl"

fn mx_noise3d_float(amplitude: f32, pivot: f32, position: vec3f, result: ptr<function, f32>) {
    let value = mx_perlin_noise_float_3d(position);
    (*result) = value * amplitude + pivot;
}
```

Generated files start with a `// Generated from … do not edit` banner.

## Source Control and Releases

Generated `.wgsl` files are ignored by `.gitignore`. To add a new hand-written file, list it in `skip_transpile.txt` and add a matching `!` exception to `.gitignore`.

Release archives and Python sdists do include the generated files. Those workflows run `mxgenwgsl.py` before packaging.

## OpenColorIO (OCIO)

OCIO cannot output WGSL. Instead, OCIO transforms are converted to WGSL **at build time**:

1. OCIO generates GLSL 4.0 for each color transform (the same code `genglsl` uses).
2. `mxgenwgsl.py --bake-ocio` converts each function to WGSL with naga and writes it to `libraries/stdlib/genwgsl/ocio/<functionName>.wgsl`.
3. During shader generation, `WgslOcioNode` includes the matching file. Python and naga are not needed at this stage.

### Requirements

* Configure with `-DMATERIALX_BUILD_OCIO=ON` (OpenColorIO 2.4+).
* naga, plus PyMaterialX built with OCIO, are needed only for baking.

When OCIO and WGSL generation are both on, CMake bakes automatically (target `MaterialXGenWgslOcioLibrary`) before generating the rest of the library.

### Re-baking manually

Re-bake after changing OCIO or the OCIO test materials:

```sh
python source/MaterialXGenWgsl/tools/mxgenwgsl.py --bake-ocio --repo . --out libraries/stdlib/genwgsl/ocio
```

By default, transforms come from TestSuite `ocio_color_management.mtlx`. Use `--ocio-mtlx` or `--ocio-config` for other materials or configs. Custom OCIO configs need their own baked files.

### Limitations

* Transforms that need LUT textures (1D/3D) are not supported. `genglsl` has the same limitation.
* Without OCIO, the validator and C++ tests skip `ocio_color_management.mtlx`.

## Related Documentation

* [Shader Generation](ShaderGeneration.md): general shader generation framework
* [`source/MaterialXGenWgsl/README.md`](../../source/MaterialXGenWgsl/README.md): back-end layout and design
* [`source/MaterialXGenWgsl/tools/README.md`](../../source/MaterialXGenWgsl/tools/README.md): transpiler internals, overload naming, `skip_transpile.txt`
* [`javascript/README.md`](../../javascript/README.md): JavaScript bindings and MaterialXView
* [`libraries/README.md`](../../libraries/README.md): data libraries and `genwgsl` targets

