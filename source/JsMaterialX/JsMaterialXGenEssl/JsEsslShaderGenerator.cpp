//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(EsslShaderGenerator)
{
    ems::class_<mx::EsslShaderGenerator, ems::base<mx::HwShaderGenerator>>("EsslShaderGenerator")
        .constructor<mx::TypeSystemPtr>()
        BIND_CLASS_FUNC("create", mx::EsslShaderGenerator, create, 0, 1, mx::TypeSystemPtr)
        ;
}
