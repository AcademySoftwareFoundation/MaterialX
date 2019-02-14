#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

void ShaderNodeImpl::initialize(ElementPtr, ShaderGenerator&, const GenOptions&)
{
}

void ShaderNodeImpl::createVariables(ShaderStage&, const ShaderNode&)
{
}

void ShaderNodeImpl::emitFunctionDefinition(ShaderStage&, const ShaderNode&, ShaderGenerator&)
{
    // default implementation has no function definition
}

void ShaderNodeImpl::emitFunctionCall(ShaderStage&, const ShaderNode&, GenContext&, ShaderGenerator&)
{
    // default implementation has no source code
}

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    return nullptr;
}

} // namespace MaterialX
