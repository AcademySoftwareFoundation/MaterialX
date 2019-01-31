//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//
#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenOptions.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGenOptions(py::module& mod)
{
    py::enum_<mx::ShaderInterfaceType>(mod, "ShaderInterfaceType")
        .value("SHADER_INTERFACE_COMPLETE", mx::ShaderInterfaceType::SHADER_INTERFACE_COMPLETE)
        .value("SHADER_INTERFACE_REDUCED", mx::ShaderInterfaceType::SHADER_INTERFACE_REDUCED);

    py::class_<mx::GenOptions>(mod, "GenOptions")
        .def_readwrite("shaderInterfaceType", &mx::GenOptions::shaderInterfaceType)
        .def_readwrite("hwTransparency", &mx::GenOptions::hwTransparency)
        .def_readwrite("targetColorSpaceOverride", &mx::GenOptions::targetColorSpaceOverride)
        .def(py::init<>());
}
