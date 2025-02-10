//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr EsslShaderGenerator_create()
    {
        return mx::EsslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(EsslShaderGenerator)
{
    ems::class_<mx::EsslShaderGenerator, ems::base<mx::GlslShaderGenerator>>("EsslShaderGenerator")
        .class_function("create", &EsslShaderGenerator_create);
}
