//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(MdlShaderGenerator)
{
    ems::class_<mx::MdlShaderGenerator, ems::base<mx::ShaderGenerator>>("MdlShaderGenerator")
        .constructor<mx::TypeSystemPtr>()
        .class_function("create", &mx::MdlShaderGenerator::create)
        ;
}
