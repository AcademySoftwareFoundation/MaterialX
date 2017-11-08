#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/SgImplementation.h>

namespace MaterialX
{

void SgImplementation::initialize(ElementPtr, ShaderGenerator&)
{
}

void SgImplementation::emitFunction(const SgNode&, ShaderGenerator&, Shader&, int, ...)
{
    // default implementation has no function definition
}

void SgImplementation::emitFunctionCall(const SgNode&, ShaderGenerator&, Shader&, int, ...)
{
    // default implementation has no source code
}

bool SgImplementation::isTransparent(const SgNode& /*node*/) const
{
    return false;
}

} // namespace MaterialX
