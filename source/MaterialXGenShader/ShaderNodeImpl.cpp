#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

void ShaderNodeImpl::initialize(GenContext&, const ShaderGenerator&, ElementPtr)
{
}

void ShaderNodeImpl::createVariables(Shader&, GenContext&, const ShaderGenerator&, const ShaderNode&) const
{
}

void ShaderNodeImpl::emitFunctionDefinition(ShaderStage&, GenContext&, const ShaderGenerator&, const ShaderNode&) const
{
}

void ShaderNodeImpl::emitFunctionCall(ShaderStage&, GenContext&, const ShaderGenerator&, const ShaderNode&) const
{
}

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    return nullptr;
}

size_t ShaderNodeImpl::getHash() const
{
    // For now use the instance pointer as the hash
    return reinterpret_cast<size_t>(this);
}

} // namespace MaterialX
