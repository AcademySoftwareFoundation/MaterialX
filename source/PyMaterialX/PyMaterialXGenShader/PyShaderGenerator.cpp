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
    py::class_<mx::ShaderGenerator, mx::ShaderGeneratorPtr>(mod, "ShaderGenerator", "Base class for shader generators All third-party shader generators should derive from this class.\n\nDerived classes should use DECLARE_SHADER_GENERATOR / DEFINE_SHADER_GENERATOR in their declaration / definition, and register with the Registry class.")
        .def("getTarget", &mx::ShaderGenerator::getTarget, "Return the name of the target this generator is for.")
        .def("generate", &mx::ShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("setColorManagementSystem", &mx::ShaderGenerator::setColorManagementSystem, "Sets the color management system.")
        .def("getColorManagementSystem", &mx::ShaderGenerator::getColorManagementSystem, "Returns the color management system.")
        .def("setUnitSystem", &mx::ShaderGenerator::setUnitSystem, "Sets the unit system.")
        .def("getUnitSystem", &mx::ShaderGenerator::getUnitSystem, "Returns the unit system.")
        .def("getTokenSubstitutions", &mx::ShaderGenerator::getTokenSubstitutions, "Return the map of token substitutions used by the generator.")
        .def("registerTypeDefs", &mx::ShaderGenerator::registerTypeDefs, "Register type definitions from the document.")
        .def("registerShaderMetadata", &mx::ShaderGenerator::registerShaderMetadata, "Register metadata that should be exported to the generated shaders.\n\nSupported metadata includes standard UI attributes like \"uiname\", \"uifolder\", \"uimin\", \"uimax\", etc. But it is also extendable by defining custom attributes using AttributeDefs. Any AttributeDef in the given document with exportable=\"true\" will be exported as shader metadata when found on nodes during shader generation. Derived shader generators may override this method to change the registration. Applications must explicitly call this method before shader generation to enable export of metadata.");
}
