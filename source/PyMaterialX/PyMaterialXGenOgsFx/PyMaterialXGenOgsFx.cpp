//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenOgsFx/OgsFxShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyOgsFxShaderGenerator : public mx::OgsFxShaderGenerator
{
  public:
    using OgsFxShaderGenerator::OgsFxShaderGenerator;

    mx::ShaderPtr generate(const std::string& shaderName, mx::ElementPtr element, const mx::GenOptions& options) override
    {
        PYBIND11_OVERLOAD(
            mx::ShaderPtr,
            mx::OgsFxShaderGenerator,
            generate,
            shaderName,
            element,
            options
        );
    }

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD(
            const std::string&,
            mx::OgsFxShaderGenerator,
            getTarget
        );
    }
};

void bindPyOgsFxShaderGenerator(py::module& mod)
{
    py::class_<mx::OgsFxShaderGenerator, mx::ShaderGenerator, PyOgsFxShaderGenerator, mx::OgsFxShaderGeneratorPtr>(mod, "OgsFxShaderGenerator")
        .def_static("create", &mx::OgsFxShaderGenerator::create)
        .def(py::init([](){ return new PyOgsFxShaderGenerator(); }))
        .def("generate", &mx::OgsFxShaderGenerator::generate)
        .def("getTarget", &mx::OgsFxShaderGenerator::getTarget);
}
