//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenGlsl/EsslShaderGenerator.h>
#include <MaterialXGenShader/Util.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(EsslShaderGenerator)
{
    ems::class_<mx::EsslShaderGenerator, ems::base<mx::HwShaderGenerator>>("EsslShaderGenerator")
        .smart_ptr_constructor("EsslShaderGenerator", &std::make_shared<mx::EsslShaderGenerator>)
        ;
}
