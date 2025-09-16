//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenHw/HwShaderGenerator.h>
#include <MaterialXGenShader/GenContext.h>

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
