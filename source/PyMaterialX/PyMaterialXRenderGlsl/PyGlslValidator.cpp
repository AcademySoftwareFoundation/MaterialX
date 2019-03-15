//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRenderGlsl/GlslValidator.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyGlslValidator(py::module& mod)
{
    py::class_<mx::GlslValidator, mx::ShaderValidator, mx::GlslValidatorPtr>(mod, "GlslValidator")
        .def_static("create", &mx::GlslValidator::create)
        .def("initialize", &mx::GlslValidator::initialize)
        .def("validateCreation", static_cast<void (mx::GlslValidator::*)(const mx::ShaderPtr)>(&mx::GlslValidator::validateCreation))
        .def("validateCreation", static_cast<void (mx::GlslValidator::*)(const mx::GlslValidator::StageMap&)>(&mx::GlslValidator::validateCreation))
        .def("validateInputs", &mx::GlslValidator::validateInputs)
        .def("validateRender", &mx::GlslValidator::validateRender)
        .def("save", &mx::GlslValidator::save)
        .def("program", &mx::GlslValidator::program);
}
