//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderMsl/MslPipelineStateObject.h>
#include <MaterialXRenderMsl/MetalFramebuffer.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyMslProgram(py::module& mod)
{
    py::class_<mx::MslProgram, mx::MslProgramPtr>(mod, "MslProgram")

        .def_static("create", &mx::MslProgram::create,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create an MSL program instance.
)docstring"))

        .def("setStages", &mx::MslProgram::setStages,
             py::arg("shader"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set up code stages to validate based on an input hardware shader.

    :param shader: Hardware shader to use.
    :type shader: Shader
)docstring"))

        .def("addStage", &mx::MslProgram::addStage,
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

        .def("getStageSourceCode", &mx::MslProgram::getStageSourceCode,
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return source code string for a given stage.

    :param stage: The name of the stage whose source code to return.
    :type stage: str
    :returns: The source code of the shader stage with the given `name`, or an
        empty string if not found.
)docstring"))

        .def("getShader", &mx::MslProgram::getShader,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the shader, if any, used to generate this program.
)docstring"))

        .def("build", &mx::MslProgram::build,
             py::arg("device"),
             py::arg("framebuffer"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Create the pipeline state object from stages specified.

    :param device: MetalDevice that pipeline state object is being created on.
    :param framebuffer: Specifies information about output frame buffer.
    :raises Exception: If the program cannot be created, listing program
        creation errors.
    :returns: Pipeline State Object identifier.
)docstring"))

        .def("prepareUsedResources", &mx::MslProgram::prepareUsedResources,
             py::arg("renderCmdEncoder"),
             py::arg("cam"),
             py::arg("geometryHandler"),
             py::arg("imageHandler"),
             py::arg("lightHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind inputs.

    :param renderCmdEncoder: Encoder that inputs will be bound to.
    :param cam: Camera object used to view the object.
    :param geometryHandler: Not currently used.
    :param imageHandler: The image handler to use for lighting and textures.
    :param lightHandler: The light handler to use for lighting and uniform
        buffers.
)docstring"))

        .def("getUniformsList", &mx::MslProgram::getUniformsList,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return list of program input uniforms.

    The program must have been created successfully first.

    :returns: Program uniforms list.
    :raises Exception: If the parsing of the program for uniforms cannot be
        performed.
)docstring"))

        .def("getAttributesList", &mx::MslProgram::getAttributesList,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return list of program input attributes.

    The program must have been created successfully first.

    :returns: Program attributes list.
    :raises Exception: If the parsing of the program for attributes cannot be
        performed.
)docstring"))

        .def("findInputs", &mx::MslProgram::findInputs,
             py::arg("variable"),
             py::arg("variableList"),
             py::arg("foundList"),
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

        .def("bind", &mx::MslProgram::bind,
             py::arg("renderCmdEncoder"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind the pipeline state object to the command encoder.

    :param renderCmdEncoder: Metal Render Command Encoder to bind the pipeline
        state object.
    :type renderCmdEncoder: MTLRenderCommandEncoder
    :returns: `False` if failed.
)docstring"))

        .def("bindUniform", &mx::MslProgram::bindUniform,
             py::arg("name"),
             py::arg("value"),
             py::arg("errorIfMissing"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind a value to the uniform with the given `name`.
)docstring"))

        .def("bindAttribute", &mx::MslProgram::bindAttribute,
             py::arg("renderCmdEncoder"),
             py::arg("inputs"),
             py::arg("mesh"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind attribute buffers to attribute inputs.

    A hardware buffer of the given attribute type is created and bound to the
    program locations for the input attribute.

    :param renderCmdEncoder: Metal Render Command Encoder on which to set
        vertex buffers.
    :type renderCmdEncoder: MTLRenderCommandEncoder
    :param inputs: Attribute inputs to bind to.
    :type inputs: Dict[str, Input]
    :param mesh: Mesh containing streams to bind.
    :type mesh: Mesh
)docstring"))

        .def("bindPartition", &mx::MslProgram::bindPartition,
             py::arg("part"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind input geometry partition (indexing)
)docstring"))

        .def("bindMesh", &mx::MslProgram::bindMesh,
             py::arg("renderCmdEncoder"),
             py::arg("mesh"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind input geometry streams.
)docstring"))

        .def("unbindGeometry", &mx::MslProgram::unbindGeometry,
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind any bound geometry.
)docstring"))

        .def("bindTextures", &mx::MslProgram::bindTextures,
             py::arg("renderCmdEncoder"),
             py::arg("lightHandler"),
             py::arg("imageHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind any input textures.
)docstring"))

        .def("bindLighting", &mx::MslProgram::bindLighting,
             py::arg("lightHandler"),
             py::arg("imageHandler"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind lighting.
)docstring"))

        .def("bindViewInformation", &mx::MslProgram::bindViewInformation,
             py::arg("camera"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind view information.
)docstring"))

        .def("bindTimeAndFrame", &mx::MslProgram::bindTimeAndFrame,
             py::arg("time") = 1.0f,
             py::arg("frame") = 1.0f,
             PYMATERIALX_DOCSTRING(R"docstring(
    Bind time and frame.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing an executable MSL program.

    There are two main interfaces which can be used: one which takes in a
    `HwShader`, and one which allows for explicit setting of shader stage code.
)docstring");

    py::class_<mx::MslProgram::Input>(mod, "Input")

        .def_readwrite("location", &mx::MslProgram::Input::location,
             PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    Program location. `-1` means an invalid location.
)docstring"))

        .def_readwrite("resourceType", &mx::MslProgram::Input::resourceType,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    Metal type of the input. `-1` means an invalid type.
)docstring"))

        .def_readwrite("size", &mx::MslProgram::Input::size,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`int`)
    The size of the input.
)docstring"))

        .def_readwrite("typeString", &mx::MslProgram::Input::typeString,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Input type string. Will only be non-empty if initialized stages with a
    `HwShader`.
)docstring"))

        .def_readwrite("value", &mx::MslProgram::Input::value,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`Value`)
    Input value. Will only be non-empty if initialized stages with a `HwShader`
    and a value was set during shader generation.
)docstring"))

        .def_readwrite("isConstant", &mx::MslProgram::Input::isConstant,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`bool`)
    Flag that states whether this input is a constant.
)docstring"))

        .def_readwrite("path", &mx::MslProgram::Input::path,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Element path (if any).
)docstring"))

        .def_readwrite("unit", &mx::MslProgram::Input::unit,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Unit.
)docstring"))

        .def_readwrite("colorspace", &mx::MslProgram::Input::colorspace,
                       PYMATERIALX_DOCSTRING(R"docstring(
    (`str`)
    Colorspace.
)docstring"))

        .def(py::init<int, int, int, std::string>(),
             py::arg("location"),
             py::arg("resourceType"),
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
)docstring");
}
