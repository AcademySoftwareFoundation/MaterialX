# MaterialX CMake Options

This file documents the MaterialX-specific CMake configuration options. Note that some options
force others on or off as noted in the descriptions below.

## Component Options

### `MATERIALX_BUILD_PYTHON` (default: `OFF`)

Build the MaterialX Python package. Requires [Python 2.7](https://www.python.org/) or greater.

The following additional options are only relevant when `MATERIALX_BUILD_PYTHON=ON`:

-   `MATERIALX_PYTHON_LTO` (default: `ON`): Enable link-time optimizations for MaterialX Python package.
-   `MATERIALX_INSTALL_PYTHON` (default: `ON`): Install the MaterialX Python package as a third-party library when the install target is built.

### `MATERIALX_BUILD_RUNTIME` (default: `OFF`)

Build the MaterialX Runtime data model library.

### `MATERIALX_BUILD_RENDER` (default: `ON`)

Build the MaterialX Render modules. This option replaces the deprecated `MATERIALX_DISABLE_BUILD_RENDER` option.

The [resources](resources) folder will be copied to the install location only if the Render modules are built.

The following additional options are only relevant when `MATERIALX_BUILD_RENDER=ON`:

-   `MATERIALX_TEST_RENDER` (default: `ON`): Run rendering tests. GPU required.
-   `MATERIALX_BUILD_VIEWER` (default: `ON`): Build the MaterialX Viewer. Requires the NanoGUI submodule to be present. Run `git submodule update --init --recursive` from the repository root to load it.
-   `MATERIALX_BUILD_CONTRIB` (default: `ON`): Build the [contributions folder](source/MaterialXContrib).
-   `MATERIALX_BUILD_OIIO` (default: `OFF`): Build OpenImageIO support for MaterialXRender. Requires [OpenImageIO](http://www.openimageio.org/).

The following options are affected when `MATERIALX_BUILD_RENDER=ON`:

-   `MATERIALX_BUILD_OSL`: forced to `ON`
-   `MATERIALX_BUILD_GLSL`: forced to `ON`

### `MATERIALX_BUILD_CROSS` (default: `OFF`)

Build the GLSL to HSLS cross-compiler. Requires [glslang](https://github.com/KhronosGroup/glslang) and spirv-cross (Autodesk internal fork; link omitted).

### `MATERIALX_BUILD_DOCS` (default: `OFF`)

Build the Doxygen docs. Requires [Doxygen](http://www.doxygen.nl/).

### `MATERIALX_BUILD_TESTS` (default: `ON`)

Build the unit tests. Only the unit tests for selected components will be built.

## Shader Generation Options

These options control building of shader code generators for each of the supported languages.

The [libraries](libraries) folder will be copied to the install location only if at least one code generator is built.

### `MATERIALX_BUILD_GEN_GLSL` (default: `ON`)

Build the GLSL shader generator back-end.

### `MATERIALX_BUILD_GEN_OSL` (default: `ON`)

Build the OSL shader generator back-end.

### `MATERIALX_BUILD_GEN_OGSXML` (default: `ON`)

Build the OGSXML shader generator back-end.

The following options are affected when `MATERIALX_BUILD_GEN_OGSXML=ON`:

-   `MATERIALX_BUILD_GEN_GLSL`: forced to `ON`

### `MATERIALX_BUILD_GEN_OGSFX` (default: `ON`)

Build the OGSFX shader generator back-end.

The following options are affected when `MATERIALX_BUILD_GEN_OGSFX=ON`:

-   `MATERIALX_BUILD_GEN_GLSL`: forced to `ON`

### `MATERIALX_BUILD_GEN_ARNOLD` (default: `ON`)

Build the Arnold OSL shader generator back-end.

The following options are affected when `MATERIALX_BUILD_GEN_ARNOLD=ON`:

-   `MATERIALX_BUILD_GEN_OSL`: forced to `ON`

### `MATERIALX_BUILD_JS` (default: `OFF`)

Build the MaterialX JavaScript package. Requires [Emscripten 1.39.7](https://emscripten.org/docs/getting_started/downloads.html) or greater.

The following additional options are only relevant when `MATERIALX_BUILD_JS=ON`:

-   `MATERIALX_EMSDK_PATH` (default: `OFF`): Path that is used when setting up the emscripten tool chain.

## Compiler Options

### `MATERIALX_WARNINGS_AS_ERRORS` (default: `OFF`)

Interpret all compiler warnings as errors.

## Other Parameters

There are additional parameters that may be overridden to influence the build. These are documented in [CMakeLists.txt](CMakeLists.txt).

-   `MATERIALX_PYTHON_VERSION`
-   `MATERIALX_PYTHON_EXECUTABLE`
-   `MATERIALX_PYTHON_OCIO_DIR`
-   `MATERIALX_PYTHON_PYBIND11_DIR`
-   `MATERIALX_OIIO_DIR`
-   `MATERIALX_INSTALL_INCLUDE_PATH`
-   `MATERIALX_INSTALL_LIB_PATH`
-   `MATERIALX_INSTALL_STDLIB_PATH`
-   `MATERIALX_OSLC_EXECUTABLE`
-   `MATERIALX_TESTRENDER_EXECUTABLE`
-   `MATERIALX_OSL_INCLUDE_PATH`
