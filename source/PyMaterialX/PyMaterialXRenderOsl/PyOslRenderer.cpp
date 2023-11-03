//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderOsl/OslRenderer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyOslRenderer(py::module& mod)
{
    py::class_<mx::OslRenderer, mx::ShaderRenderer, mx::OslRendererPtr>(mod, "OslRenderer")

        .def_static("create", &mx::OslRenderer::create,
                    py::arg("width") = 512,
                    py::arg("height") = 512,
                    py::arg_v("baseType",
                              mx::Image::BaseType::UINT8,
                              "BaseType.UINT8"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an OSL renderer instance for rendering an image of the specified
    resolution.
)docstring"))

        .def_readwrite_static("OSL_CLOSURE_COLOR_STRING",
                              &mx::OslRenderer::OSL_CLOSURE_COLOR_STRING,
                              PYMATERIALX_DOCSTRING(R"docstring(
    Color closure OSL string.
)docstring"))

        .def("initialize", &mx::OslRenderer::initialize,
             py::arg("renderContextHandle") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize the renderer.

    Takes care of the internal initialization required for program validation
    and rendering.

    :param renderContextHandle: Not currently used by the OSL renderer.
    :type renderContextHandle: RenderContextHandle
    :raises Exception: In case of initialization errors, listing them.
)docstring"))

        .def("createProgram",
             static_cast<void (mx::OslRenderer::*)(const mx::ShaderPtr)>(&mx::OslRenderer::createProgram),
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create OSL program based on an input shader.

    A valid executable and include path must be specified before calling this
    method.

    :see: `setOslCompilerExecutable()`
    :see: `setOslIncludePath()`

    Additionally, `setOslOutputFilePath()` should be set to allow for output of
    `.osl` and `.oso` files to the appropriate path location to be used as
    input for render validation.

    If render validation is not required, then the same temporary name will be
    used for all shaders validated using this method.

    :param shader: Input shader.
    :type shader: Shader
)docstring"))

        .def("createProgram",
             static_cast<void (mx::OslRenderer::*)(const mx::OslRenderer::StageMap&)>(&mx::OslRenderer::createProgram),
             py::arg("stages"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create OSL program based on shader stage source code.

    :param stages: Map of name and source code for the shader stages.
    :type stages: Dict[str, str]
)docstring"))

        .def("validateInputs", &mx::OslRenderer::validateInputs,
             PYMATERIALX_DOCSTRING(R"docstring(
    Validate inputs for the program.
)docstring"))

        .def("render", &mx::OslRenderer::render,
             PYMATERIALX_DOCSTRING(R"docstring(
    Render OSL program to disk.
    This is done by using either `testshade` or `testrender`.
    Currently only `testshade` is supported.

    Usage of both executables requires compiled source (`.oso`) files as input.
    A shader output must be set before running this test via the
    `setOslOutputName()` method to ensure that the appropriate `.oso` files can
    be located.
)docstring"))

        .def("captureImage", &mx::OslRenderer::captureImage,
             py::arg("image") = nullptr,
             PYMATERIALX_DOCSTRING(R"docstring(
    Capture the current rendered output as an image.
)docstring"))

        .def("setOslCompilerExecutable", &mx::OslRenderer::setOslCompilerExecutable,
             py::arg("executableFilePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the path to the OSL executable.

    Note that it is assumed that this references the location of the `oslc`
    executable.

    :param executableFilePath: Path to OSL compiler executable.
    :type executableFilePath: FilePath
)docstring"))

        .def("setOslIncludePath", &mx::OslRenderer::setOslIncludePath,
             py::arg("dirPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the search locations for OSL include files.

    :param dirPath: Include path(s) for the OSL compiler. This should include
        the path to `stdosl.h`.
    :type dirPath: FileSearchPath
)docstring"))

        .def("setOslOutputFilePath", &mx::OslRenderer::setOslOutputFilePath,
             py::arg("dirPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the location where compiled OSL files will reside.

    :param dirPath: Path to output location.
    :type dirPath: FilePath
)docstring"))

        .def("setShaderParameterOverrides", &mx::OslRenderer::setShaderParameterOverrides,
             py::arg("parameterOverrides"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set shader parameter strings to be added to the scene XML file.
    These strings will set parameter overrides for the shader.
)docstring"))

        .def("setOslShaderOutput", &mx::OslRenderer::setOslShaderOutput,
             py::arg("outputName"),
             py::arg("outputType"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the OSL shader output.

    This is used during render validation if `testshade` or `testrender` is
    executed.

    For `testrender` this value is used to replace the `%shader_output%` token
    in the input scene file.

    :param outputName: Name of shader output.
    :type outputName: str
    :param outputType: The MaterialX type of the output.
)docstring"))

        .def("setOslTestShadeExecutable", &mx::OslRenderer::setOslTestShadeExecutable,
             py::arg("executableFilePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the path to the OSL shading tester.

    Note that it is assumed that this references the location of the `testshade`
    executable.

    :param executableFilePath: Path to OSL `testshade` executable.
    :type executableFilePath: FilePath
)docstring"))

        .def("setOslTestRenderExecutable", &mx::OslRenderer::setOslTestRenderExecutable,
             py::arg("executableFilePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the path to the OSL rendering tester.

    Note that it is assumed that this references the location of the `testrender`
    executable.

    :param executableFilePath: Path to OSL `testrender` executable.
    :type executableFilePath: FilePath
)docstring"))

        .def("setOslTestRenderSceneTemplateFile", &mx::OslRenderer::setOslTestRenderSceneTemplateFile,
             py::arg("templateFilePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the XML scene file to use for `testrender`.

    This is a template file with the following tokens for replacement:
        - `%shader%` -- replaced with the name of the shader to use
        - `%shader_output%` -- replaced with the name of the shader output to use

    :param templateFilePath: Scene file name.
    :type templateFilePath: FilePath
)docstring"))

        .def("setOslShaderName", &mx::OslRenderer::setOslShaderName,
             py::arg("shaderName"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the name of the shader to be used for the input XML scene file.

    The value is used to replace the `%shader%` token in the file.

    :param shaderName: Name of shader.
    :type shaderName: str
)docstring"))

        .def("setOslUtilityOSOPath", &mx::OslRenderer::setOslUtilityOSOPath,
             py::arg("dirPath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the search path for dependent shaders (`.oso` files) which are used
    when rendering with `testrender`.

    :param dirPath: Path to location containing `.oso` files.
    :type dirPath: FilePath
)docstring"))

        .def("useTestRender", &mx::OslRenderer::useTestRender,
             py::arg("useTestRender"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Used to toggle to either use `testrender` or `testshade` during render
    validation.

    By default `testshade` is used.

    :param useTestRender: Indicate whether to use `testrender`.
)docstring"))

        .def("compileOSL", &mx::OslRenderer::compileOSL,
             py::arg("oslFilePath"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Compile OSL code stored in a file.

    :param oslFilePath: OSL file path.
    :type oslFilePath: FilePath
    :raises Exception: If an error occurs.
)docstring"))

        .doc() = R"docstring(
    Helper class for rendering generated OSL code to produce images.

    The main services provided are:
        - Source code validation: Use of the `oslc` executable to compile and
          test output results.
        - Introspection check: None at this time.
        - Binding: None at this time.
        - Render validation: Use of `testrender` to output rendered images.
          Assumes source compliation was successful, as it depends on the
          existence of corresponding `.oso` files.

    The path to the OSL compiler binary (e.g. `oslc.exe`) can be set via the
    `MATERIALX_OSL_BINARY_OSLC` build option.

    The path to the OSL test render binary (e.g. `testrender.exe`) can be set
    via the `MATERIALX_OSL_BINARY_TESTRENDER` build option.
)docstring";
}
