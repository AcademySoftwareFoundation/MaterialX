//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderHlsl/HlslProgram.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHlslProgram(py::module& mod)
{
    py::enum_<mx::HlslCompilerBackend>(mod, "HlslCompilerBackend")
        .value("Fxc", mx::HlslCompilerBackend::Fxc)
        .value("Dxc", mx::HlslCompilerBackend::Dxc);

    py::enum_<mx::HlslResourceType>(mod, "HlslResourceType")
        .value("CBuffer", mx::HlslResourceType::CBuffer)
        .value("Texture", mx::HlslResourceType::Texture)
        .value("Sampler", mx::HlslResourceType::Sampler)
        .value("Other",   mx::HlslResourceType::Other);

    py::class_<mx::HlslResourceBinding>(mod, "HlslResourceBinding")
        .def_readwrite("name",  &mx::HlslResourceBinding::name)
        .def_readwrite("type",  &mx::HlslResourceBinding::type)
        .def_readwrite("slot",  &mx::HlslResourceBinding::slot)
        .def_readwrite("space", &mx::HlslResourceBinding::space)
        .def_readwrite("count", &mx::HlslResourceBinding::count);

    py::enum_<mx::HlslComponentType>(mod, "HlslComponentType")
        .value("Float", mx::HlslComponentType::Float)
        .value("SInt",  mx::HlslComponentType::SInt)
        .value("UInt",  mx::HlslComponentType::UInt);

    py::class_<mx::HlslInputParameter>(mod, "HlslInputParameter")
        .def_readonly("semanticName",   &mx::HlslInputParameter::semanticName)
        .def_readonly("semanticIndex",  &mx::HlslInputParameter::semanticIndex)
        .def_readonly("componentCount", &mx::HlslInputParameter::componentCount)
        .def_readonly("componentType",  &mx::HlslInputParameter::componentType);

    py::class_<mx::HlslStageReflection>(mod, "HlslStageReflection")
        .def("isValid",      &mx::HlslStageReflection::isValid)
        .def("getBindings",  &mx::HlslStageReflection::getBindings)
        .def("getInputs",    &mx::HlslStageReflection::getInputs)
        .def("findCbuffer",  &mx::HlslStageReflection::findCbuffer)
        .def("getSlotCount", &mx::HlslStageReflection::getSlotCount, py::arg("type"), py::arg("space") = 0);

    py::class_<mx::HlslProgram, mx::HlslProgramPtr>(mod, "HlslProgram")
        .def_static("create", &mx::HlslProgram::create)
        .def_static("isDxcAvailable", &mx::HlslProgram::isDxcAvailable)
        .def("getVertexReflection", &mx::HlslProgram::getVertexReflection)
        .def("getPixelReflection",  &mx::HlslProgram::getPixelReflection)
        .def("setCompilerBackend",  &mx::HlslProgram::setCompilerBackend)
        .def("getCompilerBackend",  &mx::HlslProgram::getCompilerBackend)
        .def("setShaderModel",      &mx::HlslProgram::setShaderModel)
        .def("getShaderModel",      &mx::HlslProgram::getShaderModel)
        .def("setEntryPoints",      &mx::HlslProgram::setEntryPoints)
        .def("build",               static_cast<bool (mx::HlslProgram::*)(mx::ShaderPtr)>(&mx::HlslProgram::build))
        .def("build",               static_cast<bool (mx::HlslProgram::*)(const std::string&, const std::string&)>(&mx::HlslProgram::build))
        .def("isValid",             &mx::HlslProgram::isValid)
        .def("getCompileLog",       &mx::HlslProgram::getCompileLog)
        .def("getVertexBindings",   &mx::HlslProgram::getVertexBindings)
        .def("getPixelBindings",    &mx::HlslProgram::getPixelBindings);
}
