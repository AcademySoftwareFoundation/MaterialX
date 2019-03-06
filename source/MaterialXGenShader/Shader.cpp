//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>
#include <MaterialXGenShader/Syntax.h>
#include <MaterialXGenShader/Util.h>

#include <MaterialXCore/Document.h>
#include <MaterialXCore/Node.h>
#include <MaterialXCore/Value.h>

namespace MaterialX
{

 Shader::Shader(const string& name, ShaderGraphPtr graph)
    : _name(name)
    , _graph(graph)
{
}

ShaderStage& Shader::getStage(size_t index)
{
    return *_stages[index];
}

const ShaderStage& Shader::getStage(size_t index) const
{
    return *_stages[index];
}

ShaderStage& Shader::getStage(const string& name)
{
    auto it = _stagesMap.find(name);
    if (it == _stagesMap.end())
    {
        throw ExceptionShaderGenError("Stage '" + name + "' doesn't exist in shader '" + getName() + "'");
    }
    return *it->second;
}

const ShaderStage& Shader::getStage(const string& name) const
{
    return const_cast<Shader*>(this)->getStage(name);
}

ShaderStagePtr Shader::createStage(const string& name, ConstSyntaxPtr syntax)
{
    auto it = _stagesMap.find(name);
    if (it != _stagesMap.end())
    {
        throw ExceptionShaderGenError("Stage '" + name + "' already exist in shader '" + getName() + "'");
    }

    ShaderStagePtr s = std::make_shared<ShaderStage>(name, syntax);
    _stagesMap[name] = s;
    _stages.push_back(s.get());

    return s;
}

}
