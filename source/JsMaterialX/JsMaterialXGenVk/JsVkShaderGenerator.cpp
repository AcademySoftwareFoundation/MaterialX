//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr VkShaderGenerator_create()
    {
        return mx::VkShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(VkShaderGenerator)
{
    ems::class_<mx::VkShaderGenerator, ems::base<mx::GlslShaderGenerator>>("VkShaderGenerator")
        .class_function("create", &VkShaderGenerator_create);
}
