//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/WgslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr WgslShaderGenerator_create()
    {
        return mx::WgslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(WgslShaderGenerator)
{
    ems::class_<mx::WgslShaderGenerator, ems::base<mx::GlslShaderGenerator>>("WgslShaderGenerator")
        .class_function("create", &WgslShaderGenerator_create);
}
