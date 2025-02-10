//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr MslShaderGenerator_create()
    {
        return mx::MslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(MslShaderGenerator)
{
    ems::class_<mx::MslShaderGenerator, ems::base<mx::HwShaderGenerator>>("MslShaderGenerator")
        .class_function("create", &MslShaderGenerator_create);
}
