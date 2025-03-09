//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

namespace
{
    // Creator wrapper to avoid having to expose the TypeSystem class in JavaScript
    mx::ShaderGeneratorPtr MdlShaderGenerator_create()
    {
        return mx::MdlShaderGenerator::create();
    }
}

EMSCRIPTEN_BINDINGS(MdlShaderGenerator)
{
    ems::class_<mx::MdlShaderGenerator, ems::base<mx::ShaderGenerator>>("MdlShaderGenerator")
        .class_function("create", &MdlShaderGenerator_create);
}
