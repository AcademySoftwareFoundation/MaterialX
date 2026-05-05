//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHlsl/HlslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr HlslShaderGenerator_create()
    {
        return mx::HlslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(HlslShaderGenerator)
{
    ems::class_<mx::HlslShaderGenerator, ems::base<mx::HwShaderGenerator>>("HlslShaderGenerator")
        .class_function("create", &HlslShaderGenerator_create);
}
