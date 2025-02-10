//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr OslShaderGenerator_create()
    {
        return mx::OslShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(OslShaderGenerator)
{
    ems::class_<mx::OslShaderGenerator, ems::base<mx::ShaderGenerator>>("OslShaderGenerator")
        .class_function("create", &OslShaderGenerator_create);
}
