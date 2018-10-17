#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/GenImplementation.h>

namespace MaterialX
{

void GenImplementation::initialize(ElementPtr, ShaderGenerator&)
{
}

void GenImplementation::createVariables(const DagNode&, ShaderGenerator&, Shader&)
{
}

void GenImplementation::emitFunctionDefinition(const DagNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void GenImplementation::emitFunctionCall(const DagNode&, GenContext&, ShaderGenerator&, Shader&)
{
    // default implementation has no source code
}

Dag* GenImplementation::getDag() const
{
    return nullptr;
}

} // namespace MaterialX
