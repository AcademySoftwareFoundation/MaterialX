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
        .def_static("create", &mx::MslProgram::create)
        .def("setStages", &mx::MslProgram::setStages)
        .def("addStage", &mx::MslProgram::addStage)
        .def("getStageSourceCode", &mx::MslProgram::getStageSourceCode)
        .def("getShader", &mx::MslProgram::getShader)
        .def("build", &mx::MslProgram::build)
        .def("prepareUsedResources", &mx::MslProgram::prepareUsedResources)
        .def("getUniformsList", &mx::MslProgram::getUniformsList)
        .def("getAttributesList", &mx::MslProgram::getAttributesList)
        .def("findInputs", &mx::MslProgram::findInputs)
        .def("bind", &mx::MslProgram::bind)
        .def("bindUniform", &mx::MslProgram::bindUniform)
        .def("bindAttribute", &mx::MslProgram::bindAttribute)
        .def("bindPartition", &mx::MslProgram::bindPartition)
        .def("bindMesh", &mx::MslProgram::bindMesh)
        .def("unbindGeometry", &mx::MslProgram::unbindGeometry)
        .def("bindTextures", &mx::MslProgram::bindTextures)
        .def("bindLighting", &mx::MslProgram::bindLighting)
        .def("bindViewInformation", &mx::MslProgram::bindViewInformation)
        .def("bindTimeAndFrame", &mx::MslProgram::bindTimeAndFrame,
             py::arg("time") = 1.0f, py::arg("frame") = 1.0f);
    mod.attr("MslProgram").doc() = R"docstring(
    A class representing an executable MSL program.

    There are two main interfaces which can be used:
    one which takes in a HwShader and
    one which allows for explicit setting of shader stage code.)docstring";

    py::class_<mx::MslProgram::Input>(mod, "Input")
        .def_readwrite("location", &mx::MslProgram::Input::location)
        .def_readwrite("size", &mx::MslProgram::Input::size)
        .def_readwrite("typeString", &mx::MslProgram::Input::typeString)
        .def_readwrite("value", &mx::MslProgram::Input::value)
        .def_readwrite("isConstant", &mx::MslProgram::Input::isConstant)
        .def_readwrite("path", &mx::MslProgram::Input::path)
        .def(py::init([](int inputLocation,
                         int inputType,
                         int inputSize,
                         const std::string& inputPath)
            {
                return mx::MslProgram::Input(inputLocation, static_cast<MTLDataType>(inputType), inputSize, inputPath);
            }));
    mod.attr("Input").doc() = R"docstring(
    Structure to hold information about program inputs.

    The structure is populated by directly scanning the program so may not contain
    some inputs listed on any associated `HwShader` as those inputs may have been
    optimized out if they are unused.)docstring";
}
