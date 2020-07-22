# Unit tests

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
- Observer.cpp : Document observation.
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
- GenOgsfx.cpp : OGSFX shader generation tests which are run when the test tag `[genogsfx]` is specified.
- GenArnold.cpp :  OSL shader generation tests with Arnold target which are run when the test tag `[genarnold]` is specified.
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
- RenderOgsFx.cpp : OGSFX render tests which are run when the test tag `[renderglsl]` is specified. Currently no validator exists to perform compilation and rendering. Only shader generation is performed.
- RenderArnold.cpp : OGSFX render tests which are run when the test tag `[renderosl]` is specified. Currently no validator exists to perform compilation and rendering. Only shader generation is performed.

Per language tests will scan MaterialX files in the test suite for input Elements.

#### Per Language Render Setup

If rendering tests are enabled via the build options then code for each Element tested will be compiled and rendered if the appropriate backend support is available.
- `GLSL`:
    - Will execute on a Windows machine which supports OpenGL 4.0 or above.
- `OSL`: Uses the utilities `oslc` and `testrender` utilities from the
    [OSL distribution](https://github.com/imageworks/OpenShadingLanguage).
    - The utilities are not generated as part of the MaterialX build.
    - The test suite has been tested with version 1.9.10.
    - The following build options are required to be set:
        - `MATERIALX_OSLC_EXECUTABLE`: Full path to the `oslc` binary.
        - `MATERIALX_TESTRENDER_EXECUTABLE`: Full path to the `testrender` binary.
        - `MATERIALX_OSL_INCLUDE_PATH`: Full path to OSL include paths (i.e. location of `stdosl.h`).
- `MDL` : Uses the utility `mdlc` from the MDL SDK distribution available from [MDL distribution](https://developer.nvidia.com/mdl-sdk)
    - The test suite has been tested with mdlc version 1.0 build 327300.6313.
    - The following build options are require to be set:
        - `MATERIALX_MDLC_EXECUTABLE`: Full path to the `mdlc` binary. 

#### Test Outputs

- `gen<language>_<target>_render_doc_validation_log.txt`: Contains a log of whether input document validation check errors for a language and target pair.
- `gen<language>_<target>_render_profiling_log.txt`: Contains a log of execution times for a give language and target pair.
- `gen<language>_<target>_render_log.txt`: Contains a log of compilation and rendering checks for a language and target pair. Note that if an error occurred a reference to a per Element error file will be given.

##### HTML Preview Script
- A Python utility is provided in [`python/MaterialXTest`](../../python/MaterialXTest) which can be run to generate an HTML file with the rendered images referenced for visual checking. It assumes that both GLSL and OSL rendering has been performed.
