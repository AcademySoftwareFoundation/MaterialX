#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderNodeImpl.h>
#include <MaterialXGenShader/ShaderNode.h>
#include <MaterialXGenShader/Shader.h>
#include <MaterialXGenShader/ShaderGenerator.h>

namespace MaterialX
{

void ShaderNodeImpl::initialize(ElementPtr /*implementation*/, ShaderGenerator& /*shadergen*/, const GenOptions& /*options*/)
{
}

void ShaderNodeImpl::createVariables(const ShaderNode&, ShaderGenerator&, Shader&)
{
}

void ShaderNodeImpl::emitFunctionDefinition(const ShaderNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void ShaderNodeImpl::emitFunctionCall(const ShaderNode&, GenContext&, ShaderGenerator&, Shader&)
{
    // default implementation has no source code
}

ShaderGraph* ShaderNodeImpl::getGraph() const
{
    return nullptr;
}

} // namespace MaterialX
