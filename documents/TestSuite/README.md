# Render Test Suite

The sub-folders in the test suite contain a set of input MaterialX documents. During execution of the test suite, each file is parsed to determine renderable elements.  For each element the appropriate shader generator is used to produced source code. The source code is then compiled, and/or rendered as part of the validation test.
For the GLSL and OSL shading languages: compiled code and renders are produced. For OGSFX on the code generation step is performed.

## Folder layout

- At the top level, the [options file (_options.mtlx)](_options.mtlx) is a MaterialX document defines the set of test execution options. The values may be edited locally as desired.
- The main grouping of input files is by library: ([stdlib](stdlib) and [pbrlib](pbrlib)).
- Additional sub-folders group documents based on Element group or category. For example math tests are found in [stdlib/math](stdlib/math)), with:
    - `math.mtlx`
    - `math_operators.mtlx`
    - `transform.mtlx`
    - `trig.mtlx`, and
    - `vector_math.mtlx`

  documents containing the various Elements to test.
- It is possible to add additional tests by simply adding new MaterialX documents under the TestSuite sub-folder.
- The [Geometry](Geometry) and [Images](Images) folders provide stock input geometry and images for usage by the test suite.
- The [Utilities folder](Utilities) provides utilities used for rendering with `testrender` as well as the light configuration specification for hardware rendering.

For details on how to build the unit test module to use this test suite see the [source documentation](../../source/MaterialXTest/README.md).
