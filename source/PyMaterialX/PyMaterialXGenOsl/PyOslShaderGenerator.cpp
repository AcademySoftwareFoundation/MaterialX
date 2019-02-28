//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenOsl/OslShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyOslShaderGenerator : public mx::OslShaderGenerator
{
  public:
    using OslShaderGenerator::OslShaderGenerator;

    PyOslShaderGenerator() :
        mx::OslShaderGenerator()
    {
    }

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::OslShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::OslShaderGenerator,
            getTarget
        );
    }

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::OslShaderGenerator,
            getLanguage
        );
    }
};

void bindPyOslShaderGenerator(py::module& mod)
{
    py::class_<mx::OslShaderGenerator, mx::ShaderGenerator, PyOslShaderGenerator, mx::OslShaderGeneratorPtr>(mod, "OslShaderGenerator")
        .def_static("create", &mx::OslShaderGenerator::create)
        .def(py::init([](){ return new PyOslShaderGenerator(); }))
        .def("generate", &mx::OslShaderGenerator::generate)
        .def("getLanguage", &mx::OslShaderGenerator::getLanguage);
}
