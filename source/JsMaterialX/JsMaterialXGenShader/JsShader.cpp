//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <JsMaterialX/VectorHelper.h>

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/ShaderStage.h>

#include <emscripten/bind.h>

namespace ems = emscripten;
namespace mx = MaterialX;

std::vector<mx::ShaderNode*> getShaderNodes(mx::Shader& self)
{
    const auto& nodes = self.getGraph().getNodes();
    return std::vector<mx::ShaderNode*>(nodes.begin(), nodes.end());
}

EMSCRIPTEN_BINDINGS(Shader)
{
    ems::class_<mx::Shader>("Shader")
        .smart_ptr<std::shared_ptr<mx::Shader>>("ShaderPtr")
        .function("getNodes", &getShaderNodes, ems::allow_raw_pointers())
        .function("getSourceCode", &mx::Shader::getSourceCode)
        .function("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(size_t)>(&mx::Shader::getStage), ems::allow_raw_pointers())
        .function("getStage", static_cast<mx::ShaderStage& (mx::Shader::*)(const std::string&)>(&mx::Shader::getStage), ems::allow_raw_pointers())
        ;
}
