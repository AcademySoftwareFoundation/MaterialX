//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenArnold/ArnoldShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

class PyArnoldShaderGenerator : public mx::ArnoldShaderGenerator
{
  public:
    using ArnoldShaderGenerator::ArnoldShaderGenerator;

    const std::string& getTarget() const override
    {
        PYBIND11_OVERLOAD_PURE(
            const std::string&,
            mx::ArnoldShaderGenerator,
            getTarget
        );
    }

};


void bindPyArnoldShaderGenerator(py::module& mod)
{
    py::class_<mx::ArnoldShaderGenerator, mx::OslShaderGenerator, PyArnoldShaderGenerator, mx::ArnoldShaderGeneratorPtr>(mod, "ArnoldShaderGenerator")
        .def_static("create", &mx::ArnoldShaderGenerator::create)
        .def(py::init([](){ return new PyArnoldShaderGenerator(); }))
        .def("getTarget", &mx::ArnoldShaderGenerator::getTarget);
}
