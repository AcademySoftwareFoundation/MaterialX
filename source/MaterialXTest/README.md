# MaterialX Unit Tests

Unit tests can be run using the `MaterialXTest` executable. This can be performed using ctest or directly running the executable with the desired test tag(s). The default build options executes all tags except the ones using for shader compilation and rendering.

## 1. Core Tests

The names of the files reflect the Element type being tested for the following:

- Document.cpp
- Element.cpp
- Geom.cpp
- Look.cpp
- Material.cpp
- Node.cpp
- Types.cpp
- Value.cpp

## 2. Document utilities
- Traversal.cpp : Document traversal.
- Util.cpp : Basic utilities.

## 3. I/O Tests

- File.cpp : Basic file path tests.
- XmlIo.cpp : XML document I/O tests.

## 4. Render Test Suite

### 4.1 Test Inputs

Refer to the [test suite documentation](../../resources/Materials/TestSuite/README.md) for more information about the organization of the test suite data used for these tests.

### 4.2 Shader Generation Tests

- GenShader.cpp : Core shader generation tests which are run when the test tag `[genshader]` is specified.
- GenOsl.cpp : OSL shader generation tests which are run when the test tag `[genosl]` is specified.
- GenGlsl.cpp : GLSL shader generation tests which are run when the test tag `[genglsl]` is specified.
- GenMdl.cpp : MDL shader generation tests which are run when the test tag `[genmdl]` is specified.

Per language tests will scan MaterialX files in the test suite for input Elements.

#### Test Outputs
Depending on which tests are executed log files are produced at the location that MaterialXTest was executed.

- `gen<language>_<target>_generatetest.txt`: Contains a log of generation for a give language and target pair.
- `gen<language>_<target>_implementation_check.txt`: Contains a log of whether implementations exist for all nodedefs for a given language and target pair.

### 4.3 Rendering Tests

- Render.cpp : Core render tests which are run when the test tag `[rendercore]` is specified.
- RenderOsl.cpp : OSL render tests which are run when the test tag `[renderosl]` is specified.
- RenderGlsl.cpp : GLSL render tests which are run when the test tag `[renderglsl]` is specified.

Per language tests will scan MaterialX files in the test suite for input Elements.

#### Per Language Render Setup

If rendering tests are enabled via the build options then code for each Element tested will be compiled and rendered if the appropriate backend support is available.
- `GLSL`:
    - Will execute on a Windows machine which supports OpenGL 4.0 or above.
- `OSL`: Uses utilities from the
    [OSL distribution](https://github.com/imageworks/OpenShadingLanguage).
    - The utilities are not generated as part of the MaterialX build.
    - The test suite has been tested with version 1.9.10.
    - The following build options are required to be set:
        - `MATERIALX_OSLC_EXECUTABLE`: Full path to the `oslc` binary.
        - `MATERIALX_TESTRENDER_EXECUTABLE`: Full path to the `testrender` binary.
        - `MATERIALX_OSL_INCLUDE_PATH`: Full path to OSL include paths (i.e. location of `stdosl.h`).
- `MDL` : Uses the utility `mdlc` from the [MDL distribution](https://github.com/NVIDIA/MDL-SDK) Prebuilt binaries can be downloaded from [here](https://developer.nvidia.com/mdl-sdk).
    - The recommended MDL version is 1.6. The minimal support version is: [2019.2 (325000.1814)](https://github.com/NVIDIA/MDL-SDK/releases/tag/2019.2)
    - The following build options are require to be set:
        - `MATERIALX_MDLC_EXECUTABLE`: Full path to the `mdlc` binary for compilation testing.
        - `MATERIALX_MDL_RENDER_EXECUTABLE`: Full path to the binary for render testing.
           The build option `MATERIALX_MDL_RENDER_ARGUMENTS` should be set to provide command line arguments 
           for non-interactive rendering.
    - Note that if a render executable is specified separate compilation testing using `mdlc` will not be performed.

#### Test Outputs

- `gen<language>_<target>_render_doc_validation_log.txt`: Contains a log of whether input document validation check errors for a language and target pair.
- `gen<language>_<target>_render_profiling_log.txt`: Contains a log of execution times for a give language and target pair.
- `gen<language>_<target>_render_log.txt`: Contains a log of compilation and rendering checks for a language and target pair. Note that if an error occurred a reference to a per Element error file will be given.

##### HTML Preview Script
- A Python utility is provided in [`python/MaterialXTest`](../../python/MaterialXTest) which can be run to generate an HTML file with the rendered images referenced for visual checking. It assumes that both GLSL and OSL rendering has been performed.
