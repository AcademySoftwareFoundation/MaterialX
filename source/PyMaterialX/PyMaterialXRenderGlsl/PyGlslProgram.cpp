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
    py::class_<mx::GlslProgram, mx::GlslProgramPtr>(mod, "GlslProgram")

        .def_readwrite_static("UNDEFINED_OPENGL_RESOURCE_ID",
                              &mx::GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID,
                              PYMATERIALX_DOCSTRING(R"docstring(
    (int)
    A constant used to indicate that an OpenGL resource is not defined (yet).
)docstring"))

        .def_readwrite_static("UNDEFINED_OPENGL_PROGRAM_LOCATION",
                              &mx::GlslProgram::UNDEFINED_OPENGL_PROGRAM_LOCATION,
                              PYMATERIALX_DOCSTRING(R"docstring(
    (int)
    A constant used to indicate that an OpenGL program location is not defined
    (yet).
)docstring"))

        .def_static("create", &mx::GlslProgram::create,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create a GLSL program instance.
)docstring"))

        .def("setStages", &mx::GlslProgram::setStages,
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set up code stages to validate based on an input hardware shader.

    :param shader: Hardware shader to use.
    :type shader: Shader
)docstring"))

        .def("addStage", &mx::GlslProgram::addStage,
             py::arg("stage"),
             py::arg("sourceCode"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the code stages based on a list of stage strings.

    Refer to the ordering of stages as defined by a `HwShader`.

    :param stage: Name of the shader stage.
    :type stage: str
    :param sourceCode: Source code of the shader stage.
    :type sourceCode: str
)docstring"))

        .def("getStageSourceCode", &mx::GlslProgram::getStageSourceCode,
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return source code string for a given stage.

    :param stage: The name of the stage whose source code to return.
    :type stage: str
    :returns: The source code of the shader stage with the given `name`, or an
        empty string if not found.
)docstring"))

        .def("getShader", &mx::GlslProgram::getShader,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the shader, if any, used to generate this program.
)docstring"))

        .def("build", &mx::GlslProgram::build,
             PYMATERIALX_DOCSTRING(R"docstring(
    Build shader program data from the source code set for each shader stage.

    :raises Exception: If the program cannot be built, containing a list of
        compilation errors.
)docstring"))

        .def("hasBuiltData", &mx::GlslProgram::hasBuiltData,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if built shader program data is present.
)docstring"))

        .def("clearBuiltData", &mx::GlslProgram::clearBuiltData,
             PYMATERIALX_DOCSTRING(R"docstring(
    Clear built shader program data, if any.
)docstring"))

        .def("getUniformsList", &mx::GlslProgram::getUniformsList,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return list of program input uniforms.

    The program must have been created successfully first.

    :returns: Program uniforms list.
    :raises Exception: If the parsing of the program for uniforms cannot be
        performed.
)docstring"))

        .def("getAttributesList", &mx::GlslProgram::getAttributesList,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return list of program input attributes.

    The program must have been created successfully first.

    :returns: Program attributes list.
    :raises Exception: If the parsing of the program for attributes cannot be
        performed.
)docstring"))

        .def("findInputs", &mx::GlslProgram::findInputs,
             py::arg("variable"),
             py::arg("variableInputs"),
             py::arg("foundInputs"),
             py::arg("exactMatch"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Find the locations in the program which start with a given variable name.

    :param variable: Name of variable to search for.
    :type variable: str
    :param variableList: Map of program inputs to search.
    :type variableList: Dict[str, Input]
    :param foundList: Returned map of found program inputs. Empty if none found.
    :type foundList: Dict[str, Input]
    :param exactMatch: Search for exact variable name match.
    :type exactMatch: bool
)docstring"))

        .def("bind", &mx::GlslProgram::bind,
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind the program.

    :returns: `False` if failed.
)docstring"))

        .def("hasActiveAttributes", &mx::GlslProgram::hasActiveAttributes,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return `True` if the program has active attributes.
)docstring"))

        .def("bindUniform", &mx::GlslProgram::bindUniform,
             py::arg("name"),
             py::arg("value"),
             py::arg("errorIfMissing") = true,
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind a value to the uniform with the given `name`.
)docstring"))

        .def("bindAttribute", &mx::GlslProgram::bindAttribute,
             py::arg("inputs"),
             py::arg("mesh"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind attribute buffers to attribute inputs.

    A hardware buffer of the given attribute type is created and bound to the
    program locations for the input attribute.

    :param inputs: Attribute inputs to bind to.
    :type inputs: Dict[str, Input]
    :param mesh: Mesh containing streams to bind.
    :type mesh: Mesh
)docstring"))

        .def("bindPartition", &mx::GlslProgram::bindPartition,
             py::arg("partition"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind input geometry partition (indexing).
)docstring"))

        .def("bindMesh", &mx::GlslProgram::bindMesh,
             py::arg("mesh"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind input geometry streams.
)docstring"))

        .def("unbindGeometry", &mx::GlslProgram::unbindGeometry,
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind any bound geometry
)docstring"))

        .def("bindTextures", &mx::GlslProgram::bindTextures,
             py::arg("imageHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind any input textures
)docstring"))

        .def("bindLighting", &mx::GlslProgram::bindLighting,
             py::arg("lightHandler"),
             py::arg("imageHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind lighting.
)docstring"))

        .def("bindViewInformation", &mx::GlslProgram::bindViewInformation,
             py::arg("camera"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind view information.
)docstring"))

        .def("bindTimeAndFrame", &mx::GlslProgram::bindTimeAndFrame,
             py::arg("time") = 1.0f,
             py::arg("frame") = 1.0f,
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind time and frame.
)docstring"))

        .def("unbind", &mx::GlslProgram::unbind,
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind the program. Equivalent to binding no program.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an executable GLSL program.

    There are two main interfaces which can be used: one which takes in a
    `HwShader`, and one which allows for explicit setting of shader stage code.

    :see: https://materialx.org/docs/api/class_glsl_program.html
)docstring");

    py::class_<mx::GlslProgram::Input>(mod, "Input")

        .def_readwrite_static("INVALID_OPENGL_TYPE",
                              &mx::GlslProgram::Input::INVALID_OPENGL_TYPE,
                              PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    Constant indicating an invalid OpenGL type.
)docstring"))

        .def_readwrite("location", &mx::GlslProgram::Input::location,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    Program location. `-1` means an invalid location.
)docstring"))

        .def_readwrite("gltype", &mx::GlslProgram::Input::gltype,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    OpenGL type of the input. `-1` means an invalid type.
)docstring"))

        .def_readwrite("size", &mx::GlslProgram::Input::size,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    The size of the input.
)docstring"))

        .def_readwrite("typeString", &mx::GlslProgram::Input::typeString,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Input type string. Will only be non-empty if initialized stages with a
    `HwShader`.
)docstring"))

        .def_readwrite("value", &mx::GlslProgram::Input::value,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`Value`)
    Input value. Will only be non-empty if initialized stages with a `HwShader`
    and a value was set during shader generation.
)docstring"))

        .def_readwrite("isConstant", &mx::GlslProgram::Input::isConstant,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Flag that states whether this input is a constant.
)docstring"))

        .def_readwrite("path", &mx::GlslProgram::Input::path,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Element path (if any).
)docstring"))

        .def_readwrite("unit", &mx::GlslProgram::Input::unit,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Unit.
)docstring"))

        .def_readwrite("colorspace", &mx::GlslProgram::Input::colorspace,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Colorspace.
)docstring"))

        .def(py::init<int, int, int, std::string>(),
             py::arg("location"),
             py::arg("gltype"),
             py::arg("size"),
             py::arg("path"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a structure to hold information about program inputs.

    The structure is populated by directly scanning the program so may not contain
    some inputs listed on any associated `HwShader` as those inputs may have been
    optimized out if they are unused.

    :see: https://materialx.org/docs/api/struct_glsl_program_1_1_input.html
)docstring");
}
