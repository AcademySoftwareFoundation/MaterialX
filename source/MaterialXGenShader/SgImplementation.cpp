#include <MaterialXCore/Library.h>
#include <MaterialXGenShader/SgImplementation.h>

namespace MaterialX
{

void SgImplementation::initialize(ElementPtr, ShaderGenerator&)
{
}

void SgImplementation::createVariables(const SgNode&, ShaderGenerator&, Shader&)
{
}

void SgImplementation::emitFunctionDefinition(const SgNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void SgImplementation::emitFunctionCall(const SgNode&, const SgNodeContext&, ShaderGenerator&, Shader&)
{
    // default implementation has no source code
}

bool SgImplementation::isTransparent(const SgNode&) const
{
    return false;
}

SgNodeGraph* SgImplementation::getNodeGraph() const
{
    return nullptr;
}

} // namespace MaterialX
