//
// TM & (c) 2021 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Shader.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

EMSCRIPTEN_BINDINGS(Shader)
{
    ems::class_<mx::Shader>("Shader")
        .smart_ptr<std::shared_ptr<mx::Shader>>("ShaderPtr")
        .function("getSourceCode", &mx::Shader::getSourceCode)
        .function("getUniformValues", &mx::Shader::getUniformValues);
        ;
}
