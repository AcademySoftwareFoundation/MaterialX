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

        .def("getTarget", &mx::ShaderGenerator::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the name of the target this generator is for.
)docstring"))

        .def("generate", &mx::ShaderGenerator::generate,
             py::arg("name"),
             py::arg("element"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate a shader starting from the given `element`, translating the
    element and all dependencies upstream into shader code.
)docstring"))

        .def("setColorManagementSystem",
             &mx::ShaderGenerator::setColorManagementSystem,
             py::arg("colorManagementSystem"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the color management system for the shader generator.
)docstring"))

        .def("getColorManagementSystem",
             &mx::ShaderGenerator::getColorManagementSystem,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the color management system for the shader generator.
)docstring"))

        .def("setUnitSystem", &mx::ShaderGenerator::setUnitSystem,
             py::arg("unitSystem"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Set the unit system for the shader generator.
)docstring"))

        .def("getUnitSystem", &mx::ShaderGenerator::getUnitSystem,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the unit system for the shader generator.
)docstring"))

        .def("getTokenSubstitutions",
             &mx::ShaderGenerator::getTokenSubstitutions,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the map of token substitutions used by the generator.
)docstring"))

        .def("registerShaderMetadata",
             &mx::ShaderGenerator::registerShaderMetadata,
             py::arg("doc"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Register metadata that should be exported to the generated shaders.

    Supported metadata includes standard UI attributes like `"uiname"`,
    `"uifolder"`, `"uimin"`, `"uimax"`, etc.

    The metadata is extendable by defining custom attributes using
    `AttributeDef` elements.

    Any `AttributeDef` in the given document with `exportable="true"` will be
    exported as shader metadata when found on nodes during shader generation.

    Derived shader generators may override this method to change the
    registration.

    Applications must explicitly call this method before shader generation to
    enable export of metadata.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for shader generators.
    All third-party shader generators should derive from this class.

    :see: https://materialx.org/docs/api/class_shader_generator.html
)docstring");
}
