#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

void ShaderNodeImpl::initialize(ElementPtr, const ShaderGenerator&, GenContext&)
{
}

void ShaderNodeImpl::createVariables(Shader&, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
}

void ShaderNodeImpl::emitFunctionDefinition(ShaderStage&, const ShaderNode&, const ShaderGenerator&, GenContext&) const
{
}

void ShaderNodeImpl::emitFunctionCall(ShaderStage&, const ShaderNode&, const ShaderGenerator&, GenContext&) const
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
