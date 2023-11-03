//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslResourceBindingContext.h>
#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

#include <string>

namespace py = pybind11;
namespace mx = MaterialX;

// GLSL shader generator bindings

void bindPyGlslShaderGenerator(py::module& mod)
{
    py::class_<mx::GlslShaderGenerator, mx::HwShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator")

        .def_static("create", &mx::GlslShaderGenerator::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("generate", &mx::GlslShaderGenerator::generate,
             py::arg("name"),
             py::arg("element"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate a shader starting from the given `element`, translating the
    element and all dependencies upstream into shader code.
)docstring"))

        .def("getTarget", &mx::GlslShaderGenerator::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a unique identifier for the target this generator is for.
)docstring"))

        .def("getVersion", &mx::GlslShaderGenerator::getVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the version string for the GLSL version this generator is for.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Base class for GLSL (OpenGL Shading Language) code generation.
    A generator for a specific GLSL target should be derived from this class.

    :see: https://materialx.org/docs/api/class_glsl_shader_generator.html
)docstring");
}

void bindPyGlslResourceBindingContext(py::module &mod)
{
    py::class_<mx::GlslResourceBindingContext, mx::HwResourceBindingContext, mx::GlslResourceBindingContextPtr>(mod, "GlslResourceBindingContext")

        .def_static("create", &mx::GlslResourceBindingContext::create,
                    py::arg("uniformBindingLocation") = 0,
                    py::arg("samplerBindingLocation") = 0,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class, initialized using the given binding
    locations.
)docstring"))

        .def(py::init<size_t, size_t>(),
             py::arg("uniformBindingLocation"),
             py::arg("samplerBindingLocation"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class using the given binding locations.
)docstring"))

        .def("emitDirectives", &mx::GlslResourceBindingContext::emitDirectives,
             py::arg("context"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit directives required for binding support.
)docstring"))

        .def("emitResourceBindings",
             &mx::GlslResourceBindingContext::emitResourceBindings,
             py::arg("context"),
             py::arg("uniforms"),
             py::arg("stage"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Emit uniforms with binding information
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class representing a resource binding for GLSL shader resources.

    :see: https://materialx.org/docs/api/class_glsl_resource_binding_context.html
)docstring");
}

// Essl shader generator bindings

void bindPyEsslShaderGenerator(py::module& mod)
{
    py::class_<mx::EsslShaderGenerator, mx::GlslShaderGenerator, mx::EsslShaderGeneratorPtr>(mod, "EsslShaderGenerator")

        .def_static("create", &mx::EsslShaderGenerator::create,
                    PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("generate", &mx::EsslShaderGenerator::generate,
             py::arg("name"),
             py::arg("element"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate a shader starting from the given `element`, translating the
    element and all dependencies upstream into shader code.
)docstring"))

        .def("getTarget", &mx::EsslShaderGenerator::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a unique identifier for the target this generator is for.
)docstring"))

        .def("getVersion", &mx::EsslShaderGenerator::getVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the version string for the ESSL version this generator is for.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing an ESSL (OpenGL ES Shading Language) shader generator

    :see: https://materialx.org/docs/api/class_essl_shader_generator.html
)docstring");
}

// Glsl Vulkan shader generator bindings

void bindPyVkShaderGenerator(py::module& mod)
{
    py::class_<mx::VkShaderGenerator, mx::GlslShaderGenerator, mx::VkShaderGeneratorPtr>(mod, "VkShaderGenerator")

        .def_static("create", &mx::VkShaderGenerator::create,
             PYMATERIALX_DOCSTRING(R"docstring(
    Create an instance of this class.
)docstring"))

        .def(py::init<>(),
             PYMATERIALX_DOCSTRING(R"docstring(
    Initialize an instance of this class.
)docstring"))

        .def("generate", &mx::VkShaderGenerator::generate,
             py::arg("name"),
             py::arg("element"),
             py::arg("context"),
             PYMATERIALX_DOCSTRING(R"docstring(
    Generate a shader starting from the given `element`, translating the
    element and all dependencies upstream into shader code.
)docstring"))

        .def("getTarget", &mx::VkShaderGenerator::getTarget,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return a unique identifier for the target this generator is for.
)docstring"))

        .def("getVersion", &mx::VkShaderGenerator::getVersion,
             PYMATERIALX_DOCSTRING(R"docstring(
    Return the version string for the Vulkan GLSL version this generator is for.
)docstring"))

        .doc() = PYMATERIALX_DOCSTRING(R"docstring(
    Class implementing a Vulkan GLSL shader generator.

    :see: https://materialx.org/docs/api/class_vk_shader_generator.html
)docstring");
}
