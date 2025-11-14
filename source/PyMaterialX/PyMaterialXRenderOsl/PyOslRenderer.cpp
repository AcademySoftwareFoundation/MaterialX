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
    py::class_<mx::OslRenderer, mx::ShaderRenderer, mx::OslRendererPtr>(mod, "OslRenderer", "Helper class for rendering generated OSL code to produce images.\n\nThe main services provided are: Source code validation: Use of \"oslc\" to compile and test output results Introspection check: None at this time. Binding: None at this time. Render validation: Use of \"testrender\" to output rendered images. Assumes source compilation was success as it depends on the existence of corresponding .oso files.")
        .def_static("create", &mx::OslRenderer::create)
        .def_readwrite_static("OSL_CLOSURE_COLOR_STRING", &mx::OslRenderer::OSL_CLOSURE_COLOR_STRING)
        .def("initialize", &mx::OslRenderer::initialize, py::arg("renderContextHandle") = nullptr, "Initialize with the given implementation element.\n\nInitialization must set the name and hash for the implementation, as well as any other data needed to emit code for the node.")
        .def("createProgram", static_cast<void (mx::OslRenderer::*)(const mx::ShaderPtr)>(&mx::OslRenderer::createProgram), "Create GLSL program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("createProgram", static_cast<void (mx::OslRenderer::*)(const mx::OslRenderer::StageMap&)>(&mx::OslRenderer::createProgram), "Create GLSL program based on shader stage source code.\n\nArgs:\n    stages: Map of name and source code for the shader stages.")
        .def("validateInputs", &mx::OslRenderer::validateInputs, "Validate inputs for the program.")
        .def("render", &mx::OslRenderer::render, "Render the current program to an offscreen buffer.")
        .def("captureImage", &mx::OslRenderer::captureImage, "Capture the current contents of the off-screen hardware buffer as an image.")
        .def("setOslCompilerExecutable", &mx::OslRenderer::setOslCompilerExecutable, "Set the path to the OSL executable.\n\nArgs:\n    executableFilePath: Path to OSL compiler executable")
        .def("setOslIncludePath", &mx::OslRenderer::setOslIncludePath, "Set the search locations for OSL include files.\n\nArgs:\n    dirPath: Include path(s) for the OSL compiler. This should include the path to stdosl.h.")
        .def("setOslOutputFilePath", &mx::OslRenderer::setOslOutputFilePath, "Set the location where compiled OSL files will reside.\n\nArgs:\n    dirPath: Path to output location")
        .def("setShaderParameterOverrides", &mx::OslRenderer::setShaderParameterOverrides, "Set shader parameter strings to be added to the scene XML file.\n\nThese strings will set parameter overrides for the shader.")
        .def("setOslShaderOutput", &mx::OslRenderer::setOslShaderOutput, "Set the OSL shader output.\n\nArgs:\n    outputName: Name of shader output\n    outputType: The MaterialX type of the output")
        .def("setOslTestShadeExecutable", &mx::OslRenderer::setOslTestShadeExecutable, "Set the path to the OSL shading tester.\n\nArgs:\n    executableFilePath: Path to OSL \"testshade\" executable")
        .def("setOslTestRenderExecutable", &mx::OslRenderer::setOslTestRenderExecutable, "Set the path to the OSL rendering tester.\n\nArgs:\n    executableFilePath: Path to OSL \"testrender\" executable")
        .def("setOslTestRenderSceneTemplateFile", &mx::OslRenderer::setOslTestRenderSceneTemplateFile, "Set the XML scene file to use for testrender.\n\nThis is a template file with the following tokens for replacement: shader% : which will be replaced with the name of the shader to use shader_output% : which will be replace with the name of the shader output to use templateFilePath Scene file name\n\nArgs:\n    templateFilePath: Scene file name")
        .def("setOslShaderName", &mx::OslRenderer::setOslShaderName, "Set the name of the shader to be used for the input XML scene file.\n\nArgs:\n    shaderName: Name of shader")
        .def("setOslUtilityOSOPath", &mx::OslRenderer::setOslUtilityOSOPath, "Set the search path for dependent shaders (.oso files) which are used when rendering with testrender.\n\nArgs:\n    dirPath: Path to location containing .oso files.")
        .def("useTestRender", &mx::OslRenderer::useTestRender, "Used to toggle to either use testrender or testshade during render validation By default testshade is used.\n\nArgs:\n    useTestRender: Indicate whether to use testrender.")
        .def("compileOSL", &mx::OslRenderer::compileOSL, "Compile OSL code stored in a file.\n\nArgs:\n    oslFilePath: OSL file path.");
}
