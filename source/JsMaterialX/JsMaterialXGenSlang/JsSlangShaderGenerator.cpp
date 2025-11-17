//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenSlang/SlangShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr SlangShaderGenerator_create()
    {
        return mx::SlangShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(SlangShaderGenerator)
{
    ems::class_<mx::SlangShaderGenerator, ems::base<mx::HwShaderGenerator>>("SlangShaderGenerator")
        .class_function("create", &SlangShaderGenerator_create);
}
