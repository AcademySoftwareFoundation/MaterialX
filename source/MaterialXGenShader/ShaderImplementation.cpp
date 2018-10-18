#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/ShaderImplementation.h>

namespace MaterialX
{

void ShaderImplementation::initialize(ElementPtr, ShaderGenerator&)
{
}

void ShaderImplementation::createVariables(const ShaderNode&, ShaderGenerator&, Shader&)
{
}

void ShaderImplementation::emitFunctionDefinition(const ShaderNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void ShaderImplementation::emitFunctionCall(const ShaderNode&, GenContext&, ShaderGenerator&, Shader&)
{
    // default implementation has no source code
}

ShaderGraph* ShaderImplementation::getGraph() const
{
    return nullptr;
}

} // namespace MaterialX
