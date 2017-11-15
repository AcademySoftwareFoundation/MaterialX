#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

void SgImplementation::initialize(ElementPtr, ShaderGenerator&)
{
}

void SgImplementation::emitFunctionDefinition(const SgNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void SgImplementation::emitFunctionCall(const SgNode&, ShaderGenerator&, Shader&)
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
