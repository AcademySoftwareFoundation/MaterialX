//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
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
        .value("SHADER_INTERFACE_REDUCED", mx::ShaderInterfaceType::SHADER_INTERFACE_REDUCED)
        .export_values();

    py::enum_<mx::HwSpecularEnvironmentMethod>(mod, "HwSpecularEnvironmentMethod")
        .value("SPECULAR_ENVIRONMENT_PREFILTER", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_PREFILTER)
        .value("SPECULAR_ENVIRONMENT_FIS", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_FIS)
        .value("SPECULAR_ENVIRONMENT_NONE", mx::HwSpecularEnvironmentMethod::SPECULAR_ENVIRONMENT_NONE)
        .export_values();

    py::class_<mx::GenOptions>(mod, "GenOptions")
        .def_readwrite("shaderInterfaceType", &mx::GenOptions::shaderInterfaceType)
        .def_readwrite("fileTextureVerticalFlip", &mx::GenOptions::fileTextureVerticalFlip)
        .def_readwrite("targetColorSpaceOverride", &mx::GenOptions::targetColorSpaceOverride)
        .def_readwrite("targetDistanceUnit", &mx::GenOptions::targetDistanceUnit)
        .def_readwrite("hwTransparency", &mx::GenOptions::hwTransparency)
        .def_readwrite("hwSpecularEnvironmentMethod", &mx::GenOptions::hwSpecularEnvironmentMethod)
        .def_readwrite("hwMaxActiveLightSources", &mx::GenOptions::hwMaxActiveLightSources)
        .def_readwrite("hwNormalizeUdimTexCoords", &mx::GenOptions::hwNormalizeUdimTexCoords)
        .def_readwrite("hwWriteAlbedoTable", &mx::GenOptions::hwWriteAlbedoTable)
        .def_readwrite("emitColorTransforms", &mx::GenOptions::emitColorTransforms)
        .def(py::init<>());
}
