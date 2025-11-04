//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <PyMaterialX/PyMaterialX.h>

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenGlsl/GlslResourceBindingContext.h>
#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace py = pybind11;
namespace mx = MaterialX;

// GLSL shader generator bindings

namespace
{
    // Creator wrappers to avoid having to expose the TypeSystem class in python
    mx::ShaderGeneratorPtr GlslShaderGenerator_create()
    {
        return mx::GlslShaderGenerator::create();
    }
    mx::ShaderGeneratorPtr EsslShaderGenerator_create()
    {
        return mx::EsslShaderGenerator::create();
    }
    mx::ShaderGeneratorPtr VkShaderGenerator_create()
    {
        return mx::VkShaderGenerator::create();
    }
    mx::ShaderGeneratorPtr WgslShaderGenerator_create()
    {
        return mx::WgslShaderGenerator::create();
    }
}

void bindPyGlslShaderGenerator(py::module& mod)
{
    py::class_<mx::GlslShaderGenerator, mx::HwShaderGenerator, mx::GlslShaderGeneratorPtr>(mod, "GlslShaderGenerator", "Base class for GLSL (OpenGL Shading Language) code generation.\n\nA generator for a specific GLSL target should be derived from this class.")
        .def_static("create", &GlslShaderGenerator_create)
        .def("generate", &mx::GlslShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("getTarget", &mx::GlslShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("getVersion", &mx::GlslShaderGenerator::getVersion, "Return the version string for the ESSL version this generator is for.");
}

void bindPyGlslResourceBindingContext(py::module &mod)
{
    py::class_<mx::GlslResourceBindingContext, mx::HwResourceBindingContext, mx::GlslResourceBindingContextPtr>(mod, "GlslResourceBindingContext", "Class representing a resource binding for Glsl shader resources.")
        .def_static("create", &mx::GlslResourceBindingContext::create)
        .def(py::init<size_t, size_t>())
        .def("emitDirectives", &mx::GlslResourceBindingContext::emitDirectives)
        .def("emitResourceBindings", &mx::GlslResourceBindingContext::emitResourceBindings);
}

// Essl shader generator bindings

void bindPyEsslShaderGenerator(py::module& mod)
{
    py::class_<mx::EsslShaderGenerator, mx::GlslShaderGenerator, mx::EsslShaderGeneratorPtr>(mod, "EsslShaderGenerator", "An ESSL (OpenGL ES Shading Language) shader generator.")
        .def_static("create", &EsslShaderGenerator_create)
        .def("generate", &mx::EsslShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("getTarget", &mx::EsslShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("getVersion", &mx::EsslShaderGenerator::getVersion, "Return the version string for the ESSL version this generator is for.");
}

// Glsl Vulkan shader generator bindings

void bindPyVkShaderGenerator(py::module& mod)
{
    py::class_<mx::VkShaderGenerator, mx::GlslShaderGenerator, mx::VkShaderGeneratorPtr>(mod, "VkShaderGenerator", "A Vulkan GLSL shader generator.")
        .def_static("create", &VkShaderGenerator_create)
        .def("generate", &mx::VkShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("getTarget", &mx::VkShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("getVersion", &mx::VkShaderGenerator::getVersion, "Return the version string for the ESSL version this generator is for.");
}

// Glsl Wgsl shader generator bindings

void bindPyWgslShaderGenerator(py::module& mod)
{
    py::class_<mx::WgslShaderGenerator, mx::GlslShaderGenerator, mx::WgslShaderGeneratorPtr>(mod, "WgslShaderGenerator", "WGSL Flavor of Vulkan GLSL shader generator")
        .def_static("create", &WgslShaderGenerator_create)
        .def("generate", &mx::WgslShaderGenerator::generate, "Generate a shader starting from the given element, translating the element and all dependencies upstream into shader code.")
        .def("getTarget", &mx::WgslShaderGenerator::getTarget, "Return a unique identifier for the target this generator is for.")
        .def("getVersion", &mx::WgslShaderGenerator::getVersion, "Return the version string for the ESSL version this generator is for.");
}
