//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXRender/ShaderValidator.h>

namespace py = pybind11;
namespace mx = MaterialX;


class PyShaderValidator : public mx::ShaderValidator
{
  public:
    PyShaderValidator() :
        mx::ShaderValidator()
    {
    }

    void initialize() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            initialize
        );
    }

    void validateCreation(const mx::ShaderPtr shader) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            validateCreation,
            shader
        );
    }

    void validateCreation(const StageMap& stages) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            validateCreation,
            stages
        );
    }

    void validateInputs() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            validateInputs
        );
    }

    void validateRender() override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            validateRender
        );
    }

    void save(const mx::FilePath& filePath, bool floatingPoint) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            mx::ShaderValidator,
            save,
            filePath,
            floatingPoint
        );
    }
};

void bindPyShaderValidator(py::module& mod)
{
    py::class_<mx::ShaderValidator, PyShaderValidator, mx::ShaderValidatorPtr>(mod, "ShaderValidator")
        .def("initialize", &mx::ShaderValidator::initialize)
        .def("setImageHandler", &mx::ShaderValidator::setImageHandler)
        .def("getImageHandler", &mx::ShaderValidator::getImageHandler)
        .def("setLightHandler", &mx::ShaderValidator::setLightHandler)
        .def("getLightHandler", &mx::ShaderValidator::getLightHandler)
        .def("getGeometryHandler", &mx::ShaderValidator::getGeometryHandler)
        .def("setViewHandler", &mx::ShaderValidator::setViewHandler)
        .def("getViewHandler", &mx::ShaderValidator::getViewHandler)
        .def("validateCreation", static_cast<void (mx::ShaderValidator::*)(const mx::ShaderPtr)>(&mx::ShaderValidator::validateCreation))
        .def("validateCreation", static_cast<void (mx::ShaderValidator::*)(const mx::ShaderValidator::StageMap&)>(&mx::ShaderValidator::validateCreation))
        .def("validateInputs", &mx::ShaderValidator::validateInputs)
        .def("validateRender", &mx::ShaderValidator::validateRender)
        .def("save", &mx::ShaderValidator::save);
}
