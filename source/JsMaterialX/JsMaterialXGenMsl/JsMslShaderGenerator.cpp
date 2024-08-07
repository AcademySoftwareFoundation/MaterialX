//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenMsl/MslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(MslShaderGenerator)
{
    ems::class_<mx::MslShaderGenerator, ems::base<mx::ShaderGenerator>>("MslShaderGenerator")
        .smart_ptr_constructor("MslShaderGenerator", &std::make_shared<mx::MslShaderGenerator>)
        ;
}
