//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderOsl/OslValidator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyOslValidator(py::module& mod)
{
    py::class_<mx::OslValidator, mx::ShaderValidator, mx::OslValidatorPtr>(mod, "OslValidator")
        .def_static("create", &mx::OslValidator::create)
        .def_readwrite_static("OSL_CLOSURE_COLOR_STRING", &mx::OslValidator::OSL_CLOSURE_COLOR_STRING)
        .def("initialize", &mx::OslValidator::initialize)
        .def("validateCreation", static_cast<void (mx::OslValidator::*)(const mx::ShaderPtr)>(&mx::OslValidator::validateCreation))
        .def("validateCreation", static_cast<void (mx::OslValidator::*)(const mx::OslValidator::StageMap&)>(&mx::OslValidator::validateCreation))
        .def("validateInputs", &mx::OslValidator::validateInputs)
        .def("validateRender", &mx::OslValidator::validateRender)
        .def("save", &mx::OslValidator::save)
        .def("setOslCompilerExecutable", &mx::OslValidator::setOslCompilerExecutable)
        .def("setOslIncludePath", &mx::OslValidator::setOslIncludePath)
        .def("setOslOutputFilePath", &mx::OslValidator::setOslOutputFilePath)
        .def("setShaderParameterOverrides", &mx::OslValidator::setShaderParameterOverrides)
        .def("setOslShaderOutput", &mx::OslValidator::setOslShaderOutput)
        .def("setOslTestShadeExecutable", &mx::OslValidator::setOslTestShadeExecutable)
        .def("setOslTestRenderExecutable", &mx::OslValidator::setOslTestRenderExecutable)
        .def("setOslTestRenderSceneTemplateFile", &mx::OslValidator::setOslTestRenderSceneTemplateFile)
        .def("setOslShaderName", &mx::OslValidator::setOslShaderName)
        .def("setOslUtilityOSOPath", &mx::OslValidator::setOslUtilityOSOPath)
        .def("useTestRender", &mx::OslValidator::useTestRender)
        .def("compileOSL", &mx::OslValidator::compileOSL);
}
