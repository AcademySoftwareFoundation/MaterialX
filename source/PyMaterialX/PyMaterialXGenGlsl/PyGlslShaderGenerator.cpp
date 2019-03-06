//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenGlsl/GlslShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyGlslShaderGenerator : public mx::GlslShaderGenerator
{
  public:
    using GlslShaderGenerator::GlslShaderGenerator;

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::GlslShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    const std::string& getLanguage() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getLanguage
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getTarget
        );
    }

    const std::string& getVersion() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::GlslShaderGenerator,
            getVersion
        );
    }
};

void bindPyGlslShaderGenerator(py::module& mod)
{
    py::class_<mx::GlslShaderGenerator, mx::ShaderGenerator, PyGlslShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator")
        .def_static("create", &mx::GlslShaderGenerator::create)
        .def(py::init([](){ return new PyGlslShaderGenerator(); }))
        .def("generate", &mx::GlslShaderGenerator::generate)
        .def("getLanguage", &mx::GlslShaderGenerator::getLanguage)
        .def("getTarget", &mx::GlslShaderGenerator::getTarget)
        .def("getVersion", &mx::GlslShaderGenerator::getVersion);
}
