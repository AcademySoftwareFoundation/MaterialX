//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/GlslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(GlslShaderGenerator)
{
    ems::class_<mx::GlslShaderGenerator, ems::base<mx::ShaderGenerator>>("GlslShaderGenerator")
        .smart_ptr_constructor("GlslShaderGenerator", &std::make_shared<mx::GlslShaderGenerator>)
        ;
}
