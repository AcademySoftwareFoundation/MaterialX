#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

void ShaderNodeImpl::initialize(ElementPtr, GenContext&)
{
}

void ShaderNodeImpl::createVariables(const ShaderNode&, GenContext&, Shader&) const
{
}

void ShaderNodeImpl::emitFunctionDefinition(const ShaderNode&, GenContext&, ShaderStage&) const
{
}

void ShaderNodeImpl::emitFunctionCall(const ShaderNode&, GenContext&, ShaderStage&) const
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
