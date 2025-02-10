//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr GlslShaderGenerator_create()
    {
        return mx::GlslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(GlslShaderGenerator)
{
    ems::class_<mx::GlslShaderGenerator, ems::base<mx::HwShaderGenerator>>("GlslShaderGenerator")
        .class_function("create", &GlslShaderGenerator_create);
}
