//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GlslProgram.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslProgram(py::module& mod)
{
    py::class_<mx::GlslProgram, mx::GlslProgramPtr>(mod, "GlslProgram", "A class representing an executable GLSL program.\n\nThere are two main interfaces which can be used. One which takes in a HwShader and one which allows for explicit setting of shader stage code.")
        .def_readwrite_static("UNDEFINED_OPENGL_RESOURCE_ID", &mx::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID)
        .def_readwrite_static("UNDEFINED_OPENGL_PROGRAM_LOCATION", &mx::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION)
        .def_static("create", &mx::GlslProgram::create, "Create a GLSL program instance.")
        .def("setStages", &mx::GlslProgram::setStages, "Set up code stages to validate based on an input hardware shader.\n\nArgs:\n    shader: Hardware shader to use")
        .def("addStage", &mx::GlslProgram::addStage, "Set the code stages based on a list of stage strings.\n\nArgs:\n    stage: Name of the shader stage.\n    sourceCode: Source code of the shader stage.")
        .def("getStageSourceCode", &mx::GlslProgram::getStageSourceCode, "Get source code string for a given stage.\n\nReturns:\n    Shader stage string. String is empty if not found.")
        .def("getShader", &mx::GlslProgram::getShader, "Return the shader, if any, used to generate this program.")
        .def("build", &mx::GlslProgram::build, "Build shader program data from the source code set for each shader stage.\n\nAn exception is thrown if the program cannot be built. The exception will contain a list of compilation errors.")
        .def("hasBuiltData", &mx::GlslProgram::hasBuiltData, "Return true if built shader program data is present.")
        .def("clearBuiltData", &mx::GlslProgram::clearBuiltData, "")
        .def("getUniformsList", &mx::GlslProgram::getUniformsList, "Get list of program input uniforms.\n\nReturns:\n    Program uniforms list.")
        .def("getAttributesList", &mx::GlslProgram::getAttributesList, "Get list of program input attributes.\n\nReturns:\n    Program attributes list.")
        .def("findInputs", &mx::GlslProgram::findInputs, "Find the locations in the program which starts with a given variable name.\n\nArgs:\n    variable: Variable to search for\n    variableList: List of program inputs to search\n    foundList: Returned list of found program inputs. Empty if none found.\n    exactMatch: Search for exact variable name match.")
        .def("bind", &mx::GlslProgram::bind, "Bind the program.\n\nReturns:\n    False if failed")
        .def("hasActiveAttributes", &mx::GlslProgram::hasActiveAttributes, "Return true if the program has active attributes.")
        .def("bindUniform", &mx::GlslProgram::bindUniform, "Bind a value to the uniform with the given name.")
        .def("bindAttribute", &mx::GlslProgram::bindAttribute, "Bind attribute buffers to attribute inputs.\n\nArgs:\n    inputs: Attribute inputs to bind to\n    mesh: Mesh containing streams to bind")
        .def("bindPartition", &mx::GlslProgram::bindPartition, "Bind input geometry partition (indexing).")
        .def("bindMesh", &mx::GlslProgram::bindMesh, "Bind input geometry streams.")
        .def("unbindGeometry", &mx::GlslProgram::unbindGeometry, "Unbind any bound geometry.")
        .def("bindTextures", &mx::GlslProgram::bindTextures, "Bind any input textures.")
        .def("bindLighting", &mx::GlslProgram::bindLighting, "Bind lighting.")
        .def("bindViewInformation", &mx::GlslProgram::bindViewInformation, "Bind view information.")
        .def("bindTimeAndFrame", &mx::GlslProgram::bindTimeAndFrame, py::arg("time") = 0.0f, py::arg("frame") = 1.0f, "Bind time and frame.")
        .def("unbind", &mx::GlslProgram::unbind, "Unbind the program. Equivalent to binding no program.");

    py::class_<mx::GlslProgram::Input>(mod, "Input", "An input element within a Node or NodeDef.\n\nAn Input holds either a uniform value or a connection to a spatially-varying Output, either of which may be modified within the scope of a Material.")
        .def_readwrite_static("INVALID_OPENGL_TYPE", &mx::GlslProgram::Input::INVALID_OPENGL_TYPE)
        .def_readwrite("location", &mx::GlslProgram::Input::location)
        .def_readwrite("gltype", &mx::GlslProgram::Input::gltype)
        .def_readwrite("size", &mx::GlslProgram::Input::size)
        .def_readwrite("typeString", &mx::GlslProgram::Input::typeString)
        .def_readwrite("value", &mx::GlslProgram::Input::value)
        .def_readwrite("isConstant", &mx::GlslProgram::Input::isConstant)
        .def_readwrite("path", &mx::GlslProgram::Input::path)
        .def(py::init<int, int, int, std::string>());
}
