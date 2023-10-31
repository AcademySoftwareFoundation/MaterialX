//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenUserData.h>
#include <MaterialXGenShader/HwShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

void bindPyHwShaderGenerator(py::module& mod)
{
    mod.attr("VERTEX_STAGE") = mx::Stage::VERTEX;

    mod.attr("HW_VERTEX_INPUTS") = mx::HW::VERTEX_INPUTS;
    mod.attr("HW_VERTEX_DATA") = mx::HW::VERTEX_DATA;
    mod.attr("HW_PRIVATE_UNIFORMS") = mx::HW::PRIVATE_UNIFORMS;
    mod.attr("HW_PUBLIC_UNIFORMS") = mx::HW::PUBLIC_UNIFORMS;
    mod.attr("HW_LIGHT_DATA") = mx::HW::LIGHT_DATA;
    mod.attr("HW_PIXEL_OUTPUTS") = mx::HW::PIXEL_OUTPUTS;
    mod.attr("HW_ATTR_TRANSPARENT") =  mx::HW::ATTR_TRANSPARENT;

    py::class_<mx::HwShaderGenerator, mx::ShaderGenerator, mx::HwShaderGeneratorPtr>(mod, "HwShaderGenerator")

        .def("getClosureContexts", &mx::HwShaderGenerator::getClosureContexts,
             py::arg("node"),
             py::arg("cct"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the closure contexts defined for the given node.
)docstring"))

        .def_static("bindLightShader", &mx::HwShaderGenerator::bindLightShader,
                    py::arg("nodeDef"),
                    py::arg("lightTypeId"),
                    py::arg("context"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Bind a light shader to a light type ID, for usage in surface shaders created
    by the generator.

    The `lightTypeId` should be a unique identifier for the light type (node
    definition) and the same ID should be used when setting light parameters on
    a generated surface shader.
)docstring"))

        .def_static("unbindLightShader", &mx::HwShaderGenerator::unbindLightShader,
                    py::arg("lightTypeId"),
                    py::arg("context"),
                    PYMATERIALX_DOCSTRING(R"docstring(
    Unbind a light shader previously bound to the given light type ID.
)docstring"))

        .def_static("unbindLightShaders", &mx::HwShaderGenerator::unbindLightShaders,
                    py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Unbind all light shaders previously bound.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for shader generators targeting HW rendering.

    :see: https://materialx.org/docs/api/class_hw_shader_generator.html
)docstring");
}

void bindPyHwResourceBindingContext(py::module& mod)
{
    py::class_<mx::HwResourceBindingContext, mx::GenUserData, mx::HwResourceBindingContextPtr>(mod, "HwResourceBindingContext")

        .def("emitDirectives", &mx::HwResourceBindingContext::emitDirectives,
             py::arg("context"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit directives required for binding support.
)docstring"))

        .def("emitResourceBindings", &mx::HwResourceBindingContext::emitResourceBindings,
             py::arg("context"),
             py::arg("uniforms"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit uniforms with binding information.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a context for resource binding for hardware resources.

    :see: https://materialx.org/docs/api/class_hw_resource_binding_context.html
)docstring");
}
