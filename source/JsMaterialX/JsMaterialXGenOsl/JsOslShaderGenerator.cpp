//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenOsl/OslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(OslShaderGenerator)
{
    ems::class_<mx::OslShaderGenerator, ems::base<mx::ShaderGenerator>>("OslShaderGenerator")
        .constructor<mx::TypeSystemPtr>()
        BIND_CLASS_FUNC("create", mx::OslShaderGenerator, create, 0, 1, mx::TypeSystemPtr)
        ;
}
