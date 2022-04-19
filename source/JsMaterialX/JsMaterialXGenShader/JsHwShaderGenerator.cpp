//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/HwShaderGenerator.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(HwShaderGenerator)
{
    ems::class_<mx::HwShaderGenerator, ems::base<mx::ShaderGenerator>>("HwShaderGenerator")
        .smart_ptr<std::shared_ptr<mx::HwShaderGenerator>>("HwShaderGeneratorPtr")
        .class_function("bindLightShader", &mx::HwShaderGenerator::bindLightShader)
        .class_function("unbindLightShader", &mx::HwShaderGenerator::unbindLightShader)
        .class_function("unbindLightShaders", &mx::HwShaderGenerator::unbindLightShaders)
        ;
}
