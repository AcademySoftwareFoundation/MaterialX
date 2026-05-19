# MaterialX OSL Renderer

`materialx-osl` is a command-line renderer for generating PNG reference images from
MaterialX documents through the OSL shader generator and OSL `testrender`. It is
intended to be scriptable and to accept the same core capture arguments used by
`MaterialXView`, so it can be used as a drop-in backend for automated fidelity
tests.

## Build

The target is built from `source/MaterialXOslRender`:

```bash
cmake --build build-osl --target materialx-osl
```

The executable is written to the build runtime directory, for example:

```bash
build-osl/bin/materialx-osl
```

The CMake configuration must provide working OSL tools:

- `MATERIALX_OSL_BINARY_OSLC`: path to the OSL compiler.
- `MATERIALX_OSL_BINARY_TESTRENDER`: path to OSL `testrender`.
- `MATERIALX_OSL_INCLUDE_PATH`: optional include path passed to the OSL renderer.

At build time, the renderer also copies the OSL utility shaders from
`source/MaterialXTest/MaterialXRenderOsl/Utilities` into
`<runtime>/resources/Utilities`.

## Quick Start

```bash
materialx-osl \
  --material resources/Materials/Examples/StandardSurface/standard_surface_default.mtlx \
  --captureFilename output/standard_surface_default.png \
  --screenWidth 512 \
  --screenHeight 512 \
  --path /path/to/MaterialX
```

With a custom GLTF mesh and environment map:

```bash
materialx-osl \
  --material material.mtlx \
  --mesh ShaderBall.glb \
  --envRad san_giuseppe_bridge_2k.hdr \
  --drawEnvironment true \
  --screenWidth 512 \
  --screenHeight 512 \
  --captureFilename materialx-osl.png \
  --path /path/to/MaterialX \
  --path /path/to/assets
```

## Command Line

Usage:

```bash
materialx-osl --material <file> --captureFilename <file> [options]
```

Options use separate arguments. The `--option=value` form is not supported.

| Option | Required | Default | Description |
| --- | --- | --- | --- |
| `--material <file>` | Yes | | MaterialX document to render. The first renderable element in the document is used. |
| `--captureFilename <file>` | Yes | | Final PNG path. Parent directories are created as needed. |
| `--mesh <file>` | No | Built-in sphere scene | GLTF mesh to render. The mesh is converted to a normalized OBJ for `testrender`. |
| `--envRad <file>` | No | Utility shader default | Environment radiance image passed to the utility `envmap` shader. |
| `--screenWidth <int>` | No | `512` | Output image width in pixels. |
| `--screenHeight <int>` | No | `512` | Output image height in pixels. |
| `--path <dir>` | No | | Repeatable MaterialX and asset search path. |
| `--screenColor <r,g,b>` | No | `0,0,0` | Background color used when `--mesh` is provided and `--drawEnvironment false`. |
| `--drawEnvironment <bool>` | No | `true` | With `--mesh`, controls whether the generated scene uses the environment background or a constant background color. Values `false` and `0` disable it; all other values enable it. |
| `--enableDirectLight <bool>` | No | Ignored | Accepted for MaterialXView CLI compatibility. |
| `--shadowMap <bool>` | No | Ignored | Accepted for MaterialXView CLI compatibility. |
| `--help`, `-h` | No | | Print usage and exit. |

Unknown options are errors. Options that require a value are also errors if the
value is omitted.

## Search Paths

The renderer builds its MaterialX search path in this order:

1. Each directory passed with `--path`, in command-line order.
2. The current working directory.
3. The inferred MaterialX source root, when available.

The MaterialX standard libraries are loaded from `libraries` through this search
path. Texture and environment lookup starts with the material file's directory
and then falls back to the full search path. If `--envRad` cannot be found through
the image search path, its literal path is still passed to the environment shader.

The OSL utility directory is resolved in this order:

1. `<runtime>/resources/Utilities`
2. `resources/Utilities` found through the search path
3. `<MaterialX root>/build-osl/bin/resources/Utilities`
4. `<MaterialX root>/source/MaterialXTest/MaterialXRenderOsl/Utilities`

Startup fails if the utility directory cannot be found.

## Rendering Behavior

The renderer:

- Loads the input MaterialX document and imports the standard libraries.
- Flattens texture filenames against the material directory and search paths.
- Generates OSL for the first renderable element.
- Uses `lin_rec709` as the target color space override.
- Enables vertical flipping for file textures.
- Enables the OSL `Ci` wrapper connection mode.
- Compiles the generated OSL and utility OSL shaders through `oslc`.
- Renders through OSL `testrender`.
- Saves the captured image to `--captureFilename`.

When `--mesh` is omitted, the renderer uses the standard OSL utility scene
template, which renders a sphere.

When `--mesh` is provided, the renderer loads the GLTF mesh, normalizes its
bounding sphere to radius `2.0`, writes a temporary OBJ, and writes a scene
template with a camera at `(0, 0, 5)` looking at the origin. This path is intended
to match the MaterialXView-style framing used by reference image generation.

## Generated Files

Intermediate files are written beside the final image under:

```text
<capture parent>/.materialx-osl/
```

Typical contents include:

- Generated `.osl` and compiled `.oso` shader files.
- The OSL-rendered PNG from `testrender`.
- Compile or render diagnostic files when OSL reports errors.
- For `--mesh`, the normalized temporary OBJ.
- For `--mesh`, the generated OSL scene template.

The final image is always written to `--captureFilename`.

## Troubleshooting

- `--material is required` or `--captureFilename is required`: both options are
  mandatory.
- `OSL compiler or testrender executable was not configured`: rebuild with
  `MATERIALX_OSL_BINARY_OSLC` and `MATERIALX_OSL_BINARY_TESTRENDER` set.
- `Could not find material file`: add the document directory or MaterialX root
  with `--path`, or run from a directory where the relative path resolves.
- `Could not find OSL utilities directory`: ensure the build copied
  `resources/Utilities`, or add a MaterialX root containing the utility shaders
  with `--path`.
- `Could not find mesh file`: add the mesh directory with `--path`, or pass an
  absolute path to `--mesh`.
