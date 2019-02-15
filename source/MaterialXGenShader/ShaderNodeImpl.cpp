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

void ShaderNodeImpl::createVariables(ShaderStage&, const ShaderNode&) const
{
}

void ShaderNodeImpl::emitFunctionDefinition(ShaderStage&, const ShaderNode&, ShaderGenerator&) const
{
    // default implementation has no function definition
}

void ShaderNodeImpl::emitFunctionCall(ShaderStage&, const ShaderNode&, GenContext&, ShaderGenerator&) const
{
    // default implementation has no source code
}

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    // default implementation has no graph
    return nullptr;
}

size_t ShaderNodeImpl::getHash() const
{
    // For now use the instance pointer as the hash
    return reinterpret_cast<size_t>(this);
}

} // namespace MaterialX
