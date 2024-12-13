//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyShaderGenerator(py::module& mod)
{
    py::class_<mx::ShaderGenerator, mx::ShaderGeneratorPtr>(mod, "ShaderGenerator")
        .def("getTarget", &mx::ShaderGenerator::getTarget)
        .def("generate", &mx::ShaderGenerator::generate)
        .def("setColorManagementSystem", &mx::ShaderGenerator::setColorManagementSystem)
        .def("getColorManagementSystem", &mx::ShaderGenerator::getColorManagementSystem)
        .def("setUnitSystem", &mx::ShaderGenerator::setUnitSystem)
        .def("getUnitSystem", &mx::ShaderGenerator::getUnitSystem)
        .def("getTokenSubstitutions", &mx::ShaderGenerator::getTokenSubstitutions)
        .def("loadStructTypeDefs", &mx::ShaderGenerator::loadStructTypeDefs)
        .def("clearStructTypeDefs", &mx::ShaderGenerator::clearStructTypeDefs)
        .def("registerShaderMetadata", &mx::ShaderGenerator::registerShaderMetadata);
    mod.attr("ShaderGenerator").doc() = R"docstring(
    Base class for shader generators.

    All third-party shader generators should derive from this class.

    Derived classes should use `DECLARE_SHADER_GENERATOR` / `DEFINE_SHADER_GENERATOR`
    in their declaration / definition, and register with the `Registry` class.

    :see: https://materialx.org/docs/api/class_shader_generator.html)docstring";
}
