#include <MaterialXCore/Library.h>
#include <MaterialXShaderGen/NodeImplementation.h>

namespace MaterialX
{

void NodeImplementation::initialize(const Implementation& /*implementation*/)
{
}

void NodeImplementation::emitFunction(const SgNode&, ShaderGenerator&, Shader&)
{
    // default implementation has no function definition
}

void NodeImplementation::emitFunctionCall(const SgNode&, ShaderGenerator&, Shader&, int, ...)
{
    // default implementation has no source code
}

bool NodeImplementation::isTransparent(const SgNode& /*node*/) const
{
    return false;
}

} // namespace MaterialX
