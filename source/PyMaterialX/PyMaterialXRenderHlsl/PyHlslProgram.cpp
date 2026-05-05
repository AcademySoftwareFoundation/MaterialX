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

    py::class_<mx::HlslProgram, mx::HlslProgramPtr>(mod, "HlslProgram")
        .def_static("create", &mx::HlslProgram::create)
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
