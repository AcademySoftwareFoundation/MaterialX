//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/VkShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(VkShaderGenerator)
{
    ems::class_<mx::VkShaderGenerator, ems::base<mx::ShaderGenerator>>("VkShaderGenerator")
        .smart_ptr_constructor("VkShaderGenerator", &std::make_shared<mx::VkShaderGenerator>)
        ;
}
